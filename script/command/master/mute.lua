--[[

  A player command to mute a player
  
]]

return function(cn,tcn,reason,time)
  if not server.mute then
    return false, "mute module not loaded"
  elseif not tcn then
    return false, "#mute <cn>|\"<name>\" [reason] [time]"
  elseif not server.valid_cn(tcn) then
    tcn = server.name_to_cn_list_matches(cn,tcn)
    if not tcn then return end
  elseif server.is_muted(tcn) then
    server.player_msg(cn, "player_muted_already", {name = server.player_displayname(tcn)})
    return
  end

  server.mute(tcn, time, reason or nil)
  server.player_msg(cn, "player_mute_admin", {name = server.player_displayname(tcn)})
end
