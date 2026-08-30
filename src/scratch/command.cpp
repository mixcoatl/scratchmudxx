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
#include <scratch/instance.hpp>
#include <scratch/scratch.hpp>
#include <scratch/social.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Strings = Scratch::Algorithm::Strings;

namespace {

//! Parses a comma-separated keyword list.
//! \param text the keyword list
//! \return the keyword set
StringSetCi ParseKeywords(const String& text) {
    StringSetCi keywords;
    for (const auto& part: Strings::Split(text, ",")) {
	const auto trimmed = boost::trim_copy(part);
	if (!trimmed.empty())
	    keywords.insert(trimmed);
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
    return boost::algorithm::join(parts, ", ");
}

} // namespace

//! Default constructor.
Command::Command() noexcept :
	Thing(),
	action_(),
	keywords_(),
	social_(),
	trust_(Trust::TRUST_NONE) {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa command to copy
Command::Command(const Command& other) noexcept :
	Thing(other),
	action_(other.action_),
	keywords_(other.keywords_),
	social_(),
	trust_(other.trust_) {
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
    trust_ = other.trust_;
    if (other.social_)
	social_ = std::make_shared<Social>(*other.social_);
    else
	social_.reset();
    return *this;
}

//! Returns whether \p performer may run this command.
//! \param performer the performing user, or null for open commands only
//! \sa #GetTrust() const
bool Command::Allows(const UserPtr& performer) const noexcept {
    if (!Trust::IsDefined(trust_))
	return true;
    if (!performer)
	return false;
    if (!Trust::IsDefined(performer->GetTrust()))
	return false;
    return Trust::Allows(performer->GetTrust(), trust_);
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

    const auto trustName = data->GetString("Trust");
    trust_ = trustName.empty() ?
	Trust::TRUST_NONE : Trust::ByName(trustName);

    auto socialData = data->Get("Social");
    if (socialData) {
	social_ = std::make_shared<Social>();
	social_->ReadData(socialData);
    } else {
	social_.reset();
    }

    auto metadataData = data->Get("Metadata", std::make_shared<Data>());
    this->ReadMetadataData(metadataData);
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

    if (Trust::IsDefined(trust_))
	data->PutString("Trust", Trust::ToString(trust_));

    if (social_) {
	auto socialData = std::make_shared<Data>();
	social_->WriteData(socialData);
	data->Put("Social", socialData);
    }

    auto metadataData = std::make_shared<Data>();
    this->WriteMetadataData(metadataData);
    if (!metadataData->GetEntries().empty())
	data->Put("Metadata", metadataData);
}

void Command::CreateSocialProxy(
	std::shared_ptr<Command> command) {
    if (!command->GetSocial())
	command->SetSocial(std::make_shared<Social>());
}

}; // namespace Core
}; // namespace Scratch
