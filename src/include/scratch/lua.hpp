//! \file lua.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_LUA_HPP_
#define _SCRATCH_LUA_HPP_

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
		const String& caller) :
	    lua_(lua) {
	    lua_.callers_.push_back(caller);
	}

	//! Copy constructor.
	Caller(const Caller&) = delete;

	//! Destructor.
	~Caller() {
	    lua_.callers_.pop_back();
	}

	//! Copy assignment operator.
	Caller& operator=(const Caller&) = delete;

    private:
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

    //! Executes a string.
    //! \param str the Lua code to execute
    //! \return \c true if the string executed successfully
    //! \sa #Execute(const String&, const String&)
    virtual bool Execute(const String& str) {
	return this->Execute("Game", str);
    }

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

    //! Pushes a Lua global.
    //! \param name the global name
    //! \sa #SetGlobal(const String&)
    void GetGlobal(const String& name);

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
	struct Box {
	    std::weak_ptr<T> weak;
	};
	stagingPtr_ = std::shared_ptr<Box>(new Box{std::move(w)});
	void* mem = lua_newuserdata(lua_, sizeof(std::weak_ptr<T>));
	new (mem) std::weak_ptr<T>(
	    static_cast<Box*>(stagingPtr_.get())->weak);
	stagingPtr_.reset();
	luaL_setmetatable(lua_, meta);
    }

    //! Registers a global free function.
    //! \param name the global name
    //! \param func the C function to register
    void Register(
	const String& name,
	const lua_CFunction func);

    //! Sets the maximum nested #Execute depth.
    //! \param executeDepth the maximum nesting depth
    //! \sa #GetExecuteDepth() const
    void SetExecuteDepth(const std::size_t executeDepth) noexcept {
	executeDepth_ = executeDepth;
    }

    //! Sets a Lua global.
    //! \param name the global name
    //! \sa #GetGlobal(const String&)
    void SetGlobal(const String& name);

    //! Unregisters a global name.
    //! \param name the global name to clear
    void Unregister(const String& name);

protected:
    //! Nested #Execute caller identities.
    std::vector<String> callers_;

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

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_LUA_HPP_
