#ifndef HOPMOD_LUA_NET_WEAK_REF_HPP
#define HOPMOD_LUA_NET_WEAK_REF_HPP

#include <lua.hpp>
#include <memory>

namespace lua{

class weak_ref
{
public:
    weak_ref(std::weak_ptr<int>);
    static weak_ref create(lua_State * L);
    bool is_expired()const;
    void get(lua_State * L);
    void unref(lua_State * L);
private:
    std::weak_ptr<int> m_ref;
};

} //namespace lua

#endif

