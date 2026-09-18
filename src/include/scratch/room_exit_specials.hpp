//! \file room_exit_specials.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ROOM_EXIT_SPECIALS_HPP_
    #define _SCRATCH_ROOM_EXIT_SPECIALS_HPP_

#include <scratch/door.hpp>
#include <scratch/room_exit.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {

// Forward declarations.
class Instance;
class RoomExitSpecials;

// ScratchMUD types.
using InstancePtr = std::shared_ptr<Instance>;
using RoomExitSpecialsPtr = std::shared_ptr<RoomExitSpecials>;
using WeakInstancePtr = std::weak_ptr<Instance>;

//! The room exit specials class. \{
class RoomExitSpecials {
public:
    //! Default constructor.
    RoomExitSpecials() noexcept;

    //! Destructor.
    virtual ~RoomExitSpecials() noexcept;

    //! Returns whether passage is blocked.
    bool BlocksPassage() const noexcept {
	return Door::BlocksPassage(doorState_);
    }

    //! Gets the authored exit.
    ExitPtr GetDefinition() const noexcept {
	return definition_;
    }

    //! Gets the direction.
    Direction::DirectionEnum GetDirection() const noexcept;

    //! Gets the door state.
    //! \sa #SetDoorState(Door::DoorEnum)
    Door::DoorEnum GetDoorState() const noexcept {
	return doorState_;
    }

    //! Gets the opposite exit specials.
    RoomExitSpecialsPtr GetOpposite() const noexcept;

    //! Gets the secret exit bit.
    //! \sa #SetSecretBit(const bool)
    bool GetSecretBit() const noexcept {
	return secretBit_;
    }

    //! Gets the live target instance.
    InstancePtr GetTarget() const noexcept {
	return target_.lock();
    }

    //! Sets the authored exit.
    //! \param definition the authored exit
    void SetDefinition(const ExitPtr& definition) noexcept {
	definition_ = definition;
    }

    //! Sets the secret exit bit.
    //! \param secretBit the secret exit bit
    //! \sa #GetSecretBit() const
    void SetSecretBit(const bool secretBit) noexcept;

    //! Sets the door state.
    //! \param doorState the door state
    //! \sa #GetDoorState() const
    void SetDoorState(Door::DoorEnum doorState) noexcept;

    //! Sets the live target instance.
    //! \param target the live target instance
    void SetTarget(const InstancePtr& target) noexcept {
	target_ = target;
    }

private:
    //! The authored exit.
    ExitPtr definition_;

    //! The door state.
    Door::DoorEnum doorState_;

    //! The secret exit bit.
    bool secretBit_;

    //! The live target instance.
    WeakInstancePtr target_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_ROOM_EXIT_SPECIALS_HPP_
