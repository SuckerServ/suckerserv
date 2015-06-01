--[[
	A little clanwar command 
	Based on fairgame command + pause when someone leave and resume with delay if all players are present
	By piernov <piernov@piernov.org>
]]


local usage = "#cw <map> [<mode>] [lockteams]"

local running, started, teams_locked = false, false, false
local gamecount, countdown = 0, 6
local players = {}
local firsttime = 0
local allmapsloaded = false

local function clear_game()
	running, started, teams_locked, allmapsloaded = false, false, false, false
	gamecount, firsttime = 0, 0
	players = {}
end

local function resume()
	if not countdown then
		server.pausegame(false)
	else
		local cdown = tonumber(countdown)
		if cdown < 1 then
			server.pausegame(false)
		else
			cdown = round(cdown, 0)
			server.interval(1000, function()
				if cdown == 0 then
					server.pausegame(false)
					return -1
				else
					if cdown == 1 then
						server.msg(string.format(server.game_resume_sec, cdown))
					else
						server.msg(string.format(server.game_resume_secs, cdown))
					end
					cdown = cdown - 1
				end
			end)
		end
	end
end

local disconnect = server.event_handler("disconnect", function(cn)
	if not running then return end
	if players[tostring(server.player_id(cn))] then
		server.pausegame(true)
		players[tostring(server.player_id(cn))] = "not_loaded"
	end
end)


local activer = server.event_handler("mapchange", function(map, mode)
	gamecount = gamecount + 1

	firsttime = server.timeleft * 60000

	if gamecount > 1 then
		clear_game()
		return
	end

	if not running then return end

	server.interval(1000, function()
		server.msg(server.fairgame_waiting_message)
		for k,v in pairs(players) do
			if v == "not_loaded" then return end
		end
		allmapsloaded = true
		return -1
	end)

	server.msg(server.fairgame_demorecord_message)
	server.recorddemo()

	local cdown = tonumber(countdown)
	server.interval(1000, function()
		if allmapsloaded then
			cdown = cdown - 1
			server.msg(string.format(server.fairgame_countdown_message, cdown, (cdown > 1) and "s" or ""))

			if cdown == 0 then
				server.msg(server.fairgame_started_message)

				for _, cn in ipairs(server.players()) do
					server.player_nospawn(cn, 0)
					server.player_respawn(cn)
				end

				server.changetime(firsttime)

				started = true
				return -1
			end
		end
	end)
end)


local active = server.event_handler("maploaded", function(cn)
	if not running then return end
	if not players[server.player_id(cn)] then return end

	players[server.player_id(cn)] = "loaded"

	if not started then
		server.player_nospawn(cn, 1)
		server.player_slay(cn)
	end

	if started then
		server.unspec(cn)
		server.player_respawn(cn)
		resume()
	end
end)

local intermission = server.event_handler("intermission", function()
	clear_game()
end)


local teamchange = server.event_handler("chteamrequest", function(cn)
	if not teams_locked or not running then return end
	server.player_msg(cn, server.fairgame_teams_locked_message)
	return -1
end)

return function(cn, map, mode, lockteams)
	if running then
		server.player_msg(cn, server.fairgame_already_running_message)
		return
	end

	if lockteams == "lockteams" then teams_locked = true end

	if not map then
		return false, usage
	end

	mode = mode or server.gamemode

	if not server.parse_mode(mode) then
		return false, server.unrecognized_gamemode_message
	else
		mode = server.parse_mode(mode)
	end

	running = true
	gamecount = 0

	for _, cn in ipairs(server.players()) do
		players[tostring(server.player_id(cn))] = "not_loaded"
	end

	server.mastermode = 2
	server.mastermode_owner = -1

	server.changemap(map, mode, -1)
end
