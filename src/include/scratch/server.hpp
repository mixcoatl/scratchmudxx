//! \file server.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_SERVER_HPP_
#define _SCRATCH_SERVER_HPP_

#include <scratch/scratch.hpp>

// Forward declarations.
namespace Scratch {
namespace Core {
class Game;
}; // namespace Core
}; // namespace Scratch

namespace Scratch {
namespace Net {

// Boost types.
using Acceptor = boost::asio::ip::tcp::acceptor;
using Endpoint = boost::asio::ip::tcp::endpoint;
using Socket = boost::asio::ip::tcp::socket;

// ScratchMUD types.
using Game = Scratch::Core::Game;

//! The server class. \{
class Server {
public:
    //! Constructor.
    //! \param game the game state
    Server(Game& game) noexcept;

    //! Destructor.
    ~Server() noexcept;

    //! Starts the acceptor and begins to accept connections.
    //! \param port the network port upon which to listen
    //! \param address the network address to bind
    //! \sa #StartAcceptor(const Endpoint&)
    //! \sa #StopAcceptor()
    void StartAcceptor(
	const std::uint16_t port,
	const String& address = String());

    //! Starts the acceptor and begins to accept connections.
    //! \param endpoint the network endpoint
    //! \sa #StartAcceptor(const std::uint16_t, const String&)
    //! \sa #StopAcceptor()
    void StartAcceptor(const Endpoint& endpoint);

    //! Stops the acceptor.
    //! \sa #StartAcceptor(const std::uint16_t, const String&)
    //! \sa #StartAcceptor(const Endpoint&)
    void StopAcceptor();

protected:
    //! The game state.
    Game& game_;

    //! The acceptor.
    Acceptor acceptor_;

    //! Configures an asynchronous accept.
    void InitAsyncAccept();
};
//! \}

}; // namespace Net
}; // namespace Scratch

#endif // _SCRATCH_SERVER_HXX_
