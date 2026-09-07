//! \file social.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_SOCIAL_HPP_
#define _SCRATCH_SOCIAL_HPP_

#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

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

//! The social class. \{
class Social {
public:
    //! Default constructor.
    Social() noexcept;

    //! Copy constructor.
    //! \param other the \sa social to copy
    Social(const Social& other) noexcept;

    //! Destructor.
    virtual ~Social() noexcept;

    //! Default assignment.
    //! \param other the \sa social to assign
    Social& operator=(const Social& other) noexcept;

    //! Gets the Found template.
    //! \sa #SetFound(const String&)
    String GetFound() const noexcept {
	return found_;
    }

    //! Gets the FoundAuto template.
    //! \sa #SetFoundAuto(const String&)
    String GetFoundAuto() const noexcept {
	return foundAuto_;
    }

    //! Gets the NoArgument template.
    //! \sa #SetNoArgument(const String&)
    String GetNoArgument() const noexcept {
	return noArgument_;
    }

    //! Reads this social from a data node.
    //! \param data the Social data node to read
    //! \sa #WriteData(const DataPtr&) const
    void ReadData(const DataPtr& data) noexcept;

    //! Sets the Found template.
    //! \sa #GetFound() const
    void SetFound(const String& found) {
	found_ = found;
    }

    //! Sets the FoundAuto template.
    //! \sa #GetFoundAuto() const
    void SetFoundAuto(const String& foundAuto) {
	foundAuto_ = foundAuto;
    }

    //! Sets the NoArgument template.
    //! \sa #GetNoArgument() const
    void SetNoArgument(const String& noArgument) {
	noArgument_ = noArgument;
    }

    //! Writes this social to a data node.
    //! \param data the Social data node to write
    //! \sa #ReadData(const DataPtr&)
    void WriteData(const DataPtr& data) const noexcept;

protected:
    //! The Found template.
    //! \sa #GetFound() const
    //! \sa #SetFound(const String&)
    String found_;

    //! The FoundAuto template.
    //! \sa #GetFoundAuto() const
    //! \sa #SetFoundAuto(const String&)
    String foundAuto_;

    //! The NoArgument template.
    //! \sa #GetNoArgument() const
    //! \sa #SetNoArgument(const String&)
    String noArgument_;
};
//! \}

//! The type of a shared social pointer.
using SocialPtr = std::shared_ptr<Social>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_SOCIAL_HPP_
