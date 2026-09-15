//! \file sector.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_SECTOR_HPP_
#define _SCRATCH_SECTOR_HPP_

#include <scratch/movement.hpp>
#include <scratch/repository.hpp>
#include <scratch/thing.hpp>

namespace Scratch {
namespace Core {

// Forward declarations.
class Sector;

// ScratchMUD types.
using SectorPtr = std::shared_ptr<Sector>;

//! The sector class. \{
class Sector : public Thing {
public:
    //! Default constructor.
    Sector() noexcept;

    //! Copy constructor.
    //! \param other the sector to copy
    Sector(const Sector& other) noexcept;

    //! Destructor.
    virtual ~Sector() noexcept;

    //! Default assignment.
    //! \param other the sector to assign
    Sector& operator=(const Sector& other) noexcept;

    //! Gets the indoors bit.
    //! \sa #SetIndoorsBit(const bool)
    bool GetIndoorsBit() const noexcept {
	return indoorsBit_;
    }

    //! Gets the movement cost.
    //! \sa #SetMovementCost(const std::uint8_t)
    std::uint8_t GetMovementCost() const noexcept {
	return movementCost_;
    }

    //! Gets the movement type.
    //! \sa #SetMovementType(Movement::MovementEnum)
    Movement::MovementEnum GetMovementType() const noexcept {
	return movementType_;
    }

    //! Gets the title.
    //! \sa #SetTitle(const String&)
    String GetTitle() const noexcept {
	return title_;
    }

    //! Gets the unbreathable bit.
    //! \sa #SetUnbreathableBit(const bool)
    bool GetUnbreathableBit() const noexcept {
	return unbreathableBit_;
    }

    //! Reads this sector from a data node.
    //! \param data the data node to read
    //! \sa #WriteData(const DataPtr&) const
    void ReadData(const DataPtr& data) noexcept;

    //! Reads movement from a data node.
    //! \param data the Movement data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteMovementData(const DataPtr&) const
    void ReadMovementData(const DataPtr& data) noexcept;

    //! Reads bit flags from a data node.
    //! \param data the SectorBits data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteSectorBitsData(const DataPtr&) const
    void ReadSectorBitsData(const DataPtr& data) noexcept;

    //! Reads the title from a data node.
    //! \param data the data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteTitleData(const DataPtr&) const
    void ReadTitleData(const DataPtr& data) noexcept;

    //! Sets the indoors bit.
    //! \param indoorsBit the indoors bit
    //! \sa #GetIndoorsBit() const
    void SetIndoorsBit(const bool indoorsBit) noexcept {
	indoorsBit_ = indoorsBit;
    }

    //! Sets the movement cost.
    //! \param movementCost the movement cost
    //! \sa #GetMovementCost() const
    void SetMovementCost(const std::uint8_t movementCost) noexcept {
	movementCost_ = movementCost;
    }

    //! Sets the movement type.
    //! \param movementType the movement type
    //! \sa #GetMovementType() const
    void SetMovementType(Movement::MovementEnum movementType) noexcept {
	movementType_ = movementType;
    }

    //! Sets the name.
    //! \param name the name
    //! \sa #GetName() const
    void SetName(const String& name);

    //! Sets the title.
    //! \param title the title
    //! \sa #GetTitle() const
    void SetTitle(const String& title) noexcept {
	title_ = title;
    }

    //! Sets the unbreathable bit.
    //! \param unbreathableBit the unbreathable bit
    //! \sa #GetUnbreathableBit() const
    void SetUnbreathableBit(const bool unbreathableBit) noexcept {
	unbreathableBit_ = unbreathableBit;
    }

    //! Writes this sector to a data node.
    //! \param data the data node to write
    //! \sa #ReadData(const DataPtr&)
    void WriteData(const DataPtr& data) const noexcept;

    //! Writes movement to a data node.
    //! \param data the Movement data node to write
    //! \sa #ReadMovementData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteMovementData(const DataPtr& data) const noexcept;

    //! Writes bit flags to a data node.
    //! \param data the SectorBits data node to write
    //! \sa #ReadSectorBitsData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteSectorBitsData(const DataPtr& data) const noexcept;

    //! Writes the title to a data node.
    //! \param data the data node to write
    //! \sa #ReadTitleData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteTitleData(const DataPtr& data) const noexcept;

private:
    bool indoorsBit_;
    std::uint8_t movementCost_;
    Movement::MovementEnum movementType_;
    String title_;
    bool unbreathableBit_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_SECTOR_HPP_
