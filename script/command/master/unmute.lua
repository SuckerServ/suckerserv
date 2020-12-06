--[[

  A player command to unmute a player

]]

return function(cn,tcn)
  if not server.unmute then
    return false, "mute module not loaded"
  elseif not tcn then
    return false, "#unmute <cn>|\"<name>\""
  elseif not server.valid_cn(tcn) then
    tcn = server.name_to_cn_list_matches(cn,tcn)
    if not tcn then return end
  elseif not server.is_muted(tcn) then
    server.player_msg(cn, "player_not_muted", {name = server.player_displayname(tcn)})
    return
  end

  server.unmute(tcn)
end
