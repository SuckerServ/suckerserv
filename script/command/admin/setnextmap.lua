local normal_implementation

local function create_map_rotation_implementation(map, mode)

    local current_map_rotation = map_rotation.get_map_rotation()
    local handler_id
    
    handler_id = server.event_handler("mapchange", function()
        
        map_rotation.set_implementation(normal_implementation)
        normal_implementation = nil
        
        server.cancel_handler(handler_id)
    end)
    
    return {
        reload = function()
        end,
        
        get_gamemode = function() 
            return mode
        end,
        
        get_map_rotation = function()
            return current_map_rotation
        end,
        
        get_map_name = function()
            return map
        end
    }
end

return function(cn, map, mode)
    
    if not map_rotation then
        return false, server.maprotation_disabled_message
    end
    
    if not map then
        return false, "#setnextmap <map> [<mode>]"
    end    
    
    mode = mode or server.gamemode    
    
    if not server.parse_mode(mode) then
        return false, server.unrocognized_gamemode_message
    end
    
    local last_implementation = map_rotation.set_implementation(create_map_rotation_implementation(map, mode))
    
    normal_implementation = normal_implementation or last_implementation
end

