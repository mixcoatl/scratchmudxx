//! \file zone.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#include <scratch/data.hpp>
#include <scratch/room.hpp>
#include <scratch/string.hpp>
#include <scratch/zone.hpp>

namespace Scratch {
namespace Core {

using Strings = Scratch::Algorithm::Strings;

Zone::Zone() noexcept :
	Thing(),
	rooms_() {
    // Nothing.
}

Zone::Zone(const Zone& other) noexcept :
	Thing(other),
	rooms_() {
    for (const auto& pair: other.rooms_) {
	if (pair.second)
	    rooms_[pair.first] = std::make_shared<Room>(*pair.second);
    }
}

Zone::~Zone() noexcept {
    // Nothing.
}

Zone& Zone::operator=(const Zone& other) noexcept {
    Thing::operator=(other);
    rooms_.clear();
    for (const auto& pair: other.rooms_) {
	if (pair.second)
	    rooms_[pair.first] = std::make_shared<Room>(*pair.second);
    }
    return *this;
}

//! Creates a room with a generated name.
//! \return the created room
RoomPtr Zone::CreateRoom() noexcept {
    auto roomName = String();
    while (roomName.empty() || this->GetRoom(roomName))
	roomName = Strings::GenerateCopy();
    this->StoreRoom(roomName, std::make_shared<Room>());
    return this->GetRoom(roomName);
}

bool Zone::EraseRoom(const String& roomName) noexcept {
    return rooms_.erase(roomName) != 0;
}

RoomPtr Zone::GetRoom(const String& roomName) const noexcept {
    auto found = rooms_.find(roomName);
    return found != std::end(rooms_) ? found->second : nullptr;
}

StringSetCi Zone::GetRoomNames() const noexcept {
    StringSetCi names;
    for (const auto& pair: rooms_) {
	if (pair.second)
	    names.insert(pair.first);
    }
    return names;
}

void Zone::ReadData(const DataPtr& data) noexcept {
    if (!data)
	return;
    name_ = data->GetString("Name");

    auto roomsData = data->Get("Rooms", std::make_shared<Data>());
    this->ReadRoomsData(roomsData);

    auto metadataData = data->Get("Metadata", std::make_shared<Data>());
    this->ReadMetadataData(metadataData);
}

void Zone::ReadRoomsData(const DataPtr& data) noexcept {
    rooms_.clear();
    if (!data)
	return;
    for (const auto& entry: data->GetEntries()) {
	if (!entry.second)
	    continue;
	auto room = std::make_shared<Room>();
	room->SetName(entry.first);
	room->ReadData(entry.second);
	this->StoreRoom(entry.first, room);
    }
}

bool Zone::StoreRoom(
	const String& roomName,
	const RoomPtr& room) noexcept {
    if (roomName.empty() || !room ||
	Strings::CompareCi(Strings::SanitizeCopy(roomName), roomName))
	return false;
    room->SetName(roomName);
    room->SetZone(std::dynamic_pointer_cast<Zone>(
	    this->shared_from_this()));
    rooms_[roomName] = room;
    return true;
}

void Zone::WriteData(const DataPtr& data) const noexcept {
    if (!data)
	return;
    if (!name_.empty())
	data->PutString("Name", name_);

    auto roomsData = std::make_shared<Data>();
    this->WriteRoomsData(roomsData);
    if (roomsData->Size())
	data->Put("Rooms", roomsData);

    auto metadataData = std::make_shared<Data>();
    this->WriteMetadataData(metadataData);
    if (metadataData->Size())
	data->Put("Metadata", metadataData);
}

void Zone::WriteRoomsData(const DataPtr& data) const noexcept {
    if (!data)
	return;
    for (const auto& pair: rooms_) {
	if (!pair.second)
	    continue;
	auto roomData = std::make_shared<Data>();
	pair.second->WriteData(roomData);
	if (roomData->Size())
	    data->Put(pair.first, roomData);
    }
}

}; // namespace Core
}; // namespace Scratch
