/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#include "fungu/net/http/info.hpp"

namespace fungu{
namespace http{
    
static boost::unordered_map<std::string, standard_headers> header_fields;
    
unsigned short default_port()
{
    return 80;
}

standard_headers resolve_header_field(const char * name)
{
    boost::unordered_map<std::string, standard_headers>::const_iterator it = header_fields.find(name);
    if(it == header_fields.end()) return UNKNOWN_HEADER;
    return it->second;
}

void register_standard_headers()
{
    header_fields["accept"] = ACCEPT;
    header_fields["accept-charset"] = ACCEPT_CHARSET;
    header_fields["accept-encoding"] = ACCEPT_ENCODING;
    header_fields["accept-language"] = ACCEPT_LANGUAGE;
    header_fields["accept-ranges"] = ACCEPT_RANGES;
    header_fields["authorization"] = AUTHORIZATION;
    header_fields["cache-control"] = CACHE_CONTROL;
    header_fields["connection"] = CONNECTION;
    header_fields["cookie"] = COOKIE;
    header_fields["content-length"] = CONTENT_LENGTH;
    header_fields["content-type"] = CONTENT_TYPE;
    header_fields["date"] = DATE;
    header_fields["expect"] = EXPECT;
    header_fields["from"] = FROM;
    header_fields["host"] = HOST;
    header_fields["if-match"] = IF_MATCH;
    header_fields["if-modified-since"] = IF_MODIFIED_SINCE;
    header_fields["if-none-match"] = IF_NONE_MATCH;
    header_fields["if-range"] = IF_RANGE;
    header_fields["if-unmodified-since"] = IF_UNMODIFIED_SINCE;
    header_fields["max-forwards"] = MAX_FORWARDS;
    header_fields["pragma"] = PRAGMA;
    header_fields["proxy-authorization"] = PROXY_AUTHORIZATION;
    header_fields["range"] = RANGE;
    header_fields["referer"] = REFERER;
    header_fields["te"] = TE;
    header_fields["upgrade"] = UPGRADE;
    header_fields["user-agent"] = USER_AGENT;
    header_fields["via"] = VIA;
    header_fields["warn"] = WARN;
    header_fields["age"] = AGE;
    header_fields["allow"] = ALLOW;
    header_fields["content-encoding"] = CONTENT_ENCODING;
    header_fields["content-language"] = CONTENT_LANGUAGE;
    header_fields["content-location"] = CONTENT_LOCATION;
    header_fields["content-disposition"] = CONTENT_DISPOSITION;
    header_fields["content-md5"] = CONTENT_MD5;
    header_fields["content-range"] = CONTENT_RANGE;
    header_fields["etag"] = ETAG;
    header_fields["expires"] = EXPIRES;
    header_fields["last-modified"] = LAST_MODIFIED;
    header_fields["location"] = LOCATION;
    header_fields["proxy-authenticate"] = PROXY_AUTHENTICATE;
    header_fields["retry-after"] = RETRY_AFTER;
    header_fields["server"] = SERVER;
    header_fields["set_cookie"] = SET_COOKIE;
    header_fields["trailer"] = TRAILER;
    header_fields["transfer-encoding"] = TRANSFER_ENCODING;
    header_fields["vary"] = VARY;
    header_fields["warning"] = WARNING;
    header_fields["www-authenticate"] = WWW_AUTHENTICATE;
}

} //namespace http
} //namespace fungu
