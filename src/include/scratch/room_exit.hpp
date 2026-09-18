//! \file room_exit.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ROOM_EXIT_HPP_
#define _SCRATCH_ROOM_EXIT_HPP_

#include <scratch/direction.hpp>
#include <scratch/door.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Utility {
class Data;
}; // namespace Utility
}; // namespace Scratch

namespace Scratch {
namespace Core {

// Forward declarations.
class RoomSpecials;

using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;
using RoomSpecialsPtr = std::shared_ptr<RoomSpecials>;

//! The room exit class. \{
class RoomExit {
public:
    //! Default constructor.
    RoomExit() noexcept;

    //! Copy constructor.
    //! \param other the exit to copy
    RoomExit(const RoomExit& other) noexcept;

    //! Default assignment.
    //! \param other the exit to assign
    RoomExit& operator=(const RoomExit& other) noexcept;

    //! Gets the exit direction.
    //! \sa #SetDirection(Direction::DirectionEnum)
    Direction::DirectionEnum GetDirection() const noexcept {
	return direction_;
    }

    //! Gets the door state.
    //! \sa #SetDoorState(Door::DoorEnum)
    Door::DoorEnum GetDoorState() const noexcept {
	return doorState_;
    }

    //! Gets the secret exit bit.
    //! \sa #SetSecretBit(const bool)
    bool GetSecretBit() const noexcept {
	return secretBit_;
    }

    //! Gets the target room name.
    //! \sa #SetTarget(const String&)
    String GetTarget() const noexcept {
	return target_;
    }

    //! Gets the exit title.
    //! \sa #SetTitle(const String&)
    String GetTitle() const noexcept {
	return title_;
    }

    //! Materializes this exit into live room specials.
    //! \param roomSpecials the destination room specials
    void MaterializeInto(const RoomSpecialsPtr& roomSpecials) const noexcept;

    //! Reads this exit from a data node.
    //! \param data the exit data
    //! \sa #WriteData(const DataPtr&) const
    void ReadData(const DataPtr& data) noexcept;

    //! Sets the direction.
    //! \param direction the exit direction
    //! \sa #GetDirection() const
    void SetDirection(Direction::DirectionEnum direction) noexcept {
	direction_ = direction;
    }

    //! Sets the door state.
    //! \param doorState the door state
    //! \sa #GetDoorState() const
    void SetDoorState(Door::DoorEnum doorState) noexcept {
	doorState_ = doorState;
    }

    //! Sets the secret exit bit.
    //! \param secretBit the secret exit bit
    //! \sa #GetSecretBit() const
    void SetSecretBit(const bool secretBit) noexcept {
	secretBit_ = secretBit;
    }

    //! Sets the target room name.
    //! \param target the local or qualified room name
    //! \sa #GetTarget() const
    void SetTarget(const String& target) {
	target_ = target;
    }

    //! Sets the exit title.
    //! \param title the exit title
    //! \sa #GetTitle() const
    void SetTitle(const String& title) {
	title_ = title;
    }

    //! Writes this exit to a data node.
    //! \param data the exit data
    //! \sa #ReadData(const DataPtr&)
    void WriteData(const DataPtr& data) const noexcept;

private:
    Direction::DirectionEnum direction_;
    Door::DoorEnum doorState_;
    bool secretBit_;
    String target_;
    String title_;
};
//! \}

using ExitPtr = std::shared_ptr<RoomExit>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_ROOM_EXIT_HPP_
