return function(cn) 
    for ipmask, vars in pairs(server.ip_vars()) do
        if (vars.ban_expire or 0) > os.time() then
            server.player_msg(cn, string.format(
                server.banlist_message,
                vars.ban_name or "unknown",
                ipmask,
                vars.ban_reason or "unknown",
                vars.ban_admin or "unknown",
                server.format_duration_str(vars.ban_expire - os.time())
                ))
        end
    end	
end	
