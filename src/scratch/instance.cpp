//! \file instance.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_INSTANCE_CPP_

#include <scratch/color.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/direction.hpp>
#include <scratch/game.hpp>
#include <scratch/instance.hpp>
#include <scratch/logger.hpp>
#include <scratch/player.hpp>
#include <scratch/room.hpp>
#include <scratch/room_exit_specials.hpp>
#include <scratch/room_specials.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <scratch/world.hpp>

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Color = Scratch::Net::Color;
using Descriptor = Scratch::Net::Descriptor;
using Strings = Scratch::Algorithm::Strings;

//! Default constructor.
Instance::Instance() noexcept :
	std::enable_shared_from_this<Instance>(),
	contents_(),
	contentsWeight_(0.0),
	descriptor_(),
	gender_(Gender::GENDER_UNDEFINED),
	name_(),
	parent_(),
	player_(),
	roomSpecials_(),
	weight_(0.0),
	world_(),
	zone_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa instance to copy
Instance::Instance(const Instance& other) noexcept :
	std::enable_shared_from_this<Instance>(),
	contents_(),
	contentsWeight_(0.0),
	descriptor_(),
	gender_(other.gender_),
	name_(other.name_),
	parent_(),
	player_(other.player_),
	roomSpecials_(),
	weight_(other.weight_),
	world_(),
	zone_(other.zone_) {
    // Nothing.
}

//! Destructor.
Instance::~Instance() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the \sa instance to assign
Instance& Instance::operator=(const Instance& other) noexcept {
    gender_ = other.gender_;
    name_ = other.name_;
    player_ = other.player_;
    const auto delta = other.weight_ - weight_;
    weight_ = other.weight_;
    this->AdjustContentsWeight(parent_.lock(), delta);
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
    for (auto p = shared_from_this(); p; p = p->parent_.lock()) {
	if (p == instance)
	    return false;
    }

    auto current = instance->parent_.lock();
    if (current.get() == this)
	return true;

    if (current)
	current->RemoveChild(instance);
    instance->parent_ = shared_from_this();
    contents_.insert(instance);

    this->AdjustContentsWeight(shared_from_this(), instance->GetTotalWeight());
    return true;
}

//! Gets the parent room.
//! \return the room-bearing ancestor, or \c nullptr
InstancePtr Instance::GetParentRoom() const noexcept {
    for (auto current = std::const_pointer_cast<Instance>(
	    shared_from_this());
	 current; current = current->GetParent()) {
	if (current->GetRoom())
	    return current;
    }
    return nullptr;
}


//! Finds an instance matching \p words.
//! \param game the game state
//! \param words the name words
//! \param nth the 1-based ordinal
//! \param count the requested count
//! \return the matched instance, or \c nullptr
InstancePtr Instance::Find(
	const Game& game,
	const std::vector<String>& words,
	const unsigned nth,
	const unsigned count) const noexcept {
    if (count != 1 || !nth || words.empty())
	return nullptr;

    auto seeker = std::const_pointer_cast<Instance>(this->shared_from_this());

    if (words.size() == 1 && nth == 1) {
	const auto& name = words.front();
	if (!Strings::CompareCi(name, "me") || !Strings::CompareCi(name, "self"))
	    return seeker;
	if (!name.empty() && name.front() == '%') {
	    auto instance = game.GetInstance(name.substr(1));
	    if (!instance || !instance->Matches(name, seeker))
		return nullptr;
	    return instance;
	}
    }

    unsigned found = 0;
    const auto tryNth = [&](const InstancePtr& candidate) noexcept
	    -> InstancePtr {
	if (!candidate || candidate == seeker)
	    return nullptr;
	for (const auto& word: words) {
	    if (!candidate->Matches(word, seeker))
		return nullptr;
	}
	return ++found == nth ? candidate : nullptr;
    };

    if (auto room = seeker->GetParentRoom()) {
	for (const auto& occupant: room->GetContents())
	    if (auto match = tryNth(occupant))
		return match;
    }

    for (const auto& descriptor: game.GetDescriptors()) {
	if (!descriptor || descriptor->Closed())
	    continue;
	auto character = descriptor->GetCharacter();
	if (!character || !character->GetPlayer())
	    continue;
	if (auto match = tryNth(character))
	    return match;
    }
    return nullptr;
}

//! Gets the controlling descriptor.
//! \sa #SetDescriptor(const DescriptorPtr&)
DescriptorPtr Instance::GetDescriptor() noexcept {
    auto d = descriptor_.lock();
    if (!d || d->Closed() || d->GetCharacter().get() != this) {
	// Stale control link.
	descriptor_.reset();
	return nullptr;
    }
    return d;
}

//! Gets the room prototype.
//! \return the room, or \c nullptr
RoomPtr Instance::GetRoom() const noexcept {
    return roomSpecials_ ? roomSpecials_->GetRoom() :
	RoomPtr();
}

//! Gets the qualified room name.
String Instance::GetQualifiedRoomName() const noexcept {
    auto room = this->GetRoom();
    if (!room)
	return String();
    String qualified = room->GetQualifiedName();
    auto world = world_.lock();
    if (world && !world->GetId().empty())
	qualified += '@' + world->GetId();
    return qualified;
}

//! Matches \p name against this instance.
//! \param name the name
//! \param seeker the searching instance, or null
//! \return \c true if this instance matches \p name
bool Instance::Matches(
	const String& name,
	const InstancePtr&) const noexcept {
    if (name.empty())
	return false;

    if (name.front() == '%') {
	const auto id = name.substr(1);
	return !id.empty() && !Strings::CompareCi(name_, id);
    }

    if (player_) {
	const auto playerName = player_->GetName();
	return Strings::StartsWithCi(playerName, name);
    }
    return false;
}

namespace {

//! Resolves a move destination.
//! \param instance the moving instance
//! \param direction the direction
//! \return the destination room instance, or \c nullptr
InstancePtr ResolveMoveDestination(
	const Instance& instance,
	const Direction::DirectionEnum direction) noexcept {
    if (!Direction::IsDefined(direction))
	return nullptr;

    auto location = instance.GetParentRoom();
    if (!location)
	return nullptr;
    auto specials = location->GetRoomSpecials();
    if (!specials)
	return nullptr;
    auto exit = specials->GetExit(direction);
    if (!exit || !exit->GetTarget() || exit->BlocksPassage())
	return nullptr;
    auto destination = exit->GetTarget();

    auto sourceRoom = location->GetRoom();
    auto destRoom = destination->GetRoom();
    auto player = instance.GetPlayer();
    if (destRoom && destRoom->GetImmortalBit() &&
	    sourceRoom && !sourceRoom->GetImmortalBit() &&
	    player && player->IsMortal())
	return nullptr;

    return destination;
}

} // namespace

//! Returns whether this instance can move in \p direction.
//! \param direction the direction
//! \return \c true if the move can succeed
bool Instance::CanMove(const Direction::DirectionEnum direction) const noexcept {
    return ResolveMoveDestination(*this, direction) != nullptr;
}

//! Moves through an exit in \p direction.
//! \param direction the direction
//! \return \c true if the move succeeded
bool Instance::Move(Direction::DirectionEnum direction) noexcept {
    auto destination = ResolveMoveDestination(*this, direction);
    if (!destination) {
	if (Direction::IsDefined(direction) && this->GetParentRoom()) {
	    auto descriptor = this->GetDescriptor();
	    if (descriptor) {
		descriptor->PrintFormat(
		    "%sYou can't go %s from here!%s\r\n",
		    descriptor->GetColor(Color::C_FAILED),
		    Direction::ToString(direction).c_str(),
		    descriptor->GetColor(Color::C_NORMAL));
	    }
	}
	return false;
    }

    return destination->AddChild(shared_from_this());
}

//! Moves this instance into \p roomInstance.
//! \param roomInstance the destination room instance
//! \return \c true if the move succeeded
bool Instance::MoveTo(const InstancePtr& roomInstance) noexcept {
    bool result = false;
    InstancePtr self;
    InstancePtr sourceParent;
    WorldPtr destWorld;
    WorldPtr sourceWorld;

    if (roomInstance && roomInstance->GetRoom()) {
	destWorld = roomInstance->GetWorld();
	sourceWorld = world_.lock();
	if (destWorld && sourceWorld) {
	    self = shared_from_this();
	    if (sourceWorld == destWorld) {
		result = roomInstance->AddChild(self);
	    } else {
		sourceParent = parent_.lock();
		result = true;

		sourceWorld->RemoveInstance(self);
		if (!destWorld->AddInstance(self))
		    result = false;
		if (result && !roomInstance->AddChild(self))
		    result = false;
	    }
	}
    }

    if (!result && sourceWorld && destWorld &&
	    sourceWorld != destWorld) {
	if (auto currentWorld = self->GetWorld()) {
	    if (currentWorld != sourceWorld)
		currentWorld->RemoveInstance(self);
	}
	if (!sourceWorld->AddInstance(self)) {
	    LOGGER_WORLD() << "MoveTo rollback AddInstance failed for "
		<< self->GetName() << '.';
	} else if (sourceParent && !sourceParent->AddChild(self)) {
	    LOGGER_WORLD() << "MoveTo rollback AddChild failed for "
		<< self->GetName() << '.';
	}
    }

    return result;
}

//! Removes this instance from its world object.
void Instance::Remove() noexcept {
    if (auto world = world_.lock())
	world->RemoveInstance(shared_from_this());
}

//! Removes a child instance from this instance.
//! \param instance the instance to remove
void Instance::RemoveChild(const InstancePtr& instance) noexcept {
    if (!instance || instance->parent_.lock().get() != this)
	return;
    contents_.erase(instance);
    instance->parent_.reset();
    this->AdjustContentsWeight(shared_from_this(), -instance->GetTotalWeight());
}

//! Sets the controlling descriptor.
//! \param descriptor the descriptor, or null to clear
//! \sa #GetDescriptor()
void Instance::SetDescriptor(const DescriptorPtr& descriptor) noexcept {
    descriptor_ = descriptor;
}

//! Sets the gender.
//! \param gender the gender
//! \sa #GetGender() const
void Instance::SetGender(Gender::GenderEnum gender) noexcept {
    gender_ = gender;
    if (player_)
	player_->SetGender(gender);
}

//! Sets the player.
//! \param player the player
//! \sa #GetPlayer() const
void Instance::SetPlayer(const PlayerPtr& player) noexcept {
    player_ = player;
    gender_ = player_ ? player_->GetGender() : Gender::GENDER_UNDEFINED;
}

//! Sets the intrinsic weight.
//! \param weight the nonnegative finite weight
void Instance::SetWeight(const double weight) noexcept {
    if (!std::isfinite(weight) || weight < 0.0) {
	LOGGER_SYSTEM() << "Invalid instance weight " << weight << ".";
    } else {
	const auto delta = weight - weight_;
	weight_ = weight;
	this->AdjustContentsWeight(parent_.lock(), delta);
    }
}

//! Adjusts contents weight through ancestors.
//! \param parent the first ancestor
//! \param weight the weight delta
void Instance::AdjustContentsWeight(
	const InstancePtr& parent,
	const double weight) noexcept {
    for (auto p = parent; p; p = p->parent_.lock())
	p->contentsWeight_ += weight;
}

}; // namespace Core
}; // namespace Scratch
