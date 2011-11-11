--[[
	A player command to list all known names to player
	based on the stats.db and player_ip
]]

return function(cn, target_cn)
	if not server.find_names_by_ip then
		return false, "Not available with this database"
	end

	if not target_cn then
		return false, "#names <cn>|\"<name>\""
	end

	if not server.valid_cn(target_cn) then
		target_cn = server.name_to_cn_list_matches(cn,target_cn)
		if not target_cn then
			return
		end
	end

    local current_name = server.player_name(target_cn)
    local names = server.find_names_by_ip(server.player_ip(target_cn), current_name)

    local namelist = ""

    for index, name in ipairs(names) do
        local sep = ""
        if #namelist > 0 then sep = ", " end
        namelist = namelist .. sep .. name
    end

	server.player_msg(cn, string.format(server.names_command_message, current_name, namelist))
end
