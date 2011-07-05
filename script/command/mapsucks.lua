--[[

    A player command to lower time when mapsucks ratio is reached
    By piernov <piernov@piernov.org>

]]

local ratio = server.mapsucks_ratio
local lower_time = server.mapsucks_lower_time
local mapsucks = {}

server.event_handler("text", function(cn, text)
	    if (string.match(text, "SUCKS") or string.match(text, "sucks")) and not (string.match(text, "[^#.*]"))
		then 
		server.player_msg(cn, string.format(server.mapsucks_analysetext_message))
	    end
end)

server.event_handler("connect", function(cn)
    cn_id = server.player_id(cn)
    if not mapsucks[cn_id] then return end
    mapsucks[cn_id] = cn
end)

server.event_handler("mapchange", function(map, mode)
    mapsucks = {}
end)

return function(cn)
    for _,player in pairs(server.players()) do
        if cn == player then
            cn_id = server.player_id(cn)
            if not mapsucks[cn_id] then
                mapsucks[cn_id] = cn
                mapsucks_size = table_size(mapsucks)
                if mapsucks_size > 2 then plural = "s" else plural = "" end
                server.player_msg(cn, string.format(server.mapsucks_message, (mapsucks_size - 1)))
                server.msg(string.format(server.mapsucks_announce_message, mapsucks_size))
                if mapsucks_size > (#server.players() / ratio) then
                    server.changetime(lower_time*60*1000)
                    if lower_time > 1 then plural_time = "s" else plural_time = "" end
                    if #server.players() > 1 then plural_players = "s" else plural_players = "" end
                    if plural_players == "s" then conjugate = "" else conjugate = "s" end
                    server.msg(string.format(server.mapsuckes_timelowered_message, lower_time, plural_time, mapsucks_size, #server.players(), plural_players, conjugate))
                end
                return
            else
                server.player_msg(cn, server.mapbattle_vote_already)
                return
            end
        end
    end
    server.player_msg(cn, server.mapbattle_cant_vote_message)
    return -1
end