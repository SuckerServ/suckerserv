local MODULE_VARFILES_DIR = "./script/module/config"

local signal_loaded = server.create_event_signal("module-loaded")
local signal_unloaded = server.create_event_signal("module-unloaded")

local started = false
local modules = {}
local loaded_modules = {}
local loaded_scripts = {}
local temporary_modules = {}

local function execute_module_script(filename, environment)
    
    local chunk, error_message = loadfile(filename)
    if not chunk then
        return false, error_message
    end
    
    setfenv(chunk, environment)
    
    return (function(...) return table.unpack({...}) end)(pcall(chunk))
end

function server.unload(name)
    
    local control = loaded_modules[name]
    if not control then
        error(string.format("module '%s' not loaded", name))
    end
    
    if control.unload then
        control.unload()
    end
    
    control.cleanup_environment()
    
    loaded_modules[name] = nil
    loaded_scripts[control.filename] = nil
    
    collectgarbage()
    
    signal_unloaded(name, control.filename)
    server.log_status("Unloaded module " .. name)
end

local function unload_all_modules()
    for name in pairs(loaded_modules) do 
        local success, error_message = pcall(server.unload, name)
        if not success then
            if type(error_message) == "table" and type(error_message[1]) == "string" then
                error_message = error_message[1]
            end
            server.log_error(string.format("Error while unloading module '%s': %s", name, error_message))
        end
    end
end

local function create_module_environment()
    
    local environment = {server = {}}
    
    local event_handlers = {}
    local event_unload_handlers = {}
    local event_signals = {}
    local timers = {}
    
    environment.server.event_handler = function(name, handler)
        
        if name == "unload" or name == "shutdown" then
            event_unload_handlers[#event_unload_handlers + 1] = handler
            local connection_id = #event_unload_handlers
            return function()
                event_unload_handlers[connection_id] = nil
            end
        end
        
        local handlerId = server.event_handler(name, handler)
        event_handlers[#event_handlers + 1] = handlerId
        return handlerId
    end
    
    environment.server.create_event_signal = function(name)
        local trigger, signalId = server.create_event_signal(name)
        event_signals[#event_signals + 1] = signalId
        return trigger, signalId
    end
    
    local function add_timer(timer_function, countdown, handler)
        local handler_id = timer_function(countdown, handler)
        timers[#timers + 1] = handler_id
        return handler_id
    end
    
    environment.server.sleep = function(countdown, handler)
        return add_timer(server.sleep, countdown, handler)
    end
    
    environment.server.interval = function(countdown, handler)
        return add_timer(server.interval, countdown, handler)
    end
    
    local function cleanup()
        
        for _, unload_handler in pairs(event_unload_handlers) do
            unload_handler()
        end
        
        for _, handler_id in ipairs(event_handlers) do
            server.cancel_handler(handler_id)
        end
        
        for _, handler_id in ipairs(event_signals) do
            server.cancel_event_signal(handler_id)
        end
        
        for _, handler_id in ipairs(timers) do
            server.cancel_timer(handler_id)
        end
    end
    
    setmetatable(environment.server, {__index = _G.server, __newindex = _G.server})
    setmetatable(environment, {__index = _G, __newindex = _G})
    
    return environment, cleanup
end

local function load_module(name)
    
    local event_handlers = {}
    local event_signals = {}
    local timers = {}
    local event_unload_handlers = {}
    
    local filename = find_script(name .. ".lua")
    if not filename then
        filename = find_script(name)
        if not filename then
            error(string.format("module '%s' not found", name))
        end
    end
    
    if loaded_scripts[filename] then
        return
    end
    
    local environment, cleanup_environment = create_module_environment()
    
    local success, control = execute_module_script(filename, environment)
    
    if not success then
        cleanup_environment()
        server.log_error(control)
        return
    end
    
    control = control or {}
    control.name = name
    control.filename = filename
    control.environment = environment
    control.cleanup_environment = cleanup_environment
    
    loaded_scripts[filename] = true
    loaded_modules[name] = control
    
    signal_loaded(name, filename)
    
    if server.uptime > 0 then
        server.log_status("Loaded module " .. name)
    end
end

local function load_modules_now()
    
    for _, name in ipairs(modules) do
        load_module(name)
    end
    
    modules = {}
    
    started = true
end

function server.module(name)
    
    if started == true then
        load_module(name)
    else
        table.insert(modules, name)
    end
end

function server.load_temporary_module(name)
    
    server.module(name)
    
    if loaded_modules[name] then
        temporary_modules[#temporary_modules + 1] = name
    end
end

server.event_handler("mapchange", function()
    
    for _, name in ipairs(temporary_modules) do
        server.unload(name)
    end
    
    temporary_modules = {}
end)

server.event_handler("started", load_modules_now)
server.event_handler("shutdown", unload_all_modules)

-- Load module configuration variables
local function load_module_vars(path)
    
    local filesystem = require "filesystem"
    
    for filetype, filename in filesystem.dir(path) do
        
        local fullfilename = path .. "/" .. filename
        
        if (filetype == filesystem.DIRECTORY) and (filename ~= "." and filename ~= "..") then
            load_module_vars(fullfilename)
        elseif (filetype == filesystem.FILE or filetype == filesystem.UNKNOWN) and filename:match(".vars$") then
            exec(fullfilename)
        end
    end
end

load_module_vars(MODULE_VARFILES_DIR)

