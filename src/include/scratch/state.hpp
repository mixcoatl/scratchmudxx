//! \file state.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_STATE_HPP_
#define _SCRATCH_STATE_HPP_

#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <scratch/thing.hpp>

// Forward declarations.
namespace Scratch {
namespace Utility {
class Data;
}; // namespace Utility
}; // namespace Scratch

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;

//! The connection state class. \{
class State : public Thing {
public:
    //! Default constructor.
    State() noexcept;

    //! Copy constructor.
    //! \param other the \sa state to copy
    State(const State& other) noexcept;

    //! Destructor.
    virtual ~State() noexcept;

    //! Default assignment.
    //! \param other the \sa state to assign
    State& operator=(const State& other) noexcept;

    //! Gets the Focus hook.
    //! \sa #SetFocus(const String&)
    String GetFocus() const noexcept {
	return focus_;
    }

    //! Gets the FocusLost hook.
    //! \sa #SetFocusLost(const String&)
    String GetFocusLost() const noexcept {
	return focusLost_;
    }

    //! Gets the Prompt bit.
    //! \sa #SetPromptBit(const bool)
    bool GetPromptBit() const noexcept {
	return promptBit_;
    }

    //! Gets the Quiet bit.
    //! \sa #SetQuietBit(const bool)
    bool GetQuietBit() const noexcept {
	return quietBit_;
    }

    //! Gets the Received hook.
    //! \sa #SetReceived(const String&)
    String GetReceived() const noexcept {
	return received_;
    }

    //! Reads this state from a data node.
    //! \param data the data node to read
    //! \sa #WriteData(const DataPtr&) const
    void ReadData(const DataPtr& data) noexcept;

    //! Reads hooks from a data node.
    //! \param data the Hooks data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteHooksData(const DataPtr&) const
    void ReadHooksData(const DataPtr& data) noexcept;

    //! Reads bit flags from a data node.
    //! \param data the StateBits data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteStateBitsData(const DataPtr&) const
    void ReadStateBitsData(const DataPtr& data) noexcept;

    //! Sets the Focus hook.
    //! \sa #GetFocus() const
    void SetFocus(const String& focus) {
	focus_ = focus;
    }

    //! Sets the FocusLost hook.
    //! \sa #GetFocusLost() const
    void SetFocusLost(const String& focusLost) {
	focusLost_ = focusLost;
    }

    //! Sets the Prompt bit.
    //! \sa #GetPromptBit() const
    void SetPromptBit(const bool promptBit) noexcept {
	promptBit_ = promptBit;
    }

    //! Sets the Quiet bit.
    //! \sa #GetQuietBit() const
    void SetQuietBit(const bool quietBit) noexcept {
	quietBit_ = quietBit;
    }

    //! Sets the Received hook.
    //! \sa #GetReceived() const
    void SetReceived(const String& received) {
	received_ = received;
    }

    //! Writes this state to a data node.
    //! \param data the data node to write
    //! \sa #ReadData(const DataPtr&)
    void WriteData(const DataPtr& data) const noexcept;

    //! Writes hooks to a data node.
    //! \param data the Hooks data node to write
    //! \sa #ReadHooksData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteHooksData(const DataPtr& data) const noexcept;

    //! Writes bit flags to a data node.
    //! \param data the StateBits data node to write
    //! \sa #ReadStateBitsData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteStateBitsData(const DataPtr& data) const noexcept;

protected:
    //! The Focus hook.
    //! \sa #GetFocus() const
    //! \sa #SetFocus(const String&)
    String focus_;

    //! The FocusLost hook.
    //! \sa #GetFocusLost() const
    //! \sa #SetFocusLost(const String&)
    String focusLost_;

    //! The Prompt bit.
    //! \sa #GetPromptBit() const
    //! \sa #SetPromptBit(const bool)
    bool promptBit_;

    //! The Quiet bit.
    //! \sa #GetQuietBit() const
    //! \sa #SetQuietBit(const bool)
    bool quietBit_;

    //! The Received hook.
    //! \sa #GetReceived() const
    //! \sa #SetReceived(const String&)
    String received_;
};
//! \}

//! The type of a shared state pointer.
using StatePtr = std::shared_ptr<State>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_STATE_HPP_
