#ifndef HOPMOD_LUA_MODULES_HPP
#define HOPMOD_LUA_MODULES_HPP

namespace lua{
namespace module{

void open_net(lua_State *);
void open_net2(lua_State *);
void open_crypto(lua_State *L);
void open_cubescript(lua_State *L);
void open_geoip(lua_State *L);
void open_filesystem(lua_State *L);
void open_timer(lua_State *L);
void open_http_server(lua_State * L);

} //namespace module
} //namespace lua

extern "C" {
    int lua_packlibopen(lua_State *L);
}

#endif

