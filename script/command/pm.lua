--[[

  A player command to send a message to a player

]]

return function(cn, tcn, ...)
  if not tcn then
    return false, "#msg <cn|name> <message>"
  end

  local text = table.concat({...}, " ")

  if not server.valid_cn(tcn) then
    tcn = server.name_to_cn_list_matches(cn,tcn)
    if not tcn then
      return
    end
  end

  server.player_msg(tcn, "priv", { name = server.player_displayname(cn), msg = text })
end, "<cn|name> <message>", "Send a private message to another player", { "pm", "pchat", "pc", "msg" }
