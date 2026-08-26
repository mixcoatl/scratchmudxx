//! \file protocol_telnet.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_PROTOCOL_TELNET_HPP_
#define _SCRATCH_PROTOCOL_TELNET_HPP_

#include <scratch/protocol.hpp>

namespace Scratch {
namespace Net {

//! Per-option RFC 1143 state. \{
struct TelnetOptionState {
    //! The RFC 1143 Q-method side state. \{
    enum TelnetOptionStateEnum: std::uint8_t {
	Q_NO		= 0,	//!< Option disabled.
	Q_YES		= 1,	//!< Option enabled.
	Q_WANTNO	= 2,	//!< Negotiating disable.
	Q_WANTYES	= 3	//!< Negotiating enable.
    };
    //! \}

    //! Option state on our side.
    TelnetOptionStateEnum us: 2;

    //! Whether the opposite of our current negotiation is queued.
    bool usOpposite: 1;

    //! Option state on his side.
    TelnetOptionStateEnum him: 2;

    //! Whether the opposite of his current negotiation is queued.
    bool himOpposite: 1;
};
//! \}

//! The TELNET protocol. \{
class TelnetProtocol: public Protocol {
public:
    //! Constructor.
    //! \param descriptor the descriptor
    explicit TelnetProtocol(Descriptor& descriptor) noexcept;

    //! Destructor.
    virtual ~TelnetProtocol() noexcept;

    //! Called after an application prompt is written.
    virtual void OnPrompt() override;

    //! Called when the descriptor begins asynchronous I/O.
    virtual void OnStart() override;

    //! Processes one byte of wire input.
    //! \param byteReceived the byte to process
    virtual void Receive(const std::uint8_t byteReceived) override;

    //! Sends application output toward the wire.
    //! \param message the message to send
    virtual void Send(const String& message) override;

    //! Enables or disables Quiet (server echo / hidden client local echo).
    //! \param quiet whether Quiet is enabled
    virtual void SetQuiet(bool quiet) override;

protected:
    //! The descriptor.
    Descriptor& descriptor_;

    //! RFC 1143 state for each TELNET option.
    TelnetOptionState optionState_[256];

    //! The TELNET protocol command.
    std::uint8_t telnetCommand_;

    //! The TELNET protocol option.
    std::uint8_t telnetOption_;

    //! The TELNET-SB input buffer.
    std::ostringstream telnetSb_;

    //! The TELNET-SB bit.
    bool telnetSbBit_;

    //! Whether the current TELNET-SB exceeded MaxString.
    bool telnetSbOverflowBit_;

    //! Clears TELNET-SB collection state.
    void ClearTelnetSb();

    //! Returns whether we support enabling an option on his side.
    //! \param option the TELNET option
    bool SupportsHim(const std::uint8_t option) const noexcept;

    //! Returns whether we support enabling an option on our side.
    //! \param option the TELNET option
    bool SupportsUs(const std::uint8_t option) const noexcept;

    //! Returns whether an option is enabled on his side.
    //! \param option the TELNET option
    bool Him(const std::uint8_t option) const noexcept;

    //! Returns whether an option is enabled on our side.
    //! \param option the TELNET option
    bool Us(const std::uint8_t option) const noexcept;

    //! Asks him to enable or disable an option.
    //! \param option the TELNET option
    //! \param enable whether to enable
    void WantHim(const std::uint8_t option, const bool enable);

    //! Asks to enable or disable an option on our side.
    //! \param option the TELNET option
    //! \param enable whether to enable
    void WantUs(const std::uint8_t option, const bool enable);

    //! Handles a received DO.
    //! \param option the TELNET option
    void RecvDo(const std::uint8_t option);

    //! Handles a received DONT.
    //! \param option the TELNET option
    void RecvDont(const std::uint8_t option);

    //! Handles a received WILL.
    //! \param option the TELNET option
    void RecvWill(const std::uint8_t option);

    //! Handles a received WONT.
    //! \param option the TELNET option
    void RecvWont(const std::uint8_t option);

    //! Writes a TELNET command.
    //! \param command the TELNET command to write
    //! \sa #PutCommand(const std::uint8_t, const std::uint8_t)
    void PutCommand(const std::uint8_t command) noexcept;

    //! Writes a TELNET command.
    //! \param command the TELNET command to write
    //! \param option the TELNET command option to write
    //! \sa #PutCommand(const std::uint8_t)
    void PutCommand(
	const std::uint8_t command,
	const std::uint8_t option) noexcept;

    //! Processes TELNET-IAC input.
    //! \param byteReceived the byte to process
    void ReceiveTelnetIac(const std::uint8_t byteReceived);

    //! Processes a completed option negotiation.
    //! \param command the negotiation command
    //! \param option the TELNET option
    void ReceiveTelnetNegotiate(
	const std::uint8_t command,
	const std::uint8_t option);

    //! Processes TELNET-SB input.
    //! \param option the TELNET option
    //! \param sbReceived the TELNET-SB payload
    void ReceiveTelnetSb(
	const std::uint8_t option,
	const String& sbReceived);

    //! Processes one byte of TELNET-SB input.
    //! \param sbByteReceived the TELNET-SB byte to process
    void ReceiveTelnetSbByte(const std::uint8_t sbByteReceived);

    //! Processes a NAWS subnegotiation.
    //! \param sbReceived the TELNET-SB payload
    void ReceiveNaws(const String& sbReceived);

    //! Called when an option becomes enabled on his side.
    //! \param option the TELNET option
    void OnHimEnabled(const std::uint8_t option);

    //! Requests his terminal type (RFC 1091).
    void RequestTtype();

    //! Processes a TTYPE subnegotiation.
    //! \param sbReceived the TELNET-SB payload
    void ReceiveTtype(const String& sbReceived);
};
//! \}

}; // namespace Net
}; // namespace Scratch

#endif // _SCRATCH_PROTOCOL_TELNET_HPP_
