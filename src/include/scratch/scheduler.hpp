//! \file scheduler.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_SCHEDULER_HPP_
#define _SCRATCH_SCHEDULER_HPP_

#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {

// Boost types.
using ErrorCode = boost::system::error_code;
using IoContext = boost::asio::io_context;
using SteadyTimer = boost::asio::steady_timer;

//! The scheduler class. \{
class Scheduler {
public:
    //! A task identity.
    using TaskId = std::uint64_t;

    //! The task class. \{
    class Task {
    public:
	//! The monotonic clock.
	using Clock = boost::chrono::steady_clock;

	//! A delay or period.
	using Duration = Clock::duration;

	//! A deadline instant.
	using TimePoint = Clock::time_point;

	//! Scheduled work.
	using WorkFunc = std::function<void(Task&)>;

	//! Constructs a task.
	//! \param delay the initial delay
	//! \param id the task identity
	//! \param period the recurring period
	//! \param work the work function
	Task(
	    const Duration delay,
	    const TaskId id,
	    const Duration period,
	    WorkFunc work) noexcept;

	//! Gets the task identity.
	TaskId GetId() const noexcept {
	    return id_;
	}

	//! Gets whether the task is paused.
	bool IsPaused() const noexcept {
	    return paused_;
	}

	//! Gets whether the task is periodic.
	bool IsPeriodic() const noexcept {
	    return period_.count() > 0;
	}

    private:
	friend class Scheduler;

	//! The initial or remaining delay.
	Duration delay_;

	//! The next deadline.
	TimePoint deadline_;

	//! The task identity.
	TaskId id_;

	//! The paused bit.
	bool paused_;

	//! The recurring period.
	Duration period_;

	//! The work function.
	WorkFunc work_;
    };
    //! \}

    //! A task pointer.
    using TaskPtr = std::shared_ptr<Task>;

    //! Constructs a scheduler.
    //! \param ctx the IO context
    explicit Scheduler(IoContext& ctx) noexcept;

    //! Destructor.
    ~Scheduler() noexcept;

    //! Cancels a task.
    //! \param task the task to cancel
    //! \sa #Cancel(TaskId)
    void Cancel(const TaskPtr& task) noexcept;

    //! Cancels a task.
    //! \param id the task identity
    //! \sa #Cancel(const TaskPtr&)
    void Cancel(const TaskId id) noexcept;

    //! Cancels every task.
    void CancelAll() noexcept;

    //! Gets whether a task is running.
    //! \param task the task
    //! \return \c true if registered and not paused
    bool IsRunning(const TaskPtr& task) const noexcept;

    //! Pauses a task.
    //! \param task the task to pause
    //! \sa #Pause(TaskId)
    void Pause(const TaskPtr& task) noexcept;

    //! Pauses a task.
    //! \param id the task identity
    //! \sa #Pause(const TaskPtr&)
    void Pause(const TaskId id) noexcept;

    //! Posts zero-delay work.
    //! \param func the work function
    void Post(std::function<void()> func);

    //! Posts zero-delay work on a weak target.
    //! \param invoke the work function
    //! \param target the weak target
    template<typename T>
    void PostWeak(
	std::function<void(T&)> invoke,
	const std::weak_ptr<T>& target) {
	this->Post([target, invoke = std::move(invoke)]() {
	    if (auto locked = target.lock())
		invoke(*locked);
	});
    }

    //! Resumes a task.
    //! \param task the task to resume
    //! \sa #Resume(TaskId)
    void Resume(const TaskPtr& task) noexcept;

    //! Resumes a task.
    //! \param id the task identity
    //! \sa #Resume(const TaskPtr&)
    void Resume(const TaskId id) noexcept;

    //! Schedules work.
    //! \param func the work function
    //! \param delay the initial delay
    //! \param period the recurring period
    //! \return the task, or \c nullptr after shutdown
    TaskPtr Schedule(
	Task::WorkFunc func,
	const Task::Duration delay = Task::Duration::zero(),
	const Task::Duration period = Task::Duration::zero());

    //! Schedules recurring work.
    //! \param func the work function
    //! \param period the recurring period
    //! \return the task, or \c nullptr after shutdown
    TaskPtr ScheduleEvery(
	Task::WorkFunc func,
	const Task::Duration period) {
	return this->Schedule(std::move(func), period, period);
    }

    //! Schedules recurring work on a weak target.
    //! \param invoke the work function
    //! \param period the recurring period
    //! \param target the weak target
    //! \return the task, or \c nullptr after shutdown
    template<typename T>
    TaskPtr ScheduleEveryWeak(
	std::function<void(T&)> invoke,
	const Task::Duration period,
	const std::weak_ptr<T>& target) {
	return this->ScheduleWeak(
	    period,
	    std::move(invoke),
	    period,
	    target);
    }

    //! Schedules work on a weak target.
    //! \param delay the initial delay
    //! \param invoke the work function
    //! \param period the recurring period
    //! \param target the weak target
    //! \return the task, or \c nullptr after shutdown
    template<typename T>
    TaskPtr ScheduleWeak(
	const Task::Duration delay,
	std::function<void(T&)> invoke,
	const Task::Duration period,
	const std::weak_ptr<T>& target) {
	return this->Schedule(
	    [target, invoke = std::move(invoke)](Task&) {
		if (auto locked = target.lock())
		    invoke(*locked);
	    },
	    delay,
	    period);
    }

    //! Shuts the scheduler down.
    void Shutdown() noexcept;

private:
    //! Dispatches ready tasks after a wait completes.
    //! \param ec the wait completion error
    void Dispatch(const ErrorCode& ec) noexcept;

    //! Enqueues a task.
    //! \param task the task
    void Enqueue(const TaskPtr& task) noexcept;

    //! Finds a task.
    //! \param id the task identity
    //! \return the task, or \c nullptr
    TaskPtr Find(const TaskId id) const noexcept {
	const auto it = tasks_.find(id);
	return it != std::end(tasks_) ? it->second : TaskPtr();
    }

    //! Peeks the earliest queued task.
    //! \return the task, or \c nullptr
    TaskPtr Peek() const noexcept {
	return queue_.empty() ? TaskPtr() : queue_.begin()->second;
    }

    //! Refuses work after shutdown.
    //! \param operation the operation name
    //! \return \c true if work was refused
    bool RefuseIfShutdown(const char* operation) noexcept;

    //! Removes a task from the queue.
    //! \param task the task
    void Remove(const TaskPtr& task) noexcept {
	if (task && !task->paused_)
	    queue_.erase(task->deadline_);
    }

    //! Arms a wait until the earliest queued deadline.
    void Wait() noexcept;

    //! The IO context.
    IoContext& ctx_;

    //! The next task identity.
    TaskId nextId_;

    //! The deadline queue.
    std::map<Task::TimePoint, TaskPtr> queue_;

    //! The shutdown bit.
    bool shutdown_;

    //! The steady timer.
    SteadyTimer steadyTimer_;

    //! The task registry.
    std::map<TaskId, TaskPtr> tasks_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_SCHEDULER_HPP_
