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
#include <scratch/parser.hpp>
#include <scratch/player.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
Instance::Instance() noexcept :
	std::enable_shared_from_this<Instance>(),
	descriptor_(),
	gender_(Gender::GENDER_UNDEFINED),
	name_(),
	player_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa instance to copy
Instance::Instance(const Instance& other) noexcept :
	std::enable_shared_from_this<Instance>(),
	descriptor_(),
	gender_(other.gender_),
	name_(other.name_),
	player_(other.player_) {
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
    return *this;
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
	if (!Scratch::Algorithm::StringCompareCi(name, "me") ||
		!Scratch::Algorithm::StringCompareCi(name, "self")) {
	    return seeker;
	}
	if (!name.empty() && name.front() == '%') {
	    auto instance = game.GetInstance(name.substr(1));
	    if (!instance || !instance->Matches(seeker, name))
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
	    if (!instance->Matches(seeker, name)) {
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
//! \param seeker the searching instance, or null
//! \param name the name
//! \return \c true if this instance matches \p name
//! \sa #Matches(const String&) const
bool Instance::Matches(
	const InstancePtr&,
	const String& name) const noexcept {
    if (name.empty())
	return false;

    if (name.front() == '%') {
	const auto id = name.substr(1);
	if (id.empty())
	    return false;
	return !Scratch::Algorithm::StringCompareCi(name_, id);
    }

    if (player_) {
	const auto playerName = player_->GetName();
	return Scratch::Algorithm::StringStartsWithCi(playerName, name);
    }

    return false;
}

//! Matches \p name against this instance.
//! \param name the name
//! \return \c true if this instance matches \p name
//! \sa #Matches(const std::shared_ptr<Instance>&, const String&) const
bool Instance::Matches(const String& name) const noexcept {
    return this->Matches(InstancePtr(), name);
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

}; // namespace Core
}; // namespace Scratch
