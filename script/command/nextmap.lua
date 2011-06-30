--[[
Nextmap script
*Show next map (from hopmod trunk)
*Show second map of mapbattle (LoveForEver for Suckerserv)
--]]

function get_next_map(num, mode)
    if mode == nil then mode = server.gamemode or "ffa" end
    maps =  map_rotation.get_map_rotation(mode)
    local mapvar = maps[mode]
    local playing = 0
    for k,v in pairs(mapvar) do
        if v == server.map then 
            playing = k
        end
    end
    local countmaps = #mapvar or 0
    if playing > countmaps-2 then playing = 0 end
    local nextmap = mapvar[playing+num]
    return nextmap or mapbattle.defaultmap
end

return function(cn)

    if not map_rotation then
        server.player_msg(cn, "clients are controlling the map rotation")
    end
	
    local map1 = map_rotation.get_map_name(server.gamemode)
    local map2 = get_next_map(2)
    
    local map =  map_rotation.get_map_name(server.gamemode)
    server.player_msg(cn, string.format(server.nextmap_message, map1, map2))
end

