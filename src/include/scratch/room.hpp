//! \file room.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ROOM_HPP_
#define _SCRATCH_ROOM_HPP_

#include <scratch/direction.hpp>
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
    //! \param other the room to copy
    Room(const Room& other) noexcept;

    //! Destructor.
    virtual ~Room() noexcept;

    //! Default assignment.
    //! \param other the room to assign
    Room& operator=(const Room& other) noexcept;

    //! Removes an exit.
    //! \param direction the exit direction
    void EraseExit(Direction::DirectionEnum direction) noexcept {
	exits_.erase(direction);
    }

    //! Gets the description.
    String GetDescription() const noexcept {
	return description_;
    }

    //! Gets the immortal room bit.
    //! \sa #SetImmortalBit(const bool)
    bool GetImmortalBit() const noexcept {
	return immortalBit_;
    }

    //! Gets an exit.
    //! \param direction the exit direction
    //! \return the exit, or \c nullptr
    ExitPtr GetExit(Direction::DirectionEnum direction) const noexcept;

    //! Gets the defined exits.
    std::map<Direction::DirectionEnum, ExitPtr> GetExits() const noexcept {
	return exits_;
    }

    //! Gets the zone-qualified name.
    String GetQualifiedName() const noexcept;

    //! Gets the title.
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
    String title_;
    WeakZonePtr zone_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_ROOM_HPP_
