return function(cn)
    local version = server.version()
    local revision = server.revision()
    local uptime = server.format_duration_str(server.uptime)
    local verstr = ""

    if revision > -1 then
        verstr = server.parse_message(cn, "version", {version = version, revision = revision})
    end 

    server.player_msg(cn, "info_command", {uptime = uptime, verstr = verstr})
end
