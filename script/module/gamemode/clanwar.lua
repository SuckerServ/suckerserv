--[[
  A little clanwar command 
  Based on fairgame command + pause when someone leave and resume with delay if all players are present
  By piernov <piernov@piernov.org>
]]


local default_enabled = 1
local is_enabled

server.clanwar_running = false

local started, teams_locked = false, false
local gamecount, countdown = 0, 6
local players = {}
local allmapsloaded = false
local waitloop = 0

local function clean()
    server.clanwar_running = false
    started, teams_locked, allmapsloaded = false, false, false, false
    gamecount,waitloop = 0,0
    players = {}
end


server.event_handler("disconnect", function(cn)
    if not is_enabled then return end
    if not server.clanwar_running then return end

    if players[tostring(server.player_id(cn))] then
        server.pausegame(true)
        players[tostring(server.player_id(cn))] = "not_loaded"
    end

end)


server.event_handler("mapchange", function(map, mode)
    if not is_enabled then return end
    if not server.clanwar_running then return end

    gamecount = gamecount + 1

    if gamecount > 1 then
        clean()
        return
    end

    server.pausegame(true)

    server.interval(1000, function()
        if waitloop == 0 then
            server.msg(server.fairgame_waiting_message)
            waitloop = 1
        end

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
        -- there might be an issue if a player disconnects while this is running 
        if allmapsloaded then
            cdown = cdown - 1
            server.msg(string.format(server.fairgame_countdown_message, cdown, (cdown > 1) and "s" or ""))

            if cdown == 0 then
                server.msg(server.fairgame_started_message)

                server.pausegame(false)

                started = true
                return -1
            end
        end
    end)


end)


server.event_handler("maploaded", function(cn)
    if not is_enabled then return end

    if not server.clanwar_running then return end

    if not players[server.player_id(cn)] then return end

    players[server.player_id(cn)] = "loaded"

    if true ~= started then
        server.player_nospawn(cn, 1)
        server.player_slay(cn)
    end

    if true == started then
        server.unspec(cn)
        server.player_nospawn(cn, 0)
        server.player_respawn(cn)
    end

end)

server.event_handler("intermission", function()

    if not is_enabled then return end

    is_enabled = default_enabled

    clean()

end)


server.event_handler("chteamrequest", function(cn)

    if not is_enabled then return end

    if not teams_locked or not server.clanwar_running then return end

    server.player_msg(cn, server.fairgame_teams_locked_message)

    return -1

end)

server.event_handler("connect", function()

    if server.playercount == 1
    then
        is_enabled = default_enabled
    end

end)



server.clanwar = function(cn, enable, map, mode, lockteams)

    clean()

    if not enable
    then
        is_enabled = nil
    else
        is_enabled = true

        if lockteams == "lockteams" then
            teams_locked = true
        end

        server.clanwar_running = true
        gamecount = 0

        for _, cn in ipairs(server.players()) do
                players[tostring(server.player_id(cn))] = "not_loaded"
        end

        server.mastermode = 2
        server.mastermode_owner = -1
        server.changemap(map, mode, -1)

    end

end


return {unload = function()

    clean()

end}
