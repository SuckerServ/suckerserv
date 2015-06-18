
local set_cancel_restart = false

function server.restart()
    
    if tonumber(server.playercount) == 0 then return server.restart_now() end

    set_cancel_restart = false
    
    server.msg("restart_warning")
    
    local con_handler -- seems it must be declared before assignment to be included in event handler's closure
    con_handler = server.event_handler("connect", function(cn)
    
        if set_cancel_restart then
            server.cancel_handler(con_handler)
        else
            server.player_msg(cn, "restart_warning") 
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
    server.msg("restart_cancelled")
    set_cancel_restart = true
end
