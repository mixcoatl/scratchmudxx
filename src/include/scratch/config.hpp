//! \file config.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_CONFIG_HPP_
#define _SCRATCH_CONFIG_HPP_

#include <scratch/color.hpp>
#include <scratch/scheduler.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

// Forward declarations.
namespace Scratch {
namespace Scripting {
class LuaBindings;
}; // namespace Scripting
namespace Utility {
class Data;
}; // namespace Utility
}; // namespace Scratch

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Color = Scratch::Net::Color;
using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;

//! The Config class. \{
//! \remark Host settings document. Not a Repository.
class Config {
public:
    //! Default constructor.
    Config() noexcept;

    //! Destructor.
    virtual ~Config() noexcept;

    //! Gets the network bind address.
    //! \sa #SetAddress(const String&)
    String GetAddress() const noexcept {
	return address_;
    }

    //! Gets the bootstrap connection state name.
    //! \sa #SetBootstrapState(const String&)
    String GetBootstrapState() const noexcept {
	return bootstrapState_;
    }

    //! Gets an entry room by kind.
    //! \param kind the entry room kind
    //! \sa #GetEntryRooms() const
    String GetEntryRoom(const String& kind) const noexcept;

    //! Gets the entry room map.
    //! \sa #GetEntryRoom(const String&) const
    StringMapCi<String> GetEntryRooms() const noexcept {
	return entryRooms_;
    }

    //! Gets the house metacolor map.
    //! \sa #SetMetaColor(Color::ColorEnum, Color::ColorEnum)
    std::map<Color::ColorEnum, Color::ColorEnum> GetMetaColors() const noexcept {
	return metaColors_;
    }

    //! Gets the network listen port.
    //! \sa #SetPort(const std::uint16_t)
    std::uint16_t GetPort() const noexcept {
	return port_;
    }

    //! Gets the instanced-world grace period.
    //! \sa #ReadWorldData(const DataPtr&)
    Scheduler::Task::Duration GetWorldGracePeriod() const noexcept {
	return worldGracePeriod_;
    }

    //! Loads configuration from the fixed Data file.
    //! \return true if the file was loaded successfully
    //! \sa #Save() const
    bool Load() noexcept;

    //! Reads default colors from a data node.
    //! \param data the Colors data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteColorsData(const DataPtr&) const
    void ReadColorsData(const DataPtr& data) noexcept;

    //! Reads configuration from a data node.
    //! \param data the data node to read
    //! \sa #WriteData(const DataPtr&) const
    void ReadData(const DataPtr& data) noexcept;

    //! Reads entry rooms from a data node.
    //! \param data the Rooms data node to read
    //! \sa #ReadGameData(const DataPtr&)
    //! \sa #WriteEntryRoomsData(const DataPtr&) const
    void ReadEntryRoomsData(const DataPtr& data) noexcept;

    //! Reads game settings from a data node.
    //! \param data the Game data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteGameData(const DataPtr&) const
    void ReadGameData(const DataPtr& data) noexcept;

    //! Reads network settings from a data node.
    //! \param data the Network data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteNetworkData(const DataPtr&) const
    void ReadNetworkData(const DataPtr& data) noexcept;

    //! Reads world settings from a data node.
    //! \param data the World data node to read
    //! \sa #ReadGameData(const DataPtr&)
    //! \sa #WriteWorldData(const DataPtr&) const
    void ReadWorldData(const DataPtr& data) noexcept;

    //! Saves configuration to the fixed Data file.
    //! \return true if the file was written successfully
    //! \sa #Load()
    bool Save() const noexcept;

    //! Sets the network bind address.
    //! \sa #GetAddress() const
    void SetAddress(const String& address) {
	address_ = address;
    }

    //! Sets the bootstrap connection state name.
    //! \sa #GetBootstrapState() const
    void SetBootstrapState(const String& bootstrapState) {
	bootstrapState_ = bootstrapState;
    }

    //! Sets a house metacolor.
    //! \param meta the metacolor
    //! \param color the real color
    //! \return \c true if the metacolor was set
    //! \sa #GetMetaColors() const
    bool SetMetaColor(
	Color::ColorEnum meta,
	Color::ColorEnum color) noexcept;

    //! Sets the network listen port.
    //! \sa #GetPort() const
    void SetPort(const std::uint16_t port) {
	port_ = port;
    }

    //! Writes default colors to a data node.
    //! \param data the Colors data node to write
    //! \sa #ReadColorsData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteColorsData(const DataPtr& data) const noexcept;

    //! Writes configuration to a data node.
    //! \param data the data node to write
    //! \sa #ReadData(const DataPtr&)
    void WriteData(const DataPtr& data) const noexcept;

    //! Writes entry rooms to a data node.
    //! \param data the Rooms data node to write
    //! \sa #ReadEntryRoomsData(const DataPtr&)
    //! \sa #WriteGameData(const DataPtr&) const
    void WriteEntryRoomsData(const DataPtr& data) const noexcept;

    //! Writes game settings to a data node.
    //! \param data the Game data node to write
    //! \sa #ReadGameData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteGameData(const DataPtr& data) const noexcept;

    //! Writes network settings to a data node.
    //! \param data the Network data node to write
    //! \sa #ReadNetworkData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteNetworkData(const DataPtr& data) const noexcept;

    //! Writes world settings to a data node.
    //! \param data the World data node to write
    //! \sa #ReadWorldData(const DataPtr&)
    //! \sa #WriteGameData(const DataPtr&) const
    void WriteWorldData(const DataPtr& data) const noexcept;

protected:
    friend class Scratch::Scripting::LuaBindings;

    //! Gets a configured metacolor.
    //! \param config the configuration
    //! \param name the metacolor name
    //! \return the configured color, or \c C_UNDEFINED
    static Color::ColorEnum GetMetaColorProxy(
	std::shared_ptr<Config> config,
	const String& name) noexcept;

    //! Network bind address.
    //! \sa #GetAddress() const
    String address_;

    //! Bootstrap connection state name.
    //! \sa #GetBootstrapState() const
    String bootstrapState_;

    //! Entry room map.
    //! \sa #GetEntryRooms() const
    StringMapCi<String> entryRooms_;

    //! House metacolor map.
    //! \sa #GetMetaColors() const
    std::map<Color::ColorEnum, Color::ColorEnum> metaColors_;

    //! Network listen port.
    //! \sa #GetPort() const
    std::uint16_t port_;

    //! Instanced-world grace period.
    //! \sa #GetWorldGracePeriod() const
    Scheduler::Task::Duration worldGracePeriod_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_CONFIG_HPP_
