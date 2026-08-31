//! \file room_exit_specials.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ROOM_EXIT_SPECIALS_HPP_
    #define _SCRATCH_ROOM_EXIT_SPECIALS_HPP_

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

    //! Constructs room exit specials from an authored exit.
    //! \param definition the authored exit
    explicit RoomExitSpecials(const ExitPtr& definition) noexcept;

    //! Destructor.
    virtual ~RoomExitSpecials() noexcept;

    //! Gets the authored exit.
    ExitPtr GetDefinition() const noexcept {
	return definition_;
    }

    //! Gets the direction.
    Direction::DirectionEnum GetDirection() const noexcept;

    //! Gets the opposite exit specials.
    RoomExitSpecialsPtr GetOpposite() const noexcept;

    //! Gets the live target instance.
    InstancePtr GetTarget() const noexcept {
	return target_.lock();
    }

    //! Sets the authored exit.
    //! \param definition the authored exit
    void SetDefinition(const ExitPtr& definition) noexcept {
	definition_ = definition;
    }

    //! Sets the live target instance.
    //! \param target the live target instance
    void SetTarget(const InstancePtr& target) noexcept {
	target_ = target;
    }

private:
    //! The authored exit.
    ExitPtr definition_;

    //! The live target instance.
    WeakInstancePtr target_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_ROOM_EXIT_SPECIALS_HPP_
