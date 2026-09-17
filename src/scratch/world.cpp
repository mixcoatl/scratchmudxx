//! \file world.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_WORLD_CPP_

#include <scratch/config.hpp>
#include <scratch/game.hpp>
#include <scratch/instance.hpp>
#include <scratch/world.hpp>
namespace Scratch {
namespace Core {

// ScratchMUD types.
using Strings = Scratch::Algorithm::Strings;
using Task = Scheduler::Task;

//! Constructor.
//! \param game the game state
//! \param id the world object identity
World::World(Game& game, const String& id) noexcept :
	game_(game),
	id_(id),
	instances_(),
	pruneDeadline_(),
	pruneTask_() {
    // Nothing.
}

//! Destructor.
World::~World() noexcept {
    this->CancelPrune();
}

//! Adds an instance.
//! \param instance the instance to add
//! \return \c true if added
bool World::AddInstance(const InstancePtr& instance) noexcept {
    if (!instance || !instance->world_.expired())
	return false;
    while (instance->GetName().empty() ||
	    this->GetInstance(instance->GetName())) {
	instance->SetName(Strings::GenerateCopy());
    }
    instances_.insert(instance);
    instance->world_ = this->shared_from_this();
    this->CancelPrune();
    return true;
}

//! Cancels prune scheduling.
//! \sa #StartPrune()
void World::CancelPrune() noexcept {
    if (pruneTask_) {
	game_.GetScheduler().Cancel(pruneTask_);
	pruneTask_.reset();
    }
    this->ClearPruneDeadline();
}

//! Clears the prune deadline.
//! \sa #SetPruneDeadline(const Scheduler::Task::TimePoint&)
void World::ClearPruneDeadline() noexcept {
    pruneDeadline_.reset();
}

//! Gets an instance.
//! \param instanceName the instance name
//! \return the instance, or \c nullptr
InstancePtr World::GetInstance(const String& instanceName) const noexcept {
    const InstancePtrSet instances = instances_;
    for (auto& instance: instances) {
	if (!Strings::CompareCi(instance->GetName(), instanceName))
	    return instance;
    }
    return nullptr;
}

//! Gets the instances.
//! \sa #GetInstance(const String&) const
InstancePtrSet World::GetInstances() const noexcept {
    return instances_;
}

//! Gets prune facts.
//! \param now the current instant
//! \return the prune snapshot
PruneInfo World::GetPrune(const Task::TimePoint& now) const noexcept {
    PruneInfo info;
    info.protected_ = this->GetId().empty();
	for (const auto& instance: instances_) {
	if (instance)
	    ++info.occupants;
    }
    if (info.protected_ || info.occupants > 0 || !pruneDeadline_)
	return info;

    info.graceArmed = true;
    const auto remaining = *pruneDeadline_ - now;
    info.graceRemaining = remaining > Task::Duration::zero() ?
	remaining : Task::Duration::zero();
    return info;
}

//! Removes an instance.
//! \param instance the instance to remove
void World::RemoveInstance(const InstancePtr& instance) noexcept {
    if (!instance || instance->GetName().empty())
	return;
    auto it = instances_.find(instance);
    if (it == std::end(instances_))
	return;
    auto parent = instance->parent_.lock();
    if (parent)
	parent->RemoveChild(instance);
    const auto contents = instance->GetContents();
    for (auto& child: contents)
	instance->RemoveChild(child);
    instance->world_.reset();
    instances_.erase(it);
    const auto now = Task::Clock::now();
    if (this->GetPrune(now).occupants == 0)
	this->StartPrune();
}

//! Starts the prune grace countdown.
//! \sa #CancelPrune()
void World::StartPrune() noexcept {
    if (this->GetId().empty())
	return;
    if (pruneTask_ || pruneDeadline_)
	return;

    const auto now = Task::Clock::now();
    const auto period = game_.GetConfig()->GetWorldGracePeriod();
    this->SetPruneDeadline(now + period);

    auto weak = std::weak_ptr<World>(this->shared_from_this());
    pruneTask_ = game_.GetScheduler().Schedule(
	[weak](Task&) {
	    auto locked = weak.lock();
	    if (!locked)
		return;
	    if (!locked->game_.PruneWorld(locked, false))
		locked->CancelPrune();
	},
	period);
}

}; // namespace Core
}; // namespace Scratch
