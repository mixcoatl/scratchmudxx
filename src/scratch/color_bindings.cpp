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
    for (int i = Color::C_FIRST_REAL; i < Color::MAX_C_TYPES; ++i) {
	auto name = Color::ToString(static_cast<Color::ColorEnum>(i));
	if (name.empty())
	    continue;
	for (auto& ch: name)
	    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
	codes[name] = d.GetColor(i);
    }
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

//! Pushes an ordered array of color names.
//! \param L the \c lua_State
//! \param first the first color enumeration value
//! \param last the last color enumeration value (inclusive)
static int PushColorNameRange(
	lua_State* L,
	const int first,
	const int last) {
    lua_createtable(L, last - first + 1, 0);
    int index = 1;
    for (int i = first; i <= last; ++i) {
	auto name = Color::ToString(static_cast<Color::ColorEnum>(i));
	if (name.empty())
	    continue;
	lua_pushlstring(L, name.c_str(), name.length());
	lua_rawseti(L, -2, index++);
    }
    return 1;
}

//! Handles lua get_color_names.
static int GetColorNamesProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_color_names expects no arguments");
    return PushColorNameRange(
	L,
	Color::C_FIRST_REAL,
	Color::C_NORMAL);
}

//! Handles lua get_metacolor_names.
static int GetMetaColorNamesProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_metacolor_names expects no arguments");
    return PushColorNameRange(
	L,
	Color::C_FIRST_META,
	Color::MAX_C_TYPES - 1);
}

//! Registers Color free functions on \p lua.
//! \param lua the Lua facade
void ColorBindings::Register(Lua& lua) {
    lua.PushFunction(GetColorNamesProxy);
    lua.SetSafe("get_color_names");
    lua.PushFunction(GetMetaColorNamesProxy);
    lua.SetSafe("get_metacolor_names");
}

}; // namespace Scripting
}; // namespace Scratch
