#ifndef HOPMOD_HTTPSERVER_DIRECTORY_RESOURCE_HPP
#define HOPMOD_HTTPSERVER_DIRECTORY_RESOURCE_HPP

#include <fungu/net/http/request.hpp>
#include <map>

class directory_resource:public fungu::http::server::resource
{
public:
    void add_resource(fungu::http::server::resource & res,const fungu::const_string & name)
    {
        m_child_resources[name] = &res;
    }
    
    void remove_resource(const fungu::const_string & name)
    {
        m_child_resources.erase(name);
    }
    
    fungu::http::server::resource * resolve(const fungu::const_string & name)
    {
        std::map<fungu::const_string, fungu::http::server::resource *>::const_iterator it = m_child_resources.find(name);
        if(it == m_child_resources.end()) return NULL;
        return it->second;
    }
    
    void get_method(fungu::http::server::request & req){internal_server_error(req);}
    void put_method(fungu::http::server::request & req){internal_server_error(req);}
    void post_method(fungu::http::server::request & req){internal_server_error(req);}
    void delete_method(fungu::http::server::request & req){internal_server_error(req);}
private:
    void internal_server_error(fungu::http::server::request & req)
    {
        fungu::http::server::send_response(req, fungu::http::INTERNAL_SERVER_ERROR, "request method called on a directory_resource object\n");
    }
    
    std::map<fungu::const_string, fungu::http::server::resource *> m_child_resources;
};

#endif
