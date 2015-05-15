#include "hopmod.hpp"
#include "cube.h"
#include "game.h"
#include "authserver_functions.hpp"
extern bool reloaded; // Defined in startup.cpp

#ifdef __APPLE__
typedef __darwin_uid_t __uid_t;
#endif //__APPLE__

/* Forward declaration of Lua value io functions */
#include "lua/push_function_fwd.hpp"
namespace lua{
void push(lua_State * L, string value);
void push(lua_State * L, __uid_t value);
} //namespace lua

#include "lua/push_function.hpp"
#include <iostream>
#include <vector>
#include <string>

/*
    Lua value io functions for cube2 types
*/
namespace lua{
void push(lua_State * L, string value)
{
    lua_pushstring(L, value);
}
} //namespace lua

template<typename FunctionPointerType>
static void bind_function(lua_State * L, int table, const char * name, FunctionPointerType function)
{
    lua_pushstring(L, name);
    lua::push_function(L, function);
    lua_settable(L, table);
}

static void bind_function(lua_State * L, int table, const char * name, lua_CFunction function)
{
    lua_pushstring(L, name);
    lua_pushcfunction(L, function);
    lua_settable(L, table);
}

void bind_core_functions(lua_State * L, int T)
{
    bind_function(L, T, "adduser", authserver::adduser);
    bind_function(L, T, "deleteuser", authserver::deleteuser);
    bind_function(L, T, "clearusers", authserver::clearusers);

    bind_function(L, T, "listusers", authserver::lua_user_list);
    
    bind_function(L, T, "log_status", authserver::log_status);
    bind_function(L, T, "log_error", authserver::log_error);
    
    bind_function(L, T, "file_exists", file_exists);
    bind_function(L, T, "dir_exists", dir_exists);
    
    bind_function(L, T, "reload_lua", reload_authserver);
    bind_function(L, T, "restart_now", restart_now);
    bind_function(L, T, "shutdown", authserver::shutdown);

    bind_function(L, T, "sleep", lua::sleep);
    bind_function(L, T, "interval", lua::interval);
    bind_function(L, T, "cancel_timer", lua::cancel_timer);

    bind_function(L, T, "enet_time_get", (int (*)())enet_time_get);
}

template<int Constant>
static int get_constant(lua_State * L)
{
    if(lua_gettop(L)) luaL_error(L, "cannot set constant");
    lua_pushinteger(L, Constant);
    return 1;
}

void bind_core_constants(lua_State * L, int T)
{
}

template<typename T, bool READ_ONLY, bool WRITE_ONLY>
static int variable_accessor(lua_State * L)
{
    T * var = reinterpret_cast<T *>(lua_touserdata(L, lua_upvalueindex(1)));
    if(lua_gettop(L) > 0) // Set variable
    {
        if(READ_ONLY) luaL_error(L, "variable is read-only");
        *var = lua::to(L, 1, lua::return_tag<T>());
        event_varchanged(event_listeners(), std::make_tuple(lua_tostring(L, lua_upvalueindex(2))));
        return 0;
    }
    else // Get variable
    {
        if(WRITE_ONLY) luaL_error(L, "variable is write-only");
        lua::push(L, *var);
        return 1;
    }
}

template<bool READ_ONLY, bool WRITE_ONLY>
static int string_accessor(lua_State * L)
{
    char * var = reinterpret_cast<char *>(lua_touserdata(L, lua_upvalueindex(1)));
    if(lua_gettop(L) > 0) // Set variable
    {
        if(READ_ONLY) luaL_error(L, "variable is read-only");
        convert2cube varcubeenc(lua_tostring(L, 1));
        copystring(var, varcubeenc.str());
        event_varchanged(event_listeners(), std::make_tuple(lua_tostring(L, lua_upvalueindex(2))));
        return 0;
    }
    else // Get variable
    {
        if(WRITE_ONLY) luaL_error(L, "variable is write-only");
        convert2utf8 varutf8(var);
        lua::push(L, varutf8.str());
        return 1;
    }
}

template<typename T>
static void bind_var(lua_State * L, int table, const char * name, T & var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, &var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, variable_accessor<T, false, false>, 2);
    lua_settable(L, table);
}

static void bind_var(lua_State * L, int table, const char * name, string var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, string_accessor<false, false>, 2);
    lua_settable(L, table);
}

template<typename T>
static void bind_ro_var(lua_State * L, int table, const char * name, T & var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, &var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, variable_accessor<T, true, false>, 2);
    lua_settable(L, table);
}

static void bind_ro_var(lua_State * L, int table, const char * name, string var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, string_accessor<true, false>, 2);
    lua_settable(L, table);
}

template<typename T>
static void bind_wo_var(lua_State * L, int table, const char * name, T & var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, &var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, variable_accessor<T, false, true>, 2);
    lua_settable(L, table);
}

static void bind_wo_var(lua_State * L, int table, const char * name, string var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, string_accessor<false, true>, 2);
    lua_settable(L, table);
}

template<typename T>
static int property_accessor(lua_State * L)
{
    T (* get)() = reinterpret_cast<T (*)()>(lua_touserdata(L, lua_upvalueindex(1)));
    void (* set)(T) = reinterpret_cast<void (*)(T)>(lua_touserdata(L, lua_upvalueindex(2)));
    if(lua_gettop(L) > 0)
    {
        if(!set) luaL_error(L, "cannot set value");
        set(lua::to(L, 1, lua::return_tag<T>()));
        event_varchanged(event_listeners(), std::make_tuple(lua_tostring(L, lua_upvalueindex(3))));
        return 0;
    }
    else
    {
        if(!get) luaL_error(L, "cannot get value");
        lua::push(L, get());
        return 1;
    }
}

template<typename T>
static void bind_prop(lua_State * L, int table, const char * name, T (* get)(), void (* set)(T))
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, reinterpret_cast<void *>(get));
    lua_pushlightuserdata(L, reinterpret_cast<void *>(set));
    lua_pushstring(L, name);
    lua_pushcclosure(L, property_accessor<T>, 3);
    lua_settable(L, table);
}

void bind_core_variables(lua_State * L, int T)
{
    bind_var(L, T, "serverport", authserver::port);
    bind_var(L, T, "serverip", authserver::ip);    
    bind_var(L, T, "debug", authserver::debug);

    bind_ro_var(L, T, "tx_bytes", tx_bytes);
    bind_ro_var(L, T, "rx_bytes", rx_bytes);
    bind_ro_var(L, T, "tx_packets", tx_packets);
    bind_ro_var(L, T, "rx_packets", rx_packets);

    bind_ro_var(L, T, "reloaded", reloaded);
}
