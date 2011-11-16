return function(cn) 
    for ipmask, vars in pairs(server.ip_vars()) do
        if (vars.ban_expire or 0) > os.time() and (vars.is_gban or false) then
            server.player_msg(cn, string.format(
                "IP: %s Reason: %s%s%s",
                ipmask,
                vars.ban_reason or "unknown",
                _if(vars.ban_expire - os.time() < 31536000, string.format(blue(" (expires in: %s)"), server.format_duration_str(vars.ban_expire - os.time())), ""),
                _if(vars.ignore_gban or false, red(" (deleted)"), "")
                        ))
        end 
    end
end
