//! \file scheduler.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_SCHEDULER_CPP_

#include <scratch/logger.hpp>
#include <scratch/scheduler.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {

//! Constructs a scheduler.
//! \param ctx the IO context
Scheduler::Scheduler(IoContext& ctx) noexcept :
    ctx_(ctx),
    nextId_(1),
    queue_(),
    shutdown_(false),
    steadyTimer_(ctx),
    tasks_() {
}

//! Destructor.
Scheduler::~Scheduler() noexcept {
    this->Shutdown();
}

//! Constructs a task.
//! \param delay the initial delay
//! \param id the task identity
//! \param period the recurring period
//! \param work the work function
Scheduler::Task::Task(
    const Duration delay,
    const TaskId id,
    const Duration period,
    WorkFunc work) noexcept :
    delay_(delay),
    deadline_(),
    id_(id),
    paused_(false),
    period_(period),
    work_(std::move(work)) {
}

//! Cancels a task.
//! \param task the task to cancel
void Scheduler::Cancel(const TaskPtr& task) noexcept {
    if (!task)
	return;

    this->Remove(task);
    tasks_.erase(task->GetId());
    this->Wait();
}

//! Cancels a task.
//! \param id the task identity
void Scheduler::Cancel(const TaskId id) noexcept {
    if (auto task = this->Find(id))
	this->Cancel(task);
}

//! Cancels every task.
void Scheduler::CancelAll() noexcept {
    queue_.clear();
    tasks_.clear();
    this->Wait();
}

//! Dispatches ready tasks after a wait completes.
//! \param ec the wait completion error
void Scheduler::Dispatch(const ErrorCode& ec) noexcept {
    if (ec) {
	if (ec == boost::asio::error::operation_aborted)
	    return;

	LOGGER_SYSTEM() << "Scheduler wait error: " << ec.message();
	return;
    }

    const auto now = Task::Clock::now();

    while (auto task = this->Peek()) {
	if (task->deadline_ > now)
	    break;

	const auto fired = task->deadline_;
	const auto id = task->id_;
	this->Remove(task);

	if (task->work_) {
	    try {
		task->work_(*task);
	    } catch (const std::exception& e) {
		LOGGER_SYSTEM() << "Scheduler task " << id
		    << " threw: " << e.what();
	    }
	}

	if (shutdown_ || !tasks_.count(id) || task->paused_)
	    continue;

	if (task->period_.count() > 0) {
	    task->deadline_ = fired + task->period_;
	    this->Enqueue(task);
	} else {
	    tasks_.erase(id);
	}
    }

    if (!shutdown_)
	this->Wait();
}

//! Enqueues a task.
//! \param task the task
void Scheduler::Enqueue(const TaskPtr& task) noexcept {
    if (!task || task->paused_)
	return;

    auto deadline = task->deadline_;
    while (queue_.count(deadline))
	deadline += boost::chrono::milliseconds(1);
    task->deadline_ = deadline;
    queue_[deadline] = task;
}

//! Gets whether a task is running.
//! \param task the task
//! \return \c true if registered and not paused
bool Scheduler::IsRunning(const TaskPtr& task) const noexcept {
    if (!task || task->paused_)
	return false;

    const auto it = queue_.find(task->deadline_);
    return it != std::end(queue_) && it->second == task;
}

//! Pauses a task.
//! \param task the task to pause
void Scheduler::Pause(const TaskPtr& task) noexcept {
    if (!task || task->paused_)
	return;

    const auto now = Task::Clock::now();
    task->delay_ = task->deadline_ > now ?
	boost::chrono::duration_cast<Task::Duration>(
	    task->deadline_ - now) :
	Task::Duration::zero();
    task->paused_ = true;
    this->Remove(task);
    this->Wait();
}

//! Pauses a task.
//! \param id the task identity
void Scheduler::Pause(const TaskId id) noexcept {
    if (auto task = this->Find(id))
	this->Pause(task);
}

//! Posts zero-delay work.
//! \param func the work function
void Scheduler::Post(std::function<void()> func) {
    if (this->RefuseIfShutdown("Post"))
	return;

    boost::asio::post(ctx_, std::move(func));
}

//! Refuses work after shutdown.
//! \param operation the operation name
bool Scheduler::RefuseIfShutdown(const char* operation) noexcept {
    if (!shutdown_)
	return false;

    LOGGER_SYSTEM() << "Scheduler::" << operation
	<< " ignored after shutdown.";
    return true;
}

//! Resumes a task.
//! \param task the task to resume
void Scheduler::Resume(const TaskPtr& task) noexcept {
    if (!task || !task->paused_)
	return;

    task->paused_ = false;
    task->deadline_ = Task::Clock::now() + task->delay_;
    this->Enqueue(task);
    this->Wait();
}

//! Resumes a task.
//! \param id the task identity
void Scheduler::Resume(const TaskId id) noexcept {
    if (auto task = this->Find(id))
	this->Resume(task);
}

//! Schedules work.
//! \param func the work function
//! \param delay the initial delay
//! \param period the recurring period
//! \return the task, or \c nullptr after shutdown
Scheduler::TaskPtr Scheduler::Schedule(
    Task::WorkFunc func,
    const Task::Duration delay,
    const Task::Duration period) {
    if (this->RefuseIfShutdown("Schedule"))
	return TaskPtr();

    const TaskId id = nextId_++;
    auto task = std::make_shared<Task>(
	delay,
	id,
	period,
	std::move(func));
    task->deadline_ = Task::Clock::now() + delay;
    tasks_[id] = task;
    this->Enqueue(task);
    this->Wait();
    return task;
}

//! Shuts the scheduler down.
void Scheduler::Shutdown() noexcept {
    if (shutdown_)
	return;

    shutdown_ = true;
    this->CancelAll();
    ErrorCode ec;
    steadyTimer_.cancel(ec);
}

//! Arms a wait until the earliest queued deadline.
void Scheduler::Wait() noexcept {
    ErrorCode ec;
    steadyTimer_.cancel(ec);

    if (shutdown_ || queue_.empty())
	return;

    const auto now = Task::Clock::now();
    const auto& deadline = queue_.begin()->first;
    const auto wait = deadline > now ? deadline - now : Task::Duration::zero();
    const auto nanoseconds = boost::chrono::duration_cast<
	boost::chrono::nanoseconds>(wait);
    steadyTimer_.expires_after(SteadyTimer::duration(nanoseconds.count()));
    steadyTimer_.async_wait([this](const ErrorCode& error) {
	this->Dispatch(error);
    });
}

}; // namespace Core
}; // namespace Scratch
