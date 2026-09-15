//! \file sector.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#include <scratch/data.hpp>
#include <scratch/scratch.hpp>
#include <scratch/sector.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

Sector::Sector() noexcept :
	Thing(),
	indoorsBit_(false),
	movementCost_(1),
	movementType_(Movement::MOVEMENT_NONE),
	title_(),
	unbreathableBit_(false) {
    // Nothing.
}

Sector::Sector(const Sector& other) noexcept :
	Thing(other),
	indoorsBit_(other.indoorsBit_),
	movementCost_(other.movementCost_),
	movementType_(other.movementType_),
	title_(other.title_),
	unbreathableBit_(other.unbreathableBit_) {
    // Nothing.
}

Sector::~Sector() noexcept {
    // Nothing.
}

Sector& Sector::operator=(const Sector& other) noexcept {
    Thing::operator=(other);
    indoorsBit_ = other.indoorsBit_;
    movementCost_ = other.movementCost_;
    movementType_ = other.movementType_;
    title_ = other.title_;
    unbreathableBit_ = other.unbreathableBit_;
    return *this;
}

//! Reads this sector from a data node.
//! \param data the data node to read
//! \sa #WriteData(const DataPtr&) const
void Sector::ReadData(const DataPtr& data) noexcept {
    if (!data)
	return;

    auto movementData = data->Get("Movement", std::make_shared<Data>());
    this->ReadMovementData(movementData);
    this->ReadTitleData(data);

    auto metadataData = data->Get("Metadata", std::make_shared<Data>());
    this->ReadMetadataData(metadataData);

    auto sectorBitsData = data->Get("SectorBits", std::make_shared<Data>());
    this->ReadSectorBitsData(sectorBitsData);
}

//! Reads movement from a data node.
//! \param data the Movement data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteMovementData(const DataPtr&) const
void Sector::ReadMovementData(const DataPtr& data) noexcept {
    movementCost_ = static_cast<std::uint8_t>(data->GetNumber("Cost", 0));

    const auto movementTypeName = data->GetString("Type");
    movementType_ = movementTypeName.empty() ?
	Movement::MOVEMENT_NONE :
	Movement::ByName(movementTypeName);
    if (!Movement::IsDefined(movementType_))
	movementType_ = Movement::MOVEMENT_NONE;
}

//! Reads bit flags from a data node.
//! \param data the SectorBits data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteSectorBitsData(const DataPtr&) const
void Sector::ReadSectorBitsData(const DataPtr& data) noexcept {
    indoorsBit_ = data->GetYesNo("Indoors");
    unbreathableBit_ = data->GetYesNo("Unbreathable");
}

//! Reads the title from a data node.
//! \param data the data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteTitleData(const DataPtr&) const
void Sector::ReadTitleData(const DataPtr& data) noexcept {
    title_ = data->GetString("Title");
}

//! Sets the name.
//! \param name the name
//! \sa #GetName() const
void Sector::SetName(const String& name) {
    auto const oldDefaultTitle = boost::algorithm::to_lower_copy(this->GetName());
    Thing::SetName(name);
    if (title_ == oldDefaultTitle)
	title_ = boost::algorithm::to_lower_copy(this->GetName());
}

//! Writes this sector to a data node.
//! \param data the data node to write
//! \sa #ReadData(const DataPtr&)
void Sector::WriteData(const DataPtr& data) const noexcept {
    if (!data)
	return;

    this->WriteTitleData(data);

    auto movementData = std::make_shared<Data>();
    this->WriteMovementData(movementData);
    if (movementData->Size())
	data->Put("Movement", movementData);

    auto metadataData = std::make_shared<Data>();
    this->WriteMetadataData(metadataData);
    if (metadataData->Size())
	data->Put("Metadata", metadataData);

    auto sectorBitsData = std::make_shared<Data>();
    this->WriteSectorBitsData(sectorBitsData);
    if (sectorBitsData->Size())
	data->Put("SectorBits", sectorBitsData);
}

//! Writes movement to a data node.
//! \param data the Movement data node to write
//! \sa #ReadMovementData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Sector::WriteMovementData(const DataPtr& data) const noexcept {
    if (movementCost_)
	data->PutNumber("Cost", movementCost_);
    if (movementType_ != Movement::MOVEMENT_NONE) {
	auto const movementType = Movement::ToString(movementType_);
	data->PutString("Type", movementType);
    }
}

//! Writes bit flags to a data node.
//! \param data the SectorBits data node to write
//! \sa #ReadSectorBitsData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Sector::WriteSectorBitsData(const DataPtr& data) const noexcept {
    if (indoorsBit_)
	data->PutYesNo("Indoors", true);
    if (unbreathableBit_)
	data->PutYesNo("Unbreathable", true);
}

//! Writes the title to a data node.
//! \param data the data node to write
//! \sa #ReadTitleData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Sector::WriteTitleData(const DataPtr& data) const noexcept {
    auto const defaultTitle = boost::algorithm::to_lower_copy(this->GetName());
    if (title_ != defaultTitle)
	data->PutString("Title", title_);
}

}; // namespace Core
}; // namespace Scratch
