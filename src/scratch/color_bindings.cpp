//! \file color_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_COLOR_BINDINGS_CPP_

#include <scratch/color.hpp>
#include <scratch/color_bindings.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

// ScratchMUD types.
using Color = Scratch::Net::Color;
using WeakDescriptorPtr = std::weak_ptr<Descriptor>;

//! Assigns \c Q from \p d.
//! \param lua the Lua facade
//! \param d the descriptor
void ColorBindings::AssignQ(
	Lua& lua,
	const Descriptor& d) {
    lua.PushStringMap(Codes(d));
    lua.SetEnv("Q");
}

//! Builds the color-code map for \c Q.
//! \param d the descriptor whose palette to export
//! \remark ToString names; legacy ANSI dim aliases for Lua \c Q only.
StringMapCi<String> ColorBindings::Codes(const Descriptor& d) {
    StringMapCi<String> codes;
    Color::ForEach([&](Color::ColorEnum color, const String& name) {
	String upper = name;
	for (auto& ch: upper)
	    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
	codes[upper] = d.GetColor(static_cast<int>(color));
    });
    codes["BLACK"] = codes["CHARCOAL"];
    codes["RED"] = codes["CRIMSON"];
    codes["GREEN"] = codes["FOREST"];
    codes["YELLOW"] = codes["OCHRE"];
    codes["BLUE"] = codes["INDIGO"];
    codes["MAGENTA"] = codes["PURPLE"];
    codes["CYAN"] = codes["TEAL"];
    codes["WHITE"] = codes["SILVER"];
    return codes;
}

//! Refills \c Q when \p desc is the current \c d.
//! \param L the \c lua_State
//! \param lua the Lua facade
//! \param desc the descriptor
void ColorBindings::RefillGlobalQ(
	lua_State* L,
	Lua& lua,
	const Descriptor& desc) {
    lua.PushCallerEnv();
    if (lua_isnil(L, -1)) {
	lua_pop(L, 1);
	return;
    }
    lua_getfield(L, -1, "d");
    bool same = false;
    if (auto* weak = static_cast<WeakDescriptorPtr*>(
	    luaL_testudata(L, -1, DescriptorBindings::MetaName))) {
	auto current = weak->lock();
	same = current.get() == &desc;
	current.reset();
    }
    lua_pop(L, 2);
    if (!same)
	return;
    AssignQ(lua, desc);
}

}; // namespace Scripting
}; // namespace Scratch
