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
    
    server.msg(string.format(server.action_message, server.player_displayname(cn), text))
end

