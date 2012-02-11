--[[
	A player command to list players and their country, frags, deaths and accuracy
]]

return function(cn)
    for p in server.gplayers() do
        local country = geoip.ip_to_country(p:ip())
        local city = geoip.ip_to_city(p:ip())
        if not country or #country < 1 then city = "Unknown" end
        if not city or #city < 1 then city = "Unknown" end
        server.player_msg(cn, string.format(server.player_list_message, p:displayname(), city, country, p:frags(), p:deaths(), p:accuracy()))
    end
end

