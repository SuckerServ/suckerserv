
return function(cn)
    
    if not server.mute_spectators then
        return false, server.parse_message(cn, "missing_mute_spectator_module")
    end
    
    server.mute_spectators()
    server.player_msg(cn, "all_spectator_unmuted")
end
