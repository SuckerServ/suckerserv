#ifndef HOPMOD_LUA_NET_SSL_CONTEXT_HPP
#define HOPMOD_LUA_NET_SSL_CONTEXT_HPP

#include <lua.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/ssl.hpp>

namespace lua{

class ssl_context
{
public:
    typedef boost::shared_ptr<boost::asio::ssl::context> target_type;
    static const char * CLASS_NAME;
    static int register_class(lua_State * L);
    static int create_object(lua_State * L);
    static int push_constants(lua_State * L);
private:
    static int __gc(lua_State * L);
    static int add_verify_path(lua_State * L);
    static int load_verify_file(lua_State * L);
    static int set_options(lua_State * L);
    static int set_password_callback(lua_State * L);
    static int set_verify_mode(lua_State * L);
    static int use_certificate_chain_file(lua_State * L);
    static int use_certificate_file(lua_State * L);
    static int use_private_key_file(lua_State * L);
    static int use_rsa_private_key_file(lua_State * L);
    static int use_tmp_dh_file(lua_State * L);
};

} //namespace lua

#endif

