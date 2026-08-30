//! \file instance.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_INSTANCE_CPP_

#include <cmath>
#include <scratch/instance.hpp>
#include <scratch/logger.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
Instance::Instance() noexcept :
	contents_(),
	contentsWeight_(0.0),
	name_(),
	parent_(),
	weight_(0.0),
	world_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the instance to copy
Instance::Instance(const Instance& other) noexcept :
	std::enable_shared_from_this<Instance>(),
	contents_(),
	contentsWeight_(0.0),
	name_(other.name_),
	parent_(),
	weight_(other.weight_),
	world_() {
    // Nothing.
}

//! Destructor.
Instance::~Instance() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the instance to assign
Instance& Instance::operator=(const Instance& other) noexcept {
    name_ = other.name_;
    const auto delta = other.weight_ - weight_;
    weight_ = other.weight_;
    AdjustContentsWeight(parent_.lock(), delta);
    return *this;
}

//! Adds a child instance to this instance.
//! \param instance the instance to add
//! \return \c true if added
bool Instance::AddChild(const InstancePtr& instance) noexcept {
    if (!instance || instance.get() == this)
	return false;
    auto world = world_.lock();
    auto instanceWorld = instance->world_.lock();
    if (!world || !instanceWorld || world != instanceWorld)
	return false;
    for (auto ancestor = shared_from_this();
	    ancestor; ancestor = ancestor->parent_.lock()) {
	if (ancestor == instance)
	    return false;
    }
    auto current = instance->parent_.lock();
    if (current.get() == this)
	return true;
    if (current)
	current->RemoveChild(instance);
    instance->parent_ = shared_from_this();
    contents_.insert(instance);
    AdjustContentsWeight(shared_from_this(), instance->GetTotalWeight());
    return true;
}

//! Removes a child instance from this instance.
//! \param instance the instance to remove
void Instance::RemoveChild(const InstancePtr& instance) noexcept {
    if (!instance || instance->parent_.lock().get() != this)
	return;
    contents_.erase(instance);
    instance->parent_.reset();
    AdjustContentsWeight(shared_from_this(), -instance->GetTotalWeight());
}

//! Removes this instance from its world object.
void Instance::Remove() noexcept {
    auto world = world_.lock();
    if (world)
	world->RemoveInstance(shared_from_this());
}

//! Sets the intrinsic weight.
//! \param weight the nonnegative finite weight
void Instance::SetWeight(const double weight) noexcept {
    if (!std::isfinite(weight) || weight < 0.0) {
	LOGGER_SYSTEM() << "Invalid instance weight " << weight << ".";
    } else {
	const auto delta = weight - weight_;
	weight_ = weight;
	AdjustContentsWeight(parent_.lock(), delta);
    }
}

//! Adjusts contents weight through ancestors.
//! \param parent the first ancestor
//! \param weight the weight delta
void Instance::AdjustContentsWeight(
	const InstancePtr& parent,
	const double weight) noexcept {
    for (auto ancestor = parent;
	    ancestor; ancestor = ancestor->parent_.lock())
	ancestor->contentsWeight_ += weight;
}

}; // namespace Core
}; // namespace Scratch
