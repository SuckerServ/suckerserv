dofile("./script/command/_help.lua") -- Load player command descriptions

local player_commands = {}

local function send_command_error(cn, errmsg)
            
    local message = "Command error"
    
    if errmsg then
        message = message .. ": " .. errmsg .. "."
    else
        message = message .. "!"
    end
    
    server.player_msg(cn, red(message))
end

server.event_handler("text", function(cn, text)
    
    local arguments = server.parse_player_command(text)
    local command_prefixes = server.command_prefixes
    
    if command_prefixes == "" then return end
    
    if string.match(text, command_prefixes) then
        arguments[1] = string.sub(arguments[1], 2)
    else
        return
    end
    
    local command_name = arguments[1]
    
    local command = player_commands[command_name]
    
    if not command then
        server.player_msg(cn, red("Command not found."))
        return -1
    end
    
    arguments[1] = cn
    
    local privilege = server.player_priv_code(cn)
    
    if not (command.enabled == true) or not command._function then
        server.player_msg(cn, red("Command disabled."))
        return -1
    end
    
    if (command.require_admin == true and privilege < server.PRIV_ADMIN) or (command.require_master == true and privilege < server.PRIV_MASTER) then
        server.player_msg(cn, red("Permission denied."))
        return -1
    end
    
    local pcallret, success, errmsg = pcall(command._function, unpack(arguments))
        
    if pcallret == false then
        local message = success  -- success value is the error message returned by pcall
        server.log_error(string.format("The #%s player command failed with error: %s", command_name, message))
        server.player_msg(cn, red("Internal error"))
    end
    
    if success == false then
        send_command_error(cn, errmsg)
    end
    
    return -1
end)

local function create_command(name)

    local command = {
        name = name,
        enabled = false,
        require_admin = false, 
        require_master = false,
        _function = nil,
        control = {}
    }
    
    player_commands[name] = command
    
    return command
end

local function parse_command_list(commandlist)
    return commandlist:split("[^ \r\n\t]+")
end

local function set_commands(commandlist, fields)

    for _, cmdname in pairs(parse_command_list(commandlist)) do
        local command = player_commands[cmdname] or create_command(cmdname)
        
        for field_name, field_value in pairs(fields) do
            command[field_name] = field_value
        end
    end
end

local function foreach_command(commandlist, fun)

    for _, cmdname in ipairs(parse_command_list(commandlist)) do
        local command = player_commands[cmdname] or create_command(cmdname)
        fun(command)
    end
end

function server.enable_commands(commandlist)
    
    set_commands(commandlist, {enabled = true})
    
    foreach_command(commandlist, function(command)
        if command.control.init then command.control.init(command) end
    end)
end

function server.disable_commands(commandlist)

    set_commands(commandlist, {enabled = false})
    
    foreach_command(commandlist, function(command)

        if command.control.unload then
			command.control.unload()
			
			collectgarbage()
		end
    end)
end

function server.admin_commands(commandlist)
    return set_commands(commandlist, {require_admin = true})
end

function server.master_commands(commandlist)
    return set_commands(commandlist, {require_admin = false, require_master = true})
end

local function set_priv(command, priv)

    local already_set = command.require_admin or command.require_master
    if already_set then return end    

    if priv == "admin" then 
        
        command.require_admin = true
        command.require_master = false
        
    elseif priv == "master" then 
        
        command.require_admin = false
        command.require_master = true
        
    end
end

local function get_new_command_table(name)
    
    local command = player_commands[name] or create_command(name)
    
    if command._function then
        server.log_error(string.format("Overwriting player command '%s'", name))
    end
    
    return command
end

function player_command_filename(name)
    return "./script/command/" .. name .. ".lua"
end

function player_command_script(name, filename, priv)
    
    if not filename then
        filename = player_command_filename(name)
    end
    
    local command = get_new_command_table(name)
    
    set_priv(command, priv)
    
    local script,err = loadfile(filename)
    if not script then error(err) end
    
    command_info = command
    
    command._function = script()
    
    if type(command._function) == "table" then
        
        command.control = command._function
        command._function = command.control.run
        
        if not command.control.unload or not command.control.init then
            error(string.format("Player command script '%s' is missing a init or unload function.", filename))
        end

		if command.enabled == true then
			command.control.init()
		end
    end
    
    command_info = nil
end

function player_command_function(name, func, priv)

    local command = get_new_command_table(name)
    set_priv(command, priv)
    command._function = func
    
end

function player_command_alias(aliasname, sourcename)
    player_commands[aliasname] = player_commands[sourcename]
end

function log_unknown_player_commands()

    for name, command in pairs(player_commands) do
        if not command._function then
            server.log_error(string.format("No function loaded for player command '%s'", name))
        end
    end
    
end

function admincmd(...)

    local func = arg[1]
    local cn = arg[2]
    
    if tonumber(server.player_priv_code(cn)) < tonumber(server.PRIV_ADMIN) then
        server.player_msg(cn, red("Permission denied."))
        return
    end
    
    table.remove(arg,1)
    
    return func(unpack(arg))
end

function mastercmd(...)

    local func = arg[1]
    local cn = arg[2]
    
    if tonumber(server.player_priv_code(cn)) < tonumber(server.PRIV_MASTER) then
        server.player_msg(cn, red("Permission denied."))
        return
    end
    
    table.remove(arg,1)
    
    return func(unpack(arg))
end

player_command_function("help", function(cn, command)

    local priv_code = server.player_priv_code(cn)

    if command then
        local command_object = player_commands[command]

        if not command_object then
            server.player_msg(cn, "Unknown command")
            return
        end
        
        local description = player_command_descriptions[command]
        
        local access_denied = (command_object.require_master and priv_code < server.PRIV_MASTER) or (command_object.require_admin and priv_code < server.PRIV_ADMIN)
        
        if access_denied then
            server.player_msg(cn, "You don't have permission to get a description of this command")
            return
        end
        
        if not description then
            server.player_msg(cn, "Sorry, there is no description for the #" .. command .. " command yet")
            return
        end
        
        server.player_msg(cn, string.format("#%s %s: %s", command, description[1], green(description[2])))
        
        return
    end

    local output = ""
    
    local normal = {}
    local master = {}
    local admin = {}
    
    for name, command in pairs(player_commands) do
        if command.enabled then
            if command.require_admin then
                if priv_code == server.PRIV_ADMIN then
                    admin[#admin + 1] = name
                end
            elseif command.require_master then
                if priv_code >= server.PRIV_MASTER then
                    master[#master + 1] = name
                end
            else
                normal[#normal + 1] = name
            end
        end
    end
    
    for _, name in ipairs(normal) do
        if name ~= "help" then
            if #output > 0 then output = output .. ", " end
            output = output .. name
        end
    end
    
    if priv_code >= server.PRIV_MASTER then
        for _, name in ipairs(master) do
            if #output > 0 then output = output .. ", " end
            output = output .. green(name)
        end
    end
    
    if priv_code == server.PRIV_ADMIN then
        for _, name in ipairs(admin) do
            if #output > 0 then output = output .. ", " end
            output = output .. orange(name)
        end
    end
    
    local list_of_commands = output
    
    output = "List of commands: " .. list_of_commands .. "\nCommand descriptions: #help <command>"
    
    server.player_msg(cn, output)
end)

player_commands.help.enabled = true

