local maps = {}

local next_map
local next_map_index

local function random_map(gamemode)
    local gamemode_maps = maps[gamemode]
    return gamemode_maps[math.random(#gamemode_maps)]
end

return {
    reload = function() 
        maps = map_rotation.load_from_lists("")
    end,
    
    get_gamemode = function() 
        return server.gamemode 
    end,
    
    get_map_rotation = function()
        return read_only(maps) 
    end,
    
    get_map_name = function(selection_index, gamemode)

        -- First map since server started        
        if not next_map then
           next_map =  random_map(gamemode)
           return next_map
        end

        if next_map_index ~= selection_index then
            next_map = random_map(gamemode)
            next_map_index = selection_index
        end
        
        return next_map
    end
}

