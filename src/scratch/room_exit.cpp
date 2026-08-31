//! \file room_exit.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#include <scratch/data.hpp>
#include <scratch/room_exit.hpp>

namespace Scratch {
namespace Core {

RoomExit::RoomExit() noexcept :
	direction_(Direction::D_UNDEFINED),
	target_() {
    // Nothing.
}

RoomExit::RoomExit(const RoomExit& other) noexcept :
	direction_(other.direction_),
	target_(other.target_) {
    // Nothing.
}

RoomExit& RoomExit::operator=(const RoomExit& other) noexcept {
    direction_ = other.direction_;
    target_ = other.target_;
    return *this;
}

void RoomExit::ReadData(const DataPtr& data) noexcept {
    target_.clear();
    if (data)
	target_ = data->GetString("Target");
}

void RoomExit::WriteData(const DataPtr& data) const noexcept {
    if (data && !target_.empty())
	data->PutString("Target", target_);
}

}; // namespace Core
}; // namespace Scratch
