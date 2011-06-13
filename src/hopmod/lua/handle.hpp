#ifndef HOPMOD_LUA_HANDLE_HPP
#define HOPMOD_LUA_HANDLE_HPP

namespace lua{

class callback_function
{
public:
    callback_function(lua_State *);
    
private:
    lua_State * m_state;
    int m_table_ref;
};

} //namespace lua

#endif

