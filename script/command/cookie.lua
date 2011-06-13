--[[
    Give a cookie (present) to the people you want
    By LoveForever
]]
return function(cn, tcn, ...)
    
	if not tcn then
		return false, "#cookie <cn>"
	end
 
	if not server.valid_cn(tcn) then
		tcn = server.name_to_cn_list_matches(cn,tcn)

		if not tcn then
			return
		end
	end
    
	server.player_msg(cn, string.format(server.cookie_give_message, server.player_displayname(tcn)))
    server.player_msg(tcn, string.format(server.cookie_received_message, server.player_displayname(cn), green(text)))
end

