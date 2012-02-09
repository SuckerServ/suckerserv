/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#include "fungu/net/http/status.hpp"

namespace fungu{
namespace http{

const char * get_reason_phrase(status code)
{
    switch(code)
    {
        case CONTINUE: return "Continue";
        case SWITCHING_PROTOCOLS: return "Switching Protocols";
        
        case OK: return "OK";
        case CREATED: return "Created";
        case ACCEPTED: return "Accepted";
        case NON_AUTHORITATIVE: return "Non-Authoritive Information";
        case NO_CONTENT: return "No Content";
        case RESET_CONTENT: return "Reset Content";
        case PARTIAL_CONTENT: return "Partial Content";
        
        case MULTIPLE_CHOICES: return "Multiple Choices";
        case MOVED_PERMANENTLY: return "Moved Permanently";
        case FOUND: return "Found";
        case SEE_OTHER: return "See Other";
        case NOT_MODIFIED: return "Not Modified";
        case USE_PROXY: return "Use Proxy";
        case TEMPORARY_REDIRECT: return "Temporary Redirect";
        
        case BAD_REQUEST: return "Bad Request";
        case UNAUTHORIZED: return "Unauthorized";
        case PAYMENT_REQUIRED: return "Payment Required";
        case FORBIDDEN: return "Forbidden";
        case NOT_FOUND: return "Not Found";
        case METHOD_NOT_ALLOWED: return "Method Not Allowed";
        case NOT_ACCEPTABLE: return "Not Acceptable";
        case PROXY_AUTHENTICATION_REQUIRED: return "Proxy Authentication Required";
        case REQUEST_TIMED_OUT: return "Request Timeout";
        case CONFLICT: return "Conflict";
        case GONE: return "Gone";
        case LENGTH_REQUIRED: return "Length Required";
        case PRECONDITION_FAILED: return "Precondition Failed";
        case REQUEST_ENTITY_TOO_LARGE: return "Request Entity Too Large";
        case REQUEST_URI_TOO_LONG: return "Request-URI Too Long";
        case UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";
        case REQUESTED_RANGE_INVALID: return "Request Range Not Satisfiable";
        case EXPECTATION_FAILED: return "Expectation Failed";
        
        case INTERNAL_SERVER_ERROR: return "Internal Server Error";
        case NOT_IMPLEMENTED: return "Not Implemented";
        case BAD_GATEWAY: return "Bad Gateway";
        case SERVICE_UNAVAILABLE: return "Service Unavailable";
        case GATEWAY_TIMED_OUT: return "Gateway Timeout";
        case HTTP_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";
        default: return "Unknown Status";
    }
}

status_category to_status_category(status code)
{
    return static_cast<status_category>(code / 100);
}

const char * to_string(status code)
{
    switch(code)
    {
        case CONTINUE: return "100";
        case SWITCHING_PROTOCOLS: return "101";
        
        case OK: return "200";
        case CREATED: return "201";
        case ACCEPTED: return "202";
        case NON_AUTHORITATIVE: return "203";
        case NO_CONTENT: return "204";
        case RESET_CONTENT: return "205";
        case PARTIAL_CONTENT: return "206";
        
        case MULTIPLE_CHOICES: return "300";
        case MOVED_PERMANENTLY: return "301";
        case FOUND: return "302";
        case SEE_OTHER: return "303";
        case NOT_MODIFIED: return "304";
        case USE_PROXY: return "305";
        case TEMPORARY_REDIRECT: return "307";
        
        case BAD_REQUEST: return "400";
        case UNAUTHORIZED: return "401";
        case PAYMENT_REQUIRED: return "402";
        case FORBIDDEN: return "403";
        case NOT_FOUND: return "404";
        case METHOD_NOT_ALLOWED: return "405";
        case NOT_ACCEPTABLE: return "406";
        case PROXY_AUTHENTICATION_REQUIRED: return "407";
        case REQUEST_TIMED_OUT: return "408";
        case CONFLICT: return "409";
        case GONE: return "410";
        case LENGTH_REQUIRED: return "411";
        case PRECONDITION_FAILED: return "412";
        case REQUEST_ENTITY_TOO_LARGE: return "413";
        case REQUEST_URI_TOO_LONG: return "414";
        case UNSUPPORTED_MEDIA_TYPE: return "415";
        case REQUESTED_RANGE_INVALID: return "416";
        case EXPECTATION_FAILED: return "417";
        
        case INTERNAL_SERVER_ERROR: return "500";
        case NOT_IMPLEMENTED: return "501";
        case BAD_GATEWAY: return "502";
        case SERVICE_UNAVAILABLE: return "503";
        case GATEWAY_TIMED_OUT: return "504";
        case HTTP_VERSION_NOT_SUPPORTED: return "505";
        default: return "XXX";
    }
}

} //namespace http
} //namespace fungu
