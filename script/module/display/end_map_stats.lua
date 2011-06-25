--[[
    Show end map stats at intermission 
    By LoveForEver -- 2011 
]]

server.event_handler("intermission", function()
	for _,cn in pairs(server.players())  do
	server.player_msg(cn, string.format(server.intermission_stats_message, server.player_score(cn), server.player_frags(cn), server.player_deaths(cn), server.player_accuracy(cn)))
	end
		for _,cn in pairs(server.spectators())  do
	server.player_msg(cn, string.format(server.intermission_stats_message, server.player_score(cn), server.player_frags(cn), server.player_deaths(cn), server.player_accuracy(cn)))
	end
end)
