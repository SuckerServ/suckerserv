local max_players = server.maxplayers

local function calcOutput(players, specs)
	return players ^ 2 - players + (players * specs)
end

local output_limit = calcOutput(max_players, 0)

local function readjustCapacity()
    
    if server.playercount > server.maxplayers then return end
    
	local sc = server.speccount
	local adminslots = server.reserved_slots_occupied
	local pc = server.playercount - sc - adminslots
	local extra = -1
    
	while calcOutput(pc + (extra + 1), sc) <= output_limit do
		extra = extra + 1
	end
    
	if gamemodeinfo.teams then
		extra = extra - ((pc + extra)%2)
	end
    
	server.maxplayers = pc + extra + sc + adminslots
end

local function isOverCapacity()
	return calcOutput(server.playercount, server.speccount) > output_limit
end

server.event_handler("spectator", function(cn, value)

	if value == 0 and isOverCapacity() then
        local sid = server.player_sessionid(cn)
		server.sleep(1, function()
            if server.player_sessionid(cn) ~= sid then return end
			server.spec(cn)
			server.player_msg(cn, red("Sorry, there are too many players in the game at the moment, you'll have to wait until someone leaves the game."))
		end)
	end

	readjustCapacity()
end)

server.event_handler("disconnect", function(cn, reason)
	readjustCapacity()
end)

local function unload()
	server.maxplayers = max_players
end

return {unload = unload}
