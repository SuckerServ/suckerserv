#include <iostream>
#include <fungu/script.hpp>
using namespace fungu;
#include <boost/asio.hpp>
using namespace boost::asio;
extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include "lua/modules.hpp"

extern "C"{
int lua_packlibopen(lua_State *L);
}

static io_service main_io_service;
static lua_State * L = NULL;
static script::env cubescript_env;

io_service & get_main_io_service()
{
    return main_io_service;
}

void report_script_error(const char * msg)
{
    std::cerr<<msg<<std::endl;
}

script::env & get_script_env()
{
    return cubescript_env;
}

int main(int argc, char ** argv)
{
    if(argc == 1)
    {
        std::cerr<<"Usage: "<<argv[0]<<" filename"<<std::endl;
        return 1;
    }
    
    const char * script = argv[1];
    
    L = luaL_newstate();
    luaL_openlibs(L);
    
    lua::module::open_net(L);
    lua::module::open_timer(L);
    lua::module::open_crypto(L);
    lua::module::open_cubescript(L);
    lua::module::open_geoip(L);
    lua::module::open_filesystem(L);
    lua_packlibopen(L);
    
    switch(luaL_loadfile(L, script))
    {
        case 0: //success
            break;
        case LUA_ERRFILE:
        case LUA_ERRSYNTAX:
        case LUA_ERRMEM:
            std::cerr<<lua_tostring(L, -1)<<std::endl;
            return 1;
        default:;
    }
    
    // Create and fill arg table
    lua_newtable(L);
    for(int i = 2; i < argc; i++)
    {
        lua_pushinteger(L, i - 1);
        lua_pushstring(L, argv[i]);
        lua_settable(L, -3);
    }
    lua_setfield(L, LUA_GLOBALSINDEX, "arg");
    
    if(luaL_dofile(L, script) == 1)
    {
        std::cerr<<lua_tostring(L, -1)<<std::endl;
        return 1;
    }
    
    try
    {
        main_io_service.run();
    }
    catch(const boost::system::system_error & se)
    {
        std::cerr<<se.what()<<std::endl;
        throw;
    }
    
    return 0;
}
