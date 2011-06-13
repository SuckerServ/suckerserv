#include "ipmask.hpp"
#include "../../register_class.hpp"
#include "../../to.hpp"
#include "../../create_object.hpp"
#include <typeinfo>

namespace lua{

const char * ipmask::CLASS_NAME = "ipmask";

int ipmask::register_class(lua_State * L){
    static luaL_Reg member_functions[] = {
        {"__gc", &ipmask::__gc},
        {"__tostring", &ipmask::__tostring},
        {"__eq", &ipmask::__eq},
        {"to_string", &ipmask::__tostring},
        {NULL, NULL}
    };
    lua::register_class(L, CLASS_NAME, member_functions);
    return 0;
}

int ipmask::create_object(lua_State * L)
{
    hopmod::ip::address_prefix value;
    
    int argc = lua_gettop(L);
    if(argc == 0) luaL_error(L, "missing argument");
    int arg1_type = lua_type(L, 1);
    
    if(arg1_type == LUA_TSTRING)
    {
        try
        {
            value = hopmod::ip::address_prefix::parse(lua_tostring(L, 1));
        }
        catch(std::bad_cast)
        {
            return luaL_argerror(L, 1, "invalid ip prefix");
        }
    }
    else if(arg1_type == LUA_TNUMBER)
    {
        unsigned long prefix = static_cast<unsigned long>(lua_tointeger(L, 1));
        int bits = 32;
        
        if(argc > 1 && lua_isnumber(L, 2)) 
        {
            bits = lua_tointeger(L, 2);
            if(bits < 1 || bits > 32) luaL_argerror(L, 2, "invalid value");
        }
        
        value = hopmod::ip::address_prefix(hopmod::ip::address(prefix), bits);
    }
    else value = *lua::to<ipmask>(L, 1);
    
    lua::create_object<ipmask>(L, value);
    return 1;
}

int ipmask::__gc(lua_State * L)
{
    lua::to<ipmask>(L, 1)->~target_type();
    return 0;
}

int ipmask::__tostring(lua_State * L)
{
    target_type * self = lua::to<ipmask>(L, 1);
    std::string ipmask_string = self->to_string();
    lua_pushlstring(L, ipmask_string.c_str(), ipmask_string.length());
    return 1;
}

int ipmask::__eq(lua_State * L)
{
    target_type * self = lua::to<ipmask>(L, 1);
    target_type * another = lua::to<ipmask>(L, 2);
    lua_pushboolean(L, *self == *another);
    return 1;
}

const char * ipmask_table::CLASS_NAME = "ipmask_table";

int ipmask_table::register_class(lua_State * L){
    static luaL_Reg member_functions[] = {
        {"__gc", &ipmask_table::__gc},
        {"__index", &ipmask_table::__index},
        {"__newindex", &ipmask_table::__newindex},
        {NULL, NULL}
    };
    lua::register_class(L, CLASS_NAME, member_functions);
    return 0;
}

int ipmask_table::create_object(lua_State * L)
{
    ipmask_table::target_type * self = lua::create_object<ipmask_table>(L);
    lua_newtable(L);
    self->ref_table = luaL_ref(L, LUA_REGISTRYINDEX);
    return 1;
}

int ipmask_table::__gc(lua_State * L)
{
    ipmask_table::target_type * self = lua::to<ipmask_table>(L, 1);
    luaL_unref(L, LUA_REGISTRYINDEX, self->ref_table);
    self->~target_type();
    return 0;
}

int ipmask_table::__index(lua_State * L)
{
    target_type * self = lua::to<ipmask_table>(L, 1);
    
    hopmod::ip::address_prefix key;
    
    if(lua_type(L, 2) == LUA_TSTRING)
    {
        try
        {
            key = hopmod::ip::address_prefix::parse(lua_tostring(L, 2));
        }
        catch(std::bad_cast)
        {
            luaL_argerror(L, 2, "invalid ip prefix");
            return 0;
        }
    }
    else key = *lua::to<ipmask>(L, 2);
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->ref_table);
    
    lua_newtable(L);
    int index = 1;
    
    hopmod::ip::address_prefix_tree<int, -1> * child = &self->tree;
    while((child = child->next_match(&key)))
    {
        lua_pushinteger(L, index++);
        lua_rawgeti(L, -3, child->value());
        lua_settable(L, -3);
    }
    
    return 1;
}

int ipmask_table::__newindex(lua_State * L)
{
    target_type * self = lua::to<ipmask_table>(L, 1);
    
    hopmod::ip::address_prefix key;
    
    int arg2_type = lua_type(L, 2);
    
    if(arg2_type == LUA_TSTRING)
    {
        try
        {
            key = hopmod::ip::address_prefix::parse(lua_tostring(L, 2));
        }
        catch(std::bad_cast)
        {
            luaL_argerror(L, 2, "invalid ip prefix");
        }
    }
    else if(arg2_type == LUA_TNUMBER)
    {
        unsigned long prefix = static_cast<unsigned long>(lua_tointeger(L, 2));
        key = hopmod::ip::address_prefix(hopmod::ip::address(prefix), 32);
    }
    else key = *lua::to<ipmask>(L, 2);
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->ref_table);
    
    if(lua_type(L, 3) != LUA_TNIL)
    {
        lua_pushvalue(L, 3);
        self->tree.insert(key, luaL_ref(L, -2));
    }
    else self->tree.erase(key);
    
    return 0;
}

} //namespace lua

