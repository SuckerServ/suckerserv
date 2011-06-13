--[[

	A player command to send a message to all spectators

]]


return function(cn, ...)

	local text = ""

	for _, item in ipairs(arg) do
		item = tostring(item)
		if #item > 0 then
			if #text > 0 then
				text = text .. " "
			end
			text = text .. item
		end
	end

	for client in server.gspectators() do
		server.player_msg(client.cn, string.format(server.specmsg_command_message, server.player_name(cn), cn, text))
	end

end
