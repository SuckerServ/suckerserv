
local function get_usage_string()
  
  local output = "usage: #stats <cn>"
  
  local hidden_commands = {
    __default__ = true,
    __cn__    = true
  }
  
  for name in pairs(stats_sub_command) do
    if not hidden_commands[name] then
      output = output .. " | " .. name
    end
  end
  
  return output
end

local function current_stats(sendTo, player)

  if player ~= sendTo then
    server.player_msg(sendTo, "stats_current", { name = server.player_displayname(player) })
  end
  
  server.player_msg(sendTo, "stats_player", { score = server.player_score(player), frags = server.player_frags(player), deaths = server.player_deaths(player), acc = server.player_accuracy(player) })
  
  if gamemodeinfo.teams then
    server.player_msg(sendTo, "stats_teamkills", { tk =server.player_teamkills(player) })
  end
end

if not stats_sub_command then
  stats_sub_command = {}
end

stats_sub_command["__default__"] = function(cn)
  return current_stats(cn, cn)
end

stats_sub_command["__cn__"] = function(cn, player)
  return current_stats(cn, player)
end

return function(cn, ...)
  local sub_command = nil
  local arg = {...}
  
  if #arg > 0 then
    sub_command = arg[1]
    table.remove(arg, 1)
  end
  
  local sub_command_handler = stats_sub_command[sub_command or "__default__"]
  
  if not sub_command_handler then
    local sub_command_cn = tonumber(sub_command)
    if sub_command_cn then
      if server.valid_cn(sub_command_cn) then
        sub_command_handler = stats_sub_command.__cn__
        table.insert(arg, 1, sub_command_cn)
      else
        return false, "invalid cn"
      end
    end
  end
  
  if not sub_command_handler then
    server.player_msg(cn, get_usage_string())
    return
  end
  
  assert(sub_command_handler)
  return sub_command_handler(cn, unpack(arg))
end, "[total]", "Get your current or permanent player stats"

