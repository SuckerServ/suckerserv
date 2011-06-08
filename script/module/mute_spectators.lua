local default_enabled = server.mute_spectators_enabled_by_default == 1

local is_enabled


function server.mute_spectators(enable)

    is_enabled = enable
end


function msg_admins(cn, msg)
    
    local line = string.format(server.spectator_muted_message, server.player_displayname(cn), msg)
    
    for _, cn in ipairs(server.clients()) do
        if server.player_priv_code(cn) == server.PRIV_ADMIN then
            server.player_msg(cn, line)
        end
    end
end


server.event_handler("text", function(cn, text)

    if server.player_status_code(cn) == server.SPECTATOR and server.player_priv_code(cn) ~= server.PRIV_ADMIN and server.paused == false and is_enabled
    then
        msg_admins(cn, text)
        return -1
    end
end)

server.event_handler("mapchange", function()

    is_enabled = default_enabled
end)


return {unload = function()

    server.unref("mute_specs")
end}
