//! \file config.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_CONFIG_CPP_

#include <scratch/config.hpp>
#include <scratch/data.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;

//! Fixed path of the host configuration Data file.
static const char configFileName[] = "data/config.dat";

//! Default constructor.
Config::Config() noexcept :
	address_(),
	metaColors_(),
	port_(6767) {
    // Nothing.
}

//! Destructor.
Config::~Config() noexcept {
    // Nothing.
}

//! Returns whether \p key matches \p allowed.
static bool KeyIs(
	const String& key,
	const char* allowed) noexcept {
    return !Scratch::Algorithm::Strings::CompareCi(key, allowed);
}

//! Loads configuration from the fixed Data file.
//! \return true if the file was loaded successfully
//! \sa #Save() const
bool Config::Load() noexcept {
    auto root = std::make_shared<Data>();
    if (!root->LoadFile(configFileName))
	return false;

    for (const auto& entry: root->GetEntries()) {
	if (!KeyIs(entry.first, "Colors") &&
		!KeyIs(entry.first, "Network"))
	    return false;
    }

    String address;
    auto port = port_;
    if (auto network = root->Get("Network")) {
	for (const auto& entry: network->GetEntries()) {
	    if (!KeyIs(entry.first, "Address") &&
		    !KeyIs(entry.first, "Port"))
		return false;
	}
	address = network->GetString("Address");
	if (network->Get("Port")) {
	    const auto value = network->GetNumber("Port");
	    if (value < 1.0 || value > 65535.0)
		return false;
	    port = static_cast<std::uint16_t>(value);
	}
    }

    std::map<Color::ColorEnum, Color::ColorEnum> metaColors;
    if (auto colors = root->Get("Colors")) {
	for (const auto& entry: colors->GetEntries()) {
	    const auto meta = Color::ByName(entry.first);
	    const auto color = Color::ByName(
		colors->GetString(entry.first));
	    if (!Color::IsMetaColor(meta) || Color::IsMetaColor(color))
		return false;
	    metaColors[meta] = color;
	}
    }

    address_ = std::move(address);
    metaColors_ = std::move(metaColors);
    port_ = port;
    return true;
}

//! Saves configuration to the fixed Data file.
//! \return true if the file was written successfully
//! \sa #Load()
bool Config::Save() const noexcept {
    auto root = std::make_shared<Data>();
    if (!metaColors_.empty()) {
	auto colors = root->Put("Colors");
	if (!colors)
	    return false;
	for (const auto& entry: metaColors_) {
	    colors->PutString(
		Color::ToString(entry.first),
		Color::ToString(entry.second));
	}
    }

    auto network = root->Put("Network");
    if (!network)
	return false;
    if (!address_.empty())
	network->PutString("Address", address_);
    network->PutNumber("Port", static_cast<double>(port_));

    return root->SaveFile(configFileName);
}

//! Sets a house metacolor.
//! \param meta the metacolor
//! \param color the real color
//! \return \c true if the metacolor was set
//! \sa #GetMetaColors() const
bool Config::SetMetaColor(
	Color::ColorEnum meta,
	Color::ColorEnum color) noexcept {
    if (!Color::IsMetaColor(meta) || Color::IsMetaColor(color))
	return false;
    metaColors_[meta] = color;
    return true;
}

//! Gets a configured metacolor.
//! \param config the configuration
//! \param name the metacolor name
//! \return the configured color, or \c C_UNDEFINED
Color::ColorEnum Config::GetMetaColorProxy(
	std::shared_ptr<Config> config,
	const String& name) noexcept {
    auto result = Color::C_UNDEFINED;
    if (config) {
	const auto meta = Color::ByName(name);
	if (Color::IsMetaColor(meta)) {
	    const auto found = config->metaColors_.find(meta);
	    if (found != config->metaColors_.end())
		result = found->second;
	}
    }
    return result;
}

}; // namespace Core
}; // namespace Scratch
