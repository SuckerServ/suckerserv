
return function()
    
    if not server.mute_spectators then
        return false, "mute spectators module not loaded"
    end
    
    server.mute_spectators(true)
end
