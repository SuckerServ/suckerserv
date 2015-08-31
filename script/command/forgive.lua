--[[

  A player command to forgive a teamkill
  By piernov <piernov@piernov.org>

]]

local teamkills = {}
local event_handlers = {}


local function teamkill_callback(actor, target)
  server.player_msg(target, string.format(server.forgive_propose_message, server.player_displayname(actor)))
  local actor_id = server.player_id(actor)
  local target_id = server.player_id(target)
  if not teamkills[actor_id] then
    teamkills[actor_id] = { teamkilled_by = {}, teamkilled = {target_id}, cn = actor }
  else
    table.insert(teamkills[actor_id]["teamkilled"], target_id)
  end
  if not teamkills[target_id] then 
    teamkills[target_id] = { teamkilled_by = {actor_id}, teamkilled = {}, cn = target } 
  else
    table.insert(teamkills[target_id]["teamkilled_by"], actor_id)
  end
end)

local function connect_callback(cn)
  local cn_id = server.player_id(cn)
  if not teamkills[cn_id] then return end
  teamkills[cn_id]["cn"] = cn
end)

local function intermission_callback()
  teamkills = {}
end)

local function text_callback(cn, text)
  if (string.match(text:lower(), "^np") or string.match(text:lower(), "no problem") then
    server.player_msg(cn, string.format(server.forgive_analysetext_message))
  end
end)


local function init()
  table.insert(event_handlers, server.event_handler("teamkill", teamkill_callback))
  table.insert(event_handlers, server.event_handler("connect", connect_callback))
  table.insert(event_handlers, server.event_handler("intermission", intermission_callback))
  table.insert(event_handlers, server.event_handler("text", text_callback))
end

local function unload()
  for _, handler_id in ipairs(event_handlers) do server.cancel_handler(handler_id) end
  event_handlers = nil
  teamkills = nil
end

local function run(cn)
  local cn_id = server.player_id(cn)
  if not teamkills[cn_id] then return false, server.forgive_not_teamkilled_message end

  local actor_id = teamkills[cn_id]["teamkilled_by"][table_size(teamkills[cn_id]["teamkilled_by"])] or nil
  if not actor_id then return false, server.forgive_not_teamkilled_message end

  local actor_cn = teamkills[actor_id]["cn"]
  for _,actor_teamkilled in pairs(teamkills[actor_id]["teamkilled"]) do
    if cn_id == actor_teamkilled then
      server.player_forgive_tk(actor_cn)

      teamkills[cn_id]["teamkilled_by"][table_size(teamkills[cn_id]["teamkilled_by"])] = nil
      teamkills[actor_id]["teamkilled"][table_size(teamkills[actor_id]["teamkilled"])] = nil

      server.player_msg(actor_cn, string.format(server.forgive_actor_forgiven_message, server.player_displayname(cn)))
      server.player_msg(cn, string.format(server.forgive_target_forgiven_message, server.player_displayname(actor_cn)))
      return
    end
  end

  return false, server.forgive_not_teamkilled_message
end

return {init = init,run = run,unload = unload}
