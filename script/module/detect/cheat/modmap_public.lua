-- version doesn't force to be spectator, when mod map is used
-- for public servers, this is the better choice

local function info(cn)

	server.player_msg(cn,"You have a " .. red("modified map"))
	server.player_msg(cn,orange("You shouldn't play on this map!") .. " Please, wait for the next map")
	server.player_msg(cn,"and " .. orange("re-download the game") .. " from " .. yellow("www.sauerbraten.org"))
	server.spec(cn)
end

server.event_handler("mapcrcfail",function(cn)

	if ( server.playercount > 2 ) and ( not (server.gamemode == "coop edit") ) and ( server.is_known_map(server.map) ) then
		info(cn)
		server.msg(red("WARNING") .. "   " .. green(server.player_displayname(cn)) .. " has a " .. red("modified map"))
	end
end)

