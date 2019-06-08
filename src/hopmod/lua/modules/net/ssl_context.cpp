#include "ssl_context.hpp"
#include "module.hpp"
#include "../../register_class.hpp"
#include "../../to.hpp"
#include "../../create_object.hpp"
#include <cstring>
using namespace boost::asio;

namespace lua{

const char * ssl_context::CLASS_NAME = "ssl_context";

int ssl_context::register_class(lua_State * L){

    static luaL_Reg member_functions[] = {
        {"__gc", &ssl_context::__gc},
        {"add_verify_path", &ssl_context::add_verify_path},
        {"load_verify_file", &ssl_context::load_verify_file},
        {"set_options", &ssl_context::set_options},
        {"set_verify_mode", &ssl_context::set_verify_mode},
        {"use_certificate_chain_file", &ssl_context::use_certificate_chain_file},
        {"use_certificate_file", &ssl_context::use_certificate_file},
        {"use_private_key_file", &ssl_context::use_private_key_file},
        {"use_rsa_private_key_file", &ssl_context::use_rsa_private_key_file},
        {"use_tmp_dh_file", &ssl_context::use_tmp_dh_file},
        {NULL, NULL}
    };
    lua::register_class(L, CLASS_NAME, member_functions);
    
    return 0;
}

int ssl_context::create_object(lua_State * L)
{
    int method = luaL_checkint(L, 1);
    try
    {
        lua::create_object<ssl_context>(L, 
            boost::shared_ptr<ssl::context>(new ssl::context(static_cast<ssl::context_base::method>(method))));
    }
    catch(const boost::system::system_error & se)
    {
        luaL_error(L, "%s", se.code().message().c_str());
        return 0;
    }
    return 1;
}

int ssl_context::push_constants(lua_State * L)
{
    lua_pushinteger(L, ssl::context_base::sslv2);
    lua_setfield(L, -2, "METHOD_SSLV2");
    
    lua_pushinteger(L, ssl::context_base::sslv2_client);
    lua_setfield(L, -2, "METHOD_SSLV2_CLIENT");
    
    lua_pushinteger(L, ssl::context_base::sslv2_server);
    lua_setfield(L, -2, "METHOD_SSLV2_SERVER");
    
    lua_pushinteger(L, ssl::context_base::sslv3);
    lua_setfield(L, -2, "METHOD_SSLV3");
    
    lua_pushinteger(L, ssl::context_base::sslv3_client);
    lua_setfield(L, -2, "METHOD_SSLV3_CLIENT");
    
    lua_pushinteger(L, ssl::context_base::sslv3_server);
    lua_setfield(L, -2, "METHOD_SSLV3_SERVER");
    
    lua_pushinteger(L, ssl::context_base::tlsv1);
    lua_setfield(L, -2, "METHOD_TLSV1");
    
    lua_pushinteger(L, ssl::context_base::tlsv1_client);
    lua_setfield(L, -2, "METHOD_TLSV1_CLIENT");
    
    lua_pushinteger(L, ssl::context_base::tlsv1_server);
    lua_setfield(L, -2, "METHOD_TLSV1_SERVER");
    
    lua_pushinteger(L, ssl::context_base::sslv23);
    lua_setfield(L, -2, "METHOD_SSLV23");
    
    lua_pushinteger(L, ssl::context_base::sslv23_client);
    lua_setfield(L, -2, "METHOD_SSLV23_CLIENT");
    
    lua_pushinteger(L, ssl::context_base::sslv23_server);
    lua_setfield(L, -2, "METHOD_SSLV23_SERVER");
    
    lua_pushinteger(L, ssl::context_base::asn1);
    lua_setfield(L, -2, "FILE_FORMAT_ASN1");
    
    lua_pushinteger(L, ssl::context_base::pem);
    lua_setfield(L, -2, "FILE_FORMAT_PEM");
    
    lua_pushinteger(L, ssl::context_base::verify_peer);
    lua_setfield(L, -2, "VERIFY_PEER");
    
    lua_pushinteger(L, ssl::context_base::verify_none);
    lua_setfield(L, -2, "VERIFY_NONE");
    
    lua_pushinteger(L, ssl::context_base::verify_fail_if_no_peer_cert);
    lua_setfield(L, -2, "VERIFY_FAIL_IF_NO_PEER_CERT");
    
    lua_pushinteger(L, ssl::context_base::verify_client_once);
    lua_setfield(L, -2, "VERIFY_CLIENT_ONCE");
    
    return 0;
}

int ssl_context::__gc(lua_State * L)
{
    lua::to<ssl_context>(L, 1)->~target_type();
    return 0;
}

int ssl_context::add_verify_path(lua_State * L)
{
    target_type self = *lua::to<ssl_context>(L, 1);
    const char * path = luaL_checkstring(L, 2);
    boost::system::error_code ec;
    self->add_verify_path(path, ec);
    if(ec)
    {
        lua_pushstring(L, ec.message().c_str());
        return 1;
    }
    return 0;
}

int ssl_context::load_verify_file(lua_State * L)
{
    target_type self = *lua::to<ssl_context>(L, 1);
    const char * filename = luaL_checkstring(L, 2);
    boost::system::error_code ec;
    self->load_verify_file(filename, ec);
    if(ec)
    {
        lua_pushstring(L, ec.message().c_str());
        return 1;
    }
    return 0;
}

int ssl_context::set_options(lua_State * L)
{
    target_type self = *lua::to<ssl_context>(L, 1);
    int options = luaL_checkint(L, 2);
    boost::system::error_code ec;
    self->set_options(options, ec);
    if(ec)
    {
        lua_pushstring(L, ec.message().c_str());
        return 1;
    }
    return 0;
}

int ssl_context::set_verify_mode(lua_State * L)
{
    target_type self = *lua::to<ssl_context>(L, 1);
    int verify_mode = luaL_checkint(L, 2);
    boost::system::error_code ec;
    self->set_verify_mode(verify_mode, ec);
    if(ec)
    {
        lua_pushstring(L, ec.message().c_str());
        return 1;
    }
    return 0;
}

int ssl_context::use_certificate_chain_file(lua_State * L)
{
    target_type self = *lua::to<ssl_context>(L, 1);
    const char * filename = luaL_checkstring(L, 2);
    boost::system::error_code ec;
    self->use_certificate_chain_file(filename, ec);
    if(ec)
    {
        lua_pushstring(L, ec.message().c_str());
        return 1;
    }
    return 0;
}

int ssl_context::use_certificate_file(lua_State * L)
{
    target_type self = *lua::to<ssl_context>(L, 1);
    const char * filename = luaL_checkstring(L, 2);
    int file_format = luaL_checkint(L, 3);
    boost::system::error_code ec;
    self->use_certificate_file(filename, 
        static_cast<ssl::context_base::file_format>(file_format), ec);
    if(ec)
    {
        lua_pushstring(L, ec.message().c_str());
        return 1;
    }
    return 0;
}

int ssl_context::use_private_key_file(lua_State * L)
{
    target_type self = *lua::to<ssl_context>(L, 1);
    const char * filename = luaL_checkstring(L, 2);
    int file_format = luaL_checkint(L, 3);
    boost::system::error_code ec;
    self->use_private_key_file(filename, 
        static_cast<ssl::context_base::file_format>(file_format), ec);
    if(ec)
    {
        lua_pushstring(L, ec.message().c_str());
        return 1;
    }
    return 0;
}

int ssl_context::use_rsa_private_key_file(lua_State * L)
{
    target_type self = *lua::to<ssl_context>(L, 1);
    const char * filename = luaL_checkstring(L, 2);
    int file_format = luaL_checkint(L, 3);
    boost::system::error_code ec;
    self->use_rsa_private_key_file(filename, 
        static_cast<ssl::context_base::file_format>(file_format), ec);
    if(ec)
    {
        lua_pushstring(L, ec.message().c_str());
        return 1;
    }
    return 0;
}

int ssl_context::use_tmp_dh_file(lua_State * L)
{
    target_type self = *lua::to<ssl_context>(L, 1);
    const char * filename = luaL_checkstring(L, 2);
    boost::system::error_code ec;
    self->use_tmp_dh_file(filename, ec);
    if(ec)
    {
        lua_pushstring(L, ec.message().c_str());
        return 1;
    }
    return 0;
}

} //namespace lua

