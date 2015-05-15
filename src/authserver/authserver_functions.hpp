#ifndef HOPMOD_EXTAPI_HPP
#define HOPMOD_EXTAPI_HPP

#include "utils.hpp"

extern "C"{
#include <luajit-2.0/lua.h>
}

#include <string>
#include <vector>

namespace authserver
{
    extern void adduser(const char *name, const char *desc, const char *pubkey, const char *priv);
    extern void deleteuser(const char *name, const char *desc);
    extern void clearusers();
    extern void log_status(const char * msg);
    extern void log_error(const char * msg);
    extern void shutdown();

    int lua_user_list(lua_State *);

    extern string ip;
    extern int port;
    extern bool debug;
} //namespace server

#endif
