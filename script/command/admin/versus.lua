-- 1on1 (versus) script 
-- Version: 0.1
-- (c) 2009 Thomas
-- #versus cn1 cn2 mode map

local usage = "#versus <cn1> <cn2> <mode> <map>"


local player1_cn, player2_cn = nil, nil
local player1_id, player2_id = nil, nil
local player1_ready, player2_ready = false, false

local running, suspended = false, false
local gamecount = 0

local evthandlers = {}
local function uninstallHandlers()
  for i,handlerId in ipairs(evthandlers) do server.cancel_handler(handlerId) end
  evthandlers = {}
end

local function onActive(cn)
  if cn == player1_cn then 
    player1_ready = true
  elseif cn == player2_cn then
    player2_ready = true
  end
  
  if player1_ready and player2_ready then
    server.msg(orange(string.format("--[ Game %s!", suspended and "started" or "resumed")))
    suspended = false
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
  local p2won = server.player_score(player1_cn) < server.player_score(player2_cn)
  if server.player_score(player1_cn) > server.player_score(player2_cn) then
    server.msg("versus_win", {winner = server.player_name(p2won and player2_cn or player1_cn)})
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
    player1_cn, player1_ready = cn, false
  elseif id == player2_id then
    player2_cn, player2_ready = cn, false
  end
end

local function onDisconnect(cn)
  local id = server.player_id(cn)

  if id == player1_id or id == player2_id then 
    server.msg("versus_disconnect", {name = server.player_name(cn)})
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

return function(cn, player1, player2, mode, map)
  if running then 
    server.player_msg(cn, red("Already running"))
    return
  elseif not server.valid_cn(player1) or not server.valid_cn(player2) then
    server.player_msg(cn, "versus_invalidcn")
    return
  elseif player1 == player2 then 
     server.player_msg(cn, "versus_samecn")
     return 
  end
  
  mode = mode or server.gamemode  
  
  if not server.parse_mode(mode) then
    return false, server.parse_message(cn, "unrecognized_gamemode")
  else
    mode = server.parse_mode(mode)
  end
  
  running = true
  gamecount = 0
  player1_cn, player2_cn = tonumber(player1), tonumber(player2)
  player1_id, player2_id = server.player_id(player1), server.player_id(player2)
  
  installHandlers()
  
  server.msg("versus_announce", {player1 = server.player_name(player1), player2 = server.player_name(player2), mode = mode, map = map})
  server.msg("versus_announce", {player1 = server.player_name(player1), player2 = server.player_name(player2), mode = mode, map = map})
  server.msg("versus_announce", {player1 = server.player_name(player1), player2 = server.player_name(player2), mode = mode, map = map})
  
  server.specall()
  server.unspec(player1)
  server.unspec(player2)
  server.mastermode = 2
  server.mastermode_owner = -1
  server.pausegame(true)
  
  local countdown = 6
  
  server.interval(1000, function()
    countdown = countdown - 1
    server.msg("versus_countdown", {cdown = countdown})
    if countdown == 0 then
      server.changemap(map, mode, -1)
      return -1
    end
  end)
end
