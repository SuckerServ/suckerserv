--[[
    Some utility functions useful for dealing with HTTP requests
]]
local _ = require "underscore"

local function decodeQueryString(value)
    value = http_server.url_decode(value or "")
    value = string.gsub(value, " ", "+")
    return value
end

local function encodeQueryString(value)
    value = string.gsub(value, "+", " ")
    value = http_server.url_encode(value)
    return value
end

local function parseQueryString(query)
    local result = {}
    for param in string.gmatch(query or "", "[^&]+") do
        local name, value = table.unpack(_.to_array(string.gmatch(param, "[^=]+")))
        local decode = decodeQueryString
        result[decode(name)] = decode(value)
    end
    return result
end

local function parseCookie(cookie)
    local result = {}
    for param in string.gmatch(cookie or "", "[^; ]+") do
        local name, value = table.unpack(_.to_array(string.gmatch(param, "[^=]+")))
        local decode = decodeQueryString
        result[decode(name)] = decode(value)
    end
    return result
end

local function buildQueryString(queryTable)
    local result = ""
    for name, value in pairs(queryTable) do
        if #result > 0 then result = result .. "&" end
        local encode = encodeQueryString
        result = result .. string.format("%s=%s", encode(name), encode(value))
    end
    return result
end

local function buildAbsoluteUri(request)
    
    local query = request:uri_query()

    if query then
        query = "?" .. query
    else
        query = ""
    end
    
    return "http://" .. request:host() .. request:uri() .. query
end

http_request = {
    absolute_uri = buildAbsoluteUri,
    parse_cookie = parseCookie,
    parse_query_string = parseQueryString,
    build_query_string = buildQueryString
}

return http_request
