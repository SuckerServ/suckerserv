#include <lua.hpp>
#include "pcall.hpp"

#define ERROR_HANDLER_KEY "pcall_error_handler"

namespace cubescript{
namespace lua{

static int error_handler(lua_State * L)
{
    int argc = lua_gettop(L);
    if(!get_error_handler(L))
    {
        lua_pop(L, 1);
        return argc;
    }
    lua_insert(L, 1);
    lua_pcall(L, argc, LUA_MULTRET, 0);
    return lua_gettop(L);
}

int pcall(lua_State * L, int nargs, int nresults)
{
    lua_pushcfunction(L, error_handler);
    lua_insert(L, 1);
    int status = lua_pcall(L, nargs, nresults, 1);
    lua_remove(L, 1);
    return status;
}

void set_error_handler(lua_State * L, lua_CFunction handler_function)
{
    lua_pushliteral(L, ERROR_HANDLER_KEY);
    lua_pushcfunction(L, handler_function);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

void unset_error_handler(lua_State * L)
{
    lua_pushliteral(L, ERROR_HANDLER_KEY);
    lua_pushnil(L);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

bool get_error_handler(lua_State * L)
{
    lua_pushliteral(L, ERROR_HANDLER_KEY);
    lua_rawget(L, LUA_REGISTRYINDEX);
    return lua_type(L, -1) == LUA_TFUNCTION;
}

lua_CFunction get_pcall_error_function()
{
    return error_handler;
}

static int error_info_ref = LUA_NOREF;

int save_error_info(lua_State * L)
{
    lua_newtable(L);
    
    lua_pushliteral(L, "message");
    lua_pushvalue(L, 1);
    lua_settable(L, -3);
    
    if(error_info_ref == LUA_NOREF)
        error_info_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    else
        lua_rawseti(L, LUA_REGISTRYINDEX, error_info_ref);
    
    return 1;
}

int push_error_info(lua_State * L)
{
    if(error_info_ref == LUA_NOREF) return 0;
    lua_rawgeti(L, LUA_REGISTRYINDEX, error_info_ref);
    return 1;
}

static int traceback_ref = LUA_NOREF;

int save_traceback(lua_State * L)
{
    lua_getglobal(L, "debug");
    if(lua_type(L, -1) != LUA_TTABLE)
    {
        lua_pop(L, 1);
        return 1;
    }
    
    lua_getfield(L, -1, "traceback");
    if(lua_type(L, -1) != LUA_TFUNCTION)
    {
        lua_pop(L, 1);
        return 1;
    }
    
    lua_pushvalue(L, -3);
    lua_pushinteger(L, 2);
    lua_pcall(L, 2, 1, 0);
    
    if(traceback_ref == LUA_NOREF) 
        traceback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    else lua_rawseti(L, LUA_REGISTRYINDEX, traceback_ref);
    
    lua_pop(L, 1);
    
    return 1;
}

int push_traceback(lua_State * L)
{
    if(traceback_ref == LUA_NOREF) return 0;
    lua_rawgeti(L, LUA_REGISTRYINDEX, traceback_ref);
    return 1;
}

bool is_callable(lua_State * L, int index)
{
    int type = lua_type(L, index);
    if(type == LUA_TFUNCTION) return true;
    if(type == LUA_TUSERDATA)
    {
        lua_getmetatable(L, index);
        lua_getfield(L, -1, "__call");
        bool is_not_nil = lua_type(L, -1) != LUA_TNIL;
        lua_pop(L, 2);
        return is_not_nil;
    }
    return false;
}

} //namespace lua
} //namespace cubescript

