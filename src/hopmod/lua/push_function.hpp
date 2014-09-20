#ifndef HOPMOD_PUSH_FUNCTION_HPP
#define HOPMOD_PUSH_FUNCTION_HPP

#include <tuple>
#include <lua.hpp>
#include <string>
#include <vector>
#include "utils/function_traits.hpp"
#include "utils/tuple.hpp"

namespace lua{

template<int Arity> struct arity_tag{};
template<typename T> struct return_tag{};
template<typename T, typename FT> struct wrapper_return_tag{};

void push(lua_State *, bool);
void push(lua_State *, int);
void push(lua_State *, unsigned int);
void push(lua_State *, long);
void push(lua_State *, unsigned long);
void push(lua_State *, lua_Number);
void push(lua_State *, lua_CFunction);
void push(lua_State *, const char *);
void push(lua_State *, const char *, std::size_t);
void push(lua_State *, const std::string &);

template<typename T>
void push(lua_State * L, const std::vector<T> & value)
{
    lua_newtable(L);
    std::size_t size = value.size();
    for(unsigned int i = 0; i < size; i++)
    {
        lua_pushinteger(L, i + 1);
        push(L, value[i]);
        lua_settable(L, -3);
    }
}

inline void push(lua_State *, std::tuple<>){}

template<typename Tuple>
void push(lua_State * L, const Tuple & value)
{
    push(L, std::get<0>(value));
    push(L, utils::get_tail(value));
}

int           to(lua_State * L, int index, return_tag<int>);
unsigned int  to(lua_State * L, int index, return_tag<unsigned int>);
long          to(lua_State * L, int index, return_tag<long>);
unsigned long to(lua_State * L, int index, return_tag<unsigned long>);
lua_Number    to(lua_State * L, int index, return_tag<lua_Number>);
bool          to(lua_State * L, int index, return_tag<bool>);
const char *  to(lua_State * L, int index, return_tag<const char *>);
std::string   to(lua_State * L, int index, return_tag<std::string>);

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<typename FunctionTraits::result_type, FunctionTraits>, arity_tag<0>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    push(L, function());
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<void, FunctionTraits>, arity_tag<0>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    function();
    return 0;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<typename FunctionTraits::result_type, FunctionTraits>, arity_tag<1>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    push(L, function(to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>())));
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<void, FunctionTraits>, arity_tag<1>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    function(to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>()));
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<typename FunctionTraits::result_type, FunctionTraits>, arity_tag<2>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    push(L, function(
        to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>()),
        to(L, 2, return_tag<typename FunctionTraits::template arg<1>::type>())));
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<void, FunctionTraits>, arity_tag<2>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    function(
        to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>()),
        to(L, 2, return_tag<typename FunctionTraits::template arg<1>::type>()));
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<typename FunctionTraits::result_type, FunctionTraits>, arity_tag<3>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    push(L, function(
        to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>()),
        to(L, 2, return_tag<typename FunctionTraits::template arg<1>::type>()),
        to(L, 3, return_tag<typename FunctionTraits::template arg<2>::type>())));
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<void, FunctionTraits>, arity_tag<3>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    function(
        to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>()),
        to(L, 2, return_tag<typename FunctionTraits::template arg<1>::type>()),
        to(L, 3, return_tag<typename FunctionTraits::template arg<2>::type>()));
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<typename FunctionTraits::result_type, FunctionTraits>, arity_tag<4>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    push(L, function(
        to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>()),
        to(L, 2, return_tag<typename FunctionTraits::template arg<1>::type>()),
        to(L, 3, return_tag<typename FunctionTraits::template arg<2>::type>()),
        to(L, 4, return_tag<typename FunctionTraits::template arg<3>::type>())));
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<void, FunctionTraits>, arity_tag<4>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    function(
        to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>()),
        to(L, 2, return_tag<typename FunctionTraits::template arg<1>::type>()),
        to(L, 3, return_tag<typename FunctionTraits::template arg<2>::type>()),
        to(L, 4, return_tag<typename FunctionTraits::template arg<3>::type>()));
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<typename FunctionTraits::result_type, FunctionTraits>, arity_tag<5>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    push(L, function(
        to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>()),
        to(L, 2, return_tag<typename FunctionTraits::template arg<1>::type>()),
        to(L, 3, return_tag<typename FunctionTraits::template arg<2>::type>()),
        to(L, 4, return_tag<typename FunctionTraits::template arg<3>::type>()),
        to(L, 5, return_tag<typename FunctionTraits::template arg<4>::type>())));
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<void, FunctionTraits>, arity_tag<5>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    function(
        to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>()),
        to(L, 2, return_tag<typename FunctionTraits::template arg<1>::type>()),
        to(L, 3, return_tag<typename FunctionTraits::template arg<2>::type>()),
        to(L, 4, return_tag<typename FunctionTraits::template arg<3>::type>()),
        to(L, 5, return_tag<typename FunctionTraits::template arg<4>::type>()));
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<typename FunctionTraits::result_type, FunctionTraits>, arity_tag<6>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    push(L, function(
        to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>()),
        to(L, 2, return_tag<typename FunctionTraits::template arg<1>::type>()),
        to(L, 3, return_tag<typename FunctionTraits::template arg<2>::type>()),
        to(L, 4, return_tag<typename FunctionTraits::template arg<3>::type>()),
        to(L, 5, return_tag<typename FunctionTraits::template arg<4>::type>()),
        to(L, 5, return_tag<typename FunctionTraits::template arg<5>::type>())));
    return 1;
}

template<typename FunctionPointerType, typename FunctionTraits>
int free_func_wrapper(lua_State * L, wrapper_return_tag<void, FunctionTraits>, arity_tag<6>)
{
    FunctionPointerType function = 
        reinterpret_cast<FunctionPointerType>(
            lua_touserdata(L, lua_upvalueindex(1)));
    function(
        to(L, 1, return_tag<typename FunctionTraits::template arg<0>::type>()),
        to(L, 2, return_tag<typename FunctionTraits::template arg<1>::type>()),
        to(L, 3, return_tag<typename FunctionTraits::template arg<2>::type>()),
        to(L, 4, return_tag<typename FunctionTraits::template arg<3>::type>()),
        to(L, 5, return_tag<typename FunctionTraits::template arg<4>::type>()),
        to(L, 5, return_tag<typename FunctionTraits::template arg<5>::type>()));
    return 1;
}

template<typename FunctionPointerType>
int free_func_wrapper(lua_State * L)
{
    typedef typename std::remove_pointer<FunctionPointerType>::type FunctionType;
    typedef utils::function_traits<FunctionType> FunctionTraits;
    return free_func_wrapper<FunctionPointerType, FunctionTraits>(
        L, 
        wrapper_return_tag<typename FunctionTraits::result_type, FunctionTraits>(),
        arity_tag<FunctionTraits::arity>());
}

template<typename FunctionPointerType>
void push_function(lua_State * L, FunctionPointerType function)
{
   lua_pushlightuserdata(L, reinterpret_cast<void *>(function));
   lua_pushcclosure(L, free_func_wrapper<FunctionPointerType>, 1);
}

} //namespace lua

#endif


