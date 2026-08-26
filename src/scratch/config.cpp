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
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Fixed path of the host configuration Data file.
static const char configFileName[] = "data/config.dat";

//! Default constructor.
Config::Config() noexcept :
	address_(),
	bootstrapState_("Login"),
	metaColors_(),
	port_(6767) {
    // Nothing.
}

//! Destructor.
Config::~Config() noexcept {
    // Nothing.
}

//! Loads configuration from the fixed Data file.
//! \return true if the file was loaded successfully
//! \sa #Save() const
bool Config::Load() noexcept {
    auto root = std::make_shared<Data>();
    if (!root->LoadFile(configFileName))
	return false;

    this->ReadData(root);

    if (bootstrapState_.empty())
	return false;

    return true;
}

//! Reads configuration from a data node.
//! \param data the data node to read
//! \sa #WriteData(const DataPtr&) const
void Config::ReadData(const DataPtr& data) noexcept {
    address_.clear();
    bootstrapState_ = "Login";
    metaColors_.clear();
    port_ = 6767;

    // Read game settings.
    auto gameData = data->Get("Game", std::make_shared<Data>());
    this->ReadGameData(gameData);

    // Read network settings.
    auto networkData = data->Get("Network", std::make_shared<Data>());
    this->ReadNetworkData(networkData);

    // Read default colors.
    auto colorsData = data->Get("Colors", std::make_shared<Data>());
    this->ReadColorsData(colorsData);
}

//! Reads default colors from a data node.
//! \param data the Colors data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteColorsData(const DataPtr&) const
void Config::ReadColorsData(const DataPtr& data) noexcept {
    metaColors_.clear();
    for (const auto& entry: data->GetStringMap("")) {
	const auto meta = Color::ByName(entry.first);
	const auto assigned = Color::ByName(entry.second);
	if (!Color::IsMetaColor(meta) || Color::IsMetaColor(assigned)) {
	    LOGGER_STORAGE() << "Unknown color " << entry.first << ": " << entry.second << ".";
	    continue;
	}
	metaColors_[meta] = assigned;
    }
}

//! Reads game settings from a data node.
//! \param data the Game data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteGameData(const DataPtr&) const
void Config::ReadGameData(const DataPtr& data) noexcept {
    const auto bootstrapState = data->GetString("BootstrapState");
    if (!bootstrapState.empty())
	bootstrapState_ = bootstrapState;
}

//! Reads network settings from a data node.
//! \param data the Network data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteNetworkData(const DataPtr&) const
void Config::ReadNetworkData(const DataPtr& data) noexcept {
    address_ = data->GetString("Address");
    if (data->Get("Port")) {
	const auto value = data->GetNumber("Port");
	if (value < 1.0 || value > 65535.0)
	    LOGGER_STORAGE() << "Invalid network port " << value << ".";
	else
	    port_ = static_cast<std::uint16_t>(value);
    }
}

//! Saves configuration to the fixed Data file.
//! \return true if the file was written successfully
//! \sa #Load()
bool Config::Save() const noexcept {
    auto root = std::make_shared<Data>();
    this->WriteData(root);
    return root->SaveFile(configFileName);
}

//! Writes configuration to a data node.
//! \param data the data node to write
//! \sa #ReadData(const DataPtr&)
void Config::WriteData(const DataPtr& data) const noexcept {
    // Write game settings.
    auto gameData = std::make_shared<Data>();
    this->WriteGameData(gameData);
    if (gameData->Size())
	data->Put("Game", gameData);

    // Write network settings.
    auto networkData = std::make_shared<Data>();
    this->WriteNetworkData(networkData);
    if (networkData->Size())
	data->Put("Network", networkData);

    // Write default colors.
    auto colorsData = std::make_shared<Data>();
    this->WriteColorsData(colorsData);
    if (colorsData->Size())
	data->Put("Colors", colorsData);
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

//! Writes default colors to a data node.
//! \param data the Colors data node to write
//! \sa #ReadColorsData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Config::WriteColorsData(const DataPtr& data) const noexcept {
    StringMapCi<String> map;
    for (const auto& entry: metaColors_) {
	map[Color::ToString(entry.first)] =
	    Color::ToString(entry.second);
    }
    data->PutStringMap("", map);
}

//! Writes game settings to a data node.
//! \param data the Game data node to write
//! \sa #ReadGameData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Config::WriteGameData(const DataPtr& data) const noexcept {
    if (bootstrapState_.size())
	data->PutString("BootstrapState", bootstrapState_);
}

//! Writes network settings to a data node.
//! \param data the Network data node to write
//! \sa #ReadNetworkData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Config::WriteNetworkData(const DataPtr& data) const noexcept {
    if (!address_.empty())
	data->PutString("Address", address_);
    if (port_)
	data->PutNumber("Port", static_cast<double>(port_));
}

}; // namespace Core
}; // namespace Scratch
