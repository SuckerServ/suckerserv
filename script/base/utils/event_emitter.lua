EventEmitter = {}

function EventEmitter.init(self)
    if not self then
        self = {}
        setmetatable(self, {__index = EventEmitter})
    end
    self._listeners = {}
    return self
end

function EventEmitter:emit(event_name, ...)
    local listeners = self._listeners[event_name]
    if not listeners then return end
    for _, listener in pairs(listeners) do
        local pcall_status, error_message = pcall(listener, unpack(arg))
        if not pcall_status then
            
            if type(error_message) == "table" then
                error_message = error_message[1]
            end
            
            server.log_event_error(event_name, error_message)
        end
    end
end

function EventEmitter:on(event_name, callback)
    local listeners = self._listeners[event_name] or {}
    listeners[#listeners + 1] = callback
    self._listeners[event_name] = listeners
    return #listeners
end

function EventEmitter:once(event_name, callback)
    
    local function once_handler(...)
        self:remove_listener(event_name, once_handler)
        callback(unpack(arg))
    end
    
    self:on(event_name, once_handler)
end

function EventEmitter:remove_listener(event_name, callback)
    
    local listeners = self._listeners[event_name]
    if not listeners then return false end
    
    if type(callback) == "number" then
        table.remove(listeners, callback)
        return
    end
    
    assert(type(callback) == "function")
    
    for index, listener in pairs(listeners) do
        if listener == callback then
            table.remove(listeners, index)
            return true
        end
    end
    
    return false
end

function EventEmitter:listeners(event_name)
    if event_name then
        return self._listeners[event_name]
    else
        return self._listeners
    end
end


