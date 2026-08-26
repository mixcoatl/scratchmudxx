//! \file state.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_STATE_CPP_

#include <scratch/data.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
State::State() noexcept :
	Thing(),
	focus_(),
	focusLost_(),
	promptBit_(false),
	quietBit_(false),
	received_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa state to copy
State::State(const State& other) noexcept :
	Thing(other),
	focus_(other.focus_),
	focusLost_(other.focusLost_),
	promptBit_(other.promptBit_),
	quietBit_(other.quietBit_),
	received_(other.received_) {
    // Nothing.
}

//! Destructor.
State::~State() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the \sa state to assign
State& State::operator=(const State& other) noexcept {
    Thing::operator=(other);
    focus_ = other.focus_;
    focusLost_ = other.focusLost_;
    promptBit_ = other.promptBit_;
    quietBit_ = other.quietBit_;
    received_ = other.received_;
    return *this;
}

//! Reads this state from a data node.
//! \param data the data node to read
//! \sa #WriteData(const DataPtr&) const
void State::ReadData(const DataPtr& data) noexcept {
    if (!data)
	return;

    name_ = data->GetString("Name");

    // Read hooks.
    auto hooksData = data->Get("Hooks", std::make_shared<Data>());
    this->ReadHooksData(hooksData);

    // Read metadata.
    auto metadataData = data->Get("Metadata", std::make_shared<Data>());
    this->ReadMetadataData(metadataData);

    // Read state bits.
    auto stateBitsData = data->Get("StateBits", std::make_shared<Data>());
    this->ReadStateBitsData(stateBitsData);
}

//! Reads hooks from a data node.
//! \param data the Hooks data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteHooksData(const DataPtr&) const
void State::ReadHooksData(const DataPtr& data) noexcept {
    focus_ = data->GetString("Focus");
    focusLost_ = data->GetString("FocusLost");
    received_ = data->GetString("Received");
}

//! Reads bit flags from a data node.
//! \param data the StateBits data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteStateBitsData(const DataPtr&) const
void State::ReadStateBitsData(const DataPtr& data) noexcept {
    promptBit_ = data->GetYesNo("Prompt");
    quietBit_ = data->GetYesNo("Quiet");
}

//! Writes this state to a data node.
//! \param data the data node to write
//! \sa #ReadData(const DataPtr&)
void State::WriteData(const DataPtr& data) const noexcept {
    if (!data)
	return;

    if (!name_.empty())
	data->PutString("Name", name_);

    // Write hooks.
    auto hooksData = std::make_shared<Data>();
    this->WriteHooksData(hooksData);
    if (hooksData->Size())
	data->Put("Hooks", hooksData);

    // Write metadata.
    auto metadataData = std::make_shared<Data>();
    this->WriteMetadataData(metadataData);
    if (metadataData->Size())
	data->Put("Metadata", metadataData);

    // Write state bits.
    auto stateBitsData = std::make_shared<Data>();
    this->WriteStateBitsData(stateBitsData);
    if (stateBitsData->Size())
	data->Put("StateBits", stateBitsData);
}

//! Writes hooks to a data node.
//! \param data the Hooks data node to write
//! \sa #ReadHooksData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void State::WriteHooksData(const DataPtr& data) const noexcept {
    if (!focus_.empty())
	data->PutString("Focus", focus_);
    if (!focusLost_.empty())
	data->PutString("FocusLost", focusLost_);
    if (!received_.empty())
	data->PutString("Received", received_);
}

//! Writes bit flags to a data node.
//! \param data the StateBits data node to write
//! \sa #ReadStateBitsData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void State::WriteStateBitsData(const DataPtr& data) const noexcept {
    if (promptBit_)
	data->PutYesNo("Prompt", true);
    if (quietBit_)
	data->PutYesNo("Quiet", true);
}

}; // namespace Core
}; // namespace Scratch
