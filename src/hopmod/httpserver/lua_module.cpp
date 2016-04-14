#include <fungu/net/http/connection.hpp>
#include <fungu/net/http/request_line.hpp>
#include <fungu/net/http/header.hpp>
#include <fungu/net/http/info.hpp>
#include <fungu/net/http/response.hpp>
#include <cstdio>
#include <iostream>

#include <lua.hpp>
#include "lua/event.hpp"
#include "lua/modules/module.hpp"

lua::event_environment & event_listeners();

#include "directory_resource.hpp"
#include "proxy_resource.hpp"
#include "filesystem_resource.hpp"
using namespace fungu;

using namespace asio;

asio::io_service & get_main_io_service();
void setup_ext_to_ct_map();

proxy_resource & get_root_resource();

static int root_resource_ref = LUA_REFNIL;
static lua_State * opened_state = NULL;

static int library_instance = 0;

class request_wrapper
{
    static const char * MT;
public:
    request_wrapper(http::server::request * req)
        :m_request(req)
    {
        
    }
    
    ~request_wrapper()
    {
        if(m_request) http::server::request::destroy(*m_request);
    }
    
    static int create_object(lua_State * L, http::server::request * req)
    {
        new (lua_newuserdata(L, sizeof(request_wrapper))) request_wrapper(req);
        luaL_getmetatable(L, request_wrapper::MT);
        lua_setmetatable(L, -2);
        return 1;
    }
    
    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &request_wrapper::__gc},
            {"content_length", &request_wrapper::get_content_length},
            {"content_type", &request_wrapper::get_content_type},
            {"content_subtype", &request_wrapper::get_content_subtype},
            {"header", &request_wrapper::get_header},
            {"uri", &request_wrapper::get_uri},
            {"uri_query", &request_wrapper::get_query_string},
            {"host", &request_wrapper::get_host},
            {"async_read_content", &request_wrapper::async_read_content},
            {"client_ip", &request_wrapper::get_client_ip},
            {"content_type", &request_wrapper::get_content_type},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
        lua_pop(L, 1);
    }
    
    static http::server::request * handover_request(lua_State * L, int narg)
    {
        request_wrapper * object = instance(L, narg);
        http::server::request * request = object->m_request;
        object->m_request = NULL;
        return request;
    }
private:
    static request_wrapper * instance(lua_State * L, int narg)
    {
        return reinterpret_cast<request_wrapper *>(luaL_checkudata(L, narg, MT));
    }
    
    static request_wrapper * safe_instance(lua_State * L, int narg)
    {
        request_wrapper * object = instance(L, narg);
        if(!object->m_request) luaL_error(L, "request object has expired");
        return object;
    }
    
    static int __gc(lua_State * L)
    {
        instance(L, 1)->~request_wrapper();
        return 0;
    }
    
    static int get_content_length(lua_State * L)
    {
        request_wrapper * req = safe_instance(L, 1);
        lua_pushinteger(L, req->m_request->get_content_length());
        return 1;
    }
    
    static int get_header(lua_State * L)
    {
        request_wrapper * req = safe_instance(L, 1);
        const char * field_name = luaL_checkstring(L, 2);
        if(req->m_request->has_header_field(field_name)) 
            lua_pushstring(L, req->m_request->get_header_field(field_name).get_value());
        else lua_pushnil(L);
        return 1;
    }
    
    static int get_uri(lua_State * L)
    {
        request_wrapper * req = safe_instance(L, 1);
        lua_pushstring(L, req->m_request->get_uri());
        return 1;
    }
    
    static int get_query_string(lua_State * L)
    {
        request_wrapper * req = safe_instance(L, 1);
        lua_pushstring(L, req->m_request->get_uri_query());
        return 1;
    }
    
    static int get_host(lua_State * L)
    {
        request_wrapper * req = safe_instance(L, 1);
        lua_pushstring(L, req->m_request->get_host());
        return 1;
    }
    
    static void read_content_complete(int instance, lua_State * L, int functionRef, stream::char_vector_sink * sink, const http::connection::error & err)
    {
        if(instance != library_instance) return;
        
        lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
        luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
        
        if(err) lua_pushnil(L);
        else lua_pushlstring(L, sink->data(), sink->size());
        
        delete sink;
        
        if(lua_pcall(L, 1, 0, 0) != 0)
        {
            event_listeners().log_error("httpserver_async_read_content", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    
    static int async_read_content(lua_State * L)
    {
        request_wrapper * req = safe_instance(L, 1);
        
        luaL_checktype(L, 2, LUA_TFUNCTION);
        lua_pushvalue(L, 2);
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        
        stream::char_vector_sink * sink = new stream::char_vector_sink(req->m_request->get_content_length());
        
        req->m_request->async_read_content(*sink, std::bind(&request_wrapper::read_content_complete, library_instance, L, functionRef, sink, std::placeholders::_1));
        
        return 0;
    }
    
    static int get_client_ip(lua_State * L)
    {
        request_wrapper * req = safe_instance(L, 1);
        std::string ip = req->m_request->get_connection().remote_ip_string();
        lua_pushlstring(L, ip.c_str(), ip.length());
        return 1;
    }
    
    static int get_content_type(lua_State * L)
    {
        request_wrapper * req = safe_instance(L, 1);
        const_string content_type = const_string(req->m_request->get_content_type().type());
        lua_pushstring(L, content_type.copy().c_str());
        return 1;
    }
    
    static int get_content_subtype(lua_State * L)
    {
        request_wrapper * req = safe_instance(L, 1);
        const_string content_type = const_string(req->m_request->get_content_type().subtype());
        lua_pushstring(L, content_type.copy().c_str());
        return 1;
    }
    
    http::server::request * m_request;
};

const char * request_wrapper::MT = "http::server::request";

class response_wrapper
{
    static const char * MT;
public:
    response_wrapper(http::server::response * res)
     :m_response(res),m_content_length(0), m_called_set_content_length(false)
    {
        
    }
    
    static int create_object(lua_State * L)
    {
        http::server::request * request = request_wrapper::handover_request(L, 1);
        if(!request) return 0;
        int status_code = luaL_checkint(L, 2);
        new (lua_newuserdata(L, sizeof(response_wrapper))) response_wrapper(new http::server::response(*request, static_cast<http::status>(status_code)));
        luaL_getmetatable(L, response_wrapper::MT);
        lua_setmetatable(L, -2);
        return 1;
    }
    
    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &response_wrapper::__gc},
            {"header", &response_wrapper::add_header},
            {"set_content_length", &response_wrapper::set_content_length},
            {"async_send_header", &response_wrapper::async_send_header},
            {"send_header", &response_wrapper::send_header},
            {"async_send_body", &response_wrapper::async_send_body},
            {"send_body", &response_wrapper::send_body},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
        lua_pop(L, 1);
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT))->~response_wrapper();
        return 0;
    }
    
    static int add_header(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        const char * name = luaL_checkstring(L, 2);
        const char * value = luaL_checkstring(L, 3);
        res->m_response->add_header_field(name, value);
        return 0;
    }
    
    static int set_content_length(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        if(res->m_called_set_content_length) return luaL_error(L, "you have already called the set_content_length method");
        int length = luaL_checkint(L, 2);
        res->m_content_length = length;
        res->m_response->set_content_length(length);
        res->m_called_set_content_length = true;
        return 0;
    }
    
    void sent_header(lua_State * L, int functionRef, const http::connection::error & err)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
        luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
        
        bool sent = !err;
        lua_pushboolean(L, sent);
        
        if(lua_pcall(L, 1, 0, 0) != 0)
        {
            event_listeners().log_error("httpserver_send_header", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        
        if(m_content_length == 0) m_response = NULL;
    }
    
    static int async_send_header(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        
        luaL_checktype(L, 2, LUA_TFUNCTION);
        lua_pushvalue(L, 2);
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        
        res->m_response->async_send_header(std::bind(&response_wrapper::sent_header, res, L, functionRef, std::placeholders::_1));
        
        return 0;
    }
    
    static int send_header(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        res->m_response->send_header();
        if(res->m_content_length == 0) res->m_response = NULL;
        return 0;
    }
    
    void sent_body(int instance, lua_State * L, int functionRef, const http::connection::error & err)
    {
        if(instance != library_instance) return;
        
        lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
        luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
        
        bool sent = !err;
        lua_pushboolean(L, sent);
        
        if(lua_pcall(L, 1, 0, 0) != 0)
        {
            event_listeners().log_error("httpserver_send_body", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        
        delete m_response;
        m_response = NULL;
    }
    
    static int async_send_body(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        
        std::size_t bodylen = 0;
        const char * body = luaL_checklstring(L, 2, &bodylen);
        
        luaL_checktype(L, 2, LUA_TFUNCTION);
        lua_pushvalue(L, 2);
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        
        res->m_response->async_send_body(body, bodylen, std::bind(&response_wrapper::sent_body, res, library_instance, L, functionRef, std::placeholders::_1));
        
        return 0;
    }
    
    static int send_body(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        
        std::size_t bodylen = 0;
        const char * body = luaL_checklstring(L, 2, &bodylen);
        res->m_response->send_body(body, bodylen);
        res->m_response = NULL; // will self delete on sent body
        return 0;
    }
    
    void check_response_object(lua_State * L)
    {
        if(!m_response) luaL_error(L, "response object has expired");
    }
    
    http::server::response * m_response;
    std::size_t m_content_length;
    unsigned int m_called_set_content_length;
};

const char * response_wrapper::MT = "http::server::response";

static bool lua_isusertype(lua_State * L, int index, const char * tname)
{
    if(!lua_isuserdata(L, index)) return false;
    
    lua_getmetatable(L, index);
    luaL_getmetatable(L, tname);
    
    int equal = lua_equal(L, -1, -2);
    lua_pop(L, 2);
    
    return equal == 1;
}

class filesystem_resource_wrapper:public filesystem_resource
{
public:
    static const char * MT;
    
    filesystem_resource_wrapper(fungu::const_string fs_path, fungu::const_string bind_name)
     :filesystem_resource(fs_path, bind_name, NULL)
    {
        
    }
    
    static int register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &filesystem_resource_wrapper::__gc},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
        lua_pop(L, 1);
        
        return 1;
    }
    
    static int create_object(lua_State * L)
    {
        const char * fs_path = luaL_checkstring(L, 1);
        const char * bind_name = luaL_checkstring(L, 2);
        new (lua_newuserdata(L, sizeof(filesystem_resource_wrapper))) filesystem_resource_wrapper(const_string(std::string(fs_path)), const_string(std::string(bind_name)));
        luaL_getmetatable(L, filesystem_resource_wrapper::MT);
        lua_setmetatable(L, -2);
        return 1;
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<filesystem_resource_wrapper *>(luaL_checkudata(L, 1, MT))->~filesystem_resource_wrapper();
        return 0;
    }
};

const char * filesystem_resource_wrapper::MT = "http::server::filesystem_resource";

class resource_wrapper:public http::server::resource
{
public:
    static const char * MT;
    
    resource_wrapper()
     :m_resolve_function(LUA_REFNIL),
      m_get_function(LUA_REFNIL),
      m_put_function(LUA_REFNIL),
      m_post_function(LUA_REFNIL),
      m_delete_function(LUA_REFNIL),
      m_lua(NULL)
    {
        
    }
    
    ~resource_wrapper()
    {
        luaL_unref(m_lua, LUA_REGISTRYINDEX, m_resolve_function);
        luaL_unref(m_lua, LUA_REGISTRYINDEX, m_get_function);
        luaL_unref(m_lua, LUA_REGISTRYINDEX, m_put_function);
        luaL_unref(m_lua, LUA_REGISTRYINDEX, m_post_function);
        luaL_unref(m_lua, LUA_REGISTRYINDEX, m_delete_function);
    }
    
    http::server::resource * resolve(const const_string & uri)
    {
        if(m_resolve_function == LUA_REFNIL) return NULL;
        lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_resolve_function);
        
        lua_pushstring(m_lua, uri.copy().c_str());
        
        if(lua_pcall(m_lua, 1, 1, 0) != 0)
        {
            event_listeners().log_error("httpserver_resource_resolve", lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);
            return NULL;
        }
        else
        {
            resource_wrapper * resource = NULL;
            if(lua_isusertype(m_lua, -1, MT) || lua_isusertype(m_lua, -1, filesystem_resource_wrapper::MT))
                resource = reinterpret_cast<resource_wrapper *>(lua_touserdata(m_lua, -1));
            lua_pop(m_lua, 1);
            return resource;
        }
    }
    
    void get_method(http::server::request & req)
    {
        if(m_get_function == LUA_REFNIL)
        {
            http::server::send_response(req, http::METHOD_NOT_ALLOWED);
            return;
        }
        lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_get_function);
        
        request_wrapper::create_object(m_lua, &req);
        
        if(lua_pcall(m_lua, 1, 0, 0) != 0)
        {
            event_listeners().log_error("httpserver_resource_get", lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);
        }
    }
    
    void put_method(http::server::request & req)
    {
        if(m_put_function == LUA_REFNIL)
        {
            http::server::send_response(req, http::METHOD_NOT_ALLOWED);
            return;
        }
        lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_put_function);
        
        request_wrapper::create_object(m_lua, &req);
        
        if(lua_pcall(m_lua, 1, 0, 0) != 0)
        {
            event_listeners().log_error("httpserver_resource_put", lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);
        }
    }
    
    void post_method(http::server::request &req)
    {
        if(m_post_function == LUA_REFNIL)
        {
            http::server::send_response(req, http::METHOD_NOT_ALLOWED);
            return;
        }
        lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_post_function);
        
        request_wrapper::create_object(m_lua, &req);
        
        if(lua_pcall(m_lua, 1, 0, 0) != 0)
        {
            event_listeners().log_error("httpserver_resource_post", lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);
        }
    }
    
    void delete_method(http::server::request & req)
    {
        if(m_delete_function == LUA_REFNIL)
        {
            http::server::send_response(req, http::METHOD_NOT_ALLOWED);
            return;
        }
        lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_delete_function);
        
        request_wrapper::create_object(m_lua, &req);
        
        if(lua_pcall(m_lua, 1, 0, 0) != 0)
        {
            event_listeners().log_error("httpserver_resource_delete", lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);
        }
    }
    
    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &resource_wrapper::__gc},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
        lua_pop(L, 1);
    }
    
    static int create_object(lua_State * L)
    {
        luaL_checktype(L, 1, LUA_TTABLE);
        
        int resolve_function = LUA_REFNIL;
        int get_function = LUA_REFNIL;
        int put_function = LUA_REFNIL;
        int post_function = LUA_REFNIL;
        int delete_function = LUA_REFNIL;
        
        lua_pushstring(L, "resolve");
        lua_rawget(L, -2);
        if(lua_type(L, -1) == LUA_TFUNCTION) resolve_function = luaL_ref(L, LUA_REGISTRYINDEX);
        else lua_pop(L, 1);
        
        lua_pushstring(L, "get");
        lua_rawget(L, -2);
        if(lua_type(L, -1) == LUA_TFUNCTION) get_function = luaL_ref(L, LUA_REGISTRYINDEX);
        else lua_pop(L, 1);
        
        lua_pushstring(L, "put");
        lua_rawget(L, -2);
        if(lua_type(L, -1) == LUA_TFUNCTION) put_function = luaL_ref(L, LUA_REGISTRYINDEX);
        else lua_pop(L, 1);
        
        lua_pushstring(L, "post");
        lua_rawget(L, -2);
        if(lua_type(L, -1) == LUA_TFUNCTION) post_function = luaL_ref(L, LUA_REGISTRYINDEX);
        else lua_pop(L, 1);
        
        lua_pushstring(L, "delete");
        lua_rawget(L, -2);
        if(lua_type(L, -1) == LUA_TFUNCTION) delete_function = luaL_ref(L, LUA_REGISTRYINDEX);
        else lua_pop(L, 1);
        
        resource_wrapper * res = new (lua_newuserdata(L, sizeof(resource_wrapper))) resource_wrapper;
        
        luaL_getmetatable(L, resource_wrapper::MT);
        lua_setmetatable(L, -2);
        
        res->m_resolve_function = resolve_function;
        res->m_get_function = get_function;
        res->m_put_function = put_function;
        res->m_post_function = post_function;
        res->m_delete_function = delete_function;
        res->m_lua = L;
        
        return 1;
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<resource_wrapper *>(luaL_checkudata(L, 1, MT))->~resource_wrapper();
        return 0;
    }
    
    int m_resolve_function;
    int m_get_function;
    int m_put_function;
    int m_post_function;
    int m_delete_function;
    lua_State * m_lua;
};

const char * resource_wrapper::MT = "http::server::resource";

static int url_decode(lua_State * L)
{
    std::size_t input_len;
    const char * input = luaL_checklstring(L, 1, &input_len);
    char buffer[1024];
    std::size_t output_len = sizeof(buffer);
    lua_pushstring(L, http::pct_decode(input, input + input_len, buffer, &output_len));
    return 1;
}

static int url_encode(lua_State * L)
{
    std::size_t input_len;
    const char * input = luaL_checklstring(L, 1, &input_len);
    char buffer[1024];
    std::size_t output_len = sizeof(buffer);
    lua_pushstring(L, http::pct_encode(input, input + input_len, buffer, &output_len));
    return 1;
}

class listener_client_connection:public http::server::client_connection
{
public:
    listener_client_connection(asio::io_service & service, listener_client_connection * prev_connection)
    :http::server::client_connection(service)
    {
        m_prev = prev_connection;
        m_next = NULL;
        
        if(m_prev)
        {
            assert(prev_connection->m_next == NULL);
            prev_connection->m_next = this;
        }
    }
    
    ~listener_client_connection()
    {
        if(m_prev) m_prev->m_next = m_next;
        if(m_next) m_next->m_prev = m_prev;
    }
    
    listener_client_connection * prev_client()const{return m_prev;}
    listener_client_connection * next_client()const{return m_next;}
private:
    listener_client_connection * m_prev;
    listener_client_connection * m_next;
};

class listener
{
public:
    static const char * MT;
    
    listener(lua_State * L, const char * ip, const char * port)
     :m_L(L), 
      m_acceptor(NULL),
      m_client_head(NULL),
      m_client_tail(NULL),
      m_root_resource(NULL),
      m_root_resource_ref(LUA_REFNIL)
    {
        static directory_resource empty_resource;
        m_root_resource = &empty_resource;
        
        m_ip = ip;
        m_port = port;
        create_acceptor(ip, port);
    }
    
    ~listener()
    {
        shutdown();
    }
    
    static int create_object(lua_State * L)
    {
        const char * ip = luaL_checkstring(L, 1);
        const char * port = luaL_checkstring(L, 2);
        
        try
        {
            new (lua_newuserdata(L, sizeof(listener))) listener(L, ip, port);
        }
        catch(system_error se)
        {
            lua_pop(L, 1);
            lua_pushnil(L);
            lua_pushstring(L, se.code().message().c_str());
            return 2;
        }
        
        luaL_getmetatable(L, listener::MT);
        lua_setmetatable(L, -2);
        
        return 1;
    }
    
    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &listener::__gc},
            {"start", &listener::start},
            {"stop", &listener::stop},
            {"set_root", &listener::set_root},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
        lua_pop(L, 1);
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<listener *>(luaL_checkudata(L, 1, MT))->~listener();
        return 0;
    }
    
    static int start(lua_State * L)
    {
        listener * self = reinterpret_cast<listener *>(luaL_checkudata(L, 1, MT));
        
        luaL_checktype(L, 2, LUA_TFUNCTION);
        lua_pushvalue(L, 2);
        self->m_start_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        
        if(!self->m_acceptor)
        {
            try
            {
                self->create_acceptor(self->m_ip.c_str(), self->m_port.c_str());
            }
            catch(system_error se)
            {
                lua_pop(L, 1);
                lua_pushboolean(L, 0);
                lua_pushstring(L, se.code().message().c_str());
                return 2;
            }
        }
        
        try
        {
            self->m_acceptor->listen();
        }
        catch(system_error se)
        {
            lua_pop(L, 1);
            lua_pushboolean(L, 0);
            lua_pushstring(L, se.code().message().c_str());
            return 2;
        }
        
        self->async_accept();
        
        lua_pushboolean(L, 1);
        return 1;
    }
    
    static int stop(lua_State * L)
    {
        listener * self = reinterpret_cast<listener *>(luaL_checkudata(L, 1, MT));
        self->shutdown();
        return 0;
    }
    
    static int set_root(lua_State * L)
    {
        listener * self = reinterpret_cast<listener *>(luaL_checkudata(L, 1, MT));
        resource_wrapper * resource = reinterpret_cast<resource_wrapper *>(luaL_checkudata(L, 2, resource_wrapper::MT));
        lua_pushvalue(L, -1);
        self->m_root_resource = resource;
        luaL_unref(L, LUA_REGISTRYINDEX, self->m_root_resource_ref);
        self->m_root_resource_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        return 0;
    }
    
    void create_acceptor(const char * ip, const char * port)
    {
        delete m_acceptor;
        
        m_acceptor = new ip::tcp::acceptor(get_main_io_service());
        m_acceptor->open(ip::tcp::v4());
        m_acceptor->set_option(socket_base::reuse_address(true));
        m_acceptor->bind(ip::tcp::endpoint(ip::address_v4::from_string(ip), atoi(port)));
    }
    
    void cleanup_client_connection(int instance, listener_client_connection * client)
    {
        if(instance == library_instance)
        {
            if(client == m_client_tail) m_client_tail = client->prev_client();
            if(client == m_client_head) m_client_head = client->next_client();
        }
        
        delete client;
    }
    
    void accept_handler(int instance, listener_client_connection * client, const error_code & error)
    {
        if(instance != library_instance)
        {
            delete client;
            return;
        }
        
        if(error)
        {
            cleanup_client_connection(instance, client);
            
            if(error.value() != error::operation_aborted) 
            {
                m_acceptor->close();
                delete m_acceptor;
                m_acceptor = NULL;
                
                lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_start_callback_ref);
                lua_pushstring(m_L, error.message().c_str());
                if(lua_pcall(m_L, 1, 0, 0) != 0)
                {
                    event_listeners().log_error("httpserver_accept", lua_tostring(m_L, -1));
                    lua_pop(m_L, 1);
                }
            }
            
            return;
        }
        
        http::server::request::create(*client, *m_root_resource, std::bind(&listener::cleanup_client_connection, this, library_instance, client));
        
        async_accept();
    }
    
    void async_accept()
    {
        listener_client_connection * client = new listener_client_connection(m_acceptor->get_io_service(), m_client_tail);
        
        if(!m_client_head) m_client_head = client;
        m_client_tail = client;
        
        m_acceptor->async_accept(*client, std::bind(&listener::accept_handler, this, library_instance, client, std::placeholders::_1));
    }
    
    void shutdown()
    {
        if(!m_acceptor) return;
        
        std::error_code ec;
        m_acceptor->close(ec);
        if(ec)
        {
            std::cerr<<"listener socket close operation failed: "<<ec.message()<<std::endl;
        }
        else
        {
            delete m_acceptor;
            m_acceptor = NULL;
        }
        
        for(listener_client_connection * client = m_client_head; client; client = client->next_client())
        {
            static_cast<http::connection *>(client)->close();
        }
    }
    
    lua_State * m_L;
    
    std::string m_ip;
    std::string m_port;
    
    ip::tcp::acceptor * m_acceptor;
    listener_client_connection * m_client_head;
    listener_client_connection * m_client_tail;
    
    http::server::resource * m_root_resource;
    int m_root_resource_ref;
    int m_start_callback_ref;
};

const char * listener::MT = "http_server::listener";

static int shutdown_http_server(lua_State *)
{
    library_instance++;
    return 0;
}

namespace lua{
namespace module{

void open_http_server(lua_State * L)
{
    lua::on_shutdown(L, shutdown_http_server);
    
    http::register_standard_headers();
    setup_ext_to_ct_map();
    
    opened_state = L;
    root_resource_ref = LUA_REFNIL;
    
    static luaL_Reg functions[] = {
        {"resource", &resource_wrapper::create_object},
        {"filesystem_resource", &filesystem_resource_wrapper::create_object},
        {"response", &response_wrapper::create_object},
        {"url_decode", &url_decode},
        {"url_encode", &url_encode},
        {"listener", &listener::create_object},
        {NULL, NULL}
    };
    
    luaL_register(L, "http_server", functions);
    lua_pop(L, 1);
    
    request_wrapper::register_class(L);
    response_wrapper::register_class(L);
    resource_wrapper::register_class(L);
    filesystem_resource_wrapper::register_class(L);
    listener::register_class(L);
}

} //namespace module
} //namespace lua
