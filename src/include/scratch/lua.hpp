//! \file lua.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_LUA_HPP_
#define _SCRATCH_LUA_HPP_

#include <scratch/enum.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {
class Game;
}; // namespace Core

namespace Scripting {

// ScratchMUD types.
using Game = Scratch::Core::Game;
using StringSetCi = Scratch::StringSetCi;

namespace Detail {
struct OptionalArgument;
}; // namespace Detail

template<typename ClassT>
class ClassBinding;

//! The Lua class. \{
class Lua {
public:
    //! The caller class. \{
    class Caller {
    public:
	//! Constructor.
	//! \param lua the Lua facade
	//! \param caller the caller identity to push
	Caller(
		Lua& lua,
		const String& caller);

	//! Copy constructor.
	Caller(const Caller&) = delete;

	//! Destructor.
	~Caller();

	//! Copy assignment operator.
	Caller& operator=(const Caller&) = delete;

	//! Returns whether the frame is active.
	bool IsActive() const noexcept {
	    return active_;
	}

    private:
	//! The active frame bit.
	bool active_;

	//! The Lua facade.
	Lua& lua_;
    };
    //! \}

    //! Constructor.
    //! \param game the game state
    explicit Lua(Game& game);

    //! Copy constructor.
    Lua(const Lua&) = delete;

    //! Destructor.
    virtual ~Lua() noexcept;

    //! Copy assignment operator.
    Lua& operator=(const Lua&) = delete;

    //! Resolves a caller identity.
    //! \param L the \c lua_State
    //! \return the caller identity, or \c "Game"
    static String CheckCaller(lua_State* L);

    //! Resolves a game state.
    //! \param L the \c lua_State
    //! \return the game state
    static Game& CheckGame(lua_State* L);

    //! Resolves the Lua facade.
    //! \param L the \c lua_State
    static Lua& CheckLua(lua_State* L);

    //! Resolves a Lua string at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the Lua string
    //! \return a C++ copy of the Lua string
    static String CheckString(
	lua_State* L,
	const int index);

    //! Reads Lua string array at \p index into \p out.
    //! \param L the \c lua_State
    //! \param out the destination array
    //! \param index the stack index of the table
    //! \param error the element error
    static void CheckStringArray(
	    lua_State* L,
	    std::vector<String>& out,
	    const int index = 1,
	    const char* error = "strings expected");

    //! Reads Lua string set at \p index into \p out.
    //! \param L the \c lua_State
    //! \param out the destination set
    //! \param index the stack index of the table
    //! \param error the element error
    static void CheckStringSet(
	    lua_State* L,
	    StringSetCi& out,
	    const int index = 1,
	    const char* error = "members must be strings");

    //! Resolves weak userdata at \p index.
    //! \param L the \c lua_State
    //! \param meta the userdata metatable name
    //! \param error the invalid-handle error
    //! \param index the stack index of the userdata
    //! \return a temporary shared pointer
    template<typename T>
    static std::shared_ptr<T> CheckWeakUserdata(
	    lua_State* L,
	    const char* meta,
	    const char* error,
	    const int index = 1) {
	auto weak = static_cast<std::weak_ptr<T>*>(
	    luaL_checkudata(L, index, meta));
	auto value = weak->lock();
	if (!value)
	    luaL_argerror(L, index, error);
	return value;
    }

    //! Destroys weak userdata at index 1.
    //! \param L the \c lua_State
    //! \param meta the userdata metatable name
    template<typename T>
    static int DestroyWeakUserdata(
	    lua_State* L,
	    const char* meta) {
	static_cast<std::weak_ptr<T>*>(
	    luaL_checkudata(L, 1, meta))->~weak_ptr<T>();
	return 0;
    }

    //! Executes a string.
    //! \param str the Lua code to execute
    //! \return \c true if the string executed successfully
    //! \sa #Execute(const String&, const String&)
    virtual bool Execute(const String& str);

    //! Executes a string.
    //! \param caller the caller identity
    //! \param str the Lua code to execute
    //! \return \c true if the string executed successfully
    //! \sa #Execute(const String&)
    virtual bool Execute(
	const String& caller,
	const String& str);

    //! Returns the game state.
    Game& GetGame() noexcept {
	return game_;
    }

    //! Returns the maximum nested #Execute depth.
    //! \sa #SetExecuteDepth(std::size_t)
    std::size_t GetExecuteDepth() const noexcept {
	return executeDepth_;
    }

    //! Returns the \c lua_State.
    lua_State* GetState() const noexcept {
	return lua_;
    }

    //! Calls a C function.
    //! \param function the C function to call
    //! \param nArgs the number of arguments already on the stack for \p function
    //! \param nReturns the number of results to leave on the stack
    //! \return \c LUA_OK or a Lua error status; on failure an error object is on the stack
    //! \remark Does not longjmp. Destroy C++ locals, then \c lua_error to propagate.
    int PCall(
	const lua_CFunction function,
	const int nArgs,
	const int nReturns);

    //! Pushes a boolean.
    //! \param value the value to push
    //! \remark May longjmp on stack overflow.
    void PushBool(const bool value);

    //! Pushes the current env, or nil.
    //! \remark May longjmp on stack overflow.
    void PushCallerEnv();

    //! Pushes a C function.
    //! \param func the C function to push
    //! \remark May longjmp on stack overflow.
    void PushFunction(const lua_CFunction func);

    //! Pushes an integer.
    //! \param value the value to push
    //! \remark May longjmp on stack overflow.
    void PushInt(const lua_Integer value);

    //! Pushes a number.
    //! \param value the value to push
    //! \remark May longjmp on stack overflow.
    void PushNumber(const double value);

    //! Pushes a C++ string as a Lua string.
    //! \param s the string to push
    //! \remark Moves into #stagingString_ before push; may longjmp.
    void PushString(String s);

    //! Pushes a string-to-string map as a Lua table.
    //! \param entries the key/value pairs to push
    //! \remark Moves into #stagingStringMap_ before allocation; may longjmp.
    void PushStringMap(StringMapCi<String> entries);

    //! Pushes a #StringSetCi as a Lua array (1-based).
    //! \param strings the strings to push
    //! \remark Moves into #stagingStringSet_ before allocation; may longjmp.
    void PushStringSet(StringSetCi strings);

    //! Pushes a weak userdata for \p p, or nil when \p p is empty.
    //! \param p the shared pointer to expose as \c std::weak_ptr userdata
    //! \param meta the metatable name
    //! \remark Moves into #stagingPtr_ before userdata allocation; may longjmp.
    template<typename T>
    void PushUserdata(
	std::shared_ptr<T> p,
	const char* meta) {
	if (!p) {
	    lua_pushnil(lua_);
	    return;
	}
	stagingPtr_ = std::move(p);
	void* mem = lua_newuserdata(lua_, sizeof(std::weak_ptr<T>));
	new (mem) std::weak_ptr<T>(
	    std::static_pointer_cast<T>(stagingPtr_));
	stagingPtr_.reset();
	luaL_setmetatable(lua_, meta);
    }

    //! Pushes a weak userdata for \p w.
    //! \param w the weak pointer to store in userdata
    //! \param meta the metatable name
    //! \remark Moves into #stagingPtr_ before userdata allocation; may longjmp.
    template<typename T>
    void PushUserdata(
	std::weak_ptr<T> w,
	const char* meta) {
	stagingPtr_ = std::make_shared<std::weak_ptr<T>>(std::move(w));
	void* mem = lua_newuserdata(lua_, sizeof(std::weak_ptr<T>));
	new (mem) std::weak_ptr<T>(
	    *static_cast<std::weak_ptr<T>*>(stagingPtr_.get()));
	stagingPtr_.reset();
	luaL_setmetatable(lua_, meta);
    }

    //! Registers a metatable shell.
    //! \param L the \c lua_State
    //! \param name the metatable name
    static void RegisterMetatable(
	    lua_State* L,
	    const char* name);

    //! Sets an environment field.
    //! \param name the field name
    //! \sa #SetSafe(const String&)
    void SetEnv(const String& name);

    //! Sets the maximum nested #Execute depth.
    //! \param executeDepth the maximum nesting depth
    //! \sa #GetExecuteDepth() const
    void SetExecuteDepth(const std::size_t executeDepth) noexcept {
	executeDepth_ = executeDepth;
    }

    //! Sets a safe field.
    //! \param name the field name
    //! \sa #SetEnv(const String&)
    void SetSafe(const String& name);

    //! Sets a safe C function.
    //! \param name the field name
    //! \param function the C function
    void SetSafe(
	    const String& name,
	    lua_CFunction function);

    //! Stores the value at the top of the stack in the safe table.
    //! \param name the field name
    void SetSafeValue(const String& name);

    //! Begins a type-scoped binding.
    //! \param name the Lua type name
    template<typename ClassT>
    ClassBinding<ClassT> Class(const char* name);

    //! Registers a Game method as a Lua free function.
    //! \param name the Lua function name
    //! \param function the Game method
    template<typename ReturnT, typename... ArgsT>
    void Function(
	    const char* name,
	    ReturnT (Game::*function)(ArgsT...));

    //! Registers a Game method with optional trailing arguments.
    //! \param name the Lua function name
    //! \param function the Game method
    //! \param optional the optional argument markers
    template<typename ReturnT, typename... ArgsT,
	typename... OptionalArgsT>
    void Function(
	    const char* name,
	    ReturnT (Game::*function)(ArgsT...),
	    Detail::OptionalArgument optional,
	    OptionalArgsT... optionals);

    //! Registers a const Game method as a Lua free function.
    //! \param name the Lua function name
    //! \param function the const Game method
    template<typename ReturnT, typename... ArgsT>
    void Function(
	    const char* name,
	    ReturnT (Game::*function)(ArgsT...) const);

    //! Registers a typed free function.
    //! \param name the Lua function name
    //! \param function the free function
    template<typename ReturnT, typename... ArgsT>
    void Function(
	    const char* name,
	    ReturnT (*function)(ArgsT...));

    //! Registers a raw Lua function as a free function.
    //! \param name the Lua function name
    //! \param function the Lua function
    void RawFunction(
	    const char* name,
	    lua_CFunction function);

protected:
    //! Freezes a table.
    //! \param index the stack index of the table
    //! \remark Replaces the table at \p index with a read-only proxy.
    void FreezeTable(const int index);

    //! Initializes the safe table.
    void InitSafe();

    //! Pops the current sandbox env.
    void PopSandboxEnv();

    //! Pushes the safe table.
    void PushSafe();

    //! Creates a sandbox env.
    void PushSandboxEnv();

    //! Nested Caller identities.
    std::vector<String> callers_;

    //! Nested sandbox env registry refs.
    std::vector<int> envs_;

    //! The maximum nested #Execute depth.
    std::size_t executeDepth_;

    //! The game state.
    Game& game_;

    //! The \c lua_State.
    lua_State* lua_;

    //! Type-erased pointer moved for #PushUserdata before userdata allocation.
    std::shared_ptr<void> stagingPtr_;

    //! String moved for #PushString before \c lua_pushlstring.
    String stagingString_;

    //! String map moved for #PushStringMap before table allocation.
    StringMapCi<String> stagingStringMap_;

    //! String set moved for #PushStringSet before table allocation.
    StringSetCi stagingStringSet_;
};
//! \}

//! Pushes an ordered array of enum names.
//! \param L the \c lua_State
//! \sa Scratch::Algorithm::EnumBase::ForEach
template<typename Derived>
int PushEnumNames(lua_State* L) {
    Lua::CheckLua(L);
    lua_createtable(L, Derived::Max(), 0);
    int index = 1;
    Derived::ForEach([&](typename Derived::Enum, const String& name) {
	lua_pushlstring(L, name.c_str(), name.length());
	lua_rawseti(L, -2, index++);
    });
    return 1;
}

//! Pushes an ordered array of enum names.
//! \param L the \c lua_State
//! \param match the match predicate
//! \sa Scratch::Algorithm::EnumBase::ForEach
template<typename Derived, typename Match>
int PushEnumNames(lua_State* L, Match match) {
    Lua::CheckLua(L);
    lua_createtable(L, Derived::Max(), 0);
    int index = 1;
    Derived::ForEach(match, [&](typename Derived::Enum, const String& name) {
	lua_pushlstring(L, name.c_str(), name.length());
	lua_rawseti(L, -2, index++);
    });
    return 1;
}

}; // namespace Scripting
}; // namespace Scratch

#include <scratch/lua_bindings.hpp>

#endif // _SCRATCH_LUA_HPP_
