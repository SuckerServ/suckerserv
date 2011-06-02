--[[

	A player command to list all known names to player
	based on the stats.db and player_ip

]]


return function(cn, target_cn)

	if server.stats_use_sqlite == 0 then
		return false, "command requires sqlite"
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

	local db = sqlite3.open(server.stats_db_filename)

	local str = "Other names used by " .. server.player_name(target_cn) .. ": "
	for name, count in db:cols("SELECT DISTINCT name, count(name) as count FROM players WHERE ipaddr = '" .. server.player_ip(target_cn) .. "'") do
		str = str .. name .. "(" .. count .. "),"
	end
	server.player_msg(cn, str)

	db:close()

end
