
return function(cn)
    
    if not server.mute_spectators then
        return false, server.missing_mute_spectator_module_message
    end
    
    server.mute_spectators()
    server.player_msg(cn, server.all_spectator_unmuted_message)
end
