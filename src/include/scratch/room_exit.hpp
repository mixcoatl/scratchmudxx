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
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Utility {
class Data;
}; // namespace Utility
}; // namespace Scratch

namespace Scratch {
namespace Core {

using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;

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

    //! Reads this exit from a data node.
    //! \param data the exit data
    void ReadData(const DataPtr& data) noexcept;

    //! Gets the exit direction.
    //! \sa #SetDirection(Direction::DirectionEnum)
    Direction::DirectionEnum GetDirection() const noexcept {
	return direction_;
    }

    //! Gets the target room name.
    String GetTarget() const noexcept {
	return target_;
    }

    //! Sets the direction.
    //! \param direction the exit direction
    //! \sa #GetDirection() const
    void SetDirection(Direction::DirectionEnum direction) noexcept {
	direction_ = direction;
    }

    //! Sets the target room name.
    //! \param target the local or qualified room name
    void SetTarget(const String& target) {
	target_ = target;
    }

    //! Writes this exit to a data node.
    //! \param data the exit data
    void WriteData(const DataPtr& data) const noexcept;

private:
    //! The exit direction.
    Direction::DirectionEnum direction_;

    //! The target room name.
    String target_;
};
//! \}

using ExitPtr = std::shared_ptr<RoomExit>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_ROOM_EXIT_HPP_
