//! \file game.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_GAME_HPP_
#define _SCRATCH_GAME_HPP_

#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

// Forward declarations.
namespace Scratch {
namespace Net {
class Descriptor;
class Server;
}; // namespace Net
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
using Server = Scratch::Net::Server;
using ServerPtr = std::shared_ptr<Server>;

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

    //! Gets the shutdown flag.
    //! \sa #SetShutdown(const bool)
    bool GetShutdown() const noexcept;

    //! Constructs and returns a new descriptor.
    //! \param socket the Boost socket
    DescriptorPtr MakeDescriptor(Socket&& socket) noexcept;

    //! Closes a descriptor if needed and removes it from the index.
    //! \param descriptorName the descriptor name to erase
    //! \remark Safe to call for an already-closed or unknown name; idempotent.
    void EraseDescriptor(const String& descriptorName) noexcept;

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

    //! The server.
    ServerPtr server_;

    //! The shutdown flag.
    //! \sa #GetShutdown() const
    //! \sa #SetShutdown(const bool)
    bool shutdown_;

    //! The process signal set.
    //! \sa #InitSignals()
    SignalSet signals_;

    //! Begins waiting for process termination signals.
    void InitSignals();

    //! Stops the acceptor, descriptors, and I/O context.
    //! \sa #SetShutdown(const bool)
    void Shutdown() noexcept;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_GAME_HXX_
