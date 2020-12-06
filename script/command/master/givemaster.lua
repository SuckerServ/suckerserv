--[[
  A player command to raise player's privilege to master
]]

local trigger_event
local id_event

local function init()
    trigger_event, id_event = server.create_event_signal("givemaster-command")
end

local function unload()
    server.cancel_event_signal(id_event)
end

local usage = "#givemaster <cn>"

local function run(cn, target)
  if not target then
    return false, usage
  elseif not server.valid_cn(target) then
    return false, "CN is not valid"
  end

  server.unsetpriv(cn)
  server.player_msg(target, "givemaster", {name = server.player_displayname(cn)})
  server.admin_log(string.format("GIVEADMIN: %s gave master to %s", server.player_displayname(cn), server.player_displayname(target)))
  if not (server.player_priv_code(target) >= server.PRIV_MASTER) then
    server.setmaster(target)
  end

  trigger_event(cn, target)
end

return {init = init,run = run,unload = unload}

