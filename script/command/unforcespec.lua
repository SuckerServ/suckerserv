--[[

	A player command to disable the leave-spectator-block

]]


local usage = "#unforcespec <cn>|\"<name>\""


return function(cn, tcn)

	if not tcn then
		return false, usage
	end

	if not server.valid_cn(tcn) then
		tcn = server.name_to_cn_list_matches(cn,tcn)

		if not tcn then
			return
		end
	end

	tcn = tonumber(tcn)

	server.player_vars(tcn).modmap = false		-- when cd/modmap.lua was active

	if not server.unforce_spec then
		server.unspec(tcn)
	end

	server.unforce_spec(tcn)

end
