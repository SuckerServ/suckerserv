--[[
    Simple HTTP Client
    Copyright (C) 2010 Graham Daws
]]
local net = require "net"
local url = require "url"
local string = require "string"
local table = require "table"
local pairs = pairs
local tonumber = tonumber
local read_only = read_only
local type = type
local print = print
local debug = debug

module "http.client"

local MAX_REDIRECTS = 5

function unfold_headers(headers)
    headers = string.gsub(headers, "\r\n%s+", " ")
    return headers
end

function parse_headers(headers)
    local header = {}
    for name, value in string.gmatch(headers, "([^:\r\n]+): *([^\r\n]+)") do
        header[string.lower(name)] = value
    end
    return header
end

local function read_status_line(socket, callback)
    socket:async_read_until("\r\n", function(status_line, error_message)
        if error_message then
            callback(nil, nil, {socket_error = error_message})
            return
        end
        local version, status_code = string.match(status_line, "HTTP/(%d.%d) (%d%d%d)")
        if not version or not status_code then
            callback(nil, nil, {parse_error = "malformed status line"})
            return
        end
        callback(version, status_code)
    end)
end

local function read_headers(socket, callback)
    socket:async_read_until("\r\n\r\n", function(headers, error_message)
        if error_message then
            callback(nil, error_message)
            return
        end
        callback(parse_headers(unfold_headers(headers)))
    end)
end

local function read_body(socket, headers, callback)

    local content_length = tonumber(headers["content-length"])
    
    if not content_length then
        callback(nil, {http_error = "malformed content-length field"})
        return
    end
    
    if content_length == 0 then
        callback("")
        return
    end
    
    socket:async_read(content_length, function(data, error_message)
        if error_message then
            callback(nil, {socket_error = error_message})
            return
        end
        callback(data)
    end)
end

local function read_trailer_headers(socket, callback)
    socket:async_read_until("\r\n", function(line, error_message)
        
        if error_message then
            callback({socket_error = error_message})
            return
        end
        
        if line == "\r\n" then
            callback(nil)
        else
           read_trailer_headers(socket, callback) 
        end
    end)
end

local function read_chunked_body(socket, header, callback, body)

    body = body or ""
    
    socket:async_read_until("\r\n", function(line, error_message)
    
        if error_message then
            callback(nil, {socket_error = error_message})
            return
        end
        
        local chunk_size = tonumber(string.match(line, "^[%dabcdef]+"), 16)
        
        if chunk_size == 0 then
            
            read_trailer_headers(socket, function(errors)
                
                if errors then
                    callback(nil, errors)
                    return
                end
                
                callback(body)
            end)
            
            return
        end
        
        socket:async_read(chunk_size, function(chunk, error_message)
        
            if error_message then
                callback(nil,{socket_error = error_message})
                return
            end
            
            body = body .. chunk
            
            socket:async_read_until("\r\n", function(empty_line, error_message)
                
                -- TODO check empty_line
                 
                if error_message then
                    callback(nil,{socket_error = error_message})
                    return
                end
                
                read_chunked_body(socket, header, callback, body)
            end)
        end)
    end)
end

local body_readers = {
    identity = read_body,
    chunked = read_chunked_body
}

local function is_redirect(status_code)
    if string.sub(status_code,1,1) ~= "3" then return false end
    return status_code == "301" or status_code == "302" or status_code == "303"
end

function get(resource, callback, state)
    
    state = state or {redirects = 0}
    
    resource = url.parse(resource)
    
    local support_ssl = net.ssl ~= nil
    
    local supported_protocol = {
        http = true,
        https = support_ssl
    }
    
    if not supported_protocol[resource.scheme] or not resource.host then
        callback(nil, {http_error = "invalid url"})
        return
    end
    
    local standard_port = {
        http = 80,
        https = 443
    }
    
    local using_https = resource.scheme == "https"
    local ssl_client_socket = nil
    local ssl_context = nil
    
    if using_https then
        ssl_context = net.ssl.context(net.ssl.METHOD_TLSV1)
        ssl_context:set_verify_mode(net.ssl.VERIFY_PEER)
        ssl_context:load_verify_file("share/cacert.pem")
    end
    
    resource.path = resource.path or "/"
    
    local client = net.tcp_client()
    local host = resource.host
    local port = resource.port or standard_port[resource.scheme]
    
    local function send_request()
        
        local host_field = resource.host
        if resource.port then
            host_field = host_field .. ":" .. resource.port
        end
        
        local path = url.build_path(url.parse_path(resource.path))
        
        if resource.query then
            path = path .. "?" .. resource.query
        end
        
        local request = "GET " .. path .. " HTTP/1.1\r\nHost: " .. host_field .. "\r\nConnection: close\r\n\r\n"
        
        client:async_send(request, function(error_message)

            if error_message then
                callback(nil, {socket_error = error_message})
               return
            end
            
            read_status_line(client, function(version, status_code, errors)
                
                if errors then
                    callback(nil, errors)
                    return
                end
                
                read_headers(client, function(headers, errors)
                    
                    if errors then
                        callback(nil, errors)
                        return
                    end
                    
                    if is_redirect(status_code) then
                    
                        state.redirects = state.redirects + 1
                        
                        if state.redirects > MAX_REDIRECTS then
                            callback(nil, {http_error = "too many redirects"})
                            return
                        end
                        
                        get(headers["location"], callback, state)
                        
                        client:close()
                        return
                    end
                    
                    local read_body = body_readers[headers["transfer-encoding"] or "identity"]
                    
                    if not read_body then
                        callback(nil,{http_error = string.format("client doesn't support %s transfer encoding", headers["transfer-encoding"])})
                        return
                    end
                    
                    read_body(client, headers, function(body, errors)
                        
                        if errors then
                            callback(nil, errors)
                            return
                        end
                        
                        callback(body, {
                            version = version,
                            response_status = status_code,
                            headers = headers
                        })
                        
                        if using_https then
                            client:async_shutdown(function(error_message)
                                ssl_client_socket:close()
                            end)
                        else
                            client:close()
                        end
                    end)
                end)
            end)
        end)
    end
    
    client:async_connect(host, port, function(error_message)
        
        if error_message then
            callback(nil, {socket_error = error_message})
            return
        end
        
        if using_https then
            ssl_client_socket = client
            client = net.ssl.tcp_stream(client, ssl_context)
            client:async_handshake(net.ssl.HANDSHAKE_CLIENT, function(error_message)
                if error_message then
                    callback(nil, {ssl_error = error_message})
                    return
                end
                send_request()
            end)
            return
        end
        
        send_request()
    end)
end

function input_stream(content_type, content_length, data_source)
    
    if type(data_source) == "string" then
    
        local value = data_source
        
        data_source = function()
            if not value then
                return nil, 0
            end
            local tmp = value
            value = nil
            return tmp, #tmp
        end
        
        if not content_length or content_length > #value then
            content_length = #value
        end
        
        if not content_type then
            content_type = "text/plain"
        end
    end
    
    if not content_type then
        content_type = "application/octet-stream"
    end
    
    return {
        info = read_only({
            content_type = content_type,
            content_length = content_length
        }),
        read = data_source
    }
end

local function send_body(connection, input_stream, callback)
    local data, data_length = input_stream.read()
    if data then
        connection.socket:async_send(data, function(error_message)
            if error_message then
                callback({socket_error = error_message})
                return
            end
            send_body(connection, input_stream, callback)
        end)
    else
        if data_length == nil then
            connection.close()
            callback({io_error = "error while reading body"})
            return
        end
        callback()
    end
end

local function read_response(connection, options)
    
    local socket = connection.socket
    local completion_handler = options.complete
    
    read_status_line(socket, function(version, status_code, errors)
        
        if errors then
            completion_handler(nil, errors)
            connection.cancel_everything()
            return
        end
        
        read_headers(socket, function(headers, errors)
            
            if errors then
                completion_handler(nil, errors)
                connection.cancel_everything()
                return
            end
            
            local read_body = body_readers[headers["transfer-encoding"] or "identity"]
            
            if not read_body then
                callback(nil,{http_error = string.format("client doesn't support %s transfer encoding", headers["transfer-encoding"])})
                return
            end
            
            read_body(socket, headers, function(body, errors)
                
                if errors then
                    completion_handler(nil, errors)
                    connection.cancel_everything()
                    return
                end
                
                completion_handler(body, {
                    version = version,
                    response_status = status_code,
                    headers = headers
                })
                
                connection.next_response()
            end)
        end)
    end)
end

local function request(connection, options)

    if not connection.socket then
        options.complete({socket_error = options.socket_error})
        connection.next_request()
        return
    end
    
    local socket = connection.socket
    local next_request = connection.next_request
    local ready_to_read = connection.ready_to_read
    local body_sender
    
    local method = options.method
    local path = options.path
    local headers = options.headers
    local body = options.body
    local completion_handler = options.complete
    
    if method == "POST" or method == "PUT" and body then
        headers["Content-Type"] = body.info.content_type
        if body.info.content_length then
            headers["Content-Length"] = body.info.content_length
            body_sender = send_body
        else
            headers["Transfer-Encoding"] = "chunked"
            body_sender = nil
            error("chunked transfer encoding is unsupported")
        end
    else
        body = nil
        body_sender = function(connection, input_stream, callback) callback() end
    end
    
    local request = method .. " " .. path .. " HTTP/1.1\r\n"
    
    for name, value in pairs(headers) do
        request = request .. name .. ": " .. value .. "\r\n"
    end
    
    request = request .. "\r\n"

    socket:async_send(request, function(error_message)
        
        if error_message then
            completion_handler(nil, {socket_error = error_message})
            connection.cancel_everything()
            return
        end
        
        body_sender(connection, body, function(errors)
            
            if errors then
                completion_handler(nil, errors)
                connection.cancel_everything()
                return
            end
            
            ready_to_read(function(errors)
                
                if errors then
                    -- Another request failed while reading the response and then cancel_everything was called
                    completion_handler(nil, errors)
                    return
                end
                
                read_response(connection, options)
            end)
            
            connection.next_request()
        end)
    end)
end

function connection(hostname, port)
    
    local client = net.tcp_client()
    local connection_error_message
    local closed_connection = false
    local connected = false
    local pending_requests = {}
    local read_queue = {}
    local next_request
    
    local function close()
        client:close()
        connected = false
        closed_connection = true
        connection_error_message = connection_error_message or "connection was shutdown"
    end
    
    local function cancel_everything()
    
        if not connected then
            return
        end
        
        close()
        
        local error_info = {socket_error = connection_error_message}
        
        for _, reader in pairs(read_queue) do
            reader(error_info)
        end
        read_queue = {}
        
        for _, request in pairs(pending_requests) do
            request.complete(nil, error_info)
        end
        pending_requests = {}
    end
    
    local function ready_to_read(callback)
        table.remove(pending_requests, 1)
        read_queue[#read_queue +1] = callback
        
        if #read_queue == 1 then
            callback()
        end
    end
    
    local function next_response()
        
        table.remove(read_queue, 1)
        
        if #read_queue == 0 then
            return
        end
        
        read_queue[1]()
    end
    
    local function next_request()
        
        if #pending_requests == 0 then
            return
        end
        
        local request_options = pending_requests[1]
        
        request({
            socket = client,
            cancel_everything = cancel_everything,
            next_request = next_request,
            ready_to_read = ready_to_read,
            next_response = next_response}, request_options)
    end
    
    client:async_connect(hostname, port, function(error_message)
          
        if error_message then
            connection_error_message = error_message
            closed_connection = true
        else
            connected = true
        end
        
        next_request()
    end)
    
    local function queue_request(_, options)
    
        local function host_port_value()
            if port ~= 80 then return ":" .. port else return "" end
        end
        
        options.method = options.method or "GET"
        options.headers = options.headers or {}
        options.headers.Host = hostname .. host_port_value()
        
        pending_requests[#pending_requests + 1] = options
        if #pending_requests == 1 and connected then
            next_request()
        end
    end
    
    return {
        request = queue_request,
        close = close
    }
end

