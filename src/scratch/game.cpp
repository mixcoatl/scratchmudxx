//! \file game.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2024 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_GAME_CPP_

#include <scratch/descriptor.hpp>
#include <scratch/game.hpp>
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>
#include <scratch/server.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
Game::Game() :
	mDescriptors(),
	mIoContext(),
	mServer(),
	mShutdown(false) {
    // Nothing.
}

//! Destructor.
Game::~Game() noexcept {
    if (mServer)
	mServer.reset();
}

//! Searches for a descriptor.
//! \param descriptorName the descriptor name of the descriptor to return
//! \return the descriptor indicated by the specified descriptor name
DescriptorPtr Game::GetDescriptor(const String& descriptorName) noexcept {
    auto d = mDescriptors.find(descriptorName);
    return d != std::end(mDescriptors) ? d->second : nullptr;
}

//! Gets the descriptors.
std::set<DescriptorPtr> Game::GetDescriptors() const noexcept {
    std::set<DescriptorPtr> descriptorSet;
    for (auto& pair: mDescriptors) {
	descriptorSet.insert(pair.second);
    }
    return descriptorSet;
}

//! Returns the IO context.
IoContext& Game::GetIoContext() noexcept {
    return mIoContext;
}

//! Gets the shutdown flag.
//! \sa #SetShutdown(const bool)
bool Game::GetShutdown() const noexcept {
    return mShutdown;
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
    return mDescriptors[d->GetName()] = d;
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
    // Configure acceptor.
    mServer = std::make_shared<Server>(*this);
    mServer->StartAcceptor(6767);

    // Now run event loop.
    LOGGER_MAIN() << "Starting game loop.";
    while (!mShutdown) {
	// The IO context stops when it runs out of
	// work or when it services one its handlers.
	// Restart the context just in case.
	mIoContext.restart();

	// Run IO context.
	if (!mShutdown)
	    mIoContext.run();
    }

    if (mServer) {
	mServer->StopAcceptor();
	mServer.reset();
    }

    // We seem to be done for now.
    LOGGER_MAIN() << "Game loop completed normally.";
}

//! Sets the shutdown flag.
//! \param shutdown the shutdown flag value
//! \sa #GetShutdown() const
void Game::SetShutdown(const bool shutdown) noexcept {
    if (mShutdown == shutdown)
	return;

    mShutdown = shutdown;
    if (!shutdown) {
	mIoContext.restart();
    } else {
	mIoContext.stop();
    }
}

}; // namespace Core
}; // namespace Scratch
