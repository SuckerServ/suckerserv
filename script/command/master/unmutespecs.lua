
return function()
    
    if not server.mute_spectators then
        return false, server.missing_mute_spectator_module_message
    end
    
    server.mute_spectators()
end
