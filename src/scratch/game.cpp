//! \file game.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_GAME_CPP_

#include <scratch/command.hpp>
#include <scratch/config.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/game.hpp>
#include <scratch/instance.hpp>
#include <scratch/logger.hpp>
#include <scratch/lua.hpp>
#include <scratch/player.hpp>
#include <scratch/scratch.hpp>
#include <scratch/server.hpp>
#include <scratch/state.hpp>
#include <scratch/storage_file.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
Game::Game() :
	ioContext_(),
	commands_(std::make_shared<CommandRepository>(
		Scratch::Storage::MultiFileStorage<Command>(
			"data", "command", ".dat"))),
	commandsIndex_(),
	config_(std::make_shared<Config>()),
	descriptors_(),
	worlds_(),
	lua_(std::make_unique<Lua>(*this)),
	players_(std::make_shared<PlayerRepository>(
		Scratch::Storage::MultiFileStorage<Player>(
			"data", "player", ".dat"))),
	server_(),
	shutdown_(false),
	signals_(ioContext_),
	states_(std::make_shared<StateRepository>(
		Scratch::Storage::MultiFileStorage<State>(
			"data", "state", ".dat"))),
	users_(std::make_shared<UserRepository>(
		Scratch::Storage::MultiFileStorage<User>(
			"data", "user", ".dat"))),
	zones_(std::make_shared<ZoneRepository>(
		Scratch::Storage::MultiFileStorage<Zone>(
			"data", "zone", ".dat"))) {
    worlds_[String()] = std::make_shared<World>();

    commands_->SetChangeHook([this](const String&) {
	this->RebuildCommandIndex();
    });
    commands_->SetReloadHook([this]() {
	this->RebuildCommandIndex();
    });
    states_->SetChangeHook([this](const String& name) {
	this->ApplyStateBits(states_->Get(name));
    });
    states_->SetReloadHook([this]() {
	for (const auto& name: states_->GetIds())
	    this->ApplyStateBits(states_->Get(name));
    });
}


//! Destructor.
//! \sa #Shutdown()
Game::~Game() noexcept {
    this->Shutdown();
    if (server_)
	server_.reset();
    // Lua before repositories.
    lua_.reset();
}

//! Gets the command repository.
CommandRepositoryPtr Game::GetCommands() const noexcept {
    return commands_;
}

//! Gets the host configuration.
ConfigPtr Game::GetConfig() const noexcept {
    return config_;
}

//! Gets a descriptor.
//! \param descriptorName the descriptor name
//! \return the descriptor, or \c nullptr
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

//! Gets the names of open descriptors.
StringSetCi Game::GetDescriptorNames() const {
    StringSetCi names;
    for (auto& descriptor: this->GetDescriptors()) {
	if (descriptor && !descriptor->Closed())
	    names.insert(descriptor->GetName());
    }
    return names;
}

//! Gets the connection-state repository.
StateRepositoryPtr Game::GetStates() const noexcept {
    return states_;
}

//! Gets the user repository.
UserRepositoryPtr Game::GetUsers() const noexcept {
    return users_;
}

//! Gets the zone repository.
ZoneRepositoryPtr Game::GetZones() const noexcept {
    return zones_;
}

//! Gets a room.
//! \param name the qualified or local room name
//! \param perspective the instance supplying local scope
//! \return the room, or \c nullptr
RoomPtr Game::GetRoom(
	const String& name,
	const InstancePtr& perspective) const noexcept {
    String roomName = name;
    String zoneName;
    const auto colon = name.find(':');
    if (colon != String::npos) {
	zoneName = name.substr(0, colon);
	roomName = name.substr(colon + 1);
    } else if (perspective) {
	// Perspective zone resolution deferred to room runtime.
    }
    if (zoneName.empty() || roomName.empty())
	return nullptr;
    auto zone = zones_->Get(zoneName);
    if (zone)
	return zone->GetRoom(roomName);
    return nullptr;
}

//! Gets the player repository.
PlayerRepositoryPtr Game::GetPlayers() const noexcept {
    return players_;
}

//! Gets an instance.
//! \param instanceName the instance name
//! \return the instance, or \c nullptr
InstancePtr Game::GetInstance(const String& instanceName) const noexcept {
    for (auto& pair: worlds_) {
	if (!pair.second)
	    continue;
	auto instance = pair.second->GetInstance(instanceName);
	if (instance)
	    return instance;
    }
    return nullptr;
}

//! Gets the instance for \p player.
//! \param player the player
//! \return the instance, or \c nullptr
InstancePtr Game::GetInstanceFor(const PlayerPtr& player) noexcept {
    if (!player)
	return nullptr;
    for (auto& instance: this->GetInstances()) {
	if (instance && instance->GetPlayer() == player)
	    return instance;
    }
    return nullptr;
}

//! Gets the instances.
InstancePtrSet Game::GetInstances() const noexcept {
    InstancePtrSet instanceSet;
    for (auto& world: this->GetWorlds()) {
	if (!world)
	    continue;
	for (auto& instance: world->GetInstances())
	    instanceSet.insert(instance);
    }
    return instanceSet;
}

//! Gets a world object.
//! \param worldId the world object identity
//! \return the world object, or \c nullptr
WorldPtr Game::GetWorld(const String& worldId) const noexcept {
    auto it = worlds_.find(worldId);
    return it != std::end(worlds_) ? it->second : nullptr;
}

//! Gets the world objects.
std::set<WorldPtr> Game::GetWorlds() const noexcept {
    std::set<WorldPtr> worldSet;
    for (auto& pair: worlds_)
	worldSet.insert(pair.second);
    return worldSet;
}

//! Applies Quiet and Prompt bits.
//! \param state the connection state
//! \sa #GetDescriptors() const
//! \sa Descriptor::SetState(const StatePtr&)
void Game::ApplyStateBits(const StatePtr& state) noexcept {
    if (!state)
	return;

    const bool quiet = state->GetQuietBit();
    const bool prompt = state->GetPromptBit();
    for (auto& d: this->GetDescriptors()) {
	if (!d || d->Closed() || d->GetState() != state)
	    continue;
	d->SetQuiet(quiet);
	if (prompt)
	    d->SetPromptBit(true);
    }
}

//! Gets the IO context.
IoContext& Game::GetIoContext() noexcept {
    return ioContext_;
}

//! Gets the Lua facade.
Lua& Game::GetLua() noexcept {
    return *lua_;
}

//! Gets the shutdown flag.
//! \sa #SetShutdown(const bool)
bool Game::GetShutdown() const noexcept {
    return shutdown_;
}

//! Constructs a descriptor.
//! \param socket the Boost socket
DescriptorPtr Game::MakeDescriptor(Socket&& socket) noexcept {
    // Create descriptor.
    auto d = std::make_shared<Descriptor>(*this, std::move(socket));

    // Create descriptor name.
    while (true) {
	d->SetName(Scratch::Algorithm::Strings::GenerateCopy());
	if (!this->GetDescriptor(d->GetName()))
	    break;
    }

    // Store descriptor into descriptor index.
    descriptors_[d->GetName()] = d;

    // Start descriptor I/O.
    d->Start();

    return d;
}

//! Erases a descriptor.
//! \param descriptorName the descriptor name to erase
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

//! Loads game repositories from disk.
//! \throw std::runtime_error if a required repository cannot be loaded
//! \sa #Run()
void Game::LoadRepositories() {
    if (!config_ || !config_->Load()) {
	throw std::runtime_error("Couldn't load configuration.");
    }
    if (!states_->LoadIndex()) {
	throw std::runtime_error("Couldn't load state index.");
    } else if (!states_->Get(config_->GetBootstrapState())) {
	throw std::runtime_error("Couldn't resolve bootstrap state.");
    }
    if (!commands_->LoadIndex()) {
	throw std::runtime_error("Couldn't load command index.");
    }
    if (!users_->LoadIndex()) {
	throw std::runtime_error("Couldn't load user index.");
    }
    if (!players_->LoadIndex()) {
	throw std::runtime_error("Couldn't load player index.");
    }
    if (!zones_->LoadIndex()) {
	throw std::runtime_error("Couldn't load zone index.");
    }
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
    this->LoadRepositories();

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

    // Stop acceptor.
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
    worlds_.clear();
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
