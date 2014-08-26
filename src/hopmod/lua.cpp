#include <cassert>
#include <lua.hpp>
#include <iostream>
#include "core_bindings.hpp"
#include "events.hpp"
#include "lua/modules.hpp"
#include "lua/library_extensions.hpp"
#include "lua/error_handler.hpp"

static void load_lua_modules();
static void load_extra_os_functions(lua_State *);

static lua_State * L = NULL;
static lua::event_environment * event_environment = NULL;
static int lua_stack_size = 0;

void init_lua()
{
    L = luaL_newstate();
    luaL_openlibs(L);
    
    load_extra_os_functions(L);
    
#ifndef NO_CORE_TABLE
    lua_newtable(L);
    int core_table = lua_gettop(L);
    
    bind_core_functions(L, core_table);
    
    lua_pushliteral(L, "vars");
    lua_newtable(L);
    int vars_table = lua_gettop(L);
    
    bind_core_constants(L, vars_table);
    bind_core_variables(L, vars_table);
    
    lua_settable(L, -3); // Add vars table to core table
    lua_setglobal(L, "core"); // Add core table to global table
#endif
    
    event_environment = new lua::event_environment(L, NULL);
    
#ifndef NO_EVENTS
    register_event_idents(*event_environment); // Setup and populate the event table
#endif

    load_lua_modules();
}

void shutdown_lua()
{
    if(!L) return;
    
    delete event_environment;
    lua_close(L);
    
    event_environment = NULL;
    L = NULL;
}

void load_extra_os_functions(lua_State * L)
{
    lua_getglobal(L, "os");
    
    if(lua_type(L, -1) != LUA_TTABLE)
    {
        std::cerr<<"Lua init error: the os table is not loaded"<<std::endl;
        return;
    }
    
    lua_pushliteral(L, "getcwd");
    lua_pushcfunction(L, lua::getcwd);
    lua_settable(L, -3);
    
    lua_pushliteral(L, "mkfifo");
    lua_pushcfunction(L, lua::mkfifo);
    lua_settable(L, -3);
    
    lua_pushliteral(L, "open_fifo");
    lua_pushcfunction(L, lua::open_fifo);
    lua_settable(L, -3);
    
    lua_pushliteral(L, "usleep");
    lua_pushcfunction(L, lua::usleep);
    lua_settable(L, -3);
    
    lua_pop(L, 1);
}

lua_State * get_lua_state()
{
    return L;
}

static void load_lua_modules()
{

    static const luaL_Reg loadedlibs[] = {
       	{"net", lua::module::open_net2},
       	{"timer", lua::module::open_timer},
       	{"crypto", lua::module::open_crypto},
       	{"cubescript", lua::module::open_cubescript},
        {"filesystem", lua::module::open_filesystem},
        {"mmdb", lua::module::open_mmdb},
       	{"http_server", lua::module::open_http_server},
        {NULL, NULL}
    };

    const luaL_Reg *lib;

    for (lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 0);
        lua_pop(L, 1);  /* remove lib */
    }

    lua_packlibopen(L);
}

lua::event_environment & event_listeners()
{
    static lua::event_environment unready_event_environment;
    if(!event_environment) return unready_event_environment;
    lua_stack_size = lua_gettop(L);
    return *event_environment;
}

int get_lua_stack_size()
{
    return lua_stack_size;
}

void log_event_error(const char * event_id, const char * error_message)
{
    event_listeners().log_error(event_id, error_message);
}

