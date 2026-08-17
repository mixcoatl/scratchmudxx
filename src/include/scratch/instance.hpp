//! \file instance.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_INSTANCE_HPP_
#define _SCRATCH_INSTANCE_HPP_

#include <scratch/gender.hpp>
#include <scratch/parser.hpp>
#include <scratch/player.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Net {
class Descriptor;
}; // namespace Net
}; // namespace Scratch

namespace Scratch {
namespace Core {

// Forward declarations.
class Game;

// ScratchMUD types.
using Descriptor = Scratch::Net::Descriptor;
using DescriptorPtr = std::shared_ptr<Descriptor>;
using WeakDescriptorPtr = std::weak_ptr<Descriptor>;

//! The instance class. \{
class Instance : public std::enable_shared_from_this<Instance> {
public:
    //! Default constructor.
    Instance() noexcept;

    //! Copy constructor.
    //! \param other the \sa instance to copy
    //! \remark Controlling descriptor not copied.
    Instance(const Instance& other) noexcept;

    //! Destructor.
    virtual ~Instance() noexcept;

    //! Default assignment.
    //! \param other the \sa instance to assign
    //! \remark Controlling descriptor not assigned.
    Instance& operator=(const Instance& other) noexcept;

    //! Finds an instance matching \p line.
    //! \param game the game state
    //! \param line the targeting line
    //! \return the matched instance, or \c nullptr
    std::shared_ptr<Instance> Find(
	const Game& game,
	const String& line) const noexcept;

    //! Finds an instance matching \p phrase.
    //! \param game the game state
    //! \param phrase the targeting phrase
    //! \return the matched instance, or \c nullptr
    std::shared_ptr<Instance> Find(
	const Game& game,
	const Parser::Phrase& phrase) const noexcept;

    //! Gets the controlling descriptor.
    //! \sa #SetDescriptor(const DescriptorPtr&)
    DescriptorPtr GetDescriptor() noexcept;

    //! Gets the gender.
    //! \sa #SetGender(Gender::GenderEnum)
    Gender::GenderEnum GetGender() const noexcept {
	return gender_;
    }

    //! Gets the instance name.
    //! \sa #SetName(const String&)
    String GetName() const noexcept {
	return name_;
    }

    //! Gets the player.
    //! \sa #SetPlayer(const PlayerPtr&)
    PlayerPtr GetPlayer() const noexcept {
	return player_;
    }

    //! Matches \p name against this instance.
    //! \param seeker the searching instance, or null
    //! \param name the name
    //! \return \c true if this instance matches \p name
    //! \sa #Matches(const String&) const
    bool Matches(
	const std::shared_ptr<Instance>& seeker,
	const String& name) const noexcept;

    //! Matches \p name against this instance.
    //! \param name the name
    //! \return \c true if this instance matches \p name
    //! \sa #Matches(const std::shared_ptr<Instance>&, const String&) const
    bool Matches(const String& name) const noexcept;

    //! Sets the controlling descriptor.
    //! \param descriptor the descriptor, or null to clear
    //! \sa #GetDescriptor()
    void SetDescriptor(const DescriptorPtr& descriptor) noexcept;

    //! Sets the gender.
    //! \param gender the gender
    //! \sa #GetGender() const
    void SetGender(Gender::GenderEnum gender) noexcept;

    //! Sets the instance name.
    //! \param name the instance name
    //! \sa #GetName() const
    void SetName(const String& name) {
	name_ = name;
    }

    //! Sets the player.
    //! \param player the player
    //! \sa #GetPlayer() const
    void SetPlayer(const PlayerPtr& player) noexcept;

protected:
    //! The controlling descriptor.
    //! \sa #GetDescriptor()
    //! \sa #SetDescriptor(const DescriptorPtr&)
    WeakDescriptorPtr descriptor_;

    //! The gender.
    //! \sa #GetGender() const
    //! \sa #SetGender(Gender::GenderEnum)
    Gender::GenderEnum gender_;

    //! The instance name.
    //! \sa #GetName() const
    //! \sa #SetName(const String&)
    String name_;

    //! The player.
    //! \sa #GetPlayer() const
    //! \sa #SetPlayer(const PlayerPtr&)
    PlayerPtr player_;
};
//! \}

//! The type of a shared instance pointer.
using InstancePtr = std::shared_ptr<Instance>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_INSTANCE_HPP_
