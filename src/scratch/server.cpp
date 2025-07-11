//! \file server.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_SERVER_CPP_

#include <scratch/game.hpp>
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>
#include <scratch/server.hpp>

namespace Scratch {
namespace Net {

// Boost stypes.
using ErrorCode = boost::system::error_code;
using Resolver = boost::asio::ip::tcp::resolver;
using ResolverIterator = boost::asio::ip::tcp::resolver::iterator;
using ResolverQuery = boost::asio::ip::tcp::resolver::query;
using Socket = boost::asio::ip::tcp::socket;

//! Constructor.
//! \param game the game state
Server::Server(Game& game) noexcept :
	game_(game),
	acceptor_(game.GetIoContext()) {
    // Nothing.
}

//! Destructor.
Server::~Server() noexcept {
    this->StopAcceptor();
}

//! Starts the acceptor and begins to accept connections.
//! \param port the network port upon which to listen
//! \param address the network address to bind
//! \sa #StartAcceptor(const Endpoint&)
//! \sa #StopAcceptor()
void Server::StartAcceptor(
	const std::uint16_t port,
	const String& address) {
    // Resolve the TCP endpoint.
    // These operations throw if name resolution fails.
    Resolver resolver(game_.GetIoContext());
    ResolverQuery query(address, std::to_string(port));
    ResolverIterator iter = resolver.resolve(query);

    // Start the acceptor.
    this->StartAcceptor(iter->endpoint());
}

//! Starts the acceptor and begins to accept connections.
//! \param endpoint the network endpoint
//! \sa #StartAcceptor(const std::uint16_t, const String&)
//! \sa #StopAcceptor()
void Server::StartAcceptor(const Endpoint& endpoint) {
    // Configure acceptor.
    acceptor_.open(endpoint.protocol());
    acceptor_.bind(endpoint);
    acceptor_.listen();

    LOGGER_NETWORK() << "Server listening on " << acceptor_.local_endpoint() << ".";

    // Accept connections.
    this->InitAsyncAccept();
}

//! Stops the acceptor.
//! \sa #StartAcceptor(const std::uint16_t, const String&)
//! \sa #StartAcceptor(const Endpoint&)
void Server::StopAcceptor() {
    if (!acceptor_.is_open())
	return;

    // Close acceptor.
    boost::asio::post(game_.GetIoContext(), [&]() {
	if (acceptor_.is_open())
	    acceptor_.close();
    });
}

//! Configures an asynchronous accept.
void Server::InitAsyncAccept() {
    acceptor_.async_accept([this](ErrorCode ec, Socket&& s) {
	LOGGER_NETWORK() << "Received connection from " << s.remote_endpoint() << ".";
	game_.MakeDescriptor(std::move(s));
	this->InitAsyncAccept();
    });
}

}; // namespace net
}; // namespace scratch
