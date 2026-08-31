//! \file room.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#include <scratch/data.hpp>
#include <scratch/instance.hpp>
#include <scratch/room.hpp>
#include <scratch/room_exit_specials.hpp>
#include <scratch/room_specials.hpp>
#include <scratch/string.hpp>
#include <scratch/zone.hpp>
#include <scratch/world.hpp>

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Strings = Scratch::Algorithm::Strings;

//! Default constructor.
Room::Room() noexcept :
	Thing(),
	description_(),
	exits_(),
	immortalBit_(false),
	title_(),
	zone_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the room to copy
Room::Room(const Room& other) noexcept :
	Thing(other),
	description_(other.description_),
	exits_(),
	immortalBit_(other.immortalBit_),
	title_(other.title_),
	zone_() {
    for (const auto& pair: other.exits_) {
	if (pair.second)
	    exits_[pair.first] = std::make_shared<RoomExit>(*pair.second);
    }
    // Nothing.
}

//! Destructor.
Room::~Room() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the room to assign
Room& Room::operator=(const Room& other) noexcept {
    Thing::operator=(other);
    description_ = other.description_;
    exits_.clear();
    immortalBit_ = other.immortalBit_;
    for (const auto& pair: other.exits_) {
	if (pair.second)
	    exits_[pair.first] = std::make_shared<RoomExit>(*pair.second);
    }
    title_ = other.title_;
    zone_.reset();
    return *this;
}

//! Gets an exit.
//! \param direction the exit direction
//! \return the exit, or \c nullptr
ExitPtr Room::GetExit(const Direction::DirectionEnum direction) const noexcept {
    auto found = exits_.find(direction);
    return found != std::end(exits_) ? found->second : nullptr;
}

//! Gets the zone-qualified name.
String Room::GetQualifiedName() const noexcept {
    auto zone = zone_.lock();
    if (!zone || zone->GetName().empty())
	return this->GetName();
    return zone->GetName() + ":" + this->GetName();
}

//! Materializes this room into a world.
//! \param world the destination world
void Room::MaterializeInto(const WorldPtr& world) {
    if (!world)
	return;
    auto room = std::dynamic_pointer_cast<Room>(
	this->shared_from_this());
    if (!room)
	return;
    auto instance = std::make_shared<Instance>();
    instance->SetName(Strings::GenerateCopy());
    instance->SetZone(this->GetZone());
    auto specials = std::make_shared<RoomSpecials>(room);
    for (const auto& entry: room->GetExits()) {
	if (entry.second)
	    specials->SetExit(entry.first,
		std::make_shared<RoomExitSpecials>(entry.second));
    }
    instance->SetRoomSpecials(specials);
    world->AddInstance(instance);
}

//! Reads this room from a data node.
//! \param data the data node to read
//! \sa #WriteData(const DataPtr&) const
void Room::ReadData(const DataPtr& data) noexcept {
    if (!data)
	return;

    description_ = data->GetString("Description");
    title_ = data->GetString("Title");

    auto roomBitsData = data->Get("RoomBits", std::make_shared<Data>());
    this->ReadRoomBitsData(roomBitsData);

    auto metadataData = data->Get("Metadata", std::make_shared<Data>());
    this->ReadMetadataData(metadataData);

    auto exitsData = data->Get("Exits", std::make_shared<Data>());
    this->ReadExitsData(exitsData);
}

//! Reads bit flags from a data node.
//! \param data the RoomBits data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteRoomBitsData(const DataPtr&) const
void Room::ReadRoomBitsData(const DataPtr& data) noexcept {
    immortalBit_ = data->GetYesNo("Immortal");
}

//! Reads exits from a data node.
//! \param data the Exits data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteExitsData(const DataPtr&) const
void Room::ReadExitsData(const DataPtr& data) noexcept {
    exits_.clear();
    if (!data)
	return;
    for (const auto& entry: data->GetEntries()) {
	const auto direction = Direction::ByName(entry.first);
	if (!Direction::IsDefined(direction))
	    continue;
	auto exit = std::make_shared<RoomExit>();
	exit->SetDirection(direction);
	const auto target = data->GetString(entry.first);
	if (!target.empty())
	    exit->SetTarget(target);
	else if (entry.second)
	    exit->ReadData(entry.second);
	if (!exit->GetTarget().empty())
	    exits_[direction] = std::move(exit);
    }
}

//! Writes this room to a data node.
//! \param data the data node to write
//! \sa #ReadData(const DataPtr&)
void Room::WriteData(const DataPtr& data) const noexcept {
    if (!data)
	return;
    if (!description_.empty())
	data->PutString("Description", description_);
    if (!title_.empty())
	data->PutString("Title", title_);

    auto roomBitsData = std::make_shared<Data>();
    this->WriteRoomBitsData(roomBitsData);
    if (roomBitsData->Size())
	data->Put("RoomBits", roomBitsData);

    auto exitsData = std::make_shared<Data>();
    this->WriteExitsData(exitsData);
    if (exitsData->Size())
	data->Put("Exits", exitsData);

    auto metadataData = std::make_shared<Data>();
    this->WriteMetadataData(metadataData);
    if (metadataData->Size())
	data->Put("Metadata", metadataData);
}

//! Writes bit flags to a data node.
//! \param data the RoomBits data node to write
//! \sa #ReadRoomBitsData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Room::WriteRoomBitsData(const DataPtr& data) const noexcept {
    if (immortalBit_)
	data->PutYesNo("Immortal", true);
}

//! Writes exits to a data node.
//! \param data the Exits data node to write
//! \sa #ReadExitsData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Room::WriteExitsData(const DataPtr& data) const noexcept {
    if (!data)
	return;
    for (const auto& pair: exits_) {
	if (!pair.second || pair.second->GetTarget().empty())
	    continue;
	auto exitData = std::make_shared<Data>();
	pair.second->WriteData(exitData);
	const auto target = exitData->GetString("Target");
	if (exitData->Size() == 1 && !target.empty()) {
	    data->PutString(Direction::ToString(pair.first), target);
	} else if (exitData->Size()) {
	    data->Put(Direction::ToString(pair.first), exitData);
	}
    }
}

}; // namespace Core
}; // namespace Scratch
