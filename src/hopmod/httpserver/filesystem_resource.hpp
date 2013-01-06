#ifndef HOPMOD_HTTPSERVER_FILESYSTEM_RESOURCE_HPP
#define HOPMOD_HTTPSERVER_FILESYSTEM_RESOURCE_HPP

#include <fungu/string.hpp>
#include <fungu/net/http/request.hpp>
#include <map>

class filesystem_resource:public fungu::http::server::resource
{
public:
    filesystem_resource(fungu::const_string, fungu::const_string, filesystem_resource *);
    virtual ~filesystem_resource();
    fungu::http::server::resource * resolve(const fungu::const_string & name);
    void get_method(fungu::http::server::request & req);
    void put_method(fungu::http::server::request & req);
    void post_method(fungu::http::server::request & req);
    void delete_method(fungu::http::server::request & req);
private:
    fungu::const_string m_path;
    fungu::const_string m_index;
    filesystem_resource * m_parent;
};

#endif
