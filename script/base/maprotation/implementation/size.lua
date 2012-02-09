local big_maps = {}
local small_maps = {}

local small_single_game = server.small_single_game
local small_team_game = server.small_team_game

local function get_map_rotation(selection_index, gamemode)
    
    local small_game
    
    if server.get_gamemode_info(gamemode).teams then
        small_game = small_team_game
    else
        small_game = small_single_game
    end
    
    if server.playercount <= small_game then
        return small_maps
    else
        return big_maps
    end
end

return {
    reload = function() 
        big_maps = map_rotation.load_from_lists("big_")
        small_maps = map_rotation.load_from_lists("small_") 
    end,
    
    get_gamemode = function() 
        return server.gamemode 
    end,
    
    get_map_rotation = get_map_rotation,
    
    get_map_name = function(index, gamemode)
        return table.wrapped_index(get_map_rotation(index, gamemode)[gamemode], index, 1)
    end
}

