--[[
	A player command to list players and their country, frags, deaths and accuracy
]]

return function(cn)
    for p in server.gplayers() do
        local country = mmdb.lookup_ip(p:ip(), "country", "names", "en")
        local city = mmdb.lookup_ip(p:ip(), "city", "names", "en")
        if not country or #country < 1 then country = "Unknown" end
        if not city or #city < 1 then city = "Unknown" end
        server.player_msg(cn, "player_list", { name = p:displayname(), city = city, country = country, frags = p:frags(), deaths = p:deaths(), acc = p:accuracy() })
    end
end

