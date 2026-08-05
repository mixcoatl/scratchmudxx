//! \file server.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
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
using Tcp = boost::asio::ip::tcp;

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
    Endpoint endpoint(Tcp::v4(), port);
    if (!address.empty()) {
	Resolver resolver(game_.GetIoContext());
	ResolverQuery query(address, std::to_string(port));
	for (auto it = resolver.resolve(query); it != ResolverIterator(); ++it) {
	    endpoint = it->endpoint();
	    if (endpoint.address().is_v4())
		break;
	}
    }
    this->StartAcceptor(endpoint);
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

    // Closing the acceptor cancels any pending async_accept.
    ErrorCode errorCode;
    acceptor_.close(errorCode);
    if (errorCode) {
	LOGGER_NETWORK() << "Error closing acceptor.";
	LOGGER_NETWORK() << " >> " << errorCode;
	LOGGER_NETWORK() << " >> " << errorCode.message();
    } else {
	LOGGER_NETWORK() << "Server stopped accepting connections.";
    }
}

//! Configures an asynchronous accept.
void Server::InitAsyncAccept() {
    acceptor_.async_accept([this](ErrorCode ec, Socket&& s) {
	if (ec || game_.GetShutdown()) {
	    if (ec && ec != boost::asio::error::operation_aborted) {
		LOGGER_NETWORK() << "Error accepting connection.";
		LOGGER_NETWORK() << " >> " << ec;
		LOGGER_NETWORK() << " >> " << ec.message();
	    }
	    return;
	}

	LOGGER_NETWORK() << "Received connection from " << s.remote_endpoint() << ".";
	game_.MakeDescriptor(std::move(s));
	this->InitAsyncAccept();
    });
}

}; // namespace net
}; // namespace scratch
