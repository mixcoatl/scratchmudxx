//! \file game.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_GAME_HPP_
#define _SCRATCH_GAME_HPP_

#include <scratch/repository.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>

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
using ConfigPtr = std::shared_ptr<Config>;
using Descriptor = Scratch::Net::Descriptor;
using DescriptorPtr = std::shared_ptr<Descriptor>;
using Lua = Scratch::Scripting::Lua;
using LuaPtr = std::unique_ptr<Lua>;
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

    //! Gets the host configuration.
    ConfigPtr GetConfig() const noexcept;

    //! Searches for a descriptor.
    //! \param descriptorName the descriptor name of the descriptor to return
    //! \return the descriptor indicated by the specified descriptor name
    DescriptorPtr GetDescriptor(const String& descriptorName) noexcept;

    //! Gets the descriptors.
    std::set<DescriptorPtr> GetDescriptors() const noexcept;

    //! Gets the IO context.
    IoContext& GetIoContext() noexcept;

    //! Gets the Lua state.
    Lua& GetLua() noexcept;

    //! Gets the shutdown flag.
    //! \sa #SetShutdown(const bool)
    bool GetShutdown() const noexcept;

    //! Gets the connection-state repository.
    StateRepositoryPtr GetStates() const noexcept;

    //! Gets the user repository.
    UserRepositoryPtr GetUsers() const noexcept;

    //! Applies Quiet and Prompt bits to descriptors in \p state.
    //! \param state the connection state
    //! \sa #GetDescriptors() const
    //! \sa Descriptor::SetState(const StatePtr&)
    void ApplyStateBits(const StatePtr& state) noexcept;

    //! Constructs and returns a new descriptor.
    //! \param socket the Boost socket
    DescriptorPtr MakeDescriptor(Socket&& socket) noexcept;

    //! Closes a descriptor if needed and removes it from the index.
    //! \param descriptorName the descriptor name to erase
    //! \remark Safe to call for an already-closed or unknown name; idempotent.
    void EraseDescriptor(const String& descriptorName) noexcept;

    //! Loads game repositories from disk.
    //! \throw std::runtime_error if a required repository cannot be loaded
    //! \sa #GetStates() const
    //! \sa #Run()
    void LoadRepositories();

    //! Parses command line arguments.
    //! \param argc the number of command line arguments
    //! \param argv an array containing the command line arguments
    void ParseArguments(
	const int argc,
	const char **argv);

    //! Runs the game.
    virtual void Run();

    //! Sets the shutdown flag.
    //! \param shutdown the shutdown flag value
    //! \sa #GetShutdown() const
    //! \sa #Shutdown()
    void SetShutdown(const bool shutdown) noexcept;

protected:
    //! The IO context.
    //! \sa #GetIoContext() const
    //! \remark Must precede ASIO-dependent members (\ref descriptors_,
    //!     \ref server_, \ref signals_) so it outlives them on teardown.
    IoContext ioContext_;

    //! Host configuration.
    ConfigPtr config_;

    //! The descriptors.
    //! \sa #GetDescriptors() const
    StringMapCi<DescriptorPtr> descriptors_;

    //! The Lua state.
    //! \sa #GetLua()
    LuaPtr lua_;

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

    //! Begins waiting for process termination signals.
    void InitSignals();

    //! Stops the acceptor, descriptors, and I/O context.
    //! \sa #SetShutdown(const bool)
    void Shutdown() noexcept;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_GAME_HPP_
