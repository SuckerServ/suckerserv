#ifndef HOPMOD_LUA_CREATE_OBJECT_HPP
#define HOPMOD_LUA_CREATE_OBJECT_HPP

#include <new>
#include <luajit-2.0/lua.hpp>
#include <cassert>

namespace lua{

template<typename WrapperClass> inline
typename WrapperClass::target_type * create_object(lua_State * L, const typename WrapperClass::target_type & copy_object)
{
    typename WrapperClass::target_type * object = new (lua_newuserdata(L, sizeof(typename WrapperClass::target_type))) typename WrapperClass::target_type(copy_object);
    luaL_getmetatable(L, WrapperClass::CLASS_NAME);
    assert(lua_type(L, -1) != LUA_TNIL);
    lua_setmetatable(L, -2);
    return object;
}

template<typename WrapperClass> inline
typename WrapperClass::target_type * create_object(lua_State * L)
{
    return create_object<WrapperClass>(L, typename WrapperClass::target_type());
}

} //namespace lua

#endif

