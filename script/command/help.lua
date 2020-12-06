--[[
	A player command to list commands and show help
]]

return function(cn, command_name)
  local privilege = server.player_priv_code(cn)

  if command_name then
    local command = player_commands[command_name]

    if not command then
      return false, server.parse_message(cn, "help_unknown_command")
    elseif not is_player_command_enabled(command_name) then
      return false, server.parse_message(cn, "help_command_disabled")
    elseif privilege < command.permission then
      return false, server.parse_message(cn, "help_access_denied")
    elseif not command.help_message then
      return false, server.parse_message(cn, "help_no_description", { command_name = command_name })
    end
    
    server.player_msg(cn, "help_command", { command_name = command_name, help_parameters = command.help_parameters or "", help_message = command.help_message })
    return
  end

  local commands_per_privilege = {}
  for name, command in pairs(player_commands) do
    if is_player_command_enabled(name) then
      if not commands_per_privilege[command.permission + 1] then
        commands_per_privilege[command.permission + 1] = { name }
      else
        table.insert(commands_per_privilege[command.permission + 1], name)
      end
    end
  end

  local privilege_colours = {}
  privilege_colours[server.PRIV_NONE] = "%{white}"
  privilege_colours[server.PRIV_MASTER] = "%{green}"
  privilege_colours[server.PRIV_ADMIN] = "%{orange}"

  local list_of_command_names = {}
  for permission, commands in pairs(commands_per_privilege) do
    permission = permission - 1
    if privilege >= permission then
      table.insert(list_of_command_names, privilege_colours[permission] .. table.concat(commands, ", "))
    end
  end

  server.player_msg(cn, "help")

  for i,v in ipairs(list_of_command_names) do
    server.player_msg(cn, v)
  end
end, server.help_parameters, server.help_description
