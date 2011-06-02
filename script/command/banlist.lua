
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
				server.player_msg(cn, string.format(blue() .. "Player: " .. red() .. vars.ban_name .. " \f1IP: " .. red() .. ipmask .. blue() " Reason: " .. red() .. vars.ban_reason .. blue() " Time: " .. red() .. vars.ban_time .. blue() " Admin: " .. red() .. vars.ban_admin .. blue() " Expires in: " .. red() .. ban_min .. " Minutes"))
			end
		end
    end	
end	