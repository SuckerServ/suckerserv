
local set_cancel_restart = false

function server.restart()
    
    if tonumber(server.playercount) == 0 then return server.restart_now() end

    set_cancel_restart = false
    local warning_msg = "The server is set for a restart at the end of this game."
    
    server.msg(warning_msg)
    
    local con_handler -- seems it must be declared before assignment to be included in event handler's closure
    con_handler = server.event_handler("connect", function(cn)
    
        if set_cancel_restart then
            server.cancel_handler(con_handler)
        else
            server.player_msg(cn, warning_msg) 
        end
    end)
    
    local mapch_handler
    mapch_handler = server.event_handler("setnextgame", function()
        
        if set_cancel_restart then
           server.cancel_handler(mapch_handler)
        else
            server.restart_now()
        end
    end)
    
end

function server.cancel_restart()
    server.msg("Server restart cancelled.")
    set_cancel_restart = true
end
