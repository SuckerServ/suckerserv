--[[
    Vote for map at intermission 
    By piernov
]]

mapbattle = {}
mapbattle.delay = 1000 -- Time to wait before starting mapbattle
mapbattle.timeout = server.intermission_time - 2500 -- Time to wait for votes
mapbattle.range = 2 -- Distance between nextmap and second map in maprotation

function mapbattle.clean()
    mapbattle.votes = { {}, {} }
    mapbattle.maps = {}
    mapbattle.map_changed = false
    mapbattle.running = false
end

function mapbattle.get_next_map(num, mode)
    local maps = map_rotation.get_map_rotation(mode)[mode]
	local playing = 1
    for k,v in ipairs(maps) do
        if v == server.map then 
            playing = k
        end
    end
    if playing > (table_size(maps)-num) then playing = 1 end
    return maps[playing+num]
end

function mapbattle.winner()
    if table_size(mapbattle.votes[1]) >= table_size(mapbattle.votes[2]) then
        return mapbattle.maps[1]
    else
        return mapbattle.maps[2]
    end
end

function mapbattle.process_vote(cn, vote)
    if vote ~= "1" and vote ~= "2" then return end
    vote = tonumber(vote)

    if table_count(server.players(), cn) ~= 1 then
        server.player_msg(cn, server.mapbattle_cant_vote_message)
        return false
    else
        if mapbattle.votes[1][cn] == true or mapbattle.votes[2][cn] == true then
            server.player_msg(cn, server.mapbattle_vote_already)
            return false
        end
        mapbattle.votes[vote][cn] = true
        server.msg(string.format(server.mapbattle_vote_ok, server.player_displayname(cn), mapbattle.maps[vote]))
        return true
    end
end

function mapbattle.start(map1, map2, mode)
    mapbattle.clean()
    mapbattle.maps = { map1, map2 }
	server.msg("mapbattle_vote", {map1 = mapbattle.maps[1], map2 = mapbattle.maps[2]})
	mapbattle.running = true
	server.sleep(mapbattle.timeout, function()
        mapbattle.running = false
		if not mapbattle.map_changed then
            server.msg("mapbattle_winner", { mapbattle_winner = mapbattle.winner()})
			mapbattle.map_changed = true
		end
	end)
end

server.event_handler("setnextgame", function()
    server.next_mode = server.gamemode
    server.next_map = mapbattle.winner()
    return -1
end)

server.event_handler("intermission", function() 
    server.sleep(mapbattle.delay, function()
        mapbattle.start(map_rotation.get_map_name(server.gamemode), mapbattle.get_next_map(mapbattle.range, server.gamemode), server.gamemode)
    end)
end)

server.event_handler("mapchange", function()
    mapbattle.map_changed = true
end)

server.event_handler("text", function(cn, text)
    if mapbattle.map_changed or not mapbattle.running then return end
    if mapbattle.process_vote(cn, text) then return -1 end
end)
