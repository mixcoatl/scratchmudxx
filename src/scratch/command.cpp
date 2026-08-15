//! \file command.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_COMMAND_CPP_

#include <scratch/command.hpp>
#include <scratch/data.hpp>
#include <scratch/scratch.hpp>
#include <scratch/social.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>

namespace Scratch {
namespace Core {

namespace {

//! Parses a comma-separated keyword list.
//! \param text the keyword list
//! \return the keyword set
StringSetCi ParseKeywords(const String& text) {
    StringSetCi keywords;
    std::size_t begin = 0;
    while (begin < text.size()) {
	while (begin < text.size() &&
		(text[begin] == ' ' || text[begin] == '\t' ||
		 text[begin] == ','))
	    ++begin;
	if (begin >= text.size())
	    break;
	std::size_t end = begin;
	while (end < text.size() && text[end] != ',')
	    ++end;
	std::size_t last = end;
	while (last > begin &&
		(text[last - 1] == ' ' || text[last - 1] == '\t'))
	    --last;
	if (last > begin)
	    keywords.insert(text.substr(begin, last - begin));
	begin = end;
    }
    return keywords;
}

//! Formats keywords as a comma-separated list.
//! \param keywords the keyword set
//! \return the formatted list
String FormatKeywords(const StringSetCi& keywords) {
    if (keywords.empty())
	return String();
    std::vector<String> parts;
    parts.reserve(keywords.size());
    for (const auto& keyword: keywords)
	parts.push_back(keyword);
    return Scratch::Algorithm::StringJoin(", ", parts);
}

} // namespace

//! Default constructor.
Command::Command() noexcept :
	Thing(),
	action_(),
	keywords_(),
	permissions_(),
	social_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa command to copy
Command::Command(const Command& other) noexcept :
	Thing(other),
	action_(other.action_),
	keywords_(other.keywords_),
	permissions_(other.permissions_),
	social_() {
    if (other.social_)
	social_ = std::make_shared<Social>(*other.social_);
}

//! Destructor.
Command::~Command() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the \sa command to assign
Command& Command::operator=(const Command& other) noexcept {
    Thing::operator=(other);
    action_ = other.action_;
    keywords_ = other.keywords_;
    permissions_ = other.permissions_;
    if (other.social_)
	social_ = std::make_shared<Social>(*other.social_);
    else
	social_.reset();
    return *this;
}

//! Returns whether \p performer may run this command.
//! \param performer the performing user, or null for open commands only
//! \sa #GetPermissions() const
//! \sa #HasPermission(const String&) const
bool Command::Allows(const UserPtr& performer) const noexcept {
    if (!performer)
	return permissions_.empty();
    if (permissions_.empty())
	return true;
    for (const auto& permission: permissions_) {
	if (performer->HasPermission(permission))
	    return true;
    }
    return false;
}

//! Reads this command from a data node.
//! \param data the data node to read
//! \sa #WriteData(const DataPtr&) const
void Command::ReadData(const DataPtr& data) noexcept {
    if (!data)
	return;

    name_ = data->GetString("Name");
    action_ = data->GetString("Action");
    keywords_ = ParseKeywords(data->GetString("Keywords"));

    // Read permissions.
    auto permissionsData = data->Get("Permissions");
    if (!permissionsData)
	permissionsData = std::make_shared<Data>();
    this->ReadPermissionsData(permissionsData);

    // Read social.
    auto socialData = data->Get("Social");
    if (socialData) {
	social_ = std::make_shared<Social>();
	social_->ReadData(socialData);
    } else {
	social_.reset();
    }

    // Read metadata.
    auto metadataData = data->Get("Metadata");
    if (!metadataData)
	metadataData = std::make_shared<Data>();
    this->ReadMetadataData(metadataData);
}

//! Reads permissions from a data node.
//! \param data the Permissions data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WritePermissionsData(const DataPtr&) const
void Command::ReadPermissionsData(const DataPtr& data) noexcept {
    permissions_.clear();
    for (const auto& entry: data->GetEntries()) {
	const auto permission = data->GetString(entry.first);
	if (!permission.empty())
	    permissions_.insert(permission);
    }
}

//! Writes this command to a data node.
//! \param data the data node to write
//! \sa #ReadData(const DataPtr&)
void Command::WriteData(const DataPtr& data) const noexcept {
    if (!data)
	return;

    if (!name_.empty())
	data->PutString("Name", name_);
    if (!action_.empty())
	data->PutString("Action", action_);
    const auto keywords = FormatKeywords(keywords_);
    if (!keywords.empty())
	data->PutString("Keywords", keywords);

    // Write permissions.
    auto permissionsData = std::make_shared<Data>();
    this->WritePermissionsData(permissionsData);
    if (permissionsData->Size())
	data->Put("Permissions", permissionsData);

    // Write social.
    if (social_) {
	auto socialData = std::make_shared<Data>();
	social_->WriteData(socialData);
	data->Put("Social", socialData);
    }

    // Write metadata.
    auto metadataData = std::make_shared<Data>();
    this->WriteMetadataData(metadataData);
    if (!metadataData->GetEntries().empty())
	data->Put("Metadata", metadataData);
}

//! Writes permissions to a data node.
//! \param data the Permissions data node to write
//! \sa #ReadPermissionsData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Command::WritePermissionsData(const DataPtr& data) const noexcept {
    for (const auto& permission: permissions_)
	data->PutString("%", permission);
}

}; // namespace Core
}; // namespace Scratch
