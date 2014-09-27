#include <luajit-2.0/lua.hpp>
#include "lua/event.hpp"
#include "hopmod.hpp"
#include <iostream>
#include <asio.hpp>
using namespace asio;

static io_service main_io_service;

io_service & get_main_io_service()
{
    return main_io_service;
}

int main(int argc, char ** argv)
{
    if(argc == 1)
    {
        std::cerr<<"Usage: "<<argv[0]<<" filename"<<std::endl;
        return 1;
    }
    
    const char * script = argv[1];
    
    init_lua();
    lua_State * L = get_lua_state();
    
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
    lua_setglobal(L, "arg");
    lua_remove(L, -2);
    
    if(luaL_dofile(L, script) == 1)
    {
        std::cerr<<lua_tostring(L, -1)<<std::endl;
        return 1;
    }
    
    try
    {
        main_io_service.run();
    }
    catch(const std::system_error & se)
    {
        std::cerr<<se.what()<<std::endl;
        throw;
    }
    
    return 0;
}
