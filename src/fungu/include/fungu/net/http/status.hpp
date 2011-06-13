/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_NET_HTTP_STATUS_HPP
#define FUNGU_NET_HTTP_STATUS_HPP

namespace fungu{
namespace http{

enum status
{
    CONTINUE = 100,
    SWITCHING_PROTOCOLS,
    
    OK = 200,
    CREATED,
    ACCEPTED,
    NON_AUTHORITATIVE,
    NO_CONTENT,
    RESET_CONTENT,
    PARTIAL_CONTENT,
    
    MULTIPLE_CHOICES = 300,
    MOVED_PERMANENTLY,
    FOUND,
    SEE_OTHER,
    NOT_MODIFIED,
    USE_PROXY,
    TEMPORARY_REDIRECT = 307,
    
    BAD_REQUEST = 400,
    UNAUTHORIZED,
    PAYMENT_REQUIRED,
    FORBIDDEN,
    NOT_FOUND,
    METHOD_NOT_ALLOWED,
    NOT_ACCEPTABLE,
    PROXY_AUTHENTICATION_REQUIRED,
    REQUEST_TIMED_OUT,
    CONFLICT,
    GONE,
    LENGTH_REQUIRED,
    PRECONDITION_FAILED,
    REQUEST_ENTITY_TOO_LARGE,
    REQUEST_URI_TOO_LONG,
    UNSUPPORTED_MEDIA_TYPE,
    REQUESTED_RANGE_INVALID,
    EXPECTATION_FAILED,
    
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED,
    BAD_GATEWAY,
    SERVICE_UNAVAILABLE,
    GATEWAY_TIMED_OUT,
    HTTP_VERSION_NOT_SUPPORTED
};

enum status_category
{
    INFORMATIONAL = 1,
    SUCCESS,
    REDIRECTION,
    CLIENT_ERROR,
    SERVER_ERROR
};

status_category to_status_category(status code);
const char * get_reason_phrase(status);
const char * to_string(status);

} //namespace http
} //namespace fungu

#endif
