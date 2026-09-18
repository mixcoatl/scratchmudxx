//! \file room_specials.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ROOM_SPECIALS_HPP_
    #define _SCRATCH_ROOM_SPECIALS_HPP_

#include <scratch/direction.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {

// Forward declarations.
class Room;
class RoomExitSpecials;
class Sector;

// ScratchMUD types.
using RoomExitSpecialsPtr = std::shared_ptr<RoomExitSpecials>;
using RoomExitSpecialsMap = std::map<
    Direction::DirectionEnum,
    RoomExitSpecialsPtr>;
using RoomPtr = std::shared_ptr<Room>;
using SectorPtr = std::shared_ptr<Sector>;

//! The room specials class. \{
class RoomSpecials {
public:
    //! Default constructor.
    RoomSpecials() noexcept;

    //! Constructs room specials from an authored room.
    //! \param room the authored room
    //! \param sector the resolved sector, or null
    explicit RoomSpecials(
	const RoomPtr& room,
	const SectorPtr& sector = SectorPtr()) noexcept;

    //! Destructor.
    virtual ~RoomSpecials() noexcept;

    //! Gets an exit.
    //! \param direction the exit direction
    //! \return the exit specials, or \c nullptr
    RoomExitSpecialsPtr GetExit(
	Direction::DirectionEnum direction) const noexcept;

    //! Gets the exit specials.
    RoomExitSpecialsMap GetExits() const noexcept {
	return exits_;
    }

    //! Gets whether the room is indoors.
    bool GetIndoors() const noexcept;

    //! Gets the authored room.
    RoomPtr GetRoom() const noexcept {
	return room_;
    }

    //! Gets the resolved sector.
    SectorPtr GetSector() const noexcept {
	return sector_;
    }

    //! Sets an exit.
    //! \param direction the exit direction
    //! \param exit the exit specials, or null
    void SetExit(
	Direction::DirectionEnum direction,
	const RoomExitSpecialsPtr& exit) noexcept;

    //! Sets the authored room.
    //! \param room the authored room
    void SetRoom(const RoomPtr& room) noexcept {
	room_ = room;
    }

    //! Sets the resolved sector.
    //! \param sector the sector, or null
    void SetSector(const SectorPtr& sector) noexcept {
	sector_ = sector;
    }

private:
    //! The runtime exit specials.
    RoomExitSpecialsMap exits_;

    //! The authored room.
    RoomPtr room_;

    //! The resolved sector.
    SectorPtr sector_;
};
//! \}

using RoomSpecialsPtr = std::shared_ptr<RoomSpecials>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_ROOM_SPECIALS_HPP_
