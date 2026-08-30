//! \file instance.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_INSTANCE_CPP_

#include <scratch/descriptor.hpp>
#include <scratch/game.hpp>
#include <scratch/instance.hpp>
#include <scratch/logger.hpp>
#include <scratch/parser.hpp>
#include <scratch/player.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

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
	weight_(0.0),
	world_() {
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
	weight_(other.weight_),
	world_() {
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
    auto parent = parent_.lock();
    this->AdjustContentsWeight(parent, delta);
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

    // Ancestor cycle.
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

    // Adjust contents weight.
    const auto weight = instance->GetTotalWeight();
    this->AdjustContentsWeight(shared_from_this(), weight);
    return true;
}

//! Removes a child instance from this instance.
//! \param instance the instance to remove
void Instance::RemoveChild(const InstancePtr& instance) noexcept {
    if (!instance || instance->parent_.lock().get() != this)
	return;
    contents_.erase(instance);
    instance->parent_.reset();
    const auto weight = -instance->GetTotalWeight();
    this->AdjustContentsWeight(shared_from_this(), weight);
}

//! Removes this instance from its world object.
void Instance::Remove() noexcept {
    auto world = world_.lock();
    if (world)
	world->RemoveInstance(shared_from_this());
}

//! Finds an instance matching \p line.
//! \param game the game state
//! \param line the targeting line
//! \return the matched instance, or \c nullptr
InstancePtr Instance::Find(
	const Game& game,
	const String& line) const noexcept {
    Parser parser;
    if (!parser.Parse(line) ||
	    parser.GetSize() != 1 ||
	    parser.GetPhrase(0).GetCount() != 1)
	return nullptr;
    return this->Find(game, parser.GetPhrase(0));
}

//! Finds an instance matching \p phrase.
//! \param game the game state
//! \param phrase the targeting phrase
//! \return the matched instance, or \c nullptr
InstancePtr Instance::Find(
	const Game& game,
	const Parser::Phrase& phrase) const noexcept {
    if (phrase.GetCount() != 1)
	return nullptr;
    const auto count = phrase.GetCount();
    const auto nth = phrase.GetNth();
    const auto& words = phrase.GetWords();
    if (!nth || words.empty())
	return nullptr;

    auto seeker = std::const_pointer_cast<Instance>(this->shared_from_this());

    // Shortcuts: one name, count 1, nth 1.
    if (words.size() == 1 && count == 1 && nth == 1) {
	const auto& name = words.front();
	if (!Scratch::Algorithm::Strings::CompareCi(name, "me") ||
		!Scratch::Algorithm::Strings::CompareCi(name, "self")) {
	    return seeker;
	}
	if (!name.empty() && name.front() == '%') {
	    auto instance = game.GetInstance(name.substr(1));
	    if (!instance || !instance->Matches(name, seeker))
		return nullptr;
	    return instance;
	}
    }

    unsigned found = 0;
    for (auto& instance: game.GetInstances()) {
	if (!instance)
	    continue;
	auto matched = true;
	for (const auto& name: words) {
	    if (!instance->Matches(name, seeker)) {
		matched = false;
		break;
	    }
	}
	if (matched && ++found == nth)
	    return instance;
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
	if (id.empty())
	    return false;
	return !Scratch::Algorithm::Strings::CompareCi(name_, id);
    }

    if (player_) {
	const auto playerName = player_->GetName();
	return Scratch::Algorithm::Strings::StartsWithCi(playerName, name);
    }

    return false;
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
	auto parent = parent_.lock();
	for (auto ancestor = parent;
		ancestor;
		ancestor = ancestor->parent_.lock())
	    ancestor->contentsWeight_ += delta;
    }
}

//! Adjusts contents weight through ancestors.
//! \param parent the first ancestor
//! \param weight the weight delta
void Instance::AdjustContentsWeight(
	const InstancePtr& parent,
	const double weight) noexcept {
    for (auto ancestor = parent;
	 ancestor; ancestor = ancestor->parent_.lock()) {
	ancestor->contentsWeight_ += weight;
    }
}

}; // namespace Core
}; // namespace Scratch
