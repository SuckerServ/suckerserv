--[[
  A player command to list all known names to player
  based on the stats.db and player_ip
]]

return function(cn, target_cn)
  if not server.find_names_by_ip then
    return false, "Not available with this database"
  elseif not target_cn then
    return false, "#names <cn>|\"<name>\""
  elseif not server.valid_cn(target_cn) then
    target_cn = server.name_to_cn_list_matches(cn,target_cn)
    if not target_cn then return end
  end

  local current_name = server.player_name(target_cn)
  local names = server.find_names_by_ip(server.player_ip(target_cn), current_name)
  local namelist = table.concat(names, ", ")

  server.player_msg(cn, "names_command", {current_name = current_name, namelist = namelist})
end
