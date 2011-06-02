#include "../utils.hpp"

#include <ctime>

extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/asio.hpp>
using namespace boost::asio;

boost::asio::io_service & get_main_io_service();
void report_script_error(const char *);

class deadline_timer_wrapper:public deadline_timer
{
public:
    deadline_timer_wrapper():deadline_timer(get_main_io_service()){}
    
    static int register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        static luaL_Reg funcs[] = {
            {"__gc", &deadline_timer_wrapper::__gc},
            {"cancel", &deadline_timer_wrapper::_cancel},
            {"expires_at", &deadline_timer_wrapper::_expires_at},
            {"expires_from_now", &deadline_timer_wrapper::_expires_from_now},
            {"async_wait", &deadline_timer_wrapper::_async_wait},
            {NULL, NULL}
        };
        
        lua_pushvalue(L, -1);
        luaL_register(L, NULL, funcs);
        
        lua_setfield(L, -1, "__index");
        
        return 0;
    }
    
    static int create(lua_State * L)
    {
        new (lua_newuserdata(L, sizeof(deadline_timer_wrapper))) deadline_timer_wrapper();
        luaL_getmetatable(L, MT);
        lua_setmetatable(L, -2);
        return 1;
    }
private:
    static const char * MT;
    
    static int __gc(lua_State * L)
    {
        reinterpret_cast<deadline_timer_wrapper *>(luaL_checkudata(L, 1, MT))->~deadline_timer_wrapper();
        return 0;
    }
    
    static int _cancel(lua_State * L)
    {
        reinterpret_cast<deadline_timer_wrapper *>(luaL_checkudata(L, 1, MT))->cancel();
        return 0;
    }
    
    static int _expires_at(lua_State * L)
    {
        deadline_timer_wrapper * object = reinterpret_cast<deadline_timer_wrapper *>(luaL_checkudata(L, 1, MT));
        if(lua_gettop(L) < 2)
        {
            std::tm t = boost::posix_time::to_tm(object->expires_at());
            return std::mktime(&t);
        }
        std::time_t expire_time = static_cast<std::time_t>(luaL_checkinteger(L, 2));
        object->expires_at(boost::posix_time::from_time_t(expire_time));
        return 0;
    }
    
    static int _expires_from_now(lua_State * L)
    {
        deadline_timer_wrapper * object = reinterpret_cast<deadline_timer_wrapper *>(luaL_checkudata(L, 1, MT));
        if(lua_gettop(L) < 2) return object->expires_from_now().total_milliseconds();
        int countdown = luaL_checkinteger(L, 2);
        if(countdown < 0) return luaL_argerror(L, 2, "invalid milliseconds-duration value");
        object->expires_from_now(boost::posix_time::millisec(countdown));
        return 0;
    }
    
    static int _async_wait(lua_State * L)
    {
        deadline_timer_wrapper * object = reinterpret_cast<deadline_timer_wrapper *>(luaL_checkudata(L, 1, MT));
        
        luaL_checktype(L, 2, LUA_TFUNCTION);
        lua_pushvalue(L, 2);
        int funref = luaL_ref(L, LUA_REGISTRYINDEX);
        
        object->async_wait(boost::bind(&deadline_timer_wrapper::timer_handler, L, funref, _1));
        
        return 0;
    }
    
    static void timer_handler(lua_State * L, int funref, const boost::system::error_code & error)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, funref);
        luaL_unref(L, LUA_REGISTRYINDEX, funref);
        
        int nargs = 0;
        
        if(error)
        {
            nargs = 1;
            lua_pushstring(L, error.message().c_str());
        }
        
        if(lua_pcall(L, nargs, 0, 0) != 0)
            report_script_error(lua_tostring(L, -1));
    }
};

const char * deadline_timer_wrapper::MT = "deadline_timer_class";

class usec_timer_wrapper
{
public:
    static const char * MT;

    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        static luaL_Reg funcs[] = {
            {"__gc", &usec_timer_wrapper::__gc},
            {"elapsed", &usec_timer_wrapper::elapsed},
            {NULL, NULL}
        };
        
        lua_pushvalue(L, -1);
        luaL_register(L, NULL, funcs);
        
        lua_setfield(L, -1, "__index");
    }

    static int create(lua_State * L)
    {
        new (lua_newuserdata(L, sizeof(usec_timer_wrapper))) usec_timer_wrapper();
        luaL_getmetatable(L, MT);
        lua_setmetatable(L, -2);
        return 1;
    }
private:
    static int elapsed(lua_State * L)
    {
        usec_timer_wrapper * timer = reinterpret_cast<usec_timer_wrapper *>(luaL_checkudata(L, 1, MT));
        lua_pushnumber(L, timer->m_timer.usec_elapsed());
        return 1;
    }
    
    static int __gc(lua_State * L)
    {
        reinterpret_cast<usec_timer_wrapper *>(luaL_checkudata(L, 1, MT))->~usec_timer_wrapper();
        return 0;
    }
    
    timer m_timer;
};

const char * usec_timer_wrapper::MT = "usec_timer_class";

namespace lua{
namespace module{

void open_timer(lua_State * L)
{
    static luaL_Reg functions[] = {
        {"create", deadline_timer_wrapper::create},
        {"usec_timer", usec_timer_wrapper::create},
        {NULL, NULL}
    };
    
    luaL_register(L, "timer", functions);
    
    deadline_timer_wrapper::register_class(L);
    usec_timer_wrapper::register_class(L);
}

}//namespace module
}//namespace lua
