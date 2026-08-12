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
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Color = Scratch::Net::Color;

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

    //! Gets the house metacolor map.
    //! \sa #SetMetaColor(Color::ColorEnum, Color::ColorEnum)
    const std::map<Color::ColorEnum, Color::ColorEnum>& GetMetaColors() const noexcept {
	return metaColors_;
    }

    //! Gets the network listen port.
    //! \sa #SetPort(const std::uint16_t)
    std::uint16_t GetPort() const noexcept {
	return port_;
    }

    //! Loads configuration from the fixed Data file.
    //! \return true if the file was loaded successfully
    //! \sa #Save() const
    bool Load() noexcept;

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

protected:
    //! Network bind address.
    //! \sa #GetAddress() const
    String address_;

    //! Bootstrap connection state name.
    //! \sa #GetBootstrapState() const
    String bootstrapState_;

    //! House metacolor map.
    //! \sa #GetMetaColors() const
    std::map<Color::ColorEnum, Color::ColorEnum> metaColors_;

    //! Network listen port.
    //! \sa #GetPort() const
    std::uint16_t port_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_CONFIG_HPP_
