#include "resolver.hpp"
#include "weak_ref.hpp"
#include "module.hpp"
#include "../../register_class.hpp"
#include "../../to.hpp"
#include "../../create_object.hpp"
#include "../../error_handler.hpp"
#include <boost/asio.hpp>
using namespace boost::asio;
#include <boost/system/error_code.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>

namespace lua{

static void async_resolve_handler(boost::shared_ptr<ip::tcp::resolver>,
    lua_State * L, lua::weak_ref callback, 
    const boost::system::error_code & ec, ip::tcp::resolver::iterator iterator)
{
    if(callback.is_expired()) return;
    
    lua::get_error_handler(L);
    int error_function = lua_gettop(L);
    
    callback.get(L);
    
    if(!ec)
    {
        lua_newtable(L);
        
        int count = 1;
        for(; iterator != ip::tcp::resolver::iterator(); ++iterator)
        {
            lua_pushinteger(L, count++);
            lua_pushstring(L, iterator->endpoint().address().to_string().c_str());
            lua_settable(L, -3);
        }
    }
    else lua_pushstring(L, ec.message().c_str());
        
    if(lua_pcall(L, 1, 0, error_function) != 0)
        log_error(L, "async_resolve");
    
    lua_pop(L, 1);
    
    callback.unref(L);
}

int async_resolve(lua_State * L)
{
    boost::shared_ptr<ip::tcp::resolver> resolver(new ip::tcp::resolver(get_main_io_service(L)));
    
    const char * hostname = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    lua::weak_ref callback_ref = lua::weak_ref::create(L);
    
    ip::tcp::resolver::query query(hostname, "");
    resolver->async_resolve(query, boost::bind(async_resolve_handler, resolver, 
        L, callback_ref, _1, _2));
    
    return 0;
}

} //namespace lua

