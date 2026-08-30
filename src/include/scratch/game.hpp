//! \file game.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_GAME_HPP_
#define _SCRATCH_GAME_HPP_

#include <scratch/action.hpp>
#include <scratch/command.hpp>
#include <scratch/instance.hpp>
#include <scratch/player.hpp>
#include <scratch/repository.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>
#include <scratch/world.hpp>

// Forward declarations.
namespace Scratch {
namespace Net {
class Descriptor;
class Server;
}; // namespace Net
namespace Scripting {
class Lua;
}; // namespace Scripting
namespace Storage {
template<typename ThingT>
class FileStorage;
template<typename ThingT>
class MultiFileStorage;
}; // namespace Storage
}; // namespace Scratch

namespace Scratch {
namespace Core {

class Config;

// Boost types.
using ErrorCode = boost::system::error_code;
using IoContext = boost::asio::io_context;
using SignalSet = boost::asio::signal_set;
using Socket = boost::asio::ip::tcp::socket;

// ScratchMUD types.
using Color = Scratch::Net::Color;
using ConfigPtr = std::shared_ptr<Config>;
using CommandRepository = Scratch::Storage::Repository<
	Command, Scratch::Storage::MultiFileStorage<Command>>;
using CommandRepositoryPtr = std::shared_ptr<CommandRepository>;
using Descriptor = Scratch::Net::Descriptor;
using DescriptorPtr = std::shared_ptr<Descriptor>;
using Lua = Scratch::Scripting::Lua;
using LuaPtr = std::unique_ptr<Lua>;
using PlayerRepository = Scratch::Storage::Repository<
	Player, Scratch::Storage::MultiFileStorage<Player>>;
using PlayerRepositoryPtr = std::shared_ptr<PlayerRepository>;
using Server = Scratch::Net::Server;
using ServerPtr = std::shared_ptr<Server>;
using StateRepository = Scratch::Storage::Repository<
	State, Scratch::Storage::MultiFileStorage<State>>;
using StateRepositoryPtr = std::shared_ptr<StateRepository>;
using UserRepository = Scratch::Storage::Repository<
	User, Scratch::Storage::MultiFileStorage<User>>;
using UserRepositoryPtr = std::shared_ptr<UserRepository>;

//! The game class. \{
class Game {
public:
    //! Default constructor.
    Game();

    //! Destructor.
    //! \sa #Shutdown()
    ~Game() noexcept;

    //! Sends an action message.
    //! \param metacolor the message metacolor
    //! \param targets the audience bits
    //! \param message the message template
    //! \param subject the subject slot
    //! \param direct the direct slot
    //! \param indirect the indirect slot
    //! \param extra the extra slot
    void Action(
	const Color::ColorEnum metacolor,
	const unsigned targets,
	const String& message,
	const ActionParam& subject,
	const ActionParam& direct = ActionParam(),
	const ActionParam& indirect = ActionParam(),
	const ActionParam& extra = ActionParam());

    //! Applies Quiet and Prompt bits.
    //! \param state the connection state
    //! \sa #GetDescriptors() const
    //! \sa Descriptor::SetState(const StatePtr&)
    void ApplyStateBits(const StatePtr& state) noexcept;

    //! Dispatches a command line.
    //! \param performer the performing instance
    //! \param line the raw input line
    void DispatchCommand(
	const InstancePtr& performer,
	const String& line);

    //! Erases a descriptor.
    //! \param descriptorName the descriptor name to erase
    void EraseDescriptor(const String& descriptorName) noexcept;

    //! Finds a command.
    //! \param word the first input word
    //! \param performer the performing instance
    //! \return the matched command, or \c nullptr
    //! \sa Command::Allows(const InstancePtr&) const
    //! \sa #GetCommandsIndex() const
    CommandPtr FindCommand(
	const String& word,
	const InstancePtr& performer) const noexcept;

    //! Gets the command repository.
    CommandRepositoryPtr GetCommands() const noexcept;

    //! Gets the host configuration.
    ConfigPtr GetConfig() const noexcept;

    //! Gets the keyword command index.
    //! \sa #RebuildCommandIndex()
    const StringMapCi<CommandPtr>& GetCommandsIndex() const noexcept {
	return commandsIndex_;
    }

    //! Gets a descriptor.
    //! \param descriptorName the descriptor name
    //! \return the descriptor, or \c nullptr
    DescriptorPtr GetDescriptor(const String& descriptorName) noexcept;

    //! Gets the descriptors.
    std::set<DescriptorPtr> GetDescriptors() const noexcept;

    //! Gets an instance.
    //! \param instanceName the instance name
    //! \return the instance, or \c nullptr
    InstancePtr GetInstance(const String& instanceName) const noexcept;

    //! Gets the instance for \p player.
    //! \param player the player
    //! \return the instance, or \c nullptr
    InstancePtr GetInstanceFor(const PlayerPtr& player) noexcept;

    //! Gets the instances.
    InstancePtrSet GetInstances() const noexcept;

    //! Gets a world object.
    //! \param worldId the world object identity
    //! \return the world object, or \c nullptr
    WorldPtr GetWorld(const String& worldId) const noexcept;

    //! Gets the world objects.
    std::set<WorldPtr> GetWorlds() const noexcept;

    //! Gets the IO context.
    IoContext& GetIoContext() noexcept;

    //! Gets the Lua facade.
    Lua& GetLua() noexcept;

    //! Gets the player repository.
    PlayerRepositoryPtr GetPlayers() const noexcept;

    //! Gets the shutdown flag.
    //! \sa #SetShutdown(const bool)
    bool GetShutdown() const noexcept;

    //! Gets the connection-state repository.
    StateRepositoryPtr GetStates() const noexcept;

    //! Gets the user repository.
    UserRepositoryPtr GetUsers() const noexcept;

    //! Loads game repositories from disk.
    //! \throw std::runtime_error if a required repository cannot be loaded
    //! \sa #Run()
    void LoadRepositories();

    //! Constructs a descriptor.
    //! \param socket the Boost socket
    DescriptorPtr MakeDescriptor(Socket&& socket) noexcept;

    //! Parses command line arguments.
    //! \param argc the number of command line arguments
    //! \param argv an array containing the command line arguments
    void ParseArguments(
	const int argc,
	const char **argv);

    //! Rebuilds the keyword command index.
    //! \throw std::runtime_error on keyword conflicts
    //! \sa #GetCommandsIndex() const
    void RebuildCommandIndex();

    //! Runs the game.
    virtual void Run();

    //! Runs a command Action Lua hook with \c actor, \c command, \c line, and \c Q.
    //! \param command the command
    //! \param performer the performing instance
    //! \param line the remainder after the matched word
    void RunCommandHook(
	const CommandPtr& command,
	const InstancePtr& performer,
	const String& line);

    //! Runs social templates for \p actor.
    //! \param actor the performing instance
    //! \param social the social templates
    //! \param line the remainder after the matched command word
    //! \sa #RunCommandHook(const CommandPtr&, const InstancePtr&, const String&)
    void RunSocial(
	const InstancePtr& actor,
	const SocialPtr& social,
	const String& line);

    //! Sets the shutdown flag.
    //! \param shutdown the shutdown flag value
    //! \sa #GetShutdown() const
    //! \sa #Shutdown()
    void SetShutdown(const bool shutdown) noexcept;

protected:
    //! Expands and prints one action message.
    //! \param metacolor the message metacolor
    //! \param message the message template
    //! \param subject the subject slot
    //! \param direct the direct slot
    //! \param indirect the indirect slot
    //! \param extra the extra slot
    //! \param recipient the recipient instance
    //! \param to the recipient descriptor
    //! \sa #Action
    void ActionPerform(
	const Color::ColorEnum metacolor,
	const String& message,
	const ActionParam& subject,
	const ActionParam& direct,
	const ActionParam& indirect,
	const ActionParam& extra,
	const InstancePtr& recipient,
	Descriptor& to);

    //! Begins waiting for process termination signals.
    void InitSignals();

    //! Stops the acceptor, descriptors, and I/O context.
    //! \sa #SetShutdown(const bool)
    void Shutdown() noexcept;

    //! The IO context.
    //! \sa #GetIoContext() const
    //! \remark Must precede ASIO-dependent members (\ref descriptors_,
    //!     \ref server_, \ref signals_) so it outlives them on teardown.
    IoContext ioContext_;

    //! The command repository.
    //! \sa #GetCommands() const
    CommandRepositoryPtr commands_;

    //! The keyword command index.
    //! \sa #GetCommandsIndex() const
    //! \sa #RebuildCommandIndex()
    StringMapCi<CommandPtr> commandsIndex_;

    //! The host configuration.
    //! \sa #GetConfig() const
    ConfigPtr config_;

    //! The descriptors.
    //! \sa #GetDescriptors() const
    StringMapCi<DescriptorPtr> descriptors_;

    //! The world objects.
    //! \sa #GetWorlds() const
    StringMapCi<WorldPtr> worlds_;

    //! The Lua facade.
    //! \sa #GetLua()
    LuaPtr lua_;

    //! The player repository.
    //! \sa #GetPlayers() const
    PlayerRepositoryPtr players_;

    //! The server.
    ServerPtr server_;

    //! The shutdown flag.
    //! \sa #GetShutdown() const
    //! \sa #SetShutdown(const bool)
    bool shutdown_;

    //! The process signal set.
    //! \sa #InitSignals()
    SignalSet signals_;

    //! The connection-state repository.
    //! \sa #GetStates() const
    StateRepositoryPtr states_;

    //! The user repository.
    //! \sa #GetUsers() const
    UserRepositoryPtr users_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_GAME_HPP_
