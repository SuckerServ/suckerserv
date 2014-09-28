-- [[ Player command written by Thomas

return function(cn)
    server.player_msg(cn, "uptime", {uptime = server.format_duration_str(server.uptime)})
end
