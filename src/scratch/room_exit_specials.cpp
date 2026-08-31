//! \file room_exit_specials.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_ROOM_EXIT_SPECIALS_CPP_

#include <scratch/instance.hpp>
#include <scratch/room_exit_specials.hpp>
#include <scratch/room_specials.hpp>

namespace Scratch {
namespace Core {

RoomExitSpecials::RoomExitSpecials() noexcept :
    definition_(),
    target_() {
    // Nothing.
}

RoomExitSpecials::RoomExitSpecials(
	const ExitPtr& definition) noexcept :
    definition_(definition),
    target_() {
    // Nothing.
}

RoomExitSpecials::~RoomExitSpecials() noexcept {
    // Nothing.
}

Direction::DirectionEnum RoomExitSpecials::GetDirection() const noexcept {
    return definition_ ?
	definition_->GetDirection() :
	Direction::D_UNDEFINED;
}

RoomExitSpecialsPtr RoomExitSpecials::GetOpposite() const noexcept {
    auto target = target_.lock();
    if (!target)
	return nullptr;
    auto specials = target->GetRoomSpecials();
    return specials ?
	specials->GetExit(Direction::Opposite(this->GetDirection())) :
	RoomExitSpecialsPtr();
}

}; // namespace Core
}; // namespace Scratch
