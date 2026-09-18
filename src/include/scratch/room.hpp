//! \file room.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ROOM_HPP_
#define _SCRATCH_ROOM_HPP_

#include <boost/optional.hpp>
#include <scratch/direction.hpp>
#include <scratch/door.hpp>
#include <scratch/lighting.hpp>
#include <scratch/room_exit.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <scratch/thing.hpp>

namespace Scratch {
namespace Utility {
class Data;
}; // namespace Utility
}; // namespace Scratch

namespace Scratch {
namespace Core {

// Forward declarations.
class Instance;
class Room;
class World;
class Zone;

// ScratchMUD types.
using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;
using InstancePtr = std::shared_ptr<Instance>;
using RoomPtr = std::shared_ptr<Room>;
using WeakZonePtr = std::weak_ptr<Zone>;
using WorldPtr = std::shared_ptr<World>;
using ZonePtr = std::shared_ptr<Zone>;

//! The room class. \{
class Room : public Thing {
public:
    //! Default constructor.
    Room() noexcept;

    //! Copy constructor.
    //! \param other the \sa room to copy
    Room(const Room& other) noexcept;

    //! Destructor.
    virtual ~Room() noexcept;

    //! Default assignment.
    //! \param other the \sa room to assign
    Room& operator=(const Room& other) noexcept;

    //! Removes an exit.
    //! \param direction the exit direction
    void EraseExit(Direction::DirectionEnum direction) noexcept {
	exits_.erase(direction);
    }

    //! Gets the description.
    //! \sa #SetDescription(const String&)
    String GetDescription() const noexcept {
	return description_;
    }

    //! Gets an exit.
    //! \param direction the exit direction
    //! \return the exit, or \c nullptr
    ExitPtr GetExit(Direction::DirectionEnum direction) const noexcept;

    //! Gets the defined exits.
    std::map<Direction::DirectionEnum, ExitPtr> GetExits() const noexcept {
	return exits_;
    }

    //! Gets the immortal room bit.
    //! \sa #SetImmortalBit(const bool)
    bool GetImmortalBit() const noexcept {
	return immortalBit_;
    }

    //! Gets the lighting type.
    //! \sa #SetLighting(Lighting::LightingEnum)
    Lighting::LightingEnum GetLighting() const noexcept {
	return lighting_;
    }

    //! Gets the occupant limit.
    //! \sa #SetOccupantLimit(const boost::optional<std::uint8_t>&)
    boost::optional<std::uint8_t> GetOccupantLimit() const noexcept {
	return occupantLimit_;
    }

    //! Gets the optional indoors override.
    //! \sa #SetOverrideIndoors(const boost::optional<bool>&)
    boost::optional<bool> GetOverrideIndoors() const noexcept {
	return overrideIndoors_;
    }

    //! Gets the private room bit.
    //! \sa #SetPrivateBit(const bool)
    bool GetPrivateBit() const noexcept {
	return privateBit_;
    }

    //! Gets the zone-qualified name.
    String GetQualifiedName() const noexcept;

    //! Gets the sector catalog name.
    //! \sa #SetSector(const String&)
    String GetSector() const noexcept {
	return sector_;
    }

    //! Gets the title.
    //! \sa #SetTitle(const String&)
    String GetTitle() const noexcept {
	return title_;
    }

    //! Gets the owning zone.
    //! \return the zone, or \c nullptr
    ZonePtr GetZone() const noexcept {
	return zone_.lock();
    }

    //! Materializes this room into a world.
    //! \param world the destination world
    void MaterializeInto(const WorldPtr& world);

    //! Reads this room from a data node.
    //! \param data the room data
    void ReadData(const DataPtr& data) noexcept;

    //! Reads bit flags from a data node.
    //! \param data the RoomBits data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteRoomBitsData(const DataPtr&) const
    void ReadRoomBitsData(const DataPtr& data) noexcept;

    //! Reads exits from a data node.
    //! \param data the exits data
    void ReadExitsData(const DataPtr& data) noexcept;

    //! Sets the description.
    //! \param description the description
    void SetDescription(const String& description) {
	description_ = description;
    }

    //! Sets the immortal room bit.
    //! \param immortalBit the immortal room bit
    //! \sa #GetImmortalBit() const
    void SetImmortalBit(const bool immortalBit) noexcept {
	immortalBit_ = immortalBit;
    }

    //! Sets the lighting type.
    //! \param lighting the lighting type
    //! \sa #GetLighting() const
    void SetLighting(Lighting::LightingEnum lighting) noexcept {
	lighting_ = lighting;
    }

    //! Sets the occupant limit.
    //! \param occupantLimit the occupant limit
    //! \sa #GetOccupantLimit() const
    void SetOccupantLimit(
	    const boost::optional<std::uint8_t>& occupantLimit) noexcept {
	occupantLimit_ = occupantLimit;
    }

    //! Sets the optional indoors override.
    //! \param overrideIndoors the indoors override
    //! \sa #GetOverrideIndoors() const
    void SetOverrideIndoors(
	    const boost::optional<bool>& overrideIndoors) noexcept {
	overrideIndoors_ = overrideIndoors;
    }

    //! Sets the private room bit.
    //! \param privateBit the private room bit
    //! \sa #GetPrivateBit() const
    void SetPrivateBit(const bool privateBit) noexcept {
	privateBit_ = privateBit;
    }

    //! Sets an exit.
    //! \param direction the exit direction
    //! \param exit the exit, or null to erase it
    void SetExit(
	    Direction::DirectionEnum direction,
	    const ExitPtr& exit) noexcept {
	if (exit) {
	    auto committed = std::make_shared<RoomExit>(*exit);
	    committed->SetDirection(direction);
	    exits_[direction] = std::move(committed);
	} else {
	    this->EraseExit(direction);
	}
    }

    //! Sets the sector catalog name.
    //! \param sector the sector name
    //! \sa #GetSector() const
    void SetSector(const String& sector) noexcept {
	sector_ = sector;
    }

    //! Sets the title.
    //! \param title the title
    void SetTitle(const String& title) {
	title_ = title;
    }

    //! Sets the owning zone.
    //! \param zone the zone, or null
    void SetZone(const ZonePtr& zone) noexcept {
	zone_ = zone;
    }

    //! Writes this room to a data node.
    //! \param data the room data
    void WriteData(const DataPtr& data) const noexcept;

    //! Writes bit flags to a data node.
    //! \param data the RoomBits data node to write
    //! \sa #ReadRoomBitsData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteRoomBitsData(const DataPtr& data) const noexcept;

    //! Writes exits to a data node.
    //! \param data the exits data
    void WriteExitsData(const DataPtr& data) const noexcept;

private:
    String description_;
    std::map<Direction::DirectionEnum, ExitPtr> exits_;
    bool immortalBit_;
    Lighting::LightingEnum lighting_;
    boost::optional<std::uint8_t> occupantLimit_;
    boost::optional<bool> overrideIndoors_;
    bool privateBit_;
    String sector_;
    String title_;
    WeakZonePtr zone_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_ROOM_HPP_
