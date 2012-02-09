#include <boost/bind.hpp>
#include "hopmod.hpp"
#include "main_io_service.hpp"
#include "lua/modules/net/weak_ref.hpp"
#include "lua/error_handler.hpp"
using namespace boost::asio;

namespace lua{

static void async_wait_handler(lua_State * L,
    boost::shared_ptr<deadline_timer> timer, bool repeat, int countdown, lua::weak_ref callback, 
    const boost::system::error_code & ec)
{
    if(callback.is_expired()) return;
    
    lua::get_error_handler(L);
    int error_function = lua_gettop(L);
    
    callback.get(L);
    
    if(ec)
    {
        callback.unref(L);
        return;
    }
    
    if(lua::pcall(L, 0, 1, error_function) == 0)
    {
        repeat = repeat && lua_type(L, -1) == LUA_TNIL;
    }
    else
    {
        event_listeners().log_error(repeat ? "interval" : "sleep", lua_tostring(L, -1));
    }
    
    lua_pop(L, 2);
    
    if(repeat)
    {
        timer->expires_from_now(boost::posix_time::milliseconds(countdown));
        timer->async_wait(boost::bind(async_wait_handler, L, timer, repeat, countdown, callback, _1));
    }
    else
    {
        callback.unref(L);
    }
}

static int deadline_timer_ptr_gc(lua_State * L)
{
    boost::weak_ptr<deadline_timer> * timer = reinterpret_cast<boost::weak_ptr<deadline_timer> *>(
        luaL_checkudata(L, 1, "deadline_timer"));
    
    if(!timer->expired())
    {
        boost::system::error_code ec;
        timer->lock()->cancel(ec);
    }
    
    timer->~weak_ptr<deadline_timer>();
    return 0;
}

int async_wait(lua_State * L, bool repeat)
{
    int countdown = luaL_checkint(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    
    boost::shared_ptr<deadline_timer> timer(new deadline_timer(get_main_io_service()));
    
    timer->expires_from_now(boost::posix_time::milliseconds(countdown));
    
    timer->async_wait(boost::bind(async_wait_handler, L, timer, repeat, countdown,
        lua::weak_ref::create(L), _1));
    
    new (lua_newuserdata(L, sizeof(boost::weak_ptr<deadline_timer>))) boost::weak_ptr<deadline_timer>(timer);
    
    luaL_newmetatable(L, "deadline_timer");
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, deadline_timer_ptr_gc);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
    
    return 1;
}

int sleep(lua_State * L)
{
    return async_wait(L, false);
}

int interval(lua_State * L)
{
    return async_wait(L, true);
}

int cancel_timer(lua_State * L)
{
    boost::weak_ptr<deadline_timer> timer = *reinterpret_cast<boost::weak_ptr<deadline_timer> *>(
        luaL_checkudata(L, 1, "deadline_timer"));
    if(timer.expired()) return 0;
    boost::system::error_code ec;
    timer.lock()->cancel(ec);
    return 0;
}

} //namespace lua

static void sched_callback_handler(deadline_timer * timer, int (* fun)(void *), void * closure, 
    const boost::system::error_code & ec)
{
    fun(closure);
    delete timer;
}

void sched_callback(int (* fun)(void *), void * closure, int delay)
{
    deadline_timer * timer = new deadline_timer(get_main_io_service());
    timer->expires_from_now(boost::posix_time::milliseconds(delay));
    timer->async_wait(boost::bind(sched_callback_handler, timer, fun, closure, _1));
}

void sched_callback(int (* fun)(void *), void * closure)
{
    get_main_io_service().post(boost::bind(fun, closure));
}

