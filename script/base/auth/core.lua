
require "crypto"

auth = {} -- create table for auth namespace
local internal = {} -- internal namespace to keep local functions in reference

-- TODO setup a metatable to make these values immutable
auth.request_status = {
    SUCCESS = 0,            -- authenticated
    REQUEST_FAILED = 1,     -- client didn't respond to server's request for an auth challenge
    CHALLENGE_FAILED = 2,   -- failed to receive challenge or send challenge to client
    RESPONSE_FAILED = 3,    -- auth server failed to verify the client's response
    CANCELLED = 4,          -- client disconnected
    TIMEOUT = 5             -- not waiting any longer
}

exec("directory.lua")
exec("client.lua")

local requests = {}
local next_request_id = 1

local domain_listeners = {} -- handlers that want to be notified of a challenge completion for a domain
local clients = {} -- used to store run-once listeners and challenge cache

local function init_client_state(cn)

    if clients[cn] == nil then
        clients[cn] = {
            listeners = {},
            has_key   = {},
            authed    = {},
            requests  = {},
        }
    end
end

local function call_listeners(cn, user_id, domain, status)
    
    local listeners = domain_listeners[domain] or {}
    
    for _, listener in ipairs(listeners) do
        catch_error(listener, cn, user_id, domain, status)
    end
    
    -- Run-once/temporary listeners created for auth requests initiated by a server module
    listeners = clients[cn].listeners[domain] or {}
    
    for _, listener in ipairs(listeners) do
        catch_error(listener, cn, user_id, domain, status)
    end
    
    clients[cn].listeners[domain] = nil
    
end

local function complete_request(request, status)
    
    if status == auth.request_status.SUCCESS then
        clients[request.cn].authed[request.domain.id] = request.user_id
    end
    
    if not server.player_has_joined_game(request.cn) then
        if status == auth.request_status.SUCCESS then
            server.player_join_game(request.cn)
        else
            server.player_reject_join_game(request.cn)
        end
    end
    
    request.remote_request = nil
    
    requests[request.id] = nil
    clients[request.cn].requests[request.id] = nil
    
    call_listeners(request.cn, request.user_id, request.domain.id, status)
end

local request_object_methods = {
    complete = complete_request
}

local function create_request(cn, user_id, domain)
    
    local request_object = {
        id = next_request_id,
        user_id = user_id,
        cn = cn,
        domain = domain
    }
    
    setmetatable(request_object, {__index = request_object_methods})
    
    local request_id = next_request_id
    
    requests[request_id] = request_object
    clients[cn].requests[request_id] = request_object
    
    server.sleep(10000, function()
        
        if not requests[request_id] then
            return
        end
        
        request_object:complete(auth.request_status.TIMEOUT)
    end)
    
    next_request_id = next_request_id + 1
    
    return request_object
end

local function setup_authserver_connection(domain, callback)
    
    domain.server.connection = authp_client()
    
    local hostname = domain.server.hostname
    local port = domain.server.port
    
    domain.server.connection:connect(hostname, port, callback)
end

local function defer_request_until_connected(cn, user_id, domain)
    
    setup_authserver_connection(domain, function(error_message)
        
        if error_message then
            call_listeners(cn, user_id, domain.id, auth.request_status.CHALLENGE_FAILED)
            domain.server.connection = nil
            return
        end
        
        internal.send_request(cn, user_id, domain)
    end)
end

function internal.send_request(cn, user_id, domain)
    
    if domain.server.connection:has_failed() then
    
        domain.server.connection:disconnect()
        domain.server.connection = nil
        
        defer_request_until_connected(cn, user_id, domain)
        return
    end
    
    local remote_request = domain.server.connection:new_request(next_request_id, user_id, domain.id)
    
    local request = create_request(cn, user_id, domain)
    request.remote_request = remote_request
    
    remote_request:start(function(success)
        
        if not success then
            
            if domain.server.connection:has_failed() then
                domain.server.connection:disconnect()
                domain.server.connection = nil
            end
            
            request:complete(auth.request_status.CHALLENGE_FAILED)
            
            return
        end
        
        server.send_auth_challenge_to_client(cn, request.id, request.domain.id, remote_request:get_challenge())
        
    end)
    
end

local function start_auth_challenge(cn, user_id, domain)

    init_client_state(cn)

    local domain_id = domain
    
    domain = auth.directory.get_domain(domain)
    
    if not domain then
        server.player_msg(cn, server.auth_unknown_domain_message)
        return
    end
    
    clients[cn].has_key[domain_id] = true
    
    -- Check for previously successful auth challenge
    if clients[cn].authed[domain_id] == user_id then
        call_listeners(cn, user_id, domain_id, auth.request_status.SUCCESS)
        return
    end
    
    if not domain.server.remote then
        
        local user = domain:get_user(user_id)
        
        if not user then
            call_listeners(cn, user_id, domain_id, auth.request_status.CHALLENGE_FAILED)
            return
        end
        
        local request = create_request(cn, user_id, domain)
        
        local key = crypto.sauerecc.key(user.public_key)
        
        request.local_request = {}
        request.local_request.challenge = key:generate_challenge()
        
        server.send_auth_challenge_to_client(cn, request.id, request.domain.id, request.local_request.challenge:to_string())
        
        return
    end
    
    if not domain.server.connection then
        defer_request_until_connected(cn, user_id, domain)
        return
    end
    
    internal.send_request(cn, user_id, domain)
end

server.event_handler("request_auth_challenge", start_auth_challenge)

server.event_handler("auth_challenge_response", function(cn, request_id, answer)
    
    local request = requests[request_id]
    
    if not request then
        return
    end
    
    if request.remote_request then
    
        request.remote_request:send_answer(answer, function(success)
            
            if not success then
                request:complete(auth.request_status.RESPONSE_FAILED)
                return
            end
            
            request:complete(auth.request_status.SUCCESS)
        end)
        
    elseif request.local_request then
        
        local status
        
        if request.local_request.challenge:expected_answer(answer) then
            status = auth.request_status.SUCCESS
        else
            status = auth.request_status.RESPONSE_FAILED
        end
        
        request:complete(status)
    end

end)

server.event_handler("connect", function(cn)
    init_client_state(cn)
end)

server.event_handler("disconnect", function(cn)
    
    local client_auth_info = clients[cn]
    if not client_auth_info then return end -- DEBUG
    
    for _, request in ipairs(client_auth_info.requests) do
        request:complete(auth.request_status.CANCELLED)
    end
    
    clients[cn] = nil
end)

function auth.listener(domain_id, callback)
    
    local handlers = domain_listeners[domain_id]
    
    if not handlers then
        handlers = {}
        domain_listeners[domain_id] = handlers
    end
    
    local index = #handlers + 1
    
    handlers[index] = callback
    
    return index
end

function auth.cancel_listener(domain, index)
    table.remove(domain_listeners[domain], index)
end

local function push_request_listener(cn, domain_id, callback)

    local listeners = clients[cn].listeners[domain_id]
    
    if not listeners then
        listeners = {}
        clients[cn].listeners[domain_id] = listeners
    end
    
    table.insert(listeners, callback)
end

function auth.send_request(cn, domain_id, callback)
        
    push_request_listener(cn, domain_id, callback)
    
    server.send_auth_request(cn, domain_id)
    
    local session_id = server.player_sessionid(cn)
    
    server.interval(1000, function()
        if session_id ~= server.player_sessionid(cn) then return -1 end
        if server.player_vars(cn).maploaded then
            if not clients[cn].has_key[domain_id] then
                call_listeners(cn, "", domain_id, auth.request_status.REQUEST_FAILED)
            end
            return -1
        end
    end)
    
end

function auth.query_id(user_id, domain_id, callback)
    
    local domain = auth.directory.get_domain(domain_id)
    if not domain then error("unknown domain") end
    
    if not domain.server.remote then
        local user = domain:get_user(user_id)
        local found = user ~= nil
        callback(found)
        return
    end
    
    local function send_query()
        domain.server.connection:query_id(next_request_id, user_id, domain_id, callback)
        next_request_id = next_request_id + 1
    end
    
    if not domain.server.connection then
        
        setup_authserver_connection(domain, function(errmsg)
            
            if errmsg then
                callback(nil)
                return
            end
            
            send_query()
        end)
        
        return
    end
    
    send_query()
end

-- initialization on (re)load lua (no bots)
for p in server.gclients() do
    init_client_state(p.cn)
end
