//! \file lua_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_LUA_BINDINGS_HPP_
#define _SCRATCH_LUA_BINDINGS_HPP_

#include <boost/optional.hpp>
#include <scratch/color.hpp>
#include <exception>
#include <functional>
#include <tuple>
#include <typeindex>
#include <vector>

namespace Scratch {
namespace Scripting {

namespace Detail {

//! Converts a C++ value to and from Lua.
template<typename T, typename EnableT = void>
struct LuaValue;

//! Provides a default value for an optional argument.
template<typename T, typename EnableT = void>
struct LuaDefaultValue {
    static T Get() {
	return T();
    }
};

//! Marks a parameter supplied by the Lua environment.
template<typename T>
struct InjectedArgument {};

//! Marks a trailing function argument as optional.
struct OptionalArgument {
    OptionalArgument() noexcept :
	type(typeid(void)),
	apply() {
    }

    template<typename T>
    explicit OptionalArgument(T value) :
	type(typeid(T)),
	apply([value](void* target) {
	    *static_cast<T*>(target) = value;
	}) {
    }

    template<typename T>
    T Get() const {
	T value = LuaDefaultValue<T>::Get();
	if (apply) {
	    if (type != std::type_index(typeid(T)))
		throw std::bad_cast();
	    apply(&value);
	}
	return value;
    }

    std::type_index type;
    std::function<void(void*)> apply;
};

template<typename... T>
struct AllOptionalArguments;

template<>
struct AllOptionalArguments<>:
    std::true_type {};

template<typename FirstT, typename... RestT>
struct AllOptionalArguments<FirstT, RestT...>:
    std::integral_constant<bool,
	std::is_same<FirstT, OptionalArgument>::value &&
	AllOptionalArguments<RestT...>::value> {};

//! Converts enum values using an enum facade.
template<typename EnumT>
struct LuaEnumValue {
    using ValueT = typename EnumT::Enum;

    static ValueT Check(lua_State* L, const int index) {
	const auto name = Lua::CheckString(L, index);
	const auto value = EnumT::ByName(name);
	if (!EnumT::IsDefined(value))
	    luaL_argerror(L, index, "unknown enum value");
	return value;
    }

    static void Push(lua_State* L, const ValueT value) {
	if (!EnumT::IsDefined(value)) {
	    lua_pushnil(L);
	    return;
	}
	Lua::CheckLua(L).PushString(EnumT::ToString(value));
    }
};

//! Converts Color values to and from Lua strings.
template<>
struct LuaValue<Scratch::Net::Color::ColorEnum, void>:
    LuaEnumValue<Scratch::Net::Color> {};

//! Converts String values to and from Lua strings.
template<>
struct LuaValue<String, void> {
    static String Check(lua_State* L, const int index) {
	return Lua::CheckString(L, index);
    }

    static void Push(lua_State* L, String value) {
	Lua::CheckLua(L).PushString(std::move(value));
    }
};

//! Converts Boolean values to and from Lua booleans.
template<>
struct LuaValue<bool, void> {
    static bool Check(lua_State* L, const int index) {
	return lua_toboolean(L, index) != 0;
    }

    static void Push(lua_State* L, const bool value) {
	lua_pushboolean(L, value);
    }
};

//! Converts vector values to indexed Lua tables.
template<typename T>
struct LuaValue<std::vector<T>, void> {
    static std::vector<T> Check(lua_State* L, const int index) {
	const int table = lua_absindex(L, index);
	luaL_checktype(L, table, LUA_TTABLE);
	std::vector<T> value;
	const auto length = lua_rawlen(L, table);
	value.reserve(length);
	for (std::size_t n = 1; n <= length; ++n) {
	    lua_rawgeti(L, table, n);
	    value.push_back(LuaValue<T>::Check(L, -1));
	    lua_pop(L, 1);
	}
	return value;
    }

    static void Push(lua_State* L, std::vector<T> value) {
	lua_createtable(L, static_cast<int>(value.size()), 0);
	int index = 1;
	for (auto& element: value) {
	    LuaValue<T>::Push(L, std::move(element));
	    lua_rawseti(L, -2, index++);
	}
    }
};

//! Converts Boost optional values to Lua values or nil.
template<typename T>
struct LuaValue<boost::optional<T>, void> {
    static boost::optional<T> Check(
	    lua_State* L,
	    const int index) {
	if (lua_isnil(L, index))
	    return boost::none;
	return boost::optional<T>(LuaValue<T>::Check(L, index));
    }

    static void Push(
	    lua_State* L,
	    const boost::optional<T>& value) {
	if (!value) {
	    lua_pushnil(L);
	    return;
	}
	LuaValue<T>::Push(L, *value);
    }
};

//! Converts shared pointers to weak Lua userdata.
template<typename T>
struct LuaValue<std::shared_ptr<T>, void> {
    static std::shared_ptr<T> Check(lua_State* L, const int index) {
	if (lua_isnil(L, index))
	    return std::shared_ptr<T>();
	return Lua::CheckWeakUserdata<T>(
	    L, ClassBinding<T>::MetaName(), "invalid userdata", index);
    }

    static void Push(lua_State* L, std::shared_ptr<T> value) {
	Lua::CheckLua(L).PushUserdata(
	    std::move(value), ClassBinding<T>::MetaName());
    }
};

//! Converts weak pointers to weak Lua userdata.
template<typename T>
struct LuaValue<std::weak_ptr<T>, void> {
    static std::weak_ptr<T> Check(lua_State* L, const int index) {
	if (lua_isnil(L, index))
	    return std::weak_ptr<T>();
	luaL_checkudata(L, index, ClassBinding<T>::MetaName());
	return *static_cast<std::weak_ptr<T>*>(
	    lua_touserdata(L, index));
    }

    static void Push(lua_State* L, std::weak_ptr<T> value) {
	Lua::CheckLua(L).PushUserdata(
	    std::move(value), ClassBinding<T>::MetaName());
    }
};

//! Converts case-insensitive string sets to indexed Lua tables.
template<>
struct LuaValue<StringSetCi, void> {
    static StringSetCi Check(lua_State* L, const int index) {
	StringSetCi value;
	Lua::CheckStringSet(L, value, index);
	return value;
    }

    static void Push(lua_State* L, StringSetCi value) {
	Lua::CheckLua(L).PushStringSet(std::move(value));
    }
};

//! Converts case-insensitive string maps to keyed Lua tables.
template<>
struct LuaValue<StringMapCi<String>, void> {
    static StringMapCi<String> Check(lua_State* L, const int index) {
	const int table = lua_absindex(L, index);
	luaL_checktype(L, table, LUA_TTABLE);
	StringMapCi<String> value;
	lua_pushnil(L);
	while (lua_next(L, table) != 0) {
	    if (!lua_isstring(L, -2) || !lua_isstring(L, -1)) {
		lua_pop(L, 2);
		value.clear();
		luaL_argerror(L, index, "string map expected");
	    }
	    value[Lua::CheckString(L, -2)] = Lua::CheckString(L, -1);
	    lua_pop(L, 1);
	}
	return value;
    }

    static void Push(lua_State* L, StringMapCi<String> value) {
	Lua::CheckLua(L).PushStringMap(std::move(value));
    }
};

//! Converts userdata maps to keyed Lua tables.
template<typename T>
struct LuaValue<StringMapCi<std::shared_ptr<T>>, void> {
    static StringMapCi<std::shared_ptr<T>> Check(
	    lua_State* L,
	    const int index) {
	const int table = lua_absindex(L, index);
	luaL_checktype(L, table, LUA_TTABLE);
	StringMapCi<std::shared_ptr<T>> value;
	lua_pushnil(L);
	while (lua_next(L, table) != 0) {
	    if (!lua_isstring(L, -2)) {
		lua_pop(L, 2);
		value.clear();
		luaL_argerror(L, index, "userdata map expected");
	    }
	    value[Lua::CheckString(L, -2)] =
		LuaValue<std::shared_ptr<T>>::Check(L, -1);
	    lua_pop(L, 1);
	}
	return value;
    }

    static void Push(
	    lua_State* L,
	    StringMapCi<std::shared_ptr<T>> value) {
	lua_createtable(L, 0, static_cast<int>(value.size()));
	const int table = lua_absindex(L, -1);
	for (auto& entry: value) {
	    Lua::CheckLua(L).PushUserdata(
		entry.second, ClassBinding<T>::MetaName());
	    lua_setfield(L, table, entry.first.c_str());
	}
    }
};

//! Converts map values to keyed Lua tables.
template<typename KeyT, typename ValueT>
struct LuaValue<std::map<KeyT, ValueT>, void> {
    static std::map<KeyT, ValueT> Check(
	    lua_State* L,
	    const int index) {
	const int table = lua_absindex(L, index);
	luaL_checktype(L, table, LUA_TTABLE);
	std::map<KeyT, ValueT> result;
	lua_pushnil(L);
	while (lua_next(L, table) != 0) {
	    const auto key = LuaValue<KeyT>::Check(L, -2);
	    const auto value = LuaValue<ValueT>::Check(L, -1);
	    result[key] = value;
	    lua_pop(L, 1);
	}
	return result;
    }

    static void Push(
	    lua_State* L,
	    std::map<KeyT, ValueT> values) {
	lua_createtable(L, 0, static_cast<int>(values.size()));
	const int table = lua_absindex(L, -1);
	for (const auto& entry: values) {
	    LuaValue<KeyT>::Push(L, entry.first);
	    LuaValue<ValueT>::Push(L, entry.second);
	    lua_settable(L, table);
	}
    }
};

//! Converts set values to indexed Lua tables.
template<typename T>
struct LuaValue<std::set<T>, void> {
    static std::set<T> Check(
	    lua_State* L,
	    const int index) {
	const int table = lua_absindex(L, index);
	luaL_checktype(L, table, LUA_TTABLE);
	std::set<T> value;
	const int length = static_cast<int>(luaL_len(L, table));
	for (int i = 1; i <= length; ++i) {
	    lua_rawgeti(L, table, i);
	    value.insert(LuaValue<T>::Check(L, -1));
	    lua_pop(L, 1);
	}
	return value;
    }

    static void Push(
	    lua_State* L,
	    std::set<T> value) {
	lua_createtable(L, static_cast<int>(value.size()), 0);
	int index = 1;
	for (auto& entry: value) {
	    LuaValue<T>::Push(L, entry);
	    lua_rawseti(L, -2, index++);
	}
    }
};

//! Converts integral values to Lua integers.
template<typename T>
struct LuaValue<T, typename std::enable_if<
    std::is_integral<T>::value &&
    !std::is_same<T, bool>::value>::type> {
    static T Check(lua_State* L, const int index) {
	return static_cast<T>(luaL_checkinteger(L, index));
    }

    static void Push(lua_State* L, const T value) {
	lua_pushinteger(L, static_cast<lua_Integer>(value));
    }
};

//! Converts floating-point values to Lua numbers.
template<typename T>
struct LuaValue<T, typename std::enable_if<
    std::is_floating_point<T>::value>::type> {
    static T Check(lua_State* L, const int index) {
	return static_cast<T>(luaL_checknumber(L, index));
    }

    static void Push(lua_State* L, const T value) {
	lua_pushnumber(L, static_cast<lua_Number>(value));
    }
};

template<typename T>
struct LuaDefaultValue<T, typename std::enable_if<
    std::is_integral<T>::value &&
    !std::is_same<T, bool>::value>::type> {
    static T Get() {
	return T(0);
    }
};

template<>
struct LuaDefaultValue<bool, void> {
    static bool Get() {
	return false;
    }
};

template<typename T>
struct LuaDefaultValue<T, typename std::enable_if<
    std::is_floating_point<T>::value>::type> {
    static T Get() {
	return T(0);
    }
};

template<>
struct LuaDefaultValue<String, void> {
    static String Get() {
	return String();
    }
};

template<typename T>
struct LuaDefaultValue<std::shared_ptr<T>, void> {
    static std::shared_ptr<T> Get() {
	return std::shared_ptr<T>();
    }
};

template<typename T>
struct LuaDefaultValue<std::weak_ptr<T>, void> {
    static std::weak_ptr<T> Get() {
	return std::weak_ptr<T>();
    }
};

template<typename T>
struct LuaDefaultValue<std::vector<T>, void> {
    static std::vector<T> Get() {
	return std::vector<T>();
    }
};

template<typename T>
struct LuaDefaultValue<std::set<T>, void> {
    static std::set<T> Get() {
	return std::set<T>();
    }
};

template<typename KeyT, typename ValueT>
struct LuaDefaultValue<std::map<KeyT, ValueT>, void> {
    static std::map<KeyT, ValueT> Get() {
	return std::map<KeyT, ValueT>();
    }
};

template<typename T>
struct LuaArgument {
    using ValueT = typename std::remove_cv<
	typename std::remove_reference<T>::type>::type;

    ValueT value;

    static LuaArgument Read(lua_State* L, const int index) {
	return LuaArgument{LuaValue<ValueT>::Check(L, index)};
    }

    static LuaArgument ReadOptional(
	    lua_State* L,
	    const int index,
	    std::true_type) {
	if (index > lua_gettop(L))
	    return LuaArgument{LuaDefaultValue<ValueT>::Get()};
	return Read(L, index);
    }

    static LuaArgument ReadOptional(
	    lua_State* L,
	    const int index,
	    std::false_type) {
	return Read(L, index);
    }

    static LuaArgument ReadDefault(
	    lua_State*,
	    const int,
	    ValueT value) {
	return LuaArgument{std::move(value)};
    }

    T Get() {
	return static_cast<T>(value);
    }
};

template<typename T>
struct LuaArgument<T&> {
    using ValueT = typename std::remove_cv<T>::type;

    ValueT value;

    static LuaArgument Read(lua_State* L, const int index) {
	return LuaArgument{LuaValue<ValueT>::Check(L, index)};
    }

    static LuaArgument ReadOptional(
	    lua_State* L,
	    const int index,
	    std::true_type) {
	if (index > lua_gettop(L))
	    return LuaArgument{LuaDefaultValue<ValueT>::Get()};
	return Read(L, index);
    }

    static LuaArgument ReadOptional(
	    lua_State* L,
	    const int index,
	    std::false_type) {
	return Read(L, index);
    }

    static LuaArgument ReadDefault(
	    lua_State*,
	    const int,
	    ValueT value) {
	return LuaArgument{std::move(value)};
    }

    T& Get() {
	return value;
    }
};

template<typename T>
struct LuaArgument<const T&> {
    using ValueT = typename std::remove_cv<T>::type;

    ValueT value;

    static LuaArgument Read(lua_State* L, const int index) {
	return LuaArgument{LuaValue<ValueT>::Check(L, index)};
    }

    static LuaArgument ReadOptional(
	    lua_State* L,
	    const int index,
	    std::true_type) {
	if (index > lua_gettop(L))
	    return LuaArgument{LuaDefaultValue<ValueT>::Get()};
	return Read(L, index);
    }

    static LuaArgument ReadOptional(
	    lua_State* L,
	    const int index,
	    std::false_type) {
	return Read(L, index);
    }

    static LuaArgument ReadDefault(
	    lua_State*,
	    const int,
	    ValueT value) {
	return LuaArgument{std::move(value)};
    }

    const T& Get() const {
	return value;
    }
};

template<typename... ArgsT>
struct LuaArguments {
    using Values = std::tuple<LuaArgument<ArgsT>...>;

    template<std::size_t... Index>
    static Values ReadImpl(
	    lua_State* L,
	    std::index_sequence<Index...>,
	    const int offset) {
	return Values{
	    LuaArgument<ArgsT>::Read(
		L, static_cast<int>(Index + offset))...};
    }

    static Values ReadRoot(lua_State* L) {
	return ReadImpl(L, std::index_sequence_for<ArgsT...>(), 1);
    }

    template<std::size_t OptionalCount, std::size_t... Index>
    static Values ReadOptionalImpl(
	    lua_State* L,
	    std::index_sequence<Index...>,
	    const int offset) {
	return Values{
	    LuaArgument<ArgsT>::ReadOptional(
		L,
		static_cast<int>(Index + offset),
		std::integral_constant<bool,
		    Index >= sizeof...(ArgsT) - OptionalCount>())...};
    }

    template<std::size_t OptionalCount>
    static Values ReadOptionalRoot(lua_State* L) {
	return ReadOptionalImpl<OptionalCount>(
	    L, std::index_sequence_for<ArgsT...>(), 1);
    }

    template<std::size_t OptionalCount>
    static Values ReadOptional(lua_State* L) {
	return ReadOptionalImpl<OptionalCount>(
	    L, std::index_sequence_for<ArgsT...>(), 2);
    }

    template<std::size_t Index, std::size_t OptionalCount>
    static LuaArgument<typename std::tuple_element<
	Index, std::tuple<ArgsT...>>::type> ReadOptionalArgumentValue(
	    lua_State* L,
	    const int index,
	    const std::vector<OptionalArgument>&,
	    std::false_type) {
	using Argument = typename std::tuple_element<
	    Index, std::tuple<ArgsT...>>::type;
	return LuaArgument<Argument>::Read(L, index);
    }

    template<std::size_t Index, std::size_t OptionalCount>
    static LuaArgument<typename std::tuple_element<
	Index, std::tuple<ArgsT...>>::type> ReadOptionalArgumentValue(
	    lua_State* L,
	    const int index,
	    const std::vector<OptionalArgument>& defaults,
	    std::true_type) {
	using Argument = typename std::tuple_element<
	    Index, std::tuple<ArgsT...>>::type;
	using ValueT = typename LuaArgument<Argument>::ValueT;
	if (index <= lua_gettop(L))
	    return LuaArgument<Argument>::Read(L, index);
	const auto optionalIndex =
	    Index - sizeof...(ArgsT) + OptionalCount;
	return LuaArgument<Argument>::ReadDefault(
	    L, index, defaults[optionalIndex].template Get<ValueT>());
    }

    template<std::size_t OptionalCount, std::size_t... Index>
    static Values ReadOptionalWithDefaults(
	    lua_State* L,
	    const std::vector<OptionalArgument>& defaults,
	    std::index_sequence<Index...>,
	    const int offset) {
	return Values{
	    ReadOptionalArgumentValue<Index, OptionalCount>(
		L,
		static_cast<int>(Index + offset),
		defaults,
		std::integral_constant<bool,
		    Index >= sizeof...(ArgsT) - OptionalCount>())...};
    }

    template<std::size_t OptionalCount>
    static Values ReadOptionalRoot(
	    lua_State* L,
	    const std::vector<OptionalArgument>& defaults) {
	return ReadOptionalWithDefaults<OptionalCount>(
	    L, defaults, std::index_sequence_for<ArgsT...>(), 1);
    }

    template<std::size_t OptionalCount>
    static Values ReadOptional(
	    lua_State* L,
	    const std::vector<OptionalArgument>& defaults) {
	return ReadOptionalWithDefaults<OptionalCount>(
	    L, defaults, std::index_sequence_for<ArgsT...>(), 2);
    }
};

//! Pushes a typed return value onto the Lua stack.
template<typename ReturnT, typename TupleT>
struct LuaReturn {
    static void Push(lua_State* L, ReturnT value, TupleT&) {
	LuaValue<ReturnT>::Push(L, std::move(value));
    }
};

template<typename TupleT>
struct LuaReturn<void, TupleT> {
    static void Push(lua_State*, TupleT&) {
    }
};

//! Invokes a typed function and translates C++ exceptions to Lua errors.
template<typename FunctionT>
static int SafeInvoke(lua_State* L, FunctionT function) {
    try {
	return function();
    } catch (const std::exception& exception) {
	return luaL_error(L, "%s", exception.what());
    } catch (...) {
	return luaL_error(L, "unknown C++ exception");
    }
}

template<typename ClassT, std::size_t OptionalCount, typename SignatureT>
struct MemberInvoker;

template<typename ClassT, std::size_t OptionalCount, typename ReturnT,
    typename... ArgsT>
struct MemberInvoker<
	ClassT, OptionalCount, ReturnT (ClassT::*)(ArgsT...)> {
    using SignatureT = ReturnT (ClassT::*)(ArgsT...);
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	const int minimum = static_cast<int>(
	    sizeof...(ArgsT) - OptionalCount + 1);
	const int maximum = static_cast<int>(sizeof...(ArgsT) + 1);
	if (lua_gettop(L) < minimum || lua_gettop(L) > maximum)
	    return luaL_error(L, "invalid argument count");
	auto object = Lua::CheckWeakUserdata<ClassT>(
	    L, ClassBinding<ClassT>::MetaName(), "invalid userdata");
	auto arguments = Arguments::template ReadOptional<OptionalCount>(L);
	auto method = *static_cast<SignatureT*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	return Invoke(L, *object, arguments, method,
	    std::index_sequence_for<ArgsT...>());
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, object, arguments, method, std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (object.*method)(std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (object.*method)(
		std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

template<typename ClassT, std::size_t OptionalCount, typename ReturnT,
    typename... ArgsT>
struct MemberInvoker<
	ClassT, OptionalCount, ReturnT (ClassT::*)(ArgsT...) const> {
    using SignatureT = ReturnT (ClassT::*)(ArgsT...) const;
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	const int minimum = static_cast<int>(
	    sizeof...(ArgsT) - OptionalCount + 1);
	const int maximum = static_cast<int>(sizeof...(ArgsT) + 1);
	if (lua_gettop(L) < minimum || lua_gettop(L) > maximum)
	    return luaL_error(L, "invalid argument count");
	auto object = Lua::CheckWeakUserdata<ClassT>(
	    L, ClassBinding<ClassT>::MetaName(), "invalid userdata");
	auto arguments = Arguments::template ReadOptional<OptionalCount>(L);
	auto method = *static_cast<SignatureT*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	return Invoke(L, *object, arguments, method,
	    std::index_sequence_for<ArgsT...>());
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, object, arguments, method, std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (object.*method)(std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (object.*method)(
		std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

template<typename SignatureT>
struct ValueMemberInvoker;

template<typename ClassT, typename ReturnT, typename... ArgsT>
struct ValueMemberInvoker<ReturnT (ClassT::*)(ArgsT...)> {
    using SignatureT = ReturnT (ClassT::*)(ArgsT...);
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	if (lua_gettop(L) != static_cast<int>(sizeof...(ArgsT) + 1))
	    return luaL_error(L, "invalid argument count");
	auto* object = static_cast<ClassT*>(
	    luaL_checkudata(L, 1, ClassBinding<ClassT>::MetaName()));
	auto arguments = Arguments::template ReadOptionalRoot<0>(L);
	auto method = *static_cast<SignatureT*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	return Invoke(L, *object, arguments, method,
	    std::index_sequence_for<ArgsT...>());
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, object, arguments, method, std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (object.*method)(std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (object.*method)(
		std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

template<typename ClassT, typename ReturnT, typename... ArgsT>
struct ValueMemberInvoker<ReturnT (ClassT::*)(ArgsT...) const> {
    using SignatureT = ReturnT (ClassT::*)(ArgsT...) const;
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	if (lua_gettop(L) != static_cast<int>(sizeof...(ArgsT) + 1))
	    return luaL_error(L, "invalid argument count");
	auto* object = static_cast<const ClassT*>(
	    luaL_checkudata(L, 1, ClassBinding<ClassT>::MetaName()));
	auto arguments = Arguments::template ReadOptionalRoot<0>(L);
	auto method = *static_cast<SignatureT*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	return Invoke(L, *object, arguments, method,
	    std::index_sequence_for<ArgsT...>());
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, object, arguments, method, std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (object.*method)(std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (object.*method)(
		std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

template<typename SignatureT>
struct GameMemberInvoker;

template<typename SignatureT>
struct OptionalMethod;

template<typename ReturnT, typename... ArgsT>
struct GameMemberInvoker<ReturnT (Game::*)(ArgsT...)> {
    using SignatureT = ReturnT (Game::*)(ArgsT...);
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	if (lua_gettop(L) != static_cast<int>(sizeof...(ArgsT)))
	    return luaL_error(L, "invalid argument count");
	auto arguments = Arguments::template ReadOptionalRoot<0>(L);
	auto method = *static_cast<SignatureT*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	return Invoke(L, Lua::CheckGame(L), arguments, method,
	    std::index_sequence_for<ArgsT...>());
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    Game& game,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, game, arguments, method, std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    Game& game,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (game.*method)(std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    Game& game,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (game.*method)(
		std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

template<typename ReturnT, typename... ArgsT>
struct GameMemberInvoker<ReturnT (Game::*)(ArgsT...) const> {
    using SignatureT = ReturnT (Game::*)(ArgsT...) const;
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	if (lua_gettop(L) != static_cast<int>(sizeof...(ArgsT)))
	    return luaL_error(L, "invalid argument count");
	auto arguments = Arguments::template ReadOptionalRoot<0>(L);
	auto method = *static_cast<SignatureT*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	return Invoke(L, Lua::CheckGame(L), arguments, method,
	    std::index_sequence_for<ArgsT...>());
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    const Game& game,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, game, arguments, method, std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const Game& game,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (game.*method)(std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const Game& game,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (game.*method)(
		std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

template<std::size_t OptionalCount, typename SignatureT>
struct OptionalGameMemberInvoker;

template<std::size_t OptionalCount, typename ReturnT, typename... ArgsT>
struct OptionalGameMemberInvoker<
	OptionalCount, ReturnT (Game::*)(ArgsT...)> {
    using SignatureT = ReturnT (Game::*)(ArgsT...);
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	auto* method = static_cast<OptionalMethod<SignatureT>*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	const int minimum = static_cast<int>(
	    sizeof...(ArgsT) - OptionalCount);
	const int maximum = static_cast<int>(sizeof...(ArgsT));
	if (lua_gettop(L) < minimum || lua_gettop(L) > maximum)
	    return luaL_error(L, "invalid argument count");
	auto arguments = Arguments::template ReadOptionalRoot<OptionalCount>(
	    L, method->defaults);
	return GameMemberInvoker<SignatureT>::Invoke(
	    L, Lua::CheckGame(L), arguments, method->function,
	    std::index_sequence_for<ArgsT...>());
    }
};

template<typename SignatureT>
struct OptionalMethod {
    template<typename... OptionalArgsT>
    OptionalMethod(SignatureT method, OptionalArgsT... optional) :
	function(method),
	defaults{optional...} {
    }

    SignatureT function;
    std::vector<OptionalArgument> defaults;
};

template<typename ClassT, std::size_t OptionalCount, typename SignatureT>
struct OptionalMemberInvoker;

template<typename ClassT, std::size_t OptionalCount, typename ReturnT,
    typename... ArgsT>
struct OptionalMemberInvoker<
	ClassT, OptionalCount, ReturnT (ClassT::*)(ArgsT...)> {
    using SignatureT = ReturnT (ClassT::*)(ArgsT...);
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	auto* method = static_cast<OptionalMethod<SignatureT>*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	const int minimum = static_cast<int>(
	    sizeof...(ArgsT) - OptionalCount + 1);
	const int maximum = static_cast<int>(sizeof...(ArgsT) + 1);
	if (lua_gettop(L) < minimum || lua_gettop(L) > maximum)
	    return luaL_error(L, "invalid argument count");
	auto object = Lua::CheckWeakUserdata<ClassT>(
	    L, ClassBinding<ClassT>::MetaName(), "invalid userdata");
	auto arguments = Arguments::template ReadOptional<OptionalCount>(
	    L, method->defaults);
	auto value = Invoke(L, *object, arguments, method->function,
	    std::index_sequence_for<ArgsT...>());
	return value;
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, object, arguments, method, std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (object.*method)(std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (object.*method)(
		std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

template<typename ClassT, std::size_t OptionalCount, typename ReturnT,
    typename... ArgsT>
struct OptionalMemberInvoker<
	ClassT, OptionalCount, ReturnT (ClassT::*)(ArgsT...) const> {
    using SignatureT = ReturnT (ClassT::*)(ArgsT...) const;
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	auto* method = static_cast<OptionalMethod<SignatureT>*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	const int minimum = static_cast<int>(
	    sizeof...(ArgsT) - OptionalCount + 1);
	const int maximum = static_cast<int>(sizeof...(ArgsT) + 1);
	if (lua_gettop(L) < minimum || lua_gettop(L) > maximum)
	    return luaL_error(L, "invalid argument count");
	auto object = Lua::CheckWeakUserdata<ClassT>(
	    L, ClassBinding<ClassT>::MetaName(), "invalid userdata");
	auto arguments = Arguments::template ReadOptional<OptionalCount>(
	    L, method->defaults);
	return Invoke(L, *object, arguments, method->function,
	    std::index_sequence_for<ArgsT...>());
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, object, arguments, method, std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (object.*method)(std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (object.*method)(
		std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

//! Invokes a typed member function with an injected Game context.
template<typename ClassT, typename SignatureT>
struct InjectedMemberInvoker;

template<typename ClassT, typename ReturnT, typename... ArgsT>
struct InjectedMemberInvoker<
	ClassT, ReturnT (ClassT::*)(const Game&, ArgsT...) const> {
    using SignatureT = ReturnT (ClassT::*)(const Game&, ArgsT...) const;
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	if (lua_gettop(L) != static_cast<int>(sizeof...(ArgsT) + 1))
	    return luaL_error(L, "invalid argument count");
	auto object = Lua::CheckWeakUserdata<ClassT>(
	    L, ClassBinding<ClassT>::MetaName(), "invalid userdata");
	auto arguments = Arguments::template ReadOptional<0>(L);
	auto method = *static_cast<SignatureT*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	return Invoke(L, *object, arguments, method,
	    std::index_sequence_for<ArgsT...>());
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, object, arguments, method, std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (object.*method)(
		Lua::CheckGame(L),
		std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (object.*method)(
		Lua::CheckGame(L),
		std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

template<typename ClassT, typename ReturnT, typename... ArgsT>
struct InjectedMemberInvoker<
	ClassT, ReturnT (ClassT::*)(Game&, ArgsT...) const> {
    using SignatureT = ReturnT (ClassT::*)(Game&, ArgsT...) const;
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	if (lua_gettop(L) != static_cast<int>(sizeof...(ArgsT) + 1))
	    return luaL_error(L, "invalid argument count");
	auto object = Lua::CheckWeakUserdata<ClassT>(
	    L, ClassBinding<ClassT>::MetaName(), "invalid userdata");
	auto arguments = Arguments::template ReadOptional<0>(L);
	auto method = *static_cast<SignatureT*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	return Invoke(L, *object, arguments, method,
	    std::index_sequence_for<ArgsT...>());
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, object, arguments, method, std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (object.*method)(
		Lua::CheckGame(L),
		std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const ClassT& object,
	    typename Arguments::Values& arguments,
	    SignatureT method,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (object.*method)(
		Lua::CheckGame(L),
		std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

//! Invokes a typed class function with an injected Game context.
template<typename ClassT, typename SignatureT>
struct ClassGameFunctionInvoker;

template<typename ClassT, typename ReturnT, typename ContextT,
    typename... ArgsT>
struct ClassGameFunctionInvoker<
	ClassT, ReturnT (*)(std::shared_ptr<ClassT>, ContextT&, ArgsT...)> {
    using SignatureT =
	ReturnT (*)(std::shared_ptr<ClassT>, ContextT&, ArgsT...);
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	if (lua_gettop(L) != static_cast<int>(sizeof...(ArgsT) + 1))
	    return luaL_error(L, "invalid argument count");
	auto object = Lua::CheckWeakUserdata<ClassT>(
	    L, ClassBinding<ClassT>::MetaName(), "invalid userdata");
	auto arguments = Arguments::template ReadOptional<0>(L);
	auto function = *static_cast<SignatureT*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	return Invoke(L, object, Lua::CheckGame(L), arguments, function,
	    std::index_sequence_for<ArgsT...>());
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    const std::shared_ptr<ClassT>& object,
	    ContextT& context,
	    typename Arguments::Values& arguments,
	    SignatureT function,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, object, context, arguments, function,
	    std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const std::shared_ptr<ClassT>& object,
	    ContextT& context,
	    typename Arguments::Values& arguments,
	    SignatureT function,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (*function)(
		object, context, std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    const std::shared_ptr<ClassT>& object,
	    ContextT& context,
	    typename Arguments::Values& arguments,
	    SignatureT function,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (*function)(
		object, context, std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

template<typename SignatureT, std::size_t OptionalCount = 0>
struct FreeFunctionInvoker;

template<std::size_t OptionalCount, typename ReturnT, typename... ArgsT>
struct FreeFunctionInvoker<
	ReturnT (*)(ArgsT...), OptionalCount> {
    using SignatureT = ReturnT (*)(ArgsT...);
    using Arguments = LuaArguments<ArgsT...>;

    static int Call(lua_State* L) {
	const int minimum = static_cast<int>(
	    sizeof...(ArgsT) - OptionalCount);
	const int maximum = static_cast<int>(sizeof...(ArgsT));
	if (lua_gettop(L) < minimum || lua_gettop(L) > maximum)
	    return luaL_error(L, "invalid argument count");
	auto arguments = Arguments::template ReadOptionalRoot<OptionalCount>(L);
	auto function = *static_cast<SignatureT*>(
	    lua_touserdata(L, lua_upvalueindex(1)));
	return Invoke(L, arguments, function,
	    std::index_sequence_for<ArgsT...>());
    }

    template<std::size_t... Index>
    static int Invoke(
	    lua_State* L,
	    typename Arguments::Values& arguments,
	    SignatureT function,
	    std::index_sequence<Index...>) {
	return InvokeResult(
	    L, arguments, function, std::index_sequence<Index...>(),
	    typename std::is_void<ReturnT>::type());
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    typename Arguments::Values& arguments,
	    SignatureT function,
	    std::index_sequence<Index...>,
	    std::true_type) {
	return SafeInvoke(L, [&]() {
	    (*function)(std::get<Index>(arguments).Get()...);
	    return 0;
	});
    }

    template<std::size_t... Index>
    static int InvokeResult(
	    lua_State* L,
	    typename Arguments::Values& arguments,
	    SignatureT function,
	    std::index_sequence<Index...>,
	    std::false_type) {
	return SafeInvoke(L, [&]() {
	    auto value = (*function)(
		std::get<Index>(arguments).Get()...);
	    LuaValue<ReturnT>::Push(L, std::move(value));
	    return 1;
	});
    }
};

template<typename T>
static int ValueGc(lua_State* L) {
    static_cast<T*>(luaL_checkudata(
	L, 1, ClassBinding<T>::MetaName()))->~T();
    return 0;
}

template<typename T>
static int NonOwningGc(lua_State* L) {
    return Lua::DestroyWeakUserdata<T>(
	L, ClassBinding<T>::MetaName());
}

}; // namespace Detail

//! Returns an injected-argument marker.
template<typename T>
inline Detail::InjectedArgument<T> Injected() {
    return Detail::InjectedArgument<T>();
}

//! Returns an inferred optional-argument marker.
inline Detail::OptionalArgument Optional() {
    return Detail::OptionalArgument();
}

//! Returns an explicit optional-argument default.
template<typename T>
Detail::OptionalArgument Optional(T value) {
    return Detail::OptionalArgument(std::move(value));
}

//! The type binding builder. \{
template<typename ClassT>
class ClassBinding {
public:
    ClassBinding(Lua& lua, const char* name) :
	lua_(lua),
	name_(name),
	valueOwned_(false) {
	auto* L = lua_.GetState();
	luaL_newmetatable(L, name_);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);
	Set("__gc", &Detail::NonOwningGc<ClassT>);
    }

    //! Selects non-owning userdata.
    ClassBinding& NonOwning() {
	valueOwned_ = false;
	Set("__gc", &Detail::NonOwningGc<ClassT>);
	return *this;
    }

    //! Selects value-owned userdata.
    ClassBinding& Value() {
	valueOwned_ = true;
	Set("__gc", &Detail::ValueGc<ClassT>);
	return *this;
    }

    template<typename ReturnT, typename... ArgsT>
    ClassBinding& Function(
	    const char* name,
	    ReturnT (ClassT::*method)(ArgsT...)) {
	SetMethod(
	    name,
	    method,
	    valueOwned_ ?
	    &Detail::ValueMemberInvoker<
		ReturnT (ClassT::*)(ArgsT...)>::Call :
	    &Detail::MemberInvoker<
		ClassT, 0, ReturnT (ClassT::*)(ArgsT...)>::Call);
	return *this;
    }

    template<typename ReturnT, typename... ArgsT>
    ClassBinding& Function(
	    const char* name,
	    ReturnT (ClassT::*method)(ArgsT...) const) {
	SetMethod(
	    name,
	    method,
	    valueOwned_ ?
	    &Detail::ValueMemberInvoker<
		ReturnT (ClassT::*)(ArgsT...) const>::Call :
	    &Detail::MemberInvoker<
		ClassT, 0, ReturnT (ClassT::*)(ArgsT...) const>::Call);
	return *this;
    }

    template<typename ReturnT, typename... ArgsT, typename... OptionalArgsT>
    ClassBinding& Function(
	    const char* name,
	    ReturnT (ClassT::*method)(ArgsT...),
	    Detail::OptionalArgument optional,
	    OptionalArgsT... optionals) {
	static_assert(
	    Detail::AllOptionalArguments<OptionalArgsT...>::value,
	    "optional arguments must use Optional()");
	static_assert(sizeof...(OptionalArgsT) + 1 <= sizeof...(ArgsT),
	    "too many optional arguments");
	SetOptionalMethod(
	    name,
	    &Detail::OptionalMemberInvoker<
		ClassT, sizeof...(OptionalArgsT) + 1,
		ReturnT (ClassT::*)(ArgsT...)>::Call,
	    method, optional, optionals...);
	return *this;
    }

    template<typename ReturnT, typename... ArgsT, typename... OptionalArgsT>
    ClassBinding& Function(
	    const char* name,
	    ReturnT (ClassT::*method)(ArgsT...) const,
	    Detail::OptionalArgument optional,
	    OptionalArgsT... optionals) {
	static_assert(
	    Detail::AllOptionalArguments<OptionalArgsT...>::value,
	    "optional arguments must use Optional()");
	static_assert(sizeof...(OptionalArgsT) + 1 <= sizeof...(ArgsT),
	    "too many optional arguments");
	SetOptionalMethod(
	    name,
	    &Detail::OptionalMemberInvoker<
		ClassT, sizeof...(OptionalArgsT) + 1,
		ReturnT (ClassT::*)(ArgsT...) const>::Call,
	    method, optional, optionals...);
	return *this;
    }

    template<typename ReturnT, typename... ArgsT>
    ClassBinding& Function(
	    const char* name,
	    ReturnT (ClassT::*method)(const Game&, ArgsT...) const,
	    Detail::InjectedArgument<Game> injected) {
	(void)injected;
	SetMethod(
	    name, method,
	    &Detail::InjectedMemberInvoker<
		ClassT,
		ReturnT (ClassT::*)(const Game&, ArgsT...) const>::Call);
	return *this;
    }

    template<typename ReturnT, typename... ArgsT>
    ClassBinding& Function(
	    const char* name,
	    ReturnT (ClassT::*method)(Game&, ArgsT...) const,
	    Detail::InjectedArgument<Game> injected) {
	(void)injected;
	SetMethod(
	    name, method,
	    &Detail::InjectedMemberInvoker<
		ClassT,
		ReturnT (ClassT::*)(Game&, ArgsT...) const>::Call);
	return *this;
    }

    template<typename ReturnT, typename... ArgsT>
    ClassBinding& Function(
	    const char* name,
	    ReturnT (*function)(ArgsT...)) {
	SetMethod(
	    name, function,
	    &Detail::FreeFunctionInvoker<ReturnT (*)(ArgsT...)>::Call);
	return *this;
    }

    template<typename ReturnT, typename ContextT, typename... ArgsT>
    typename std::enable_if<
	std::is_same<
	    typename std::remove_cv<ContextT>::type,
	    Game>::value,
	ClassBinding&>::type Function(
	    const char* name,
	    ReturnT (*function)(
		std::shared_ptr<ClassT>, ContextT&, ArgsT...),
	    Detail::InjectedArgument<Game> injected) {
	(void)injected;
	SetMethod(
	    name, function,
	    &Detail::ClassGameFunctionInvoker<
		ClassT,
		ReturnT (*)(
		    std::shared_ptr<ClassT>, ContextT&, ArgsT...)>::Call);
	return *this;
    }

    template<typename BaseT, typename ReturnT, typename... ArgsT>
    typename std::enable_if<
	std::is_base_of<BaseT, ClassT>::value &&
	!std::is_same<BaseT, ClassT>::value,
	ClassBinding&>::type Function(
	    const char* name,
	    ReturnT (BaseT::*method)(ArgsT...)) {
	return Function(
	    name,
	    static_cast<ReturnT (ClassT::*)(ArgsT...)>(method));
    }

    template<typename BaseT, typename ReturnT, typename... ArgsT>
    typename std::enable_if<
	std::is_base_of<BaseT, ClassT>::value &&
	!std::is_same<BaseT, ClassT>::value,
	ClassBinding&>::type Function(
	    const char* name,
	    ReturnT (BaseT::*method)(ArgsT...) const) {
	return Function(
	    name,
	    static_cast<ReturnT (ClassT::*)(ArgsT...) const>(method));
    }

    ClassBinding& RawFunction(
	    const char* name,
	    lua_CFunction function) {
	Set(name, function);
	return *this;
    }

    static const char* MetaName() {
	return metaName_;
    }

private:
    template<typename SignatureT>
    void SetMethod(
	    const char* name,
	    SignatureT method,
	    lua_CFunction function) {
	auto* L = lua_.GetState();
	auto* memory = lua_newuserdata(L, sizeof(SignatureT));
	new (memory) SignatureT(method);
	lua_pushcclosure(L, function, 1);
	luaL_getmetatable(L, name_);
	lua_pushvalue(L, -2);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
    }

    template<typename SignatureT, typename... OptionalArgsT>
    void SetOptionalMethod(
	    const char* name,
	    lua_CFunction function,
	    SignatureT method,
	    OptionalArgsT... optional) {
	auto* L = lua_.GetState();
	using Binding = Detail::OptionalMethod<SignatureT>;
	auto* memory = lua_newuserdata(L, sizeof(Binding));
	new (memory) Binding(method, optional...);
	lua_pushcclosure(L, function, 1);
	luaL_getmetatable(L, name_);
	lua_pushvalue(L, -2);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
    }

    void Set(const char* name, lua_CFunction function) {
	auto* L = lua_.GetState();
	luaL_getmetatable(L, name_);
	lua_pushcfunction(L, function);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
    }

    Lua& lua_;
    const char* name_;
    bool valueOwned_;

public:
    static const char* metaName_;
};

template<typename ClassT>
const char* ClassBinding<ClassT>::metaName_ = nullptr;

template<typename ClassT>
ClassBinding<ClassT> Lua::Class(const char* name) {
    ClassBinding<ClassT>::metaName_ = name;
    return ClassBinding<ClassT>(*this, name);
}

template<typename ReturnT, typename... ArgsT>
void Lua::Function(
	const char* name,
	ReturnT (Game::*function)(ArgsT...)) {
    auto* L = GetState();
    using SignatureT = ReturnT (Game::*)(ArgsT...);
    auto* memory = lua_newuserdata(L, sizeof(SignatureT));
    new (memory) SignatureT(function);
    lua_pushcclosure(
	L, &Detail::GameMemberInvoker<SignatureT>::Call, 1);
    SetSafeValue(name);
}

template<typename ReturnT, typename... ArgsT, typename... OptionalArgsT>
void Lua::Function(
	const char* name,
	ReturnT (Game::*function)(ArgsT...),
	Detail::OptionalArgument optional,
	OptionalArgsT... optionals) {
    static_assert(
	Detail::AllOptionalArguments<OptionalArgsT...>::value,
	"optional arguments must use Optional()");
    static_assert(sizeof...(OptionalArgsT) + 1 <= sizeof...(ArgsT),
	"too many optional arguments");
    auto* L = GetState();
    using SignatureT = ReturnT (Game::*)(ArgsT...);
    using MethodT = Detail::OptionalMethod<SignatureT>;
    auto* memory = lua_newuserdata(L, sizeof(MethodT));
    new (memory) MethodT(function, optional, optionals...);
    lua_pushcclosure(
	L,
	&Detail::OptionalGameMemberInvoker<
	    sizeof...(OptionalArgsT) + 1, SignatureT>::Call,
	1);
    SetSafeValue(name);
}

template<typename ReturnT, typename... ArgsT>
void Lua::Function(
	const char* name,
	ReturnT (Game::*function)(ArgsT...) const) {
    auto* L = GetState();
    using SignatureT = ReturnT (Game::*)(ArgsT...) const;
    auto* memory = lua_newuserdata(L, sizeof(SignatureT));
    new (memory) SignatureT(function);
    lua_pushcclosure(
	L, &Detail::GameMemberInvoker<SignatureT>::Call, 1);
    SetSafeValue(name);
}

template<typename ReturnT, typename... ArgsT>
void Lua::Function(
	const char* name,
	ReturnT (*function)(ArgsT...)) {
    auto* L = GetState();
    using SignatureT = ReturnT (*)(ArgsT...);
    auto* memory = lua_newuserdata(L, sizeof(SignatureT));
    new (memory) SignatureT(function);
    lua_pushcclosure(
	L, &Detail::FreeFunctionInvoker<SignatureT>::Call, 1);
    SetSafeValue(name);
}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_LUA_BINDINGS_HPP_
