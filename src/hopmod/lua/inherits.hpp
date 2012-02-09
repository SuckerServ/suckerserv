#ifndef HOPMOD_LUA_INHERITS_HPP
#define HOPMOD_LUA_INHERITS_HPP

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>

namespace lua{

template<typename SubclassWrapper, typename SuperclassWrapper>
void inherits(lua_State * L)
{
    //BOOST_STATIC_ASSERT(boost::is_base_of<typename SuperclassWrapper::target_type, typename SubclassWrapper::target_type>::value);
    lua_getfield(L, LUA_REGISTRYINDEX, SuperclassWrapper::CLASS_NAME);
    lua_getfield(L, LUA_REGISTRYINDEX, SubclassWrapper::CLASS_NAME);
    lua_setmetatable(L, -2);
    lua_pop(L, 1);
}

} //namespace lua

#endif

