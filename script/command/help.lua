
return function(cn, command_name)
    
    local privilege = server.player_priv_code(cn)
    
    if command_name then
    
        local command = player_commands[command_name]
        
        if not command then
            return false, "unknown command"
        end
        
        if not is_player_command_enabled(command_name) then
            return false, "this command is disabled"
        end
        
        if privilege < command.permission then
            return false, "access denied"
        end
        
        if not command.help_message then
            return false, "no description found for #" .. command_name .. " command_name"
        end
        
        server.player_msg(cn, string.format("#%s %s: %s", command_name, 
            command.help_parameters or "", green(command.help_message)))
        
        return
    end
    
    local commands_per_privilege = {}
    
    for name, command in pairs(player_commands) do
        if is_player_command_enabled(name) then
            local array = commands_per_privilege[command.permission + 1] or {}
            array[#array + 1] = name
            commands_per_privilege[command.permission + 1] = array
        end
    end
    
    local privilege_colours = {}
    privilege_colours[server.PRIV_MASTER] = green
    privilege_colours[server.PRIV_ADMIN] = orange
    
    local list_of_command_names = {}
    
    for permission, commands in pairs(commands_per_privilege) do
        permission = permission - 1
        if privilege >= permission then
            local colouring = privilege_colours[permission] or function(text) return text end
            list_of_command_names[#list_of_command_names + 1] = colouring(table.concat(commands, ", "))
        end
    end
    
    server.player_msg(cn, "Command descriptions: #help <command>")
    
    for i,v in ipairs(list_of_command_names) do
        if i == 1 then v = "List of command names: " .. v end
        server.player_msg(cn, v)
    end
end, "[command]", "List all player commands available or show command description and usage"
