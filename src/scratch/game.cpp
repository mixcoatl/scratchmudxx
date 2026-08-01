//! \file game.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_GAME_CPP_

#include <scratch/config.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/game.hpp>
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>
#include <scratch/server.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
Game::Game() :
	ioContext_(),
	config_(std::make_shared<Config>()),
	descriptors_(),
	server_(),
	shutdown_(false),
	signals_(ioContext_) {
    // Nothing.
}

//! Destructor.
//! \sa #Shutdown()
Game::~Game() noexcept {
    this->Shutdown();
    if (server_)
	server_.reset();
}

//! Gets the host configuration.
ConfigPtr Game::GetConfig() const noexcept {
    return config_;
}

//! Searches for a descriptor.
//! \param descriptorName the descriptor name of the descriptor to return
//! \return the descriptor indicated by the specified descriptor name
DescriptorPtr Game::GetDescriptor(const String& descriptorName) noexcept {
    auto d = descriptors_.find(descriptorName);
    return d != std::end(descriptors_) ? d->second : nullptr;
}

//! Gets the descriptors.
std::set<DescriptorPtr> Game::GetDescriptors() const noexcept {
    std::set<DescriptorPtr> descriptorSet;
    for (auto& pair: descriptors_) {
	descriptorSet.insert(pair.second);
    }
    return descriptorSet;
}

//! Returns the IO context.
IoContext& Game::GetIoContext() noexcept {
    return ioContext_;
}

//! Gets the shutdown flag.
//! \sa #SetShutdown(const bool)
bool Game::GetShutdown() const noexcept {
    return shutdown_;
}

//! Constructs and returns a new descriptor.
//! \param socket the Boost socket
DescriptorPtr Game::MakeDescriptor(Socket&& socket) noexcept {
    // Create descriptor.
    auto d = std::make_shared<Descriptor>(*this, std::move(socket));

    // Create descriptor name.
    while (true) {
	d->SetName(Scratch::Algorithm::StringGenerateCopy());
	if (!this->GetDescriptor(d->GetName()))
	    break;
    }

    // Store descriptor into descriptor index.
    descriptors_[d->GetName()] = d;

    // Start descriptor I/O.
    d->Start();

    return d;
}

//! Closes a descriptor if needed and removes it from the index.
//! \param descriptorName the descriptor name to erase
//! \remark Safe to call for an already-closed or unknown name; idempotent.
void Game::EraseDescriptor(const String& descriptorName) noexcept {
    auto it = descriptors_.find(descriptorName);
    if (it == std::end(descriptors_))
	return;

    // Hold reference across close so cancelled completions stay valid
    // when this path closes socket itself (not via Descriptor::Close).
    DescriptorPtr d = it->second;
    descriptors_.erase(it);

    if (d->Closed())
	return;

    d->Close();
    // Close() posts another EraseDescriptor (no-op). Keep |d| alive
    // until after that post and any aborted handlers already queued.
    boost::asio::post(ioContext_, [d]() mutable {});
}

//! Parses command line arguments.
//! \param argc the number of command line arguments
//! \param argv an array containing the command line arguments
void Game::ParseArguments(
	const int argc,
	const char **argv) {
    // Nothing.
}

//! Runs the game.
void Game::Run() {
    if (!config_->Load())
	throw std::runtime_error("Couldn't load configuration.");

    // Configure acceptor.
    server_ = std::make_shared<Server>(*this);
    server_->StartAcceptor(config_->GetPort(), config_->GetAddress());

    // Wait for SIGINT / SIGTERM so we can shut down cleanly.
    this->InitSignals();

    // Now run event loop.
    LOGGER_MAIN() << "Starting game loop.";
    while (!shutdown_) {
	// The IO context stops when it runs out of
	// work or when it services one its handlers.
	// Restart the context just in case.
	ioContext_.restart();

	// Run IO context.
	if (!shutdown_)
	    ioContext_.run();
    }

    // Stop acceotr.
    if (server_) {
	server_->StopAcceptor();
	server_.reset();
    }

    // We seem to be done for now.
    LOGGER_MAIN() << "Game loop completed normally.";
}

//! Sets the shutdown flag.
//! \param shutdown the shutdown flag value
//! \sa #GetShutdown() const
//! \sa #Shutdown()
void Game::SetShutdown(const bool shutdown) noexcept {
    if (shutdown_ == shutdown)
	return;

    shutdown_ = shutdown;
    if (!shutdown) {
	ioContext_.restart();
    } else {
	this->Shutdown();
    }
}

//! Stops the acceptor, descriptors, and I/O context.
//! \sa #SetShutdown(const bool)
void Game::Shutdown() noexcept {
    // Acceptor.
    if (server_)
	server_->StopAcceptor();

    // Descriptors.
    for (auto d: this->GetDescriptors())
	d->Close();

    // Maps; Close() defers EraseDescriptor via post.
    descriptors_.clear();
    ioContext_.stop();
}

//! Begins waiting for process termination signals.
void Game::InitSignals() {
    signals_.add(SIGINT);
    signals_.add(SIGTERM);

    signals_.async_wait([this](const ErrorCode& errorCode, const int signum) {
	if (errorCode) {
	    if (errorCode != boost::asio::error::operation_aborted) {
		LOGGER_SYSTEM() << "Error waiting for signal.";
		LOGGER_SYSTEM() << " >> " << errorCode;
		LOGGER_SYSTEM() << " >> " << errorCode.message();
	    }
	    return;
	}

	LOGGER_MAIN() << "Received " << strsignal(signum) << " signal; shutting down.";
	this->SetShutdown(true);
    });
}

}; // namespace Core
}; // namespace Scratch
