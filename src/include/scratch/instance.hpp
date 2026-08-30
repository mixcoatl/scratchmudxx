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
#include <scratch/world.hpp>

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
using WeakInstancePtr = std::weak_ptr<Instance>;
using WeakInstancePtrSet =
	std::set<WeakInstancePtr, std::owner_less<WeakInstancePtr>>;
using WeakWorldPtr = std::weak_ptr<World>;

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

    //! Adds a child instance to this instance.
    //! \param instance the instance to add
    //! \return \c true if added
    //! \sa #RemoveChild(const InstancePtr&)
    bool AddChild(const InstancePtr& instance) noexcept;

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

    //! Gets the contained instances.
    //! \sa #AddChild(const InstancePtr&)
    //! \sa #RemoveChild(const InstancePtr&)
    InstancePtrSet GetContents() const noexcept {
	return contents_;
    }

    //! Gets the contents weight.
    //! \sa #GetTotalWeight() const
    double GetContentsWeight() const noexcept {
	return contentsWeight_;
    }

    //! Gets the total weight.
    //! \sa #GetWeight() const
    //! \sa #GetContentsWeight() const
    double GetTotalWeight() const noexcept {
	return weight_ + contentsWeight_;
    }

    //! Gets the intrinsic weight.
    //! \sa #SetWeight(const double)
    double GetWeight() const noexcept {
	return weight_;
    }

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

    //! Gets the parent instance.
    //! \sa #AddChild(const InstancePtr&)
    //! \sa #RemoveChild(const InstancePtr&)
    InstancePtr GetParent() const noexcept {
	return parent_.lock();
    }

    //! Gets the player.
    //! \sa #SetPlayer(const PlayerPtr&)
    PlayerPtr GetPlayer() const noexcept {
	return player_;
    }

    //! Gets the world object.
    //! \sa World::AddInstance(const InstancePtr&)
    WorldPtr GetWorld() const noexcept {
	return world_.lock();
    }

    //! Matches \p name against this instance.
    //! \param name the name
    //! \param seeker the searching instance, or null
    //! \return \c true if this instance matches \p name
    bool Matches(
	const String& name,
	const InstancePtr& seeker = nullptr) const noexcept;

    //! Removes this instance from its world object.
    //! \sa World::RemoveInstance(const InstancePtr&)
    void Remove() noexcept;

    //! Removes a child instance from this instance.
    //! \param instance the instance to remove
    //! \sa #AddChild(const InstancePtr&)
    void RemoveChild(const InstancePtr& instance) noexcept;

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

    //! Sets the intrinsic weight.
    //! \param weight the nonnegative finite weight
    //! \sa #GetWeight() const
    void SetWeight(const double weight) noexcept;

protected:
    friend class World;

    //! Adjusts contents weight through ancestors.
    //! \param parent the first ancestor
    //! \param weight the weight delta
    static void AdjustContentsWeight(
	const InstancePtr& parent,
	const double weight) noexcept;

    //! The contained instances.
    //! \sa #GetContents() const
    InstancePtrSet contents_;

    //! The contents weight.
    //! \sa #GetContentsWeight() const
    double contentsWeight_;

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

    //! The parent instance.
    //! \sa #GetParent() const
    WeakInstancePtr parent_;

    //! The player.
    //! \sa #GetPlayer() const
    //! \sa #SetPlayer(const PlayerPtr&)
    PlayerPtr player_;

    //! The intrinsic weight.
    //! \sa #GetWeight() const
    //! \sa #SetWeight(const double)
    double weight_;

    //! The owning world object.
    //! \sa #GetWorld() const
    WeakWorldPtr world_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_INSTANCE_HPP_
