local running = false
local runned = false
local teams_locked = false
local gamecount = 0
local players = {}

local function clear_game()
    running = false
    runned = false
    teams_locked = false
    gamecount = 0
    players = {}
end

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
    else
        local countdown = 6

        server.interval(1000, function()
            countdown = countdown - 1
            if countdown > 1 then plural = "s"; else plural = ""; end
            server.msg(string.format(server.fairgame_countdown_message, countdown, plural))
            
            if countdown == 0 then
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
        return false, "#fairgame <map> [<mode>] [lockteams]"
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
