-- 1on1 (versus) script 
-- Version: 0.1
-- (c) 2009 Thomas
-- #versus cn1 cn2 mode map

local running = false

local player1_cn = nil
local player1_id = nil
local player1_ready = false

local player2_cn = nil
local player2_id = nil
local player2_ready = false

local gamecount = 0
local suspended = false

local evthandlers = {}

local uinstallHandlers -- function

local function onActive(cn)
    
    if cn == player1_cn then 
    
        player1_ready = true
        
    elseif cn == player2_cn then
    
        player2_ready = true
        
    end
    
    if player1_ready and player2_ready then
        
        if suspended then
            server.msg(orange("--[ Game resumed!"))
            suspended = false
        else
            server.msg(orange("--[ Game started!"))
        end
        
        server.pausegame(false)
    end
end

local function onMapchange(map, mode)

    gamecount = gamecount + 1
    
    if gamecount > 1 then
        running = false
        uninstallHandlers()
        return
    end
    
    server.pausegame(true)
    server.recorddemo()
    
    server.msg(orange("--[ Starting Demo Recording"))
    server.msg(orange("--[ Waiting until all Players loaded the Map."))
    
end

local function onIntermission()

    if server.player_score(player1_cn) > server.player_score(player2_cn) then
    
        server.msg(green("--[ 1on1 Game ended - " .. green(server.player_name(player1_cn)) .. " won the Game!"))       
    
    elseif server.player_score(player1_cn) < server.player_score(player2_cn) then
    
        server.msg(green("--[ 1on1 Game ended - " .. green(server.player_name(player2_cn)) .. " won the Game!"))
    else
        server.msg("--[ 1on1 Game ended - No Winner!")
    end
    
    running = false
	server.mastermode = 0
	server.unspecall()
end


local function onConnect(cn)

    local id = server.player_id(cn)
    
    if id == player1_id then 
        player1_cn = cn
        player1_ready = false
    end
    
    if id == player2_id then
        player2_cn = cn
        player2_ready = false
    end
end

local function onDisconnect(cn)

    local id = server.player_id(cn)

    if id == player1_id or id == player2_id then 
    
        server.msg(red("--[ Opponent " .. green(server.player_name(cn)) .. " Disconnected Pausing Game"))
        server.pausegame(true)
        
        suspended = true
    end
end

local function installHandlers()
    
    local connect = server.event_handler("connect", onConnect)
    local disconnect = server.event_handler("disconnect", onDisconnect)
    local active = server.event_handler("maploaded", onActive)
    local mapchange = server.event_handler("mapchange", onMapchange)
    local intermission = server.event_handler("intermission", onIntermission)
    
    table.insert(evthandlers, connect)
    table.insert(evthandlers, disconnect)
    table.insert(evthandlers, active)
    table.insert(evthandlers, mapchange)
    table.insert(evthandlers, intermission)
    
end

uninstallHandlers = function()
    for i,handlerId in ipairs(evthandlers) do server.cancel_handler(handlerId) end
    evthandlers = {}
end

return function(cn, player1, player2, mode, map)
    
    if running then 
        server.player_msg(cn, red("Already running"))
        return
    end
    
    if not server.valid_cn(player1) or not server.valid_cn(player2) then
        server.player_msg(cn, red("Invalid CN given for the first or second argument."))
        return
    end
    
    if player1 == player2 then 
        server.player_msg(cn, red("player 1 and player 2 have the same CN."))
       return 
    end
    
    running = true
    gamecount = 0
    player1_cn = tonumber(player1)
    player2_cn = tonumber(player2)
    player1_id = server.player_id(player1) 
    player2_id = server.player_id(player2)
    
    installHandlers()
    
    server.msg(green("--[ 1on1 - " .. red(server.player_name(player1)) .. " against " .. red(server.player_name(player2)) .. " mode: " .. orange(mode)  .. " map: " .. orange(map) ))
    server.msg(green("--[ 1on1 - " .. red(server.player_name(player1)) .. " against " .. red(server.player_name(player2)) .. " mode: " .. orange(mode)  .. " map: " .. orange(map) ))
    server.msg(green("--[ 1on1 - " .. red(server.player_name(player1)) .. " against " .. red(server.player_name(player2)) .. " mode: " .. orange(mode)  .. " map: " .. orange(map) ))
    
    server.specall()
    server.unspec(player1)
    server.unspec(player2)
    server.mastermode = 2
    server.mastermode_owner = -1
    server.pausegame(true)
    
    local countdown = 6
    
    server.interval(1000, function()
        
        countdown = countdown - 1
        
        server.msg(orange(string.format("-- [ Loading the map in %i seconds", countdown)))
        
        if countdown == 0 then
            server.changemap(map, mode, -1)
            return -1
        end
        
    end)
    
end
