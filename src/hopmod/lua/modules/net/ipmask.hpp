#ifndef HOPMOD_LUA_NET_IPMASK_HPP
#define HOPMOD_LUA_NET_IPMASK_HPP

#include "../../../net/prefix_tree.hpp"
#include <luajit-2.0/lua.hpp>

namespace lua{

class ipmask
{
public:
    typedef hopmod::ip::address_prefix target_type;
    static const char * CLASS_NAME;
    static int register_class(lua_State * L);
    static int create_object(lua_State * L);
private:
    static int __gc(lua_State * L);
    static int __tostring(lua_State * L);
    static int __eq(lua_State * L);
};

class ipmask_table
{
public:
    struct object
    {
        int ref_table;
        hopmod::ip::address_prefix_tree<int, -1> tree;
    };
    typedef object target_type;
    static const char * CLASS_NAME;
    static int register_class(lua_State * L);
    static int create_object(lua_State * L);
private:
    static int __gc(lua_State * L);
    static int __index(lua_State * L);
    static int __newindex(lua_State * L);

};

} //namespace lua

#endif

