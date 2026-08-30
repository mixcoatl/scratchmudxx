//! \file world.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_WORLD_CPP_

#include <scratch/instance.hpp>
#include <scratch/world.hpp>

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Strings = Scratch::Algorithm::Strings;

//! Constructor.
//! \param id the world object identity
World::World(const String& id) noexcept :
	id_(id),
	instances_() {
    // Nothing.
}

//! Destructor.
World::~World() noexcept {
    while (!instances_.empty())
	RemoveInstance(*instances_.begin());
}

//! Adds an instance.
//! \param instance the instance to add
//! \return \c true if added
bool World::AddInstance(const InstancePtr& instance) noexcept {
    if (!instance || !instance->world_.expired())
	return false;
    while (instance->GetName().empty() || GetInstance(instance->GetName())) {
	instance->SetName(Strings::GenerateCopy());
    }
    instances_.insert(instance);
    instance->world_ = shared_from_this();
    return true;
}

//! Gets an instance.
//! \param instanceName the instance name
//! \return the instance, or \c nullptr
InstancePtr World::GetInstance(const String& instanceName) const noexcept {
    const InstancePtrSet instances = instances_;
    for (auto& instance: instances) {
	if (instance && !Strings::CompareCi(
		instance->GetName(), instanceName))
	    return instance;
    }
    return nullptr;
}

//! Gets the instances.
InstancePtrSet World::GetInstances() const noexcept {
    return instances_;
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
}

}; // namespace Core
}; // namespace Scratch
