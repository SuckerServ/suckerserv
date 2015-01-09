server = {}

local properties = core.vars

setmetatable(server,{
    
    __index = function(table, key)
        
        local value = core[key]
        
        if not value then
            value = properties[key]
            if type(value) == "function" then
                value = value()
            end
        end
        
        return value
    end,
    
    __newindex = function(table, key, value)
        
        local existing_property = properties[key]
        
        if existing_property and type(existing_property) == "function" then
            existing_property(value)
        else
            core[key] = value
        end
        
    end
});

server.event_handler = event_listener.add
server.cancel_handler = event_listener.remove
server.cancel_handlers = event_listener.clear_all
server.create_event_signal = event_listener.create_event
server.cancel_event_signal = event_listener.destroy_event

