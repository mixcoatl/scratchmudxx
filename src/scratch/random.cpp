//! \file random.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_RANDOM_CPP_

#include <scratch/random.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Math {

//! Default constructor.
Random::Random() noexcept :
	algorithm_(),
	distribution_(0, std::numeric_limits<int>::max()) {
    this->Seed();
}

//! Copy constructor.
//! \param other the \sa random to copy
Random::Random(const Random& other) noexcept :
	algorithm_(other.algorithm_),
	distribution_(other.distribution_) {
    this->Seed();
}

//! Destructor.
Random::~Random() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the \sa random to assign
Random& Random::operator=(const Random& other) noexcept {
    algorithm_ = other.algorithm_;
    distribution_ = other.distribution_;
    return *this;
}

//! Rolls dice.
//! \param nDice the number of dice to roll
//! \param nDiceSides the number of sides for each die
//! \param modifier the dice roll modifier
int Random::Dice(
	const std::uint8_t nDice,
	const std::uint8_t nDiceSides,
	const int modifier) noexcept {
    auto result = 0;
    for (auto diceN = 0; diceN < nDice; ++diceN) {
        result += this->Next(1, nDiceSides);
    }
    return result + modifier;
}

//! Returns the next random value.
//! \param limit the upper bound of the value to return
//! \sa Next(const int)
//! \sa Next(const int, const int)
int Random::Next() noexcept {
    return distribution_(algorithm_);
}

//! Returns the next random value.
//! \param limit the upper bound of the value to return
//! \sa Next()
//! \sa Next(const int, const int)
int Random::Next(const int limit) noexcept {
    return this->Next() % limit;
}

//! Returns the next random value.
//! \param minimum the minimum value to return
//! \param maximum the maximum value to return
//! \sa Next()
//! \sa Next(const int)
int Random::Next(
	const int minimum,
	const int maximum) noexcept {
    return this->Next(maximum - minimum + 1) + minimum;
}

//! Sets the seed.
//! \sa #Seed(const boost::uintmax_t)
void Random::Seed() noexcept {
    // Current time.
    auto now = std::chrono::system_clock::now();
    auto nowEpoch = now.time_since_epoch();
    auto nowEpochNs = std::chrono::duration_cast<
		      std::chrono::nanoseconds>(nowEpoch).count();

    // Seed.
    this->Seed(nowEpochNs);
}

//! Sets the seed.
//! \param seed the seed to set
//! \sa #Seed()
void Random::Seed(const boost::uintmax_t seed) noexcept {
    algorithm_.seed(seed);
    distribution_.reset();
}

}; // namespace Math
}; // namespace Scratch
