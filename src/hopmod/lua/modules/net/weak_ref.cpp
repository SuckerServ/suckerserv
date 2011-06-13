#include <boost/shared_ptr.hpp>
#include <cassert>
#include "weak_ref.hpp"

namespace lua{

weak_ref::weak_ref(boost::weak_ptr<int> ref)
 :m_ref(ref)
{
    
}

static int __gc(lua_State * L)
{
    reinterpret_cast<boost::shared_ptr<int> *>(lua_touserdata(L, 1))->~shared_ptr<int>();
    return 0;
}

weak_ref weak_ref::create(lua_State * L)
{
    boost::shared_ptr<int> ref(new int(LUA_NOREF));
    
    lua_newtable(L);
    
    lua_pushinteger(L, 1);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    
    lua_pushinteger(L, 2);
    new (lua_newuserdata(L, sizeof(boost::shared_ptr<int>))) boost::shared_ptr<int>(ref);
    
    lua_newtable(L);
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, __gc);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
    
    lua_settable(L, -3);
    
    *ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_pop(L, 1);
    
    return weak_ref(boost::weak_ptr<int>(ref));
}

bool weak_ref::is_expired()const
{
    return m_ref.expired();
}

void weak_ref::get(lua_State * L)
{
    assert(!is_expired());
    lua_rawgeti(L, LUA_REGISTRYINDEX, *(m_ref.lock()));
    lua_pushinteger(L, 1);
    lua_gettable(L, -2);
    lua_replace(L, -2);
}

void weak_ref::unref(lua_State * L)
{
    assert(!is_expired());
    luaL_unref(L, LUA_REGISTRYINDEX, *(m_ref.lock()));
}

} //namespace lua

