//! \file Logger.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_LOGGER_CPP_

#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Utility {

//! Default constructor.
Logger::Logger() noexcept :
	buffer_(),
	sourceFile_(nullptr),
	sourceLine_(0),
	topic_(nullptr) {
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
    if (topic_ && *topic_ != '\0')
      std::cout << '[' << topic_ << ']' << ' ';

    // Print message.
    if (buffer_.str().size())
	std::cout << buffer_.str() << ' ';

    // Print filename and line number.
    if (sourceFile_ && *sourceFile_ != '\0' && sourceLine_)
	std::cout << '{' << sourceFile_ << ':' << sourceLine_ << '}';

    // Flush Logger.
    std::cout << std::endl;
}

}; // namespace Utility
}; // namespace Scratch
