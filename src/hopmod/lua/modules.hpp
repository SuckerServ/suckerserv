#ifndef HOPMOD_LUA_MODULES_HPP
#define HOPMOD_LUA_MODULES_HPP

namespace lua{
namespace module{

int open_net2(lua_State *);
int open_crypto(lua_State *L);
int open_cubescript(lua_State *L);
int open_mmdb(lua_State *L);
int open_filesystem(lua_State *L);
int open_timer(lua_State *L);
int open_http_server(lua_State * L);

} //namespace module
} //namespace lua

extern "C" {
    int lua_packlibopen(lua_State *L);
}

#endif

