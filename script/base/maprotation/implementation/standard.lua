local maps = {}

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
    
    get_map_name = function(index, gamemode)
        return table.wrapped_index(maps[gamemode], index, 1)
    end
}

