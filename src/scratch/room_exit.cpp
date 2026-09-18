//! \file room_exit.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#include <scratch/data.hpp>
#include <scratch/room_exit.hpp>
#include <scratch/room_exit_specials.hpp>
#include <scratch/room_specials.hpp>

namespace Scratch {
namespace Core {

RoomExit::RoomExit() noexcept :
	direction_(Direction::D_UNDEFINED),
	doorState_(Door::DOOR_NONE),
	secretBit_(false),
	target_(),
	title_() {
    // Nothing.
}

RoomExit::RoomExit(const RoomExit& other) noexcept :
	direction_(other.direction_),
	doorState_(other.doorState_),
	secretBit_(other.secretBit_),
	target_(other.target_),
	title_(other.title_) {
    // Nothing.
}

RoomExit& RoomExit::operator=(const RoomExit& other) noexcept {
    direction_ = other.direction_;
    doorState_ = other.doorState_;
    secretBit_ = other.secretBit_;
    target_ = other.target_;
    title_ = other.title_;
    return *this;
}

//! Materializes this exit into live room specials.
//! \param roomSpecials the destination room specials
void RoomExit::MaterializeInto(
	const RoomSpecialsPtr& roomSpecials) const noexcept {
    if (!roomSpecials)
	return;
    auto live = std::make_shared<RoomExitSpecials>();
    live->SetDefinition(std::make_shared<RoomExit>(*this));
    live->SetDoorState(this->GetDoorState());
    live->SetSecretBit(this->GetSecretBit());
    roomSpecials->SetExit(this->GetDirection(), live);
}

//! Reads this exit from a data node.
//! \param data the exit data
//! \sa #WriteData(const DataPtr&) const
void RoomExit::ReadData(const DataPtr& data) noexcept {
    target_.clear();
    title_.clear();
    secretBit_ = false;
    doorState_ = Door::DOOR_NONE;
    if (!data)
	return;

    target_ = data->GetString("Target");
    title_ = data->GetString("Title");
    secretBit_ = data->GetYesNo("Secret");

    auto doorStateName = data->GetString("Door");
    if (!doorStateName.empty()) {
	const auto doorState = Door::ByName(doorStateName);
	if (Door::IsDefined(doorState))
	    doorState_ = doorState;
    }
}

//! Writes this exit to a data node.
//! \param data the exit data
//! \sa #ReadData(const DataPtr&)
void RoomExit::WriteData(const DataPtr& data) const noexcept {
    if (!data)
	return;
    if (!target_.empty())
	data->PutString("Target", target_);
    if (!title_.empty())
	data->PutString("Title", title_);
    if (secretBit_)
	data->PutYesNo("Secret", true);
    if (doorState_ != Door::DOOR_NONE) {
	const auto doorState = Door::ToString(doorState_);
	if (!doorState.empty())
	    data->PutString("Door", doorState);
    }
}

}; // namespace Core
}; // namespace Scratch
