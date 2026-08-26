//! \file enum.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ENUM_HPP_
#define _SCRATCH_ENUM_HPP_

#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <type_traits>
#include <utility>

namespace Scratch {
namespace Algorithm {

//! The enum base class. \{
template<typename Derived>
class EnumBase {
public:
    //! Enum match predicate.
    template<typename EnumT>
    using EnumMatch = bool (*)(EnumT) noexcept;

    //! Match predicate that accepts every value.
    //! \param value the enum value
    template<typename EnumT>
    static constexpr bool MatchAll(EnumT) noexcept {
	return true;
    }

    //! Calls \p fn for each matched enum value and name.
    //! \param match the match predicate
    //! \param fn the callback
    template<typename Match, typename FuncT>
    static void ForEach(Match match, FuncT&& fn) {
	using Enum = typename Derived::Enum;
	using Underlying = typename std::underlying_type<Enum>::type;
	auto predicate = static_cast<EnumMatch<Enum>>(match);
	for (Underlying value = 0; value < Derived::Max(); ++value) {
	    auto e = static_cast<Enum>(value);
	    if (!predicate(e))
		continue;
	    auto name = Derived::ToString(e);
	    if (name.empty())
		continue;
	    fn(e, name);
	}
    }

    //! Calls \p fn for each enum value and name.
    //! \param fn the callback
    template<typename FuncT>
    static void ForEach(FuncT&& fn) {
	using Enum = typename Derived::Enum;
	ForEach(MatchAll<Enum>, std::forward<FuncT>(fn));
    }

protected:
    //! Destructor.
    ~EnumBase() = default;
};
//! \}

}; // namespace Algorithm
}; // namespace Scratch

#endif // _SCRATCH_ENUM_HPP_
