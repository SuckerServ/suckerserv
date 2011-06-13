#include "event.hpp"
#include <iostream>
#include <sstream>

namespace lua{

event_base::event_base(const char * text_id, int numeric_id)
 :m_text_id(text_id), 
  m_numeric_id(numeric_id)
{
    if(m_numeric_id == -1)
        m_numeric_id = assign_numeric_id();
}

const char * event_base::text_id()const
{
    return m_text_id;
}

int event_base::numeric_id()const
{
    return m_numeric_id;
}

int event_base::assign_numeric_id()
{
    static int id = 0;
    id++;
    return id;
}

event_environment::event_environment(lua_State * L, void (* log_error_function)(const char *))
 :m_state(L),
  m_log_error_function(log_error_function)
{
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "event");
    m_text_id_index = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_newtable(L);
    m_numeric_id_index = luaL_ref(L, LUA_REGISTRYINDEX);
}

event_environment::event_environment()
 :m_state(NULL), m_log_error_function(NULL)
{
    
}

void event_environment::register_event_idents(event_base ** events)
{
    lua_State * L = m_state;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_text_id_index);
    assert(lua_type(L, -1) == LUA_TTABLE);
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_numeric_id_index);
    assert(lua_type(L, -1) == LUA_TTABLE);
    
    for(event_base ** event = events; *event; event++)
    {
        lua_newtable(L);
           
        lua_newtable(L);
        lua_pushliteral(L, "event_id");
        lua_pushstring(L, (*event)->text_id());
        lua_settable(L, -3);
        lua_setmetatable(L, -2);
        
        lua_pushstring(L, (*event)->text_id());
        lua_pushvalue(L, -2);
        lua_settable(L, -5); // new index in text_id_index table
        
        lua_pushinteger(L, (*event)->numeric_id());
        lua_pushvalue(L, -2);
        lua_settable(L, -4); // new index in numeric_id_index table
        
        lua_pop(L, 1);
    }
    
    lua_pop(L, 2);
}

lua_State * event_environment::push_listeners_table(const char * text_id, int num_id)
{
    assert(is_ready());
    
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_numeric_id_index);
    assert(lua_type(m_state, -1) == LUA_TTABLE);
    
    int bottom = lua_gettop(m_state);
    
    lua_pushinteger(m_state, num_id);
    lua_gettable(m_state, -2);
    assert(lua_type(m_state, -1) == LUA_TTABLE);    
    lua_replace(m_state, bottom);
    
    return m_state;
}

void event_environment::log_error(const char * text_id, const char * error_message)
{
    assert(is_ready());
    
    std::stringstream format;
    format<<"Error on "<<text_id<<" event: "<<error_message<<std::endl;
    
    if(m_log_error_function)
        m_log_error_function(format.str().c_str());
    else std::cerr<<format.str();
}

bool event_environment::is_ready()
{
    return m_state;
}

void event_environment::add_listener(const char * event_id)
{
    assert(lua_type(m_state, -1) == LUA_TFUNCTION);
    
    lua_State * L = m_state;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_text_id_index);
    lua_pushstring(L, event_id);
    lua_gettable(L, -2);
    
    if(lua_type(L, -1) != LUA_TTABLE) return;

    std::size_t listener_table_length = lua_objlen(L, -1);
    
    lua_pushinteger(L, listener_table_length + 1);
    lua_pushvalue(L, -4);
    lua_settable(L, -3);
    
    lua_pop(L, 3);
}

void event_environment::add_listener(const char * event_id, lua_CFunction function)
{
    lua_pushcfunction(m_state, function);
    add_listener(event_id);
}

void event_environment::clear_listeners(const event_base & event)
{
    lua_State * L = m_state;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_text_id_index);
    
    lua_newtable(L);
    
    lua_pushstring(L, event.text_id());
    lua_pushvalue(L, -2);
    lua_settable(L, -4);
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_numeric_id_index);
    
    lua_pushinteger(L, event.numeric_id());
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    
    lua_pop(L, 3);
}

lua_State * event_environment::lua_state()
{
    return m_state;
}

} //namespace lua

