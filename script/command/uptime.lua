-- [[ Player command written by Thomas

return function(cn)
    server.player_msg(cn, string.format(server.uptime_message, server.format_duration(server.uptime / 1000)))
end
