--[[

    A client that sends player auth requests to a Cube 2 master server
    
    Copyright (C) 2009 Graham Daws
    
    MAINTAINER
        Graham
    
    GUIDELINES
        
    TODO
        
]]

require "net"

local function request_constructor(socket, id, name, domain)
    
    local completed = false
    local completion_handler
    local challenged = false
    local challenge
    
    local function send_handler(errmsg)
        if errmsg and not completed and completion_handler then
            completed = true
            completion_handler(false)
        end
    end
    
    local function start(unused, handler)
        
        if completed then error("request object expired") end
        
        local request_line = string.format("reqauth %i %s", id, name)
        
        if domain then
            request_line = request_line .. " " .. domain
        end
        
        request_line = request_line .. "\n"
        
        socket:async_send(request_line, send_handler)
        
        completion_handler = handler
    end
        
    local function has_completed()
        return completed
    end
    
    local function get_challenge()
        if not challenged then error("haven't received the challenge yet") end
        return challenge
    end
    
    local function send_answer(unused, answer, handler)
        completion_handler = handler
        local reply_line = string.format("confauth %i %s\n", id, answer)
        socket:async_send(reply_line, send_handler)
    end
    
    local reply = {}
    
    function reply.chalauth(aChallenge)
        
        if challenged then
            reply.failauth()
            return
        end
        
        challenged = true
        challenge = aChallenge
        
        completion_handler(true)
    end
    
    function reply.failauth()
        completed = true
        completion_handler(false)
    end
    
    function reply.succauth()
        
        if not challenged then
            reply.failauth()
            return
        end
        
        completed = true
        completion_handler(true)
    end
    
    local function process_reply(unused, arguments)
        if completed then return end
        (reply[arguments[1]] or reply.failauth)(arguments[3])
    end

    local function process_failure()
        completed = true
        completion_handler(false)
    end
    
    return {
        start = start,
        get_challenge = get_challenge,
        send_answer = send_answer,
        has_completed = has_completed,
        process_reply = process_reply,
        process_failure = process_failure
    }
end

local function query_request_constructor(socket, id, name, domain, callback)
    
    local completed = false
    local request_line = string.format("QueryId %i %s %s\n", id, name, domain)
    
    local function has_completed()
        return completed
    end
    
    local function process_failure()
        completed = true
        callback(nil)
    end
    
    local function send_handler(errmsg)
        if errmsg and not completed and completion_handler then
            process_failure()
        end
    end
    
    local function process_reply(unused, arguments)
        
        if arguments[1] == "FoundId" then
            callback(true)
        else
            callback(false)
        end
        
        completed = true
    end
    
    socket:async_send(request_line, send_handler)
    
    return {
        has_completed = has_completed,
        process_reply = process_reply,
        process_failure = process_failure
    }
end

local function client_constructor()

    local socket = net.tcp_client()
    local pending_requests = {}
    local read_failed = false
    
    local function read_error()
        
        read_failed = true
        
        for _, request in pairs(pending_requests) do
            request:process_failure()
        end

        pending_requests = {}

    end

    local function read_reply()
        socket:async_read_until("\n", function(line, errmsg)
            
            if not line then
                read_error()        
                return
            end
           
            local arguments = line:split("[^ \n]+")
            
            local request_id = tonumber(arguments[2])
            local request = pending_requests[request_id]
            request:process_reply(arguments)
            
            if request:has_completed() then
                pending_requests[request_id] = nil
            end
            
            read_reply()
        end)
    end
    
    local function connect(unused, ip, port, callback)
        socket:async_connect(ip, port, function(errmsg)

            if not errmsg then
                read_reply()
            end
            
            callback(errmsg)
        end)
    end
    
    local function disconnect()
        socket:close()
    end
    
    local function new_request(unused, request_id, name, domain)
        
        if pending_requests[request_id] then error("request_id collison") end
        
        local object = request_constructor(socket, request_id, name, domain)
        pending_requests[request_id] = object
        
        return object
    end
    
    local function has_failed()
        return read_failed
    end
    
    local function query_id(unused, request_id, name, domain, callback)
        if pending_requests[request_id] then error("request_id collision") end
        pending_requests[request_id] = query_request_constructor(socket, request_id, name, domain, callback)
    end
    
    return {
        connect = connect,
        disconnect = disconnect,
        new_request = new_request,
        has_failed = has_failed,
        query_id = query_id
    }
end

authp_client = client_constructor
