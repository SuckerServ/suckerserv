--[[
  A player command to send a name of a found cheater to the log
]]

local events = {}
local usage = "#cheater <cn>|\"<name>\""

return function(cn,cheat)
  if not cheat then
    return false, usage
  elseif not server.valid_cn(cheat) then
    cheat = server.name_to_cn_list_matches(cn,cheat)
    if not cheat then return end
  end

  cheat, cn = tonumber(cheat), tonumber(cn)
  
  if cheat == cn then
    return false, "You can't report yourself"
  elseif not server.player_vars(cn).cheater then
    server.player_vars(cn).cheater = {}
  end

  local cheat_report = (server.player_vars(cn).cheater[cheat] or 0) + 1
  server.player_vars(cn).cheater[cheat] = cheat_report

  if cheat_report > 4 then
    server.player_msg(cn, "cheater_spam")
  end

  if cheat_report < 8 then
    if server.player_connection_time(cheat) > 10 then
      server.player_msg(cn, "cheater_thanks")
      for p in server.gplayers() do
          if p:priv_code() >= server.PRIV_MASTER then p:msg("cheater_admin", { actor = server.player_displayname(cn), victim = server.player_displayname(cheat) }) end
      end
      server.log("CHEATER: " .. server.player_displayname(cheat) .. "(" .. cheat .. ") was reported by " .. server.player_displayname(cn) .. "(" .. cn .. ")")
    end
  end
end
