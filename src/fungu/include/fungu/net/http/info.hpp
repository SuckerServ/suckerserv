/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_NET_HTTP_INFO_HPP
#define FUNGU_NET_HTTP_INFO_HPP

#include <string>
#include <boost/unordered_map.hpp>

namespace fungu{
namespace http{

enum standard_headers
{
    UNKNOWN_HEADER = 0,
    ACCEPT,
    ACCEPT_CHARSET,
    ACCEPT_ENCODING,
    ACCEPT_LANGUAGE,
    ACCEPT_RANGES,
    AUTHORIZATION,
    CACHE_CONTROL,
    CONNECTION,
    COOKIE,
    CONTENT_LENGTH,
    CONTENT_TYPE,
    DATE,
    EXPECT,
    FROM,
    HOST,
    IF_MATCH,
    IF_MODIFIED_SINCE,
    IF_NONE_MATCH,
    IF_RANGE,
    IF_UNMODIFIED_SINCE,
    MAX_FORWARDS,
    PRAGMA,
    PROXY_AUTHORIZATION,
    RANGE,
    REFERER,
    TE,
    UPGRADE,
    USER_AGENT,
    VIA,
    WARN,
    AGE,
    ALLOW,
    CONTENT_ENCODING,
    CONTENT_LANGUAGE,
    CONTENT_LOCATION,
    CONTENT_DISPOSITION,
    CONTENT_MD5,
    CONTENT_RANGE,
    ETAG,
    EXPIRES,
    LAST_MODIFIED,
    LOCATION,
    PROXY_AUTHENTICATE,
    RETRY_AFTER,
    SERVER,
    SET_COOKIE,
    TRAILER,
    TRANSFER_ENCODING,
    VARY,
    WARNING,
    WWW_AUTHENTICATE
};

unsigned short default_port();
standard_headers resolve_header_field(const char *);
void register_standard_headers();

} //namespace http
} //namespace fungu

#endif
