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
    return !Scratch::Algorithm::StringCompareCi(key, allowed);
}

//! Loads configuration from the fixed Data file.
//! \return true if the file was loaded successfully
//! \sa #Save() const
bool Config::Load() noexcept {
    auto root = std::make_shared<Data>();
    if (!root->LoadFile(configFileName))
	return false;

    for (const auto& entry: root->GetEntries()) {
	if (!KeyIs(entry.first, "Network"))
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

    address_ = std::move(address);
    port_ = port;
    return true;
}

//! Saves configuration to the fixed Data file.
//! \return true if the file was written successfully
//! \sa #Load()
bool Config::Save() const noexcept {
    auto root = std::make_shared<Data>();
    auto network = root->Put("Network");
    if (!network)
	return false;
    if (!address_.empty())
	network->PutString("Address", address_);
    network->PutNumber("Port", static_cast<double>(port_));

    return root->SaveFile(configFileName);
}

}; // namespace Core
}; // namespace Scratch
