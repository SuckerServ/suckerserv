#include <lua.hpp>

#ifdef WITH_MMDB

#include <vector>
#include "module.hpp"

extern "C"{
#include <maxminddb.h>
}

struct MaxMind
{
    MMDB_s db;
    int status;
};
std::vector<MaxMind*> maxMinds;

static int lookup_ip(lua_State * L)
{
    int gai_error, mmdb_error;

    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "id");

    int index = lua_tointeger(L, -1);
    lua_pop(L, 1);

    const char * ipaddr = luaL_checkstring(L, 2);
    const char * arg1 = luaL_checkstring(L, 3);
    const char * arg2 = luaL_checkstring(L, 4);

    const char * arg3;

    if(lua_gettop(L) > 4)
        arg3 = luaL_checkstring(L, 5);
    else
        arg3 = NULL;

    MaxMind *maxMind = maxMinds[index-1];

    if(maxMind->status != MMDB_SUCCESS) return luaL_error(L, "missing MMDB database");

    MMDB_lookup_result_s result = MMDB_lookup_string(&maxMind->db, ipaddr, &gai_error, &mmdb_error);

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

static int close_mmdb(lua_State * L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "id");

    int index = lua_tointeger(L, -1);
    lua_pop(L, 1);

    MaxMind *maxMind = maxMinds[index-1];
    if(maxMind)
    {
        if(maxMind->status == MMDB_SUCCESS)
            MMDB_close(&maxMind->db);
        delete maxMind;
        maxMinds.erase(maxMinds.begin()+index-1);
    }
    return 1;
}

static int load_mmdb_database(lua_State * L)
{
    MaxMind *maxMind = new MaxMind();
    maxMind->status = -1;

    const char * filename = luaL_checkstring(L, 1);
    maxMind->status = MMDB_open(filename, MMDB_MODE_MMAP, &maxMind->db);

    if (maxMind->status != MMDB_SUCCESS) {
        delete maxMind;
        return luaL_error(L, MMDB_strerror(maxMind->status));
    }

    maxMinds.push_back(maxMind);

    lua_createtable(L, 0, 1);

    lua_pushnumber(L, maxMinds.size());
    lua_setfield(L, -2, "id");

    lua_pushcfunction(L, lookup_ip);
    lua_setfield(L, -2, "lookup_ip");

    lua_pushcfunction(L, close_mmdb);
    lua_setfield(L, -2, "close");

    return 1;
}

static int shutdown_mmdb(lua_State * L)
{
    for(auto maxMind : maxMinds)
    {
        if(maxMind->status == MMDB_SUCCESS) MMDB_close(&maxMind->db);
        delete maxMind;
    }
    maxMinds.clear();
    return 0;
}

namespace lua{
namespace module{

int open_mmdb(lua_State * L)
{
    static luaL_Reg functions[] = {
        {"load_mmdb_database", load_mmdb_database},
        {"lookup_ip", lookup_ip},
        {"close_mmdb", close_mmdb},
        {NULL, NULL}
    };
    
    luaL_register(L, "mmdb", functions);
    lua_pop(L, 1);
    
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

static int close_mmdb(lua_State * L)
{
    return 1;
}

void open_mmdb(lua_State * L)
{
    static luaL_Reg functions[] = {
        {"load_mmdb_database", load_mmdb_database},
        {"lookup_ip", lookup_ip},
        {"close_mmdb", close_mmdb},
        {NULL, NULL}
    };
    
    luaL_newlib(L, functions);
}

} //namespace module
} //namespace lua 

#endif
