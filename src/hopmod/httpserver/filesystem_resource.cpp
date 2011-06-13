#include "filesystem_resource.hpp"
#include <fungu/net/http/request.hpp>
#include <fungu/net/http/response.hpp>
using namespace fungu;
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <iostream>

static const std::size_t SEND_BUFFER_SIZE = 65535;
static std::map<const_string, const_string> content_types;

void setup_ext_to_ct_map()
{
    content_types[const_string::literal("gif")] = const_string::literal("image/gif");
    content_types[const_string::literal("jpg")] = const_string::literal("image/jpeg");
    content_types[const_string::literal("png")] = const_string::literal("image/png");
    
    content_types[const_string::literal("txt")]  = const_string::literal("text/plain");
    content_types[const_string::literal("html")] = const_string::literal("text/html");
    content_types[const_string::literal("htm")] = const_string::literal("text/html");
    content_types[const_string::literal("css")]  = const_string::literal("text/css");
    content_types[const_string::literal("js")]   = const_string::literal("text/javascript");
}

static const char * get_content_type_from_extension(const char * ext)
{
    std::map<const_string, const_string>::const_iterator it = content_types.find(const_string(ext, ext+strlen(ext)-1));
    if(it == content_types.end()) return "application/octet-stream";
    else return it->second.c_str();
}

static const char * get_file_extension(const char * filename)
{
    const char * dot = " ";
    for(const char * c = filename; *c; c++)
        if(*c == '.') dot = c;
    return dot + 1;
}

static const char * format_datetime(time_t timestamp, char * buffer, std::size_t buffersize)
{
    const tm * current_gmtime = gmtime(&timestamp);
        
    if(!current_gmtime)
    {
        buffer[0] = '\0';
        return buffer;
    }
    
    strftime(buffer, buffersize, "%a, %d %b %Y %T GMT", current_gmtime);
    return buffer;
}

filesystem_resource::filesystem_resource(const_string path, const_string index, filesystem_resource * parent)
 :m_path(path), m_index(index), m_parent(parent)
{
    
}

filesystem_resource::~filesystem_resource()
{
    if(m_parent && m_parent->m_parent) delete m_parent;
}

http::server::resource * filesystem_resource::resolve(const const_string & name)
{
    const_string parts[3];
    parts[0] = m_path;
    parts[1] = const_string::literal("/");
    parts[2] = name;
    
    const_string absolute_filename = join(parts, 3);
    
    struct stat file_info;
    if(stat(absolute_filename.c_str(), &file_info) != 0) return NULL;
    
    return new filesystem_resource(absolute_filename, m_index, this);
}

void finish_send_file(FILE * sourceFile, char * buffer, http::server::response * response, 
    filesystem_resource * resource)
{
    delete resource;
    delete response;
    delete [] buffer;
    fclose(sourceFile);
}

void send_file(FILE * source_file, char * buffer, std::size_t buffer_size,
    http::server::response * response, filesystem_resource * resource, 
    const http::connection::error & error)
{
    if(error || feof(source_file) || ferror(source_file))
    {
        finish_send_file(source_file, buffer, response, resource);
        return;
    }
    
    std::size_t read_size = fread(buffer, sizeof(char), buffer_size, source_file);
    if(!read_size)
    {
        finish_send_file(source_file, buffer, response, resource);
        return;
    }
    
    response->async_send_body(buffer, read_size, boost::bind(send_file, source_file, buffer, 
        buffer_size, response, resource, _1));
}


void filesystem_resource::get_method(http::server::request & req)
{
    const char * filename = m_path.c_str();
    struct stat file_info;
    if(stat(filename, &file_info) !=0)
    {
        http::server::send_response(req, http::NOT_FOUND);
        delete this;
        return;
    }
    
    const_string index_filename;
    if(file_info.st_mode & S_IFDIR)
    {
        const_string parts[3];
        parts[0] = m_path;
        parts[1] = const_string::literal("/");
        parts[2] = m_index;
        
        index_filename = join(parts, 3);
        filename = index_filename.c_str();
        
        if(stat(filename, &file_info) !=0)
        {
            http::server::send_response(req, http::NOT_FOUND);
            delete this;
            return;
        }
    }
    
    FILE * file = fopen(filename, "r");
    if(!file)
    {
        http::server::send_response(req, http::NOT_FOUND);
        delete this;
        return;
    }
    
    if(req.has_header_field("if-modified-since"))
    {
        time_t last_modified = http::parse_date(req.get_header_field("if-modified-since").get_value());
        
        if(last_modified == file_info.st_mtime)
        {
            fclose(file);
            http::server::send_response(req, http::NOT_MODIFIED);
            delete this;
            return;
        }
    }
    
    http::server::response * res = new http::server::response(req, http::OK);
    res->add_header_field("Content-Type", get_content_type_from_extension(get_file_extension(filename)));
    
    char last_modified_buffer[64];
    format_datetime(file_info.st_mtime, last_modified_buffer, sizeof(last_modified_buffer));
    res->add_header_field("Last-Modified", last_modified_buffer);
    
    res->set_content_length(file_info.st_size);
    
    res->send_header();
    
    if(file_info.st_size)
    {
        char * buffer = new char[SEND_BUFFER_SIZE];
        send_file(file, buffer, SEND_BUFFER_SIZE, res, this, http::connection::error());
    }
}

void filesystem_resource::put_method(http::server::request & req)
{
    http::server::send_response(req, http::METHOD_NOT_ALLOWED);
}

void filesystem_resource::post_method(http::server::request & req)
{
    http::server::send_response(req, http::METHOD_NOT_ALLOWED);
}

void filesystem_resource::delete_method(http::server::request & req)
{
    http::server::send_response(req, http::METHOD_NOT_ALLOWED);
}
