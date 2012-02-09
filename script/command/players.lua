--[[
	A player command to list players and their country, frags, deaths and accuracy
]]

return function(cn)
    for p in server.gplayers() do
        server.player_msg(cn, string.format(server.player_list_message, p:displayname(), geoip.ip_to_country(p:ip()), p:frags(), p:deaths(), p:accuracy()))
    end
end

