//! \file protocol_telnet.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_PROTOCOL_TELNET_CPP_

#define TELCMDS
#define TELOPTS

#include <scratch/descriptor.hpp>
#include <scratch/logger.hpp>
#include <scratch/protocol_telnet.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Net {

//! Returns a printable TELNET command name.
//! \param cmd the TELNET command byte
static String GetTelcmdName(const std::uint8_t cmd) {
    if (TELCMD_OK(cmd))
	return TELCMD(cmd);
    return std::to_string(static_cast<unsigned>(cmd));
}

//! Returns a printable TELNET option name.
//! \param opt the TELNET option byte
static String GetTeloptName(const std::uint8_t opt) {
    if (TELOPT_OK(opt))
	return TELOPT(opt);
    return std::to_string(static_cast<unsigned>(opt));
}

//! Constructor.
//! \param descriptor the descriptor
TelnetProtocol::TelnetProtocol(Descriptor& descriptor) noexcept :
	descriptor_(descriptor),
	optionState_(),
	telnetCommand_(/* None */ 0),
	telnetOption_(/* None */ 0),
	telnetSb_(),
	telnetSbBit_(false),
	telnetSbOverflowBit_(false) {
    // Nothing.
}

//! Destructor.
TelnetProtocol::~TelnetProtocol() noexcept {
    // Nothing.
}

//! Called after an application prompt is written.
void TelnetProtocol::OnPrompt() {
    // Without Suppress-Go-Ahead, signal end-of-output with GA.
    if (!this->Us(TELOPT_SGA))
	this->PutCommand(GA);
}

//! Called when the descriptor begins asynchronous I/O.
void TelnetProtocol::OnStart() {
    // Offer SGA on both sides and NAWS/TTYPE from him. Announce WONT ECHO so
    // clients keep local echo; WantUs(ECHO) later for password hiding.
    // NAWS is RFC 1073 (window size); TTYPE is RFC 1091; SGA is orthogonal to app prompts.
    this->PutCommand(WONT, TELOPT_ECHO);
    this->WantUs(TELOPT_SGA, true);
    this->WantHim(TELOPT_SGA, true);
    this->WantHim(TELOPT_NAWS, true);
    this->WantHim(TELOPT_TTYPE, true);
}

//! Processes one byte of wire input.
//! \param byteReceived the byte to process
void TelnetProtocol::Receive(const std::uint8_t byteReceived) {
    switch (telnetCommand_) {
    case /* None */ 0:
	if (byteReceived == IAC) {
	    telnetCommand_ = IAC;
	} else if (telnetSbBit_) {
	    this->ReceiveTelnetSbByte(byteReceived);
	} else {
	    descriptor_.DeliverByte(byteReceived);
	}
	break;
    case IAC:
	this->ReceiveTelnetIac(byteReceived);
	break;
    case DO:   case DONT:
    case WILL: case WONT:
	this->ReceiveTelnetNegotiate(telnetCommand_, byteReceived);
	telnetCommand_ = /* None */ 0;
	telnetOption_  = /* None */ 0;
	break;
    case SB:
	telnetOption_ = byteReceived;
	telnetSb_.str(String());
	telnetSb_.clear();
	telnetSbBit_ = true;
	telnetSbOverflowBit_ = false;
	telnetCommand_ = /* None */ 0;
	break;
    default:
	LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " has unknown TELNET state IAC " << GetTelcmdName(telnetCommand_) << ".";
	telnetCommand_ = /* None */ 0;
	telnetOption_  = /* None */ 0;
	break;
    }
}

//! Sends application output toward the wire.
//! \param message the message to send
void TelnetProtocol::Send(const String& message) {
    String escaped;
    escaped.reserve(message.size());
    for (const unsigned char ch: message) {
	escaped.push_back(static_cast<char>(ch));
	if (ch == IAC)
	    escaped.push_back(static_cast<char>(IAC));
    }
    descriptor_.WriteRaw(escaped);
}

//! Returns whether we support enabling an option on his side.
//! \param option the TELNET option
bool TelnetProtocol::SupportsHim(const std::uint8_t option) const noexcept {
    switch (option) {
    case TELOPT_NAWS:
    case TELOPT_SGA:
    case TELOPT_TTYPE:
	return true;
    default:
	return false;
    }
}

//! Returns whether we support enabling an option on our side.
//! \param option the TELNET option
//! \remark ECHO is enabled only via WantUs (password hiding), not by
//!         accepting an unsolicited DO ECHO.
bool TelnetProtocol::SupportsUs(const std::uint8_t option) const noexcept {
    switch (option) {
    case TELOPT_SGA:
	return true;
    default:
	return false;
    }
}

//! Returns whether an option is enabled on his side.
//! \param option the TELNET option
bool TelnetProtocol::Him(const std::uint8_t option) const noexcept {
    return optionState_[option].him == TelnetOptionState::Q_YES;
}

//! Returns whether an option is enabled on our side.
//! \param option the TELNET option
bool TelnetProtocol::Us(const std::uint8_t option) const noexcept {
    return optionState_[option].us == TelnetOptionState::Q_YES;
}

//! Asks him to enable or disable an option.
//! \param option the TELNET option
//! \param enable whether to enable
void TelnetProtocol::WantHim(const std::uint8_t option, const bool enable) {
    auto& state = optionState_[option];
    if (enable) {
	switch (state.him) {
	case TelnetOptionState::Q_NO:
	    state.him = TelnetOptionState::Q_WANTYES;
	    this->PutCommand(DO, option);
	    break;
	case TelnetOptionState::Q_YES:
	    // Nothing.
	    break;
	case TelnetOptionState::Q_WANTNO:
	    if (!state.himOpposite)
		state.himOpposite = true;
	    break;
	case TelnetOptionState::Q_WANTYES:
	    if (state.himOpposite)
		state.himOpposite = false;
	    break;
	default:
	    // Nothing.
	    break;
	}
    } else {
	switch (state.him) {
	case TelnetOptionState::Q_NO:
	    // Nothing.
	    break;
	case TelnetOptionState::Q_YES:
	    state.him = TelnetOptionState::Q_WANTNO;
	    this->PutCommand(DONT, option);
	    break;
	case TelnetOptionState::Q_WANTNO:
	    if (state.himOpposite)
		state.himOpposite = false;
	    break;
	case TelnetOptionState::Q_WANTYES:
	    if (!state.himOpposite)
		state.himOpposite = true;
	    break;
	default:
	    // Nothing.
	    break;
	}
    }
}

//! Asks to enable or disable an option on our side.
//! \param option the TELNET option
//! \param enable whether to enable
void TelnetProtocol::WantUs(const std::uint8_t option, const bool enable) {
    auto& state = optionState_[option];
    if (enable) {
	switch (state.us) {
	case TelnetOptionState::Q_NO:
	    state.us = TelnetOptionState::Q_WANTYES;
	    this->PutCommand(WILL, option);
	    break;
	case TelnetOptionState::Q_YES:
	    // Nothing.
	    break;
	case TelnetOptionState::Q_WANTNO:
	    if (!state.usOpposite)
		state.usOpposite = true;
	    break;
	case TelnetOptionState::Q_WANTYES:
	    if (state.usOpposite)
		state.usOpposite = false;
	    break;
	default:
	    // Nothing.
	    break;
	}
    } else {
	switch (state.us) {
	case TelnetOptionState::Q_NO:
	    // Nothing.
	    break;
	case TelnetOptionState::Q_YES:
	    state.us = TelnetOptionState::Q_WANTNO;
	    this->PutCommand(WONT, option);
	    break;
	case TelnetOptionState::Q_WANTNO:
	    if (state.usOpposite)
		state.usOpposite = false;
	    break;
	case TelnetOptionState::Q_WANTYES:
	    if (!state.usOpposite)
		state.usOpposite = true;
	    break;
	default:
	    // Nothing.
	    break;
	}
    }
}

//! Handles a received DO.
//! \param option the TELNET option
void TelnetProtocol::RecvDo(const std::uint8_t option) {
    auto& state = optionState_[option];
    switch (state.us) {
    case TelnetOptionState::Q_NO:
	if (this->SupportsUs(option)) {
	    state.us = TelnetOptionState::Q_YES;
	    this->PutCommand(WILL, option);
	} else {
	    this->PutCommand(WONT, option);
	}
	break;
    case TelnetOptionState::Q_YES:
	// Nothing.
	break;
    case TelnetOptionState::Q_WANTNO:
	LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " got DO answering WONT for " << GetTeloptName(option) << ".";
	if (!state.usOpposite) {
	    state.us = TelnetOptionState::Q_NO;
	} else {
	    state.us = TelnetOptionState::Q_YES;
	    state.usOpposite = false;
	}
	break;
    case TelnetOptionState::Q_WANTYES:
	if (!state.usOpposite) {
	    state.us = TelnetOptionState::Q_YES;
	} else {
	    state.us = TelnetOptionState::Q_WANTNO;
	    state.usOpposite = false;
	    this->PutCommand(WONT, option);
	}
	break;
    default:
	// Nothing.
	break;
    }
}

//! Handles a received DONT.
//! \param option the TELNET option
void TelnetProtocol::RecvDont(const std::uint8_t option) {
    auto& state = optionState_[option];
    switch (state.us) {
    case TelnetOptionState::Q_NO:
	// Nothing.
	break;
    case TelnetOptionState::Q_YES:
	state.us = TelnetOptionState::Q_NO;
	this->PutCommand(WONT, option);
	break;
    case TelnetOptionState::Q_WANTNO:
	if (!state.usOpposite) {
	    state.us = TelnetOptionState::Q_NO;
	} else {
	    state.us = TelnetOptionState::Q_WANTYES;
	    state.usOpposite = false;
	    this->PutCommand(WILL, option);
	}
	break;
    case TelnetOptionState::Q_WANTYES:
	state.us = TelnetOptionState::Q_NO;
	state.usOpposite = false;
	break;
    default:
	// Nothing.
	break;
    }
}

//! Handles a received WILL.
//! \param option the TELNET option
void TelnetProtocol::RecvWill(const std::uint8_t option) {
    auto& state = optionState_[option];
    switch (state.him) {
    case TelnetOptionState::Q_NO:
	if (this->SupportsHim(option)) {
	    state.him = TelnetOptionState::Q_YES;
	    this->PutCommand(DO, option);
	    this->OnHimEnabled(option);
	} else {
	    this->PutCommand(DONT, option);
	}
	break;
    case TelnetOptionState::Q_YES:
	// Nothing.
	break;
    case TelnetOptionState::Q_WANTNO:
	LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " got WILL answering DONT for " << GetTeloptName(option) << ".";
	if (!state.himOpposite) {
	    state.him = TelnetOptionState::Q_NO;
	} else {
	    state.him = TelnetOptionState::Q_YES;
	    state.himOpposite = false;
	    this->OnHimEnabled(option);
	}
	break;
    case TelnetOptionState::Q_WANTYES:
	if (!state.himOpposite) {
	    state.him = TelnetOptionState::Q_YES;
	    this->OnHimEnabled(option);
	} else {
	    state.him = TelnetOptionState::Q_WANTNO;
	    state.himOpposite = false;
	    this->PutCommand(DONT, option);
	}
	break;
    default:
	// Nothing.
	break;
    }
}

//! Handles a received WONT.
//! \param option the TELNET option
void TelnetProtocol::RecvWont(const std::uint8_t option) {
    auto& state = optionState_[option];
    switch (state.him) {
    case TelnetOptionState::Q_NO:
	// Nothing.
	break;
    case TelnetOptionState::Q_YES:
	state.him = TelnetOptionState::Q_NO;
	this->PutCommand(DONT, option);
	break;
    case TelnetOptionState::Q_WANTNO:
	if (!state.himOpposite) {
	    state.him = TelnetOptionState::Q_NO;
	} else {
	    state.him = TelnetOptionState::Q_WANTYES;
	    state.himOpposite = false;
	    this->PutCommand(DO, option);
	}
	break;
    case TelnetOptionState::Q_WANTYES:
	state.him = TelnetOptionState::Q_NO;
	state.himOpposite = false;
	break;
    default:
	// Nothing.
	break;
    }
}

//! Writes a TELNET command.
//! \param command the TELNET command to write
//! \sa #PutCommand(const std::uint8_t, const std::uint8_t)
void TelnetProtocol::PutCommand(const std::uint8_t command) noexcept {
    const char bytes[] = {
	static_cast<char>(IAC),
	static_cast<char>(command)
    };
    descriptor_.WriteRaw(String(bytes, sizeof(bytes)));
}

//! Writes a TELNET command.
//! \param command the TELNET command to write
//! \param option the TELNET command option to write
//! \sa #PutCommand(const std::uint8_t)
void TelnetProtocol::PutCommand(
	const std::uint8_t command,
	const std::uint8_t option) noexcept {
    const char bytes[] = {
	static_cast<char>(IAC),
	static_cast<char>(command),
	static_cast<char>(option)
    };
    descriptor_.WriteRaw(String(bytes, sizeof(bytes)));
}

//! Clears TELNET-SB collection state.
void TelnetProtocol::ClearTelnetSb() {
    telnetSb_.str(String());
    telnetSb_.clear();
    telnetSbBit_ = false;
    telnetSbOverflowBit_ = false;
    telnetOption_ = /* None */ 0;
}

//! Processes TELNET-IAC input.
//! \param byteReceived the byte to process
void TelnetProtocol::ReceiveTelnetIac(const std::uint8_t byteReceived) {
    // Inside SB, RFC 854 only allows IAC IAC (literal 255) and IAC SE.
    if (telnetSbBit_) {
	switch (byteReceived) {
	case IAC:
	    this->ReceiveTelnetSbByte(byteReceived);
	    break;
	case SE:
	    if (!telnetSbOverflowBit_)
		this->ReceiveTelnetSb(telnetOption_, telnetSb_.str());
	    this->ClearTelnetSb();
	    break;
	default:
	    LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " received unexpected IAC " << GetTelcmdName(byteReceived) << " during SB; aborting.";
	    this->ClearTelnetSb();
	    break;
	}
	telnetCommand_ = /* None */ 0;
	return;
    }

    switch (byteReceived) {
    case EC:
	descriptor_.Backspace();
	break;
    case EL:
	descriptor_.BackspaceLine();
	break;
    case IAC:
	descriptor_.DeliverByte(byteReceived);
	break;
    case SE:
    case NOP:
    case DM:
    case BREAK:
    case IP:
    case AO:
    case GA:
	// Nothing.
	break;
    case AYT:
	descriptor_.BackspaceLine();
	this->PutCommand(EL);
	descriptor_.SetPromptBit(true);
	break;
    case DO:   case DONT:
    case WILL: case WONT:
    case SB:
	// Wait for option byte.
	telnetCommand_ = byteReceived;
	return;
    default:
	LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " received unknown IAC " << GetTelcmdName(byteReceived) << ".";
	break;
    }

    telnetCommand_ = /* None */ 0;
    telnetOption_  = /* None */ 0;
}

//! Processes a completed option negotiation.
//! \param command the negotiation command
//! \param option the TELNET option
void TelnetProtocol::ReceiveTelnetNegotiate(
	const std::uint8_t command,
	const std::uint8_t option) {
    switch (command) {
    case DO:
	this->RecvDo(option);
	break;
    case DONT:
	this->RecvDont(option);
	break;
    case WILL:
	this->RecvWill(option);
	break;
    case WONT:
	this->RecvWont(option);
	break;
    default:
	// Nothing.
	break;
    }
}

//! Processes TELNET-SB input.
//! \param option the TELNET option
//! \param sbReceived the TELNET-SB payload
void TelnetProtocol::ReceiveTelnetSb(
	const std::uint8_t option,
	const String& sbReceived) {
    switch (option) {
    case TELOPT_NAWS:
	if (this->Him(TELOPT_NAWS)) {
	    this->ReceiveNaws(sbReceived);
	} else {
	    LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " ignored NAWS SB; option not enabled.";
	}
	break;
    case TELOPT_TTYPE:
	if (this->Him(TELOPT_TTYPE)) {
	    this->ReceiveTtype(sbReceived);
	} else {
	    LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " ignored TTYPE SB; option not enabled.";
	}
	break;
    default:
	// Nothing.
	break;
    }
}

//! Processes one byte of TELNET-SB input.
//! \param sbByteReceived the TELNET-SB byte to process
void TelnetProtocol::ReceiveTelnetSbByte(const std::uint8_t sbByteReceived) {
    if (descriptor_.Closed() || telnetSbOverflowBit_)
	return;

    const auto pos = telnetSb_.tellp();
    if (pos < 0 || static_cast<std::size_t>(pos) >= MaxString) {
	telnetSbOverflowBit_ = true;
	telnetSb_.str(String());
	telnetSb_.clear();
	LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " TELNET-SB exceeded " << MaxString << " bytes.";
	return;
    }

    telnetSb_ << static_cast<char>(sbByteReceived);
}

//! Processes a NAWS subnegotiation.
//! \param sbReceived the TELNET-SB payload
void TelnetProtocol::ReceiveNaws(const String& sbReceived) {
    // IAC SB NAWS WIDTH[1] WIDTH[0] HEIGHT[1] HEIGHT[0] IAC SE
    if (sbReceived.size() != 4) {
	LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " sent NAWS payload length " << sbReceived.size() << " (expected 4); discarded.";
	return;
    }

    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::memcpy(&width, sbReceived.data(), sizeof(width));
    std::memcpy(&height, sbReceived.data() + sizeof(width), sizeof(height));
    width = boost::endian::big_to_native(width);
    height = boost::endian::big_to_native(height);

    if (width == descriptor_.GetWindowWidth() &&
	height == descriptor_.GetWindowHeight())
	return;

    descriptor_.SetWindowSize(width, height);
    LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " NAWS " << width << "x" << height << ".";
}

//! Called when an option becomes enabled on his side.
//! \param option the TELNET option
void TelnetProtocol::OnHimEnabled(const std::uint8_t option) {
    switch (option) {
    case TELOPT_TTYPE:
	this->RequestTtype();
	break;
    default:
	// Nothing.
	break;
    }
}

//! Requests his terminal type (RFC 1091).
void TelnetProtocol::RequestTtype() {
    const char bytes[] = {
	static_cast<char>(IAC),
	static_cast<char>(SB),
	static_cast<char>(TELOPT_TTYPE),
	static_cast<char>(TELQUAL_SEND),
	static_cast<char>(IAC),
	static_cast<char>(SE)
    };
    descriptor_.WriteRaw(String(bytes, sizeof(bytes)));
}

//! Processes a TTYPE subnegotiation.
//! \param sbReceived the TELNET-SB payload
void TelnetProtocol::ReceiveTtype(const String& sbReceived) {
    // IAC SB TTYPE IS <terminal type> IAC SE
    if (sbReceived.empty()) {
	LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " sent empty TTYPE payload.";
	return;
    }

    const auto qualifier = static_cast<std::uint8_t>(sbReceived[0]);
    if (qualifier != TELQUAL_IS) {
	LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " sent unexpected TTYPE qualifier " << static_cast<unsigned>(qualifier) << ".";
	return;
    }

    // Keep printable ASCII, then normalize whitespace (trim / collapse).
    String terminalType = sbReceived.substr(1);
    terminalType.erase(
	std::remove_if(
	    terminalType.begin(),
	    terminalType.end(),
	    [](const unsigned char ch) { return !std::isprint(ch); }),
	terminalType.end());
    Scratch::Algorithm::Strings::Normalize(terminalType);

    if (terminalType.empty()) {
	LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " sent empty TTYPE after sanitize.";
	return;
    }

    if (terminalType == descriptor_.GetTerminalType())
	return;

    descriptor_.SetTerminalType(terminalType);
    LOGGER_NETWORK() << "Descriptor " << descriptor_.GetName() << " TTYPE " << terminalType << ".";
}

}; // namespace Net
}; // namespace Scratch
