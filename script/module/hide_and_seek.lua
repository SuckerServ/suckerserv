-- (c) 2010 by |DM|Thomas & |noVI:pure_ascii

hide_and_seek = false
local hah_active_player = -1
local hah_seek_team = ""
local hah_next_player = -1
local hah_actor_frags = 0
local MAP_LOADED_ALL = false
local PLAYER_ACTIVE = { }
local hah_active_player_can_spawn = false
local GAME_ACTIVE = false
local caught_players = { }
local server_change_team_request = { } 
local can_vote = true
local has_time = 7 -- minutes
local hide_time = 20 -- seconds
local last_mapvote = server.uptime
local player_stats = { }
local player_waitlist = { }
local last_warn = { }
local is_intermission = false

local fog_enabled = false

local START_AVERAGE = 200
local MAX_CAMP_VALUE = 50
local average_distances = {}
local last_positions = {}
local camp_warn = {}
local camp_check = false

local function warn(cn, msg) 
    server.player_msg(cn, red() .. "WARNING: " .. white() .. msg)
end

local function check_camper(cn)
    if hide_and_seek == false then return end
    if is_intermission then return end
    if not camp_check then return end
    if not server.valid_cn(cn) then return end
    if server.player_team(cn) ~= "seek" then 

        local player_id = server.player_sessionid(cn)

        local x, y, z = server.player_pos(cn)
        local last_pos = last_positions[player_id] or {x = x, y = y, z = z}

        local distance = math.sqrt((x - last_pos.x)^2 + (y - last_pos.y)^2 + (z - last_pos.z)^2)
        average_distances[player_id] = ((average_distances[player_id] or START_AVERAGE)/2) + (distance/2)

        last_positions[player_id] = {x = x, y = y, z = z}

        if average_distances[player_id] <= MAX_CAMP_VALUE then
            warn(cn, "You have to move, or you will be relocated!")
            if camp_warn[player_id] ~= nil then
                camp_warn[player_id] = camp_warn[player_id] + 1
                if camp_warn[player_id] == 3 then
                    camp_warn[player_id] = nil
                    average_distances[player_id] = nil
                    server.player_slay(cn)
                    server.player_respawn(cn)
                    server.msg(string.format("%s was relocated for camping!", server.player_displayname(cn)))
                end
            else
                camp_warn[player_id] = 1
            end

        end

    end
    server.sleep(10000, function() check_camper(cn) end)
end

local function fog(cn) 
     if not fog_enabled or cn >= 128 then return end
     local enable = (server.player_team(cn) == hah_seek_team and server.player_status(cn) ~= "spectator")
     if enable then
         server.editvar(cn, "fog", 400)
         server.editvar(cn, "fogcolour", 0)
     else
         server.editvar(cn, "fog", 999999)
         server.editvar(cn, "fogcolour", 0)
     end
     server.sleep(1, function()
         local i = 0
         while (i < 10) do
             server.player_msg(cn, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n")
             i = i + 1
         end
     end)
end

local function reset_on_mapchange()
    hah_actor_frags = 0
    MAP_LOADED_ALL = false
    PLAYER_ACTIVE = { }
    hah_active_player_can_spawn = false
    GAME_ACTIVE = false
    PLAYER_SWITCHED = false
    caught_players = { }
    server_change_team_request = { } 
    can_vote = false
    player_stats = { }
    last_warn = { }
    is_intermission = false
end

local function setteam(cn)
    local jointeam = "hide"
    
    if caught_players[cn] ~= nil or hah_active_player == cn then
        return hah_seek_team
    end
    
    if server.player_team(cn) ~= jointeam then    
        server_change_team_request[cn] = true 
        server.changeteam(cn, jointeam)
    end
    
    return jointeam
end

local function relocate_players()
    if server.valid_cn(hah_active_player) then
        hah_seek_team = server.player_team(hah_active_player)
    end
    for i, cn in ipairs(server.players()) do
        if server.player_status(cn) ~= "spectator" then
          setteam(cn)
        end
    end 
end

local function relocate_vars(set_seek)
    if not server.valid_cn(hah_next_player) then
            if #server.players() == 0 then
                server.msg("failed to get seek player.")
            return
        end
        while not server.valid_cn(hah_next_player) do
            for i, cn_ in ipairs(server.players()) do
                if math.random(0, 1) == 1 then
                    hah_next_player = cn_
                end
            end           
        end
                
              
        server.msg(green() .. "Set random Player as Next Seek Player: " .. server.player_name(hah_next_player))
        
    end
    
    if set_seek ~= nil then
        if set_seek == true then
            return 
        end
    end
    
    hah_active_player = hah_next_player 
        
    server_change_team_request[hah_active_player] = true 
    server.changeteam(hah_active_player, "seek")
    
    for i, cn_ in ipairs(server.players()) do
        if server.player_status(cn_) == "dead" and server.player_status(cn_) == "spectator" and cn_ ~= hah_active_player then
            server.player_nospawn(cn_, 0)
            server.player_slay(cn_)
            server.player_respawn(cn_)
        end
    end   
    
    relocate_players()
end

local function check_game(check_only)

    if not GAME_ACTIVE then
        print("check_game called when game not active")
        return false
    end

    local hide = 0
    local seek = 0

    for i, cn_ in ipairs(server.players()) do
        if server.player_team(cn_) == "hide" then
            hide = hide + 1  
        end
        if server.player_team(cn_) == "seek" then
            seek = seek + 1  
        end
    end

    if hide == 0 or seek == 0 and not is_intermission then
        if not check_only then
            server.msg(green() .. "No players left, game ended.")
        end
        for i, cn_ in ipairs(server.clients()) do
            fog(cn_)   
        end
        server.sleep(5, function() server.changetime(0) end)
        return false
    end

    return true
end

server.event_handler("spectator", function(cn, val)
    if hide_and_seek == false then return end
    if val == 0 then
        setteam(cn)
        fog(cn)
    else
        check_game()
    end
end)

server.event_handler("connect", function(cn)
    if hide_and_seek == false then return end
    server.spec(cn)

    local str = "\f3PLEASE READ: "..orange().."This server is currently running the Hide and Seek mode, make sure you have the current map and understand how the mode works, before you are asking to be unspecced! thank you!"

    server.sleep(5000, function() 
        if server.valid_cn(cn) then
            server.player_msg(cn, str) 
        end
    end)
    server.sleep(30000, function()
        if server.valid_cn(cn) then
            server.player_msg(cn, str) 
        end
    end)
    server.sleep(60000, function()
        if server.valid_cn(cn) then
            server.player_msg(cn, str) 
        end
    end)

end)

server.event_handler("damage", function(client, actor)
    if hide_and_seek == false then return end
    if client == actor then
        return -1
    end
    if server.player_team(actor) == server.player_team(client) and actor ~= client then
        local set = false
        if last_warn[actor] == nil then 
            last_warn[actor] = server.uptime
            set = true
        end
        if (server.uptime - last_warn[actor]) > 10000 or set then
            warn(actor,  "You can't frag your teammates in this mode!")
            last_warn[actor] = server.uptime
        end
        return -1
    end
    if server.player_team(actor) ~= hah_seek_team and actor ~= hah_active_player and client ~= actor then
        local set = false
        if last_warn[actor] == nil then 
            last_warn[actor] = server.uptime
            set = true
        end
        if (server.uptime - last_warn[actor]) > 10000 or set then
            warn(actor, "You are not allowed to attack the seek Player!!!")
            last_warn[actor] = server.uptime
        end 
        return -1       
    end
end)

server.event_handler("frag", function(client, actor)
    if hide_and_seek == false then return end
    if hah_active_player == actor and client ~= actor then
        hah_actor_frags = hah_actor_frags + 1
        if hah_actor_frags == 1 then
            server.msg(red() .. string.upper("Next seek player set: \f1") .. server.player_name(client))
            hah_next_player = client
        end
    end
    if client ~= actor then
        if hah_seek_team == server.player_team(actor) then
            local count = 0
            for i, cn_ in ipairs(server.players()) do
                if hah_seek_team ~= server.player_team(cn_) then
                    count = count + 1  
                end
            end
            count = count - 1
            local str = "Players"
            if count == 1 then
                str = "Player"
            end
            server.msg(orange() .. server.player_name(actor) .. " got " .. server.player_name(client) .. " - " .. count .. " " .. str .. " left!")
          
            player_stats[actor] = (player_stats[actor] or 0) + 1
        
            server_change_team_request[client] = true 
            server.changeteam(client, hah_seek_team)
            fog(client)

            caught_players[client] = true
            if count == 0 then
                for i, cn_ in ipairs(server.clients()) do
                    fog(cn_)    -- TODO: why here?
                end
                server.sleep(5, function() server.changetime(0) end)
            end
        end
    end
end)

server.event_handler("suicide", function(cn)
    if hide_and_seek == false then return end
    if server_change_team_request[cn] == true then return end
    if hah_seek_team ~= server.player_team(cn) then
        hah_actor_frags = hah_actor_frags + 1
        if hah_actor_frags == 1 then
            server.msg(blue() .. "Next seek player set: " .. server.player_name(cn))
            hah_next_player = cn
        end
        local count = 0
        for i, cn_ in ipairs(server.clients()) do
            if server.player_status(cn_) ~= "spectator" and hah_seek_team ~= server.player_team(cn_) then
                count = count + 1  
            end
        end
        count = count - 1
        server.msg(green() .. server.player_name(cn) .. " suicided and became a seeker - " .. count .. " Players left!")
        server_change_team_request[cn] = true 
        server.changeteam(cn, hah_seek_team)
        fog(cn, true)
        if not can_vote then -- became a seeker before main seeker spawned
            server.sleep(5, function() warn(cn, "Spawn delayed for " .. hide_time .. " seconds!") end)
            server.player_nospawn(cn, 1)
            server.sleep(hide_time * 1000, function()
                if server.valid_cn(cn) then
                    server.player_nospawn(cn, 0)
                    server.player_respawn(cn)
                end
            end)
        end
        caught_players[cn] = true
        if count == 0 then
            server.sleep(5, function() server.changetime(0) end)
        end
    end
end)

server.event_handler("intermission", function(actor, client)
    if hide_and_seek == false then return end

    GAME_ACTIVE = false
    is_intermission = true

    local mapping = {}        -- continuous table needed for sorting
    for k, v in pairs(player_stats) do table.insert(mapping, k) end
    if #mapping > 0 then      -- # also only works on continuous tables

        table.sort(mapping, function(a, b) return player_stats[a] > player_stats[b] end)

        local best_msg = {}

        for k, cn in ipairs(mapping) do 
            if server.valid_cn(cn) then
                table.insert(best_msg, red(server.player_name(cn) .. "(" .. player_stats[cn] .. ")"))
            end
        end  

        server.msg(blue("Best Seekers: ") .. table.concat(best_msg, blue(", ")))
    end

    for i, cn in ipairs(server.clients()) do
        server.player_nospawn(cn, 1)
    end

    --server.intermission = server.gamemillis + 10000  -- TODO: unneeded ???
    
    --local starttime = round((server.intermission - server.gamemillis)) -- TODO: always 10000 ???
    
    --server.sleep((starttime - 10), function() -- riscy, but should work :P
    --    if (server.uptime - last_mapvote) > 10000 then -- check if master/admin changed map
    --            server.changemap(server.map)
    --    end
    --end)
end)

server.event_handler("mapchange", function()
    if hide_and_seek == false then return end
        server.msg(green() .. "waiting for clients to load the map...")
    
    for k, v in pairs(player_waitlist) do 
        if player_waitlist[k] ~= nil then -- to be sure
            if server.valid_cn(k) then
                server.unspec(k)
                server.sleep(500, function() 
                    server.msg(blue() .. "Unspecced: " .. red() .. server.player_name(k))
                end)
            end
            player_waitlist[k] = nil
        end
    end


    for i, cn in ipairs(server.players()) do
        server.player_slay(cn)
        server.player_nospawn(cn, 1)
    end

    reset_on_mapchange() 
end)

server.event_handler("reteam", function(cn, old, new)
    if hide_and_seek == false then return end
    if server_change_team_request[cn] == true then
        server_change_team_request[cn] = false
        return
    end
    server_change_team_request[cn] = true 
    server.changeteam(cn, old)
    server.player_msg(cn, red() .. "You can't switch the team!")
end)

server.event_handler("mapvote", function(cn, map, mode)
    if hide_and_seek == false then return end
    if can_vote == false then
        server.player_msg(cn, red() .. "Please wait until the seek player spawned for a new mapvote!")
        return -1
    else
        -- if mode ~= "teamplay" then
        --  server.player_msg(cn, red() .. "Hide and Seek can only be played in Teamplay-Mode!")
        --  return -1
        --end
        if not string.find(mode, "team") then
            server.player_msg(cn, red() .. "Hide & Seek can only be played in Team-Modes!")
            return -1
        end
    end
    last_mapvote = server.uptime
end)


function server.start_game()

        GAME_ACTIVE = true
       
        
        server.msg(red() .. "Maploading finished!")

        relocate_vars()

        for i, cn_ in ipairs(server.players()) do -- (players): no spawn + force spawn
            if cn_ ~= hah_active_player then
                server.player_nospawn(cn_, 0)
                server.player_respawn(cn_)
            end
        end  

	server.player_freeze(hah_active_player)
	server.player_slay(hah_active_player)

        for i, cn_ in ipairs(server.clients()) do
            fog(cn_)   
        end

        

        server.sleep(10, function()
            if server.player_status(hah_active_player) == "spectator" then
                relocate_vars(true)
            end
            
            if check_game(true) then -- check if seeker isnt disconnected ...
                server.msg(green() .. "Go and hide, the seek player will spawn in " .. hide_time .. " seconds!") 
            else
                return -- TODO: does this help ???
            end 
            
            if server.player_status(hah_next_player) == "spectator" then
                 server.msg("seek fail")
                 relocate_vars(true)
                 hah_active_player = hah_next_player
            end
            
            server.sleep((hide_time - 5) * 1000, function()
                if check_game(true) then 
                    server.msg(blue() .. "5 seconds, run!") 
                end
            end)
            server.sleep((hide_time - 3) * 1000, function()
                if check_game(true) then 
                    server.msg(red() .. "3 seconds...")
                end 
            end)
            server.sleep((hide_time - 2) * 1000, function()
                if check_game(true) then 
                    server.msg(red() .. "2 seconds...") 
                end
            end)
            server.sleep((hide_time - 1) * 1000, function()
                if check_game(true) then 
                    server.msg(red() .. "1 second....") 
                end
                stop_kill_event = true
            end)
            server.sleep(hide_time * 1000, function()
                if check_game(true) then 
                    server.player_nospawn(hah_active_player, 0) -- allow seek player to spawn
		    server.player_unfreeze(hah_active_player)
                    server.player_respawn(hah_active_player) -- spawn seek player
                    server.msg(yellow() .. "Seek Player spawned!")
                    can_vote = true
                    server.changetime(has_time * 60 * 1000) -- reset time
                    for i, cn_ in ipairs(server.players()) do 
                        check_camper(cn_)
                    end  
                end
            end)
        end)
end


server.event_handler("maploaded", function(cn)
    if hide_and_seek == false then return end

    if GAME_ACTIVE == true then
        return
    end
    
    PLAYER_ACTIVE[cn] = true
    local canstart = true

    if cn == hah_active_player then
       server.player_nospawn(hah_active_player, 1)
       server.player_slay(hah_active_player)
    end

    for _, cn_ in ipairs(server.players()) do -- wait for clients to load the map ....
        if PLAYER_ACTIVE[cn_] == nil then
            canstart = false
        end
    end

    if canstart then
       server.start_game()
    end
end)

server.event_handler("disconnect", function(cn, reason)
    if not hide_and_seek then return end
    check_game()
    if server.playercount == 0 then
        server.mastermode = 0
        hide_and_seek = false
    end
end)


player_command_function("has", function(cn, enable)
    local usage = "#has [0|1]"

    enable = tonumber(enable)
    if not (server.player_priv_code(cn) >= server.PRIV_MASTER or server.player_status(cn) ~= "spectator") then
        warn(cn, "Permission denied.")
        return
    end
    if not enable or enable > 1 then
        return false, usage
    end
    if enable > 0 then
        if hide_and_seek == true then
            server.player_msg(cn, red("Hide & Seek already running."))
            return
        end
        if #server.players() < 2 then
            server.player_msg(cn, red("Hide & Seek requires at least 2 players."))
        end
        server.broadcast_mapmodified = false
        hide_and_seek = true
        server.hide_and_seek = 1 -- changes weapon ammo amount
        server.mastermode = 2
        server.msg("mastermode is now locked (2)")
        server.msg(green() .. "Hide and Seek Mode enabled!")
        can_vote = true
    else
        if hide_and_seek == false then
            server.player_msg(cn, red("Hide & Seek not running."))
            return
        end
        if not (server.player_priv_code(cn) >= server.PRIV_MASTER) then
            server.player_msg(cn, red("Permission denied."))
            return
        end
        server.broadcast_mapmodified = true
        server.hide_and_seek = 0
        server.msg(blue() .. "Hide and Seek Mode disabled!")
        server.mastermode = 0
        server.msg("mastermode is now open (0)")
        hide_and_seek = false
        for i, cn_ in ipairs(server.clients()) do
            server.player_nospawn(cn_, 0)
        end
    end
end)

player_command_function("add", function(cn, cnadd)
    if not hide_and_seek then return end
    if not (server.player_priv_code(cn) >= server.PRIV_MASTER or server.player_status(cn) ~= "spectator") then
        warn(cn, "Permission denied.")
        return
    end
    if cnadd == nil then
        cnadd = cn
    end
    cnadd = tonumber(cnadd)
    if server.valid_cn(cnadd) then
        if player_waitlist[cnadd] ~= nil then
            warn(cn, server.player_name(cnadd) .. " is already on the waitlist.")
            return
        end       
        if server.player_status(cnadd) ~= "spectator" then
            warn(cn, server.player_name(cnadd) .. " isn't a spectator, you can't add this player.")
            return
        else
            server.msg(red() .. server.player_name(cn) .. blue() .. " added " .. red() .. server.player_name(cnadd) ..  blue() .. " to the waitlist!")
            server.player_msg(cnadd, blue() .. "You will be unspecced automaticly after this game by the server!")
            server.sleep(5000, function()
                if server.valid_cn(cnadd) then 
                    server.player_msg(cnadd, blue() .. "You will be unspecced automaticly after this game by the server!")
                end
            end)

            player_waitlist[cnadd] = true
        end
    else 
        server.player_msg(cn, red() .. cnadd .. " isn't a valid cn.")
    end
end)

player_command_function("fog", function(cn, enable)
    enable = tonumber(enable)
    if not (server.player_priv_code(cn) >= server.PRIV_MASTER) then
        warn(cn, "Permission denied.")
        return
    end
    if enable > 0 then
        fog_enabled = true
        server.msg(blue() .. "Fog enabled!")
    else
        fog_enabled = false
        server.msg(blue() .. "Fog disabled!")
    end
end)


player_command_function("move", function(cn, enable)
    enable = tonumber(enable)
    if not (server.player_priv_code(cn) >= server.PRIV_MASTER) then
        warn(cn, "Permission denied.")
        return
    end
    if enable > 0 then
        if not camp_check then
            for i, cn_ in ipairs(server.players()) do 
                check_camper(cn_)
            end  
        end
        camp_check = true
        server.msg(blue() .. "Camp check enabled!")
    else
        camp_check = false
        server.msg(blue() .. "Camp check disabled!")
    end
end)