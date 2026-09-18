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
    doorState_(Door::DOOR_NONE),
    secretBit_(false),
    target_() {
    // Nothing.
}

RoomExitSpecials::~RoomExitSpecials() noexcept {
    // Nothing.
}

//! Gets the direction.
Direction::DirectionEnum RoomExitSpecials::GetDirection() const noexcept {
    return definition_ ?
	definition_->GetDirection() :
	Direction::D_UNDEFINED;
}

//! Gets the opposite exit specials.
RoomExitSpecialsPtr RoomExitSpecials::GetOpposite() const noexcept {
    auto target = target_.lock();
    if (!target)
	return nullptr;
    auto specials = target->GetRoomSpecials();
    return specials ?
	specials->GetExit(Direction::Opposite(this->GetDirection())) :
	RoomExitSpecialsPtr();
}

//! Sets the door state.
//! \param doorState the door state
//! \sa #GetDoorState() const
void RoomExitSpecials::SetDoorState(
	const Door::DoorEnum doorState) noexcept {
    if (doorState_ == doorState)
	return;
    doorState_ = doorState;

    auto opposite = this->GetOpposite();
    if (!opposite)
	return;

    auto definition = opposite->GetDefinition();
    if (!definition || definition->GetDoorState() == Door::DOOR_NONE)
	return;

    opposite->SetDoorState(doorState);
}

//! Sets the secret exit bit.
//! \param secretBit the secret exit bit
//! \sa #GetSecretBit() const
void RoomExitSpecials::SetSecretBit(const bool secretBit) noexcept {
    if (secretBit_ == secretBit)
	return;
    secretBit_ = secretBit;

    auto opposite = this->GetOpposite();
    if (!opposite)
	return;

    opposite->SetSecretBit(secretBit);
}

}; // namespace Core
}; // namespace Scratch
