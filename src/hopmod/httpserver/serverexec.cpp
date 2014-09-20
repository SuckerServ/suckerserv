/*
    This is the fallback serverexec resource, look in script/base/web/serverexec.lua for actual servexec resource used.
*/
#include "serverexec.hpp"
#include "../hopmod.hpp"
#include <fungu/net/http/response.hpp>
#include <fungu/script.hpp>

using namespace fungu;

class script_post:public stream::char_vector_sink
{
public:
    script_post(http::server::request & req)
     :stream::char_vector_sink(req.get_content_length()),
      m_request(req),
      m_response(NULL)
    {
        m_request.async_read_content(*this, std::bind(&script_post::read_content_completed, this, std::placeholders::_1));
    }
    
    virtual ~script_post()
    {
        delete m_response;
    }
private:
    virtual std::string eval_code(const char *, const char *, http::status *, const char **)=0;
    
    void read_content_completed(const http::connection::error & err)
    {
        if(err)
        {
            http::server::request::destroy(m_request);
            return;
        }
        
        http::status status_code = http::OK;
        
        const char * response_content_type = "text/plain";
        m_eval_result = eval_code(data(), data()+size()-1, &status_code, &response_content_type);
        
        if(m_eval_result.length() == 0 && status_code == http::OK) status_code = http::NO_CONTENT;
        
        m_response = new http::server::response(m_request, status_code);
        m_response->add_header_field("Content-type", response_content_type);
        m_response->set_content_length(m_eval_result.length());
        m_response->send_header();
        
        if(m_eval_result.length())
        {
            m_response->async_send_body(m_eval_result.data(), m_eval_result.length(), std::bind(&script_post::send_body_completed, this, std::placeholders::_1));
        }
    }
    
    void send_body_completed(const http::connection::error & err)
    {
        delete this;
    }

    http::server::request & m_request;
    std::string m_eval_result;
    http::server::response * m_response;
};

class cubescript_post:public script_post
{
public:
    cubescript_post(http::server::request & req)
     :script_post(req){}
     virtual ~cubescript_post(){}
private:
    std::string eval_code(const char * start, const char * last, http::status * response_status, const char ** response_ct)
    {
        std::string result;
        
        *response_status = http::BAD_REQUEST;
        *response_ct = "text/plain";
        
        try
        {
            script::env_frame frame(&get_script_env());
            result = script::execute_text(const_string(start, last), &frame).to_string().copy();
            *response_status = http::OK;
        }
        catch(script::error error)
        {
            result = error.get_error_message();
        }
        catch(script::error_trace * error)
        {
            result = get_script_error_message(error);
        }
        
        return result;
    }
};

http::server::resource * serverexec_resource::resolve(const const_string &)
{
    return NULL;
}

void serverexec_resource::get_method(http::server::request & req)
{
    http::server::send_response(req, http::METHOD_NOT_ALLOWED);
}

void serverexec_resource::put_method(http::server::request & req)
{
    http::server::send_response(req, http::METHOD_NOT_ALLOWED);
}

void serverexec_resource::post_method(http::server::request & req)
{
    if(req.get_connection().remote_ip_v4_ulong() & 0x7F000000)
    {
        http::server::send_response(req, http::FORBIDDEN, "only accepting requests from 127.0.0.1\n");
        return;
    }
    
    http::content_type ct = req.get_content_type();
    bool text_type = const_string(ct.type()) == const_string::literal("text");
    if(text_type && const_string(ct.subtype()) == const_string::literal("x-cubescript")) new cubescript_post(req);
    else http::server::send_response(req, http::BAD_REQUEST, "unsupported content type");
}

void serverexec_resource::delete_method(http::server::request & req)
{
    http::server::send_response(req, http::METHOD_NOT_ALLOWED);
}
