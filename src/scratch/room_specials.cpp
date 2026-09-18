//! \file room_specials.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_ROOM_SPECIALS_CPP_

#include <scratch/room.hpp>
#include <scratch/room_exit_specials.hpp>
#include <scratch/room_specials.hpp>
#include <scratch/sector.hpp>

namespace Scratch {
namespace Core {

RoomSpecials::RoomSpecials() noexcept :
    exits_(),
    room_(),
    sector_() {
    // Nothing.
}

RoomSpecials::RoomSpecials(
	const RoomPtr& room,
	const SectorPtr& sector) noexcept :
    exits_(),
    room_(room),
    sector_(sector) {
    // Nothing.
}

RoomSpecials::~RoomSpecials() noexcept {
    // Nothing.
}

//! Gets an exit.
//! \param direction the exit direction
//! \return the exit specials, or \c nullptr
RoomExitSpecialsPtr RoomSpecials::GetExit(
	Direction::DirectionEnum direction) const noexcept {
    auto found = exits_.find(direction);
    return found != std::end(exits_) ? found->second : nullptr;
}

//! Gets whether the room is indoors.
bool RoomSpecials::GetIndoors() const noexcept {
    if (room_ && room_->GetOverrideIndoors())
	return *room_->GetOverrideIndoors();
    if (sector_)
	return sector_->GetIndoorsBit();
    return false;
}

//! Sets an exit.
//! \param direction the exit direction
//! \param exit the exit specials, or null
void RoomSpecials::SetExit(
	Direction::DirectionEnum direction,
	const RoomExitSpecialsPtr& exit) noexcept {
    if (exit)
	exits_[direction] = exit;
    else
	exits_.erase(direction);
}

}; // namespace Core
}; // namespace Scratch
