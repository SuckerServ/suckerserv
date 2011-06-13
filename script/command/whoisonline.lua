
return function(cn,tcn)

	if not tcn then
		for p in server.gclients() do
			if p:pvars().reserved_name then
				server.player_msg(cn, green(p:name()) .. " (" .. p.cn .. "): " .. yellow(p:pvars().reserved_name))
			elseif p:pvars().stats_auth_name then
				server.player_msg(cn, green(p:name()) .. " (" .. p.cn .. "): " .. yellow(p:pvars().stats_auth_name))
			end
		end
	else
		if not server.valid_cn(tcn) then
			tcn = server.name_to_cn_list_matches(cn,tcn)

			if not tcn then
				return
			end
		end

		if server.player_pvars(tcn).reserved_name then
			server.player_msg(cn, green(server.player_name(tcn)) .. " (" .. tcn .. "): " .. yellow(server.player_pvars(tcn).reserved_name))
		elseif server.player_pvars(tcn).stats_auth_name then
			server.player_msg(cn, green(server.player_name(tcn)) .. " (" .. tcn .. "): " .. yellow(server.player_pvars(tcn).stats_auth_name))
		end
	end

end
