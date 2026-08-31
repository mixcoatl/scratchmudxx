//! \file instance.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_INSTANCE_HPP_
#define _SCRATCH_INSTANCE_HPP_

#include <scratch/direction.hpp>
#include <scratch/gender.hpp>
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
class Room;
class RoomSpecials;
class Zone;

// ScratchMUD types.
using Descriptor = Scratch::Net::Descriptor;
using DescriptorPtr = std::shared_ptr<Descriptor>;
using RoomPtr = std::shared_ptr<Room>;
using RoomSpecialsPtr = std::shared_ptr<RoomSpecials>;
using WeakDescriptorPtr = std::weak_ptr<Descriptor>;
using WeakInstancePtr = std::weak_ptr<Instance>;
using WeakInstancePtrSet =
	std::set<WeakInstancePtr, std::owner_less<WeakInstancePtr>>;
using ZonePtr = std::shared_ptr<Zone>;
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

    //! Finds an instance matching \p words.
    //! \param game the game state
    //! \param words the name words
    //! \param nth the 1-based ordinal
    //! \param count the requested count
    //! \return the matched instance, or \c nullptr
    InstancePtr Find(
	const Game& game,
	const std::vector<String>& words,
	unsigned nth = 1,
	unsigned count = 1) const noexcept;

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

    //! Gets the parent room.
    //! \return the room-bearing ancestor, or \c nullptr
    InstancePtr GetParentRoom() const noexcept;

    //! Gets the player.
    //! \sa #SetPlayer(const PlayerPtr&)
    PlayerPtr GetPlayer() const noexcept {
	return player_;
    }

    //! Gets the qualified room name (\c zone:room, optional \c @world).
    //! \return the room reference, or empty if this is not a room instance
    String GetQualifiedRoomName() const noexcept;

    //! Gets the room prototype.
    //! \return the room, or \c nullptr
    RoomPtr GetRoom() const noexcept;

    //! Gets the room specials.
    //! \return the room specials, or \c nullptr
    RoomSpecialsPtr GetRoomSpecials() const noexcept {
	return roomSpecials_;
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

    //! Gets the world object.
    //! \sa World::AddInstance(const InstancePtr&)
    WorldPtr GetWorld() const noexcept {
	return world_.lock();
    }

    //! Gets the source zone.
    //! \return the zone, or \c nullptr
    ZonePtr GetZone() const noexcept {
	return zone_;
    }

    //! Matches \p name against this instance.
    //! \param name the name
    //! \param seeker the searching instance, or null
    //! \return \c true if this instance matches \p name
    bool Matches(
	const String& name,
	const InstancePtr& seeker = nullptr) const noexcept;

    //! Returns whether this instance can move in \p direction.
    //! \param direction the direction
    //! \return \c true if the move can succeed
    //! \sa #Move(Direction::DirectionEnum)
    bool CanMove(Direction::DirectionEnum direction) const noexcept;

    //! Moves through an exit in \p direction.
    //! \param direction the direction
    //! \return \c true if the move succeeded
    //! \sa Direction::DirectionEnum
    //! \sa #AddChild(const InstancePtr&)
    bool Move(Direction::DirectionEnum direction) noexcept;

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

    //! Sets the room specials.
    //! \param roomSpecials the room specials, or null
    void SetRoomSpecials(const RoomSpecialsPtr& roomSpecials) noexcept {
	roomSpecials_ = roomSpecials;
    }

    //! Sets the source zone.
    //! \param zone the zone, or null
    void SetZone(const ZonePtr& zone) noexcept {
	zone_ = zone;
    }

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

    //! The room specials.
    //! \sa #GetRoomSpecials() const
    //! \sa #SetRoomSpecials(const RoomSpecialsPtr&)
    RoomSpecialsPtr roomSpecials_;

    //! The intrinsic weight.
    //! \sa #GetWeight() const
    //! \sa #SetWeight(const double)
    double weight_;

    //! The owning world object.
    //! \sa #GetWorld() const
    WeakWorldPtr world_;

    //! The source zone.
    //! \sa #GetZone() const
    //! \sa #SetZone(const ZonePtr&)
    ZonePtr zone_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_INSTANCE_HPP_
