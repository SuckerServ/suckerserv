--[[
	A player command to list players and their country, frags, deaths and accuracy
]]

return function(cn)
    for p in server.gplayers() do
        local country = server.mmdatabase:lookup_ip(p:ip(), "country", "names", "en")
        if not country or #country < 1 then country = "Unknown" end
        server.player_msg(cn, "player_list", { name = p:displayname(), country = country, frags = p:frags(), deaths = p:deaths(), acc = p:accuracy() })
    end
end

