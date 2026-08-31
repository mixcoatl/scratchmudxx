//! \file zone.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ZONE_HPP_
#define _SCRATCH_ZONE_HPP_

#include <scratch/repository.hpp>
#include <scratch/room.hpp>
#include <scratch/thing.hpp>

namespace Scratch {
namespace Core {

using RoomMap = StringMapCi<RoomPtr>;

//! The zone class. \{
class Zone : public Thing {
public:
    //! Default constructor.
    Zone() noexcept;

    //! Copy constructor.
    //! \param other the zone to copy
    Zone(const Zone& other) noexcept;

    //! Destructor.
    virtual ~Zone() noexcept;

    //! Default assignment.
    //! \param other the zone to assign
    Zone& operator=(const Zone& other) noexcept;

    //! Creates a room with a generated name.
    //! \return the created room
    RoomPtr CreateRoom() noexcept;

    //! Erases a room.
    //! \param roomName the zone-local room name
    //! \return whether the room was erased
    bool EraseRoom(const String& roomName) noexcept;

    //! Gets a room.
    //! \param roomName the zone-local room name
    //! \return the room, or \c nullptr
    RoomPtr GetRoom(const String& roomName) const noexcept;

    //! Gets room names.
    StringSetCi GetRoomNames() const noexcept;

    //! Reads this zone from a data node.
    //! \param data the zone data
    void ReadData(const DataPtr& data) noexcept;

    //! Reads rooms from a data node.
    //! \param data the rooms data
    void ReadRoomsData(const DataPtr& data) noexcept;

    //! Stores a room.
    //! \param roomName the zone-local room name
    //! \param room the room to store
    //! \return whether the room was stored
    bool StoreRoom(const String& roomName, const RoomPtr& room) noexcept;

    //! Writes this zone to a data node.
    //! \param data the zone data
    void WriteData(const DataPtr& data) const noexcept;

    //! Writes rooms to a data node.
    //! \param data the rooms data
    void WriteRoomsData(const DataPtr& data) const noexcept;

private:
    RoomMap rooms_;
};
//! \}

using ZonePtr = std::shared_ptr<Zone>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_ZONE_HPP_
