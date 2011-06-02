
-- helper
--
-- send message and set person to spectator
local function stresser(cn)

	server.player_msg(cn,"You have a " .. red("modified map"))
	server.player_msg(cn,orange("You cannot play on this map!") .. " Please, wait for the next map")
	server.player_msg(cn,"and " .. orange("re-download the game") .. " from " .. yellow("www.sauerbraten.org"))
	server.spec(cn)
end

-- safe player, when he has a modmap [and mode is not coop and map is known]
server.event_handler("mapcrcfail",function(cn)

	if ( server.playercount > 2 ) and ( not (server.gamemode == "coop edit") ) and ( server.is_known_map(server.map) ) then
		server.player_vars(cn).modmap = true
		stresser(cn)
		server.msg(green(server.player_displayname(cn)) .. " has a " .. red("modified map"))
	end

end)

-- check safed player, when he tries to leave spectator
server.event_handler("spectator",function(cn,joined)

	if ( joined == 0 ) and ( server.player_vars(cn).modmap ) then
		stresser(cn)
	end

end)

