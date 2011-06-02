
function server.write_server_status(filename, filemode)

	if not filemode then
		filemode = "a+"
	end

	local out = io.open(filename, filemode)

	local status_rows = "PLAYERS MAP MODE MASTER HOST PORT DESCRIPTION\n"
	local host = server.serverip
	if #host == 0 then host="<ANY>" end
	local mm = server.mastermode
	local desc = string.gsub(server.servername, " ", "_")
	if #desc == 0 then desc = "<NONE>" end

	local mapname = server.map
	if #mapname == 0 then mapname = "<NONE>" end

	status_rows = status_rows .. string.format("%i/%i %s %s %i %s %i %s", server.playercount, server.maxclients, mapname, server.gamemode, mm, host, server.serverport, desc)

	out:write(tabulate(status_rows))
	out:write("\n")

	if server.playercount > 0 then

		local player_rows = "CN LAG PING IP CO NAME TIME STATE PRIV\n"

		for p in server.gall() do

			local country = geoip.ip_to_country_code(p:ip())
			if #country == 0 then country = "?" end

			local priv = p:priv()
			if server.master == cn then priv = "*" .. priv end

			local player_row = string.format("%i %i %i %s %s %s %s %s %s",
				p.cn, p:lag(), p:ping(), p:ip(), country, p:name(), format_duration(p:connection_time()), p:status(), priv)

			player_rows = player_rows .. player_row .. "\n"

		end

		out:write("\n")
		out:write(tabulate(player_rows))
	end

	out:flush()
	out:close()
end

