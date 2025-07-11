//! \file game.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
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

// Boost types.
using ErrorCode = boost::system::error_code;
using IoContext = boost::asio::io_context;
using Socket = boost::asio::ip::tcp::socket;

// ScratchMUD types.
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
    ~Game() noexcept;

    //! Searches for a descriptor.
    //! \param descriptorName the descriptor name of the descriptor to return
    //! \return the descriptor indicated by the specified descriptor name
    DescriptorPtr GetDescriptor(const String& descriptorName) noexcept;

    //! Gets the descriptors.
    std::set<DescriptorPtr> GetDescriptors() const noexcept;

    //! Gets the IO context.
    IoContext& GetIoContext() noexcept;

    //! Gets the server.
    Server& GetServer() noexcept;

    //! Gets the shutdown flag.
    //! \sa #SetShutdown(const bool)
    bool GetShutdown() const noexcept;

    //! Constructs and returns a new descriptor.
    //! \param socket the Boost socket
    DescriptorPtr MakeDescriptor(Socket&& socket) noexcept;

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
    void SetShutdown(const bool shutdown) noexcept;

protected:
    //! The descriptors.
    //! \sa #GetDescriptors() const
    StringMapCi<DescriptorPtr> descriptors_;

    //! The IO context.
    //! \sa #GetIoContext() const
    IoContext ioContext_;

    //! The server.
    //! \sa #GetServer()
    ServerPtr server_;

    //! The shutdown flag.
    //! \sa #GetShutdown() const
    //! \sa #SetShutdown(const bool)
    bool shutdown_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_GAME_HXX_
