#ifndef HOPMOD_HTTPSERVER_SERVEREXEC_HPP
#define HOPMOD_HTTPSERVER_SERVEREXEC_HPP

#include <fungu/net/http/request.hpp>
#include <fungu/string.hpp>

class serverexec_resource:public fungu::http::server::resource
{
public:
    fungu::http::server::resource * resolve(const fungu::const_string & name);
    void get_method(fungu::http::server::request & req);
    void put_method(fungu::http::server::request & req);
    void post_method(fungu::http::server::request & req);
    void delete_method(fungu::http::server::request & req);
};

#endif
