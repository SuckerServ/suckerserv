--[[
	A player command to allow all players to vote to kick other players.
	Copyright (C) 2009 Thomas
]]

local usage = "#votekick <cn>|\"<name>\""
local votes = {}

local function check_kick(cn)
	local required_votes = round((server.playercount / 2), 0)
	if required_votes < server.votekick_min_votes_required then required_votes = server.votekick_min_votes_required end
	if votes[tostring(server.player_id(cn))].votes >= required_votes then
		server.kick(cn, 3600, "server", "votekick by "..votes[tostring(server.player_id(cn))].votes.." players")
		server.msg(string.format(server.votekick_passed_message, server.player_displayname(cn)))
        votes[tostring(server.player_id(cn))] = nil
	end
    return required_votes
end

local function check_player_on_disconnect(cn)
	local actor_id = tostring(server.player_id(cn))
	for p in server.gclients() do
		victim_id = tostring(server.player_id(p.cn))
		if votes[victim_id] then
			if votes[victim_id][actor_id] then
				votes[victim_id][actor_id] = nil
				votes[victim_id].votes = votes[victim_id].votes - 1
			end
			check_kick(p.cn)
		end
	end
end

local function init()
	votes = {}
	server.event_handler("disconnect", function(cn) check_player_on_disconnect(cn) end)
end

local function unload()
	votes = nil
end

local function run(cn,victim)
	if server.playercount < 3 then
		return false, "There aren't enough players here for votekick to work."
	end
    
	if victim and not server.valid_cn(victim) then
		victim = server.name_to_cn_list_matches(cn,victim)
	end
	
	if not victim then
		return false, usage
	end
	
	local actor_id = tostring(server.player_id(cn))
	
	if victim == cn then
		return false, "You can't vote to kick yourself."
	end
	
	if server.player_priv(victim) == "admin" then
		return false, "You can't vote to kick a server admin!"
	end
	
	victim_id = tostring(server.player_id(victim))
	
	if not votes[victim_id] then
		votes[victim_id] = {}
	end

	if votes[victim_id][actor_id] then
		return false, "You have already voted for this player to be kicked."
	end

	votes[victim_id][actor_id] = true
	
    if not votes[victim_id].votes then
        votes[victim_id].votes = 0
    end
	
    votes[victim_id].votes = votes[victim_id].votes + 1

	local required_votes = check_kick(victim)
	local msg = string.format(server.votekick_vote_message, server.player_displayname(cn), server.player_displayname(victim), votes[victim_id].votes, required_votes)

	if server.player_priv_code(victim) == server.PRIV_MASTER then
		for p in server.gplayers() do
			if not p.cn == victim then
				p:msg(msg)
			end
		end
	else
		server.msg(msg)
	end
end

return {init = init,run = run,unload = unload}