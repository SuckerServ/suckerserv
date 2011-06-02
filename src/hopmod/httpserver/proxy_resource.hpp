#ifndef HOPMOD_HTTPSERVER_PROXY_RESOURCE_HPP
#define HOPMOD_HTTPSERVER_PROXY_RESOURCE_HPP

#include <boost/function.hpp>

class proxy_resource:public fungu::http::server::resource
{
public:
    proxy_resource()
    :m_resource(NULL)
    {
        
    }
    
    template<typename UnsetCallbackFunction>
    void set_resource(fungu::http::server::resource * res, UnsetCallbackFunction unset_callback)
    {
        if(m_unset_callback) m_unset_callback();
        m_resource = res;
        m_unset_callback = unset_callback;
    }
    
    fungu::http::server::resource * resolve(const fungu::const_string & uri)
    {
        if(!m_resource) return NULL;
        return m_resource->resolve(uri);
    }
    
    void get_method(fungu::http::server::request & req)
    {
        if(!m_resource)
        {
            unset_error(req);
            return;
        }
        m_resource->get_method(req);
    }
    
    void put_method(fungu::http::server::request & req)
    {
        if(!m_resource)
        {
            unset_error(req);
            return;
        }
        m_resource->put_method(req);
    }
    
    void post_method(fungu::http::server::request & req)
    {
        if(!m_resource)
        {
            unset_error(req);
            return;
        }
        m_resource->post_method(req);
    }
    
    void delete_method(fungu::http::server::request & req)
    {
        if(!m_resource)
        {
            unset_error(req);
            return;
        }
        m_resource->delete_method(req);
    }
private:
    void unset_error(fungu::http::server::request & req)
    {
        fungu::http::server::send_response(req, fungu::http::INTERNAL_SERVER_ERROR, "null resource\n");
    }
    
    fungu::http::server::resource * m_resource;
    boost::function0<void> m_unset_callback;
};

#endif
