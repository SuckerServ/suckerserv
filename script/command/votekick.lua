--[[

	A player command to allow all players to vote to kick other players

	Copyright (C) 2009 Thomas

]]

local MIN_VOTES_REQUIRED = 6

local priv_master = server.PRIV_MASTER
local votes = {}

local usage = "#votekick <cn>|\"<name>\""


local function check_kick(cn)

	local required_votes = round((server.playercount / 2), 0)

	if votes[cn].votes >= required_votes then

		server.kick(cn,3600,"server","votekick")
		server.msg(server.votekick_passed_message, server.player_displayname(cn))
	end

end


local function check_player_on_disconnect(tcn,dcn)

	if votes[tcn] then

		if votes[tcn][dcn] then

			votes[tcn][dcn] = nil
			votes[tcn].votes = votes[tcn].votes - 1
		end

		check_kick(tcn)
	end

end


local function init()

	votes = {}

	server.event_handler("disconnect", function(cn, reason)

		if votes[cn] then

			votes[cn] = nil

		else

			for p in server.gclients() do

				check_player_on_disconnect(p.cn)
			end
		end

	end)
end


local function unload()
	
end


local function run(cn,kick_who)

	if server.playercount < 3 then

		return false, "There aren't enough players here for votekick to work"
	end
    
	if not kick_who then

		return false, usage
	end

	if not server.valid_cn(kick_who) then

		kick_who = server.name_to_cn_list_matches(cn,kick_who)

		if not kick_who then

			return
		end
	end

	kick_who = tonumber(kick_who)
	cn = tonumber(cn)

	if kick_who == cn then

		return false, "You can't vote to kick yourself"
	end
	
	if server.player_priv(kick_who) == "admin" then
		return false, "You can't vote to kick a server admin!"
	end 

	if not votes[kick_who] then

		votes[kick_who] = {}
	end

	if votes[kick_who][cn] then

		return false, "You have already voted for this player to be kicked"
	end

	votes[kick_who][cn] = true
    if not votes[kick_who].votes then

        votes[kick_who].votes = 0
    end
    votes[kick_who].votes = votes[kick_who].votes + 1

	local required_votes = math.min(round((server.playercount / 2), 0), MIN_VOTES_REQUIRED)
	local msg = green(server.player_displayname(cn)) .. " voted to kick " .. red(server.player_displayname(kick_who)) .. "\n" .. "Votes: " .. votes[kick_who].votes .. " of " .. required_votes

	if server.player_priv_code(kick_who) == priv_master then

		for p in server.gplayers() do

			if not (p.cn == kick_who) then

				p:msg(msg)
			end
		end

	else

		server.msg(msg)
	end

	check_kick(kick_who)

end


return {init = init,run = run,unload = unload}
