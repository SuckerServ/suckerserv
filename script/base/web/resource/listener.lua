require "http_server"
local Json = require "Json"

local listeners = {}
local specialHandlerConstructors = {}

specialHandlerConstructors.timeupdate = function(eventName, listener, dequeueFunction)
    return server.event_handler("timeupdate", function(minutesLeft, secondsLeft)
        table.insert(listener.queue, {name = "timeupdate", args = {minutesLeft, secondsLeft}})
        dequeueFunction()
        return mins
    end)
end

local function defaultHandlerConstructor(eventName, listener, dequeueFunction)
    return server.event_handler(eventName, function(...)
        table.insert(listener.queue, {name = eventName, args = {...}})
        dequeueFunction()
    end)
end

local function createListener(events)
    
    local nextId = #listeners + 1
    local listener = {}
    local last_request_time = nil
    local handlers = {}
    
    listener.queue = {}
    listener.request = nil
    
    local function destroy_listener()
        listener = nil
        for _, handlerId in ipairs(handlers) do
            server.cancel_handler(handlerId)
        end
    end
    
    local function dequeueEvents()
        
        if not listener or not listener.request or #listener.queue == 0 then
            return
        end
    
        http_response.send_json(listener.request, listener.queue)
        
        listener.queue = {}
        listener.request = nil
        
        last_request_time = server.enet_time_get()
    end
    
    for _, eventName in ipairs(events) do
        local handlerConstructor = specialHandlerConstructors[eventName] or defaultHandlerConstructor
        table.insert(handlers, handlerConstructor(eventName, listener, dequeueEvents))
    end
    
    server.interval(30000, function()
        
        if not listener.request and (not last_request_time or server.enet_time_get() - last_request_time > 29999) then
            destroy_listener()
            return -1
        end
        
        table.insert(listener.queue, {name = "keep-alive", args = {}})
        dequeueEvents()
    end)
    
    listener.resource = http_server.resource({
        get = function(request)
        
            if web_admin.require_backend_login(request) then
                return
            end
            
            if listener.request then
                http_response.send_error(listener.request, 503, "request has been overloaded")
            end
            
            listener.request = request
            
            dequeueEvents()
        end,
        
        delete = function(request)
            
            if web_admin.require_backend_login(request) then
                return
            end
            
            listener = nil
        end
    })
    
    listeners[nextId] = listener
    return nextId
end

http_server_root["listener"] = http_server.resource({

    resolve = function(name)
        local listener = listeners[tonumber(name)]
        if not listener then return nil end
        return listener.resource
    end,
    
    post = function(request)
        
        request:async_read_content(function(content)
        
            if web_admin.require_backend_login(request) then
                return
            end
            
            local events = Json.Decode(content)
            local id = createListener(events)
            
            http_response.send_json(request, {listenerURI = "/listener/" .. id})
        end)
    end
})
