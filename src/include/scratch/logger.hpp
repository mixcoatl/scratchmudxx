//! \file logger.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_LOGGER_HPP_
#define _SCRATCH_LOGGER_HPP_

#include <scratch/scratch.hpp>

namespace Scratch {
namespace Utility {

//! The Logger class. \{
class Logger {
public:
    //! Default constructor.
    Logger() noexcept;

    //! Destructor.
    ~Logger() noexcept;

    //! Writes to the logger.
    //! \tparam T the type of the specified value
    //! \param value the value to write to the logger
    //! \return a reference to this logger
    template<class T>
    Logger& operator<<(const T& value) noexcept {
        buffer_ << value;
	return *this;
    }

    //! Gets the source filename.
    //! \sa #SetSourceFile(const char*)
    const char* GetSourceFile() const noexcept {
        return sourceFile_;
    }

    //! Gets the source line.
    //! \sa #SetSourceLine(const int)
    int GetSourceLine() const noexcept {
        return sourceLine_;
    }

    //! Gets the logger topic.
    //! \sa #SetTopic(const char*)
    const char* GetTopic() const noexcept {
        return topic_;
    }

    //! Sets the source filename.
    //! \sa #GetSourceFile() const
    Logger& SetSourceFile(const char* sourceFile) noexcept {
        sourceFile_ = sourceFile;
        return *this;
    }

    //! Sets the source line.
    //! \sa #GetSourceLine() const
    Logger& SetSourceLine(const int sourceLine) noexcept {
        sourceLine_ = sourceLine;
        return *this;
    }

    //! Gets the logger topic.
    //! \sa #GetTopic() const
    Logger& SetTopic(const char* topic) noexcept {
        topic_ = topic;
        return *this;
    }

protected:
    //! The message buffer.
    std::ostringstream buffer_;

    //! The source filename.
    //! \sa #GetSourceFile() const
    //! \sa #SetSourceFile(const char*)
    const char* sourceFile_;

    //! The source line.
    //! \sa #GetSourceLine() const
    //! \sa #SetSourceLine(const int)
    int sourceLine_;

    //! The logger topic.
    //! \sa #GetTopic() const
    //! \sa #SetTopic(const char*)
    const char* topic_;
};
//! \}

}; // namespace Utility
}; // namespace Scratch

//! Constructs a logger.
//! \param topic the logger topic
#define LOGGER(topic) \
    Scratch::Utility::Logger(). \
        SetSourceFile(__FILE__). \
        SetSourceLine(__LINE__). \
        SetTopic(topic)

//! Logger shortcuts. \{
#define LOGGER_ASSERT()         LOGGER("Assert")        //!< Code assertions.
#define LOGGER_MAIN()           LOGGER("Main")          //!< Program entry point.
#define LOGGER_SYSTEM()         LOGGER("System")        //!< System errors, etc.
//! \}

#endif // _SCRATCH_LOGGER_HXX_
