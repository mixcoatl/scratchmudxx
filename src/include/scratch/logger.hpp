//! \file logger.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2024 scratchmud.org
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
        mBuffer << value;
	return *this;
    }

    //! Gets the source filename.
    //! \sa #SetSourceFilename(const char*)
    const char* GetSourceFilename() const noexcept {
        return mSourceFilename;
    }

    //! Gets the source line.
    //! \sa #SetSourceLine(const int)
    int GetSourceLine() const noexcept {
        return mSourceLine;
    }

    //! Gets the logger topic.
    //! \sa #SetTopic(const char*)
    const char* GetTopic() const noexcept {
        return mTopic;
    }

    //! Sets the source filename.
    //! \sa #GetSourceFilename() const
    Logger& SetSourceFilename(const char* source_fname) noexcept {
        mSourceFilename = source_fname;
        return *this;
    }

    //! Sets the source line.
    //! \sa #GetSourceLine() const
    Logger& SetSourceLine(const int source_line) noexcept {
        mSourceLine = source_line;
        return *this;
    }

    //! Gets the logger topic.
    //! \sa #GetTopic() const
    Logger& SetTopic(const char* topic) noexcept {
        mTopic = topic;
        return *this;
    }

protected:
    //! The message buffer.
    std::ostringstream mBuffer;

    //! The source filename.
    //! \sa #GetSourceFilename() const
    //! \sa #SetSourceFilename(const char*)
    const char* mSourceFilename;

    //! The source line.
    //! \sa #GetSourceLine() const
    //! \sa #SetSourceLine(const int)
    int mSourceLine;

    //! The logger topic.
    //! \sa #GetTopic() const
    //! \sa #SetTopic(const char*)
    const char* mTopic;
};
//! \}

}; // namespace Utility
}; // namespace Scratch

//! Constructs a logger.
//! \param topic the logger topic
#define LOGGER(topic) \
    Scratch::Utility::Logger(). \
        SetSourceFilename(__FILE__). \
        SetSourceLine(__LINE__). \
        SetTopic(topic)

//! Logger shortcuts. \{
#define LOGGER_ASSERT()         LOGGER("Assert")        //!< Code assertions.
#define LOGGER_MAIN()           LOGGER("Main")          //!< Program entry point.
#define LOGGER_NETWORK()        LOGGER("Network")       //!< Network activity.
#define LOGGER_SYSTEM()         LOGGER("System")        //!< System errors, etc.
//! \}

#endif // _SCRATCH_LOGGER_HXX_
