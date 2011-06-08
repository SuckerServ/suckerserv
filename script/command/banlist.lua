
return function(cn) 
	for ipmask, vars in pairs(server.ip_vars()) do
		if vars.ban_expire then
			if vars.ban_expire > os.time() then
				if vars.ban_reason == "" then
					vars.ban_reason = "none"
				end	
				if vars.ban_name == nil then
					vars.ban_name = ""
				end
				local ban_min = round((vars.ban_expire - os.time()) / 60)
				server.player_msg(cn, string.format(server.banlist_message, vars.ban_name, ipmask, vars.ban_reason, vars.ban_time, vars.ban_admin, ban_min))
			end
		end
    end	
end	