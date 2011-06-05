return function(cn)

    if not map_rotation then
        server.player_msg(cn, "clients are controlling the map rotation")
    end
    
    local map =  map_rotation.get_map_name(server.gamemode)
    server.player_msg(cn, string.format(server.nextmap_message, map))
end

