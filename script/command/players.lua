--[[

	A player command to list players with frags, deaths and accuracies

	Copyright (C) 2009 Thomas

]]

return function(cn)
	for p in server.gplayers() do
		server.player_msg(cn, "Player: " .. p:displayname() .. "Country:" .. geoip.ip_to_country(p:ip()) .. " Frags: " .. p:frags() .. " Deaths: " .. p:deaths() .. " Acc: " .. p:accuracy() .. "%")
	end
end

