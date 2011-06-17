--[[
    A little clanwar command 
    Based on fairgame command + pause when someone leave and resume with delay if all players are present
    By piernov <piernov@piernov.org>
]]


local running = false
local runned = false
local teams_locked = false
local gamecount = 0
local players = {}
local countdown = 6

local function clear_game()
    running = false
    runned = false
    teams_locked = false
    gamecount = 0
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
    if players[server.player_id(cn)] then server.pausegame(true) end
    players[server.player_id(cn)] = "not_loaded"
end)

local active = server.event_handler("maploaded", function(cn)

    if not running then return; end
	if not players[server.player_id(cn)] then return; end
    players[server.player_id(cn)] = "loaded"

    if not runned then
        server.player_nospawn(cn, 1)
        server.player_slay(cn)
    end
    
    for k,v in pairs(players) do
        if v == "not_loaded" then return end
    end

    if runned then 
        server.unspec(cn)
        server.player_respawn(cn)
        resume()
    else
        local cdown = tonumber(countdown)
        server.interval(1000, function()
            cdown = cdown - 1
            if cdown > 1 then plural = "s"; else plural = ""; end
            server.msg(string.format(server.fairgame_countdown_message, cdown, plural))
            if cdown == 0 then
                server.pausegame(false)
                server.msg(server.fairgame_started_message)
                for _, cn in ipairs(server.players()) do server.player_nospawn(cn, 0); server.player_respawn(cn) end
                runned = true
                return -1
            end
        end)
    end
end)

local intermission = server.event_handler("intermission", function()
    clear_game()
end)

local mapchange = server.event_handler("mapchange", function(map, mode)
    if not running then return; end
    gamecount = gamecount + 1
    
    if gamecount > 1 then
        clear_game()
        return
    end

    server.msg(server.fairgame_demorecord_message)
    server.recorddemo()
    server.msg(server.fairgame_waiting_message)
end)

local teamchange = server.event_handler("chteamrequest", function(cn)
    if not teams_locked then return end
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
        return false, "#cw <map> [<mode>] [lockteams]"
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
        players[server.player_id(cn)] = "not_loaded"
    end

    server.mastermode = 2
    server.mastermode_owner = -1
    
    server.changemap(map, mode, -1)
    server.pausegame(true)
end
