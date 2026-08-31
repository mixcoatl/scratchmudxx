//! \file world.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_WORLD_HPP_
    #define _SCRATCH_WORLD_HPP_

#include <boost/optional.hpp>
#include <scratch/scheduler.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

// Forward declarations.
class Game;
class Instance;
class World;

// ScratchMUD types.
using InstancePtr = std::shared_ptr<Instance>;
using InstancePtrSet = std::set<InstancePtr>;
using WorldPtr = std::shared_ptr<World>;

//! World prune snapshot. \{
struct PruneInfo {
    //! Grace countdown bit.
    bool graceArmed = false;

    //! Grace remaining.
    Scheduler::Task::Duration graceRemaining =
	Scheduler::Task::Duration::zero();

    //! Occupant count.
    std::size_t occupants = 0;

    //! Protected-world bit.
    bool protected_ = false;

    //! Returns whether the world is ready to prune.
    bool IsReady() const noexcept {
	return !protected_ && occupants == 0 && graceArmed &&
	    graceRemaining <= Scheduler::Task::Duration::zero();
    }

    //! \copydoc IsReady()
    explicit operator bool() const noexcept {
	return IsReady();
    }

    //! Returns whether the world is not ready to prune.
    bool operator!() const noexcept {
	return !IsReady();
    }
};
//! \}

//! The World class. \{
class World : public std::enable_shared_from_this<World> {
public:
    //! Constructor.
    //! \param game the game state
    //! \param id the world object identity
    explicit World(Game& game, const String& id = String()) noexcept;

    //! Destructor.
    virtual ~World() noexcept;

    //! Adds an instance.
    //! \param instance the instance to add
    //! \return \c true if added
    //! \sa #RemoveInstance(const InstancePtr&)
    bool AddInstance(const InstancePtr& instance) noexcept;

    //! Clears the prune deadline.
    //! \sa #SetPruneDeadline(const Scheduler::Task::TimePoint&)
    void ClearPruneDeadline() noexcept;

    //! Gets the world object identity.
    //! \sa #World(Game&, const String&)
    String GetId() const noexcept {
	return id_;
    }

    //! Gets an instance.
    //! \param instanceName the instance name
    //! \return the instance, or \c nullptr
    //! \sa #GetInstances() const
    InstancePtr GetInstance(const String& instanceName) const noexcept;

    //! Gets the instances.
    //! \sa #GetInstance(const String&) const
    InstancePtrSet GetInstances() const noexcept;

    //! Gets prune facts.
    //! \param now the current instant
    //! \return the prune snapshot
    PruneInfo GetPrune(const Scheduler::Task::TimePoint& now) const noexcept;

    //! Gets a live room instance.
    //! \param qualifiedName the qualified room name
    //! \return the live room instance, or \c nullptr
    //! \sa #GetInstances() const
    InstancePtr GetRoomInstance(const String& qualifiedName) const noexcept;

    //! Gets the source zone name.
    //! \sa #SetSourceZone(const String&)
    String GetSourceZone() const noexcept {
	return sourceZone_;
    }

    //! Links room exits within this world.
    //! \sa Game::LinkWorlds()
    void LinkRoomExits() noexcept;

    //! Removes an instance.
    //! \param instance the instance to remove
    //! \sa #AddInstance(const InstancePtr&)
    void RemoveInstance(const InstancePtr& instance) noexcept;

    //! Sets the prune deadline.
    //! \param deadline the prune deadline
    //! \sa #ClearPruneDeadline()
    void SetPruneDeadline(const Scheduler::Task::TimePoint& deadline) noexcept {
	pruneDeadline_ = deadline;
    }

    //! Sets the source zone name.
    //! \param sourceZone the zone name
    //! \sa #GetSourceZone() const
    void SetSourceZone(const String& sourceZone) noexcept {
	sourceZone_ = sourceZone;
    }

protected:
    friend class Game;

    //! Cancels prune scheduling.
    //! \sa #StartPrune()
    void CancelPrune() noexcept;

    //! Starts the prune grace countdown.
    //! \sa #CancelPrune()
    void StartPrune() noexcept;

    //! The game state.
    Game& game_;

    //! The world object identity.
    String id_;

    //! The instance objects.
    InstancePtrSet instances_;

    //! The prune deadline.
    boost::optional<Scheduler::Task::TimePoint> pruneDeadline_;

    //! The prune task.
    Scheduler::TaskPtr pruneTask_;

    //! The source zone name.
    String sourceZone_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_WORLD_HPP_
