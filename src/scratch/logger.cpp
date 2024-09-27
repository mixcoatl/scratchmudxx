//! \file Logger.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2024 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_LOGGER_CPP_

#include <scratch/Logger.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Utility {

//! Default constructor.
Logger::Logger() noexcept :
	mBuffer(),
	mSourceFilename(nullptr),
	mSourceLine(0),
	mTopic(nullptr) {
  // Nothing.
}

//! Destructor.
Logger::~Logger() noexcept {
    // Print current time.
    auto now = boost::chrono::system_clock::now();
    auto nowc = boost::chrono::system_clock::to_time_t(now);
    auto nowtm = std::localtime(&nowc);
    std::cout << std::put_time(nowtm, "%F %T") << ' ';

    // Print Logger topic.
    if (mTopic && *mTopic != '\0')
      std::cout << '[' << mTopic << ']' << ' ';

    // Print message.
    if (mBuffer.str().size())
	std::cout << mBuffer.str() << ' ';

    // Print filename and line number.
    if (mSourceFilename && *mSourceFilename != '\0' && mSourceLine > 0)
	std::cout << '{' << mSourceFilename << ':' << mSourceLine << '}';

    // Flush Logger.
    std::cout << std::endl;
}

}; // namespace Utility
}; // namespace Scratch
