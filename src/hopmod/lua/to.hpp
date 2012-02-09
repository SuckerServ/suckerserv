#ifndef HOPMOD_LUA_TO_HPP
#define HOPMOD_LUA_TO_HPP

#include <cassert>

namespace lua{

// Checks that the value at the given index is a WrapperClass type and then returns a pointer to the object
template<typename WrapperClass> inline
typename WrapperClass::target_type * to(lua_State * L, int index)
{
    assert(index > 0);
    
    lua_getfield(L, LUA_REGISTRYINDEX, WrapperClass::CLASS_NAME);
    int class_mt_index = lua_gettop(L);
    
    lua_pushnil(L);
    
    void * ptr = lua_touserdata(L, index);
    if(!ptr || !lua_getmetatable(L, index)) 
        luaL_typerror(L, index, WrapperClass::CLASS_NAME);
    
    do
    {
        lua_remove(L, lua_gettop(L) - 1);
        if(lua_equal(L, -1, class_mt_index))
        {
            lua_pop(L, 2);
            return reinterpret_cast<typename WrapperClass::target_type *>(ptr);
        }
    }while(lua_getmetatable(L, -1));
    
    luaL_typerror(L, index, WrapperClass::CLASS_NAME);
    return NULL;
}

} //namespace lua

#endif

