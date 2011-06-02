--[[

	A player command to set a leave-spectator-block

]]


local usage = "#forcespec <cn>|\"<name>\""


return function(cn, tcn, time)

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

	if not server.force_spec then
		return false, "force_spec module is not loaded"
	end

	server.force_spec(tcn,time)

end
