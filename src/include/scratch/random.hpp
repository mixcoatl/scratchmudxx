//! \file random.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_RANDOM_HPP_
#define _SCRATCH_RANDOM_HPP_

#include <scratch/scratch.hpp>

namespace Scratch {
namespace Math {

//! The random class. \{
class Random {
public:
    //! The type of the RNG algorithm.
    using Algorithm = boost::mt19937_64;

    //! The type of the RNG distribution.
    using Distribution = boost::uniform_int<int>;

    //! Default constructor.
    Random() noexcept;

    //! Copy constructor.
    //! \param other the \sa random to copy
    Random(const Random& other) noexcept;

    //! Destructor.
    virtual ~Random() noexcept;

    //! Default assignment.
    //! \param other the \sa random to assign
    Random& operator=(const Random& other) noexcept;

    //! Rolls dice.
    //! \param nDice the number of dice to roll
    //! \param nDiceSides the number of sides for each die
    //! \param modifier the dice roll modifier
    int Dice(
	const std::uint8_t nDice,
	const std::uint8_t nDiceSides,
	const int modifier = 0) noexcept;

    //! Returns the next random value.
    //! \sa #Next(const int)
    //! \sa #Next(const int, const int)
    int Next() noexcept;

    //! Returns the next random value.
    //! \param limit the upper bound of the value to return
    //! \sa #Next()
    //! \sa #Next(const int, const int)
    int Next(const int limit) noexcept;

    //! Returns the next random value.
    //! \param minimum the minimum value to return
    //! \param maximum the maximum value to return
    //! \sa #Next()
    //! \sa #Next(const int)
    int Next(
	const int minimum,
	const int maximum) noexcept;

    //! Sets the seed.
    //! \sa #Seed(const boost::uintmax_t)
    void Seed() noexcept;

    //! Sets the seed.
    //! \param seed the seed to set
    //! \sa #Seed()
    void Seed(const boost::uintmax_t seed) noexcept;

protected:
    //! The RNG algorithm.
    Algorithm algorithm_;

    //! The RNG distribution.
    Distribution distribution_;
};
//! \}

}; // namespace Math
}; // namespace Scratch

#endif // _SCRATCH_RANDOM_HXX_
