
local connections = {}
local add_listener, remove_listener

function add_listener(event_id, listener_function)
    
    local listeners = event[event_id]

    if not listeners then
        return
    end
    
    listeners[#listeners + 1] = listener_function
    
    connections[#connections + 1] = {listeners, #listeners}
    
    local connection_id = #connections
    
    return function()
        return remove_listener(connection_id)
    end
end

function remove_listener(connection_id)

    if type(connection_id) == "function" then
        local disconnector = connection_id
        disconnector()
        return
    end
    
    local connection = connections[connection_id]
    if not connection then return end
    table.remove(connection[1], connection[2])
    connections[connection_id] = nil
end

local function clear_listeners()
    for connection_id in pairs(connections) do
        remove_listener(connection_id) 
    end
end

local function trigger_event(event_id, ...)
    local listeners = event[event_id]
    if not listeners then return end
    
    local prevent_default = false
    
    for _, listener in pairs(listeners) do
        local pcall_status, result = pcall(listener, unpack({...}))
        if type(result) == "table" then
            result = result[1]
        end
        if not pcall_status then
            server.log_event_error(event_id, result or "unknown error")
        else
            prevent_default = prevent_default or (result == true)
        end
    end
    
    return prevent_default
end

local function create_event(event_id)
    
    event[event_id] = {}
    
    return function(...)
        trigger_event(event_id, unpack({...}))
    end
end

local function destroy_event(event_id)
    event[event_id] = nil
end

event_listener = {
    add = add_listener,
    remove = remove_listener,
    clear_all = clear_listeners,
    create_event = create_event,
    destroy_event = destroy_event,
    trigger_event = trigger_event
}
