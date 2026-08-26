//! \file protocol.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_PROTOCOL_HPP_
#define _SCRATCH_PROTOCOL_HPP_

#include <scratch/scratch.hpp>

namespace Scratch {
namespace Net {

// Forward declarations.
class Descriptor;

//! The protocol interface. \{
class Protocol {
public:
    //! Destructor.
    virtual ~Protocol() noexcept {
	// Nothing.
    }

    //! Called after an application prompt is written.
    virtual void OnPrompt() {
	// Nothing.
    }

    //! Called when the descriptor begins asynchronous I/O.
    virtual void OnStart() {
	// Nothing.
    }

    //! Processes one byte of wire input.
    //! \param byteReceived the byte to process
    virtual void Receive(const std::uint8_t byteReceived) = 0;

    //! Sends application output toward the wire.
    //! \param message the message to send
    virtual void Send(const String& message) = 0;
};
//! \}

}; // namespace Net
}; // namespace Scratch

#endif // _SCRATCH_PROTOCOL_HPP_
