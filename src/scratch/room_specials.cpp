//! \file room_specials.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_ROOM_SPECIALS_CPP_

#include <scratch/room.hpp>
#include <scratch/room_exit_specials.hpp>
#include <scratch/room_specials.hpp>

namespace Scratch {
namespace Core {

RoomSpecials::RoomSpecials() noexcept :
    exits_(),
    room_() {
    // Nothing.
}

RoomSpecials::RoomSpecials(const RoomPtr& room) noexcept :
    exits_(),
    room_(room) {
    // Nothing.
}

RoomSpecials::~RoomSpecials() noexcept {
    // Nothing.
}

RoomExitSpecialsPtr RoomSpecials::GetExit(
	Direction::DirectionEnum direction) const noexcept {
    auto found = exits_.find(direction);
    return found != std::end(exits_) ? found->second : nullptr;
}

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
