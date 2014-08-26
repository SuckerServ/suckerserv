#include <lua.hpp>

#ifdef WITH_MMDB

#include "module.hpp"

extern "C"{
#include <maxminddb.h>
}

static MMDB_s mmdb;
static int mmdb_status = -1;

static int load_mmdb_database(lua_State * L)
{
    const char * filename = luaL_checkstring(L, 1);
    if(mmdb_status == MMDB_SUCCESS) MMDB_close(&mmdb);
    mmdb_status = MMDB_open(filename, MMDB_MODE_MMAP, &mmdb);

    if (mmdb_status != MMDB_SUCCESS) {
        return luaL_error(L, MMDB_strerror(mmdb_status));
    }

    return 1;
}

static int lookup_ip(lua_State * L)
{
    int gai_error, mmdb_error;
    if(mmdb_status != MMDB_SUCCESS) return luaL_error(L, "missing MMDB database");

    const char * ipaddr = luaL_checkstring(L, 1);
    const char * arg1 = luaL_checkstring(L, 2);
    const char * arg2 = luaL_checkstring(L, 3);

    const char * arg3;
    if(lua_gettop(L) > 3)
        arg3 = luaL_checkstring(L, 4);
    else
        arg3 = NULL;

    MMDB_lookup_result_s result = MMDB_lookup_string(&mmdb, ipaddr, &gai_error, &mmdb_error);

    if (gai_error != 0) {
        return luaL_error(L, gai_strerror(gai_error));
    }

    if (mmdb_error != MMDB_SUCCESS) {
        return luaL_error(L, MMDB_strerror(mmdb_error));
    }

    MMDB_entry_data_s entry_data;

    if (result.found_entry) {
        int status = MMDB_get_value(&result.entry, &entry_data, arg1, arg2, arg3, NULL);


        if (status != MMDB_SUCCESS) {
            return luaL_error(L, MMDB_strerror(status));
        }

        if (!entry_data.has_data || entry_data.type != MMDB_DATA_TYPE_UTF8_STRING) {
            lua_pushstring(L, "");
        }

        lua_pushlstring(L, entry_data.utf8_string, entry_data.data_size);
    } else {
        lua_pushstring(L, "");
    }

    return 1;
}

static int shutdown_mmdb(lua_State * L)
{
    if(mmdb_status == MMDB_SUCCESS) MMDB_close(&mmdb);
    mmdb_status = -1;
    return 0;
}

namespace lua{
namespace module{

int open_mmdb(lua_State * L)
{
    static luaL_Reg functions[] = {
        {"load_mmdb_database", load_mmdb_database},
        {"lookup_ip", lookup_ip},
        {NULL, NULL}
    };
    
    luaL_newlib(L, functions);
    
    lua::on_shutdown(L, shutdown_mmdb);

    return 1;
}

} //namespace module
} //namespace lua

#else

namespace lua{ 
namespace module{ 

static int load_mmdb_database(lua_State * L)
{
    lua_pushboolean(L, false);
    return 1;
}

static int lookup_ip(lua_State * L)
{
    lua_pushstring(L, "");
    return 1;
}

void open_mmdb(lua_State * L)
{
    static luaL_Reg functions[] = {
        {"load_mmdb_database", load_mmdb_database},
        {"lookup_ip", lookup_ip},
        {NULL, NULL}
    };
    
    luaL_newlib(L, functions);
}

} //namespace module
} //namespace lua 

#endif
