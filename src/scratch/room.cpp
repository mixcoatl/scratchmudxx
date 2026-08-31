//! \file room.cpp
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

Room::Room() noexcept :
	Thing(),
	description_(),
	exits_(),
	title_(),
	zone_() {
    // Nothing.
}

Room::Room(const Room& other) noexcept :
	Thing(other),
	description_(other.description_),
	exits_(),
	title_(other.title_),
	zone_() {
    for (const auto& pair: other.exits_) {
	if (pair.second)
	    exits_[pair.first] = std::make_shared<RoomExit>(*pair.second);
    }
    // Nothing.
}

Room::~Room() noexcept {
    // Nothing.
}

Room& Room::operator=(const Room& other) noexcept {
    Thing::operator=(other);
    description_ = other.description_;
    exits_.clear();
    for (const auto& pair: other.exits_) {
	if (pair.second)
	    exits_[pair.first] = std::make_shared<RoomExit>(*pair.second);
    }
    title_ = other.title_;
    zone_.reset();
    return *this;
}

ExitPtr Room::GetExit(const Direction::DirectionEnum direction) const noexcept {
    auto found = exits_.find(direction);
    return found != std::end(exits_) ? found->second : nullptr;
}

String Room::GetQualifiedName() const noexcept {
    auto zone = zone_.lock();
    if (!zone || zone->GetName().empty())
	return this->GetName();
    return zone->GetName() + ":" + this->GetName();
}

void Room::ReadData(const DataPtr& data) noexcept {
    if (!data)
	return;

    description_ = data->GetString("Description");
    title_ = data->GetString("Title");

    auto metadataData = data->Get("Metadata", std::make_shared<Data>());
    this->ReadMetadataData(metadataData);

    auto exitsData = data->Get("Exits", std::make_shared<Data>());
    this->ReadExitsData(exitsData);
}

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

void Room::WriteData(const DataPtr& data) const noexcept {
    if (!data)
	return;
    if (!description_.empty())
	data->PutString("Description", description_);
    if (!title_.empty())
	data->PutString("Title", title_);

    auto exitsData = std::make_shared<Data>();
    this->WriteExitsData(exitsData);
    if (exitsData->Size())
	data->Put("Exits", exitsData);

    auto metadataData = std::make_shared<Data>();
    this->WriteMetadataData(metadataData);
    if (metadataData->Size())
	data->Put("Metadata", metadataData);
}

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
