local signal_loaded = server.create_event_signal("module-loaded")
local signal_unloaded = server.create_event_signal("module-unloaded")

local started = false
local modules = {}
local loaded_modules = {}
local loaded_scripts = {}

local environment = {}
environment.server = {}

local function createEnvironment()
    local env = table.deepcopy(environment)
    setmetatable(env.server, {__index = _G.server, __newindex = _G.server})
    setmetatable(env, {__index = _G, __newindex = _G})
    return env
end

local function run_lua_script(filename)
    local loaded, errorMessage = loadfile(filename)
    if not loaded then error(errorMessage) end
    setfenv(loaded, createEnvironment())
    return loaded()
end

local script_extension_handlers = {
    lua      = run_lua_script,
    cs       = server.execCubeScriptFile,
    _default = server.execCubeScriptFile
}

local script_paths = {
    [01] = "%s",
    [02] = "%s.lua",
    [03] = "%s.cs",
    [04] = "./script/%s.lua",
    [05] = "./script/%s",
    [06] = "./script/module/%s.lua",
    [07] = "./script/module/%s",
    [08] = "./script/%s.cs",
    [09] = "./conf/%s",
    [10] = "./conf/%s.lua",
    [11] = "./conf/%s.cs"
}

local function find_script(filename)
    
    local real_filename
    
    for _, path in ipairs(script_paths) do
        
        local candidateFilename = string.format(path, filename)
        
        if server.file_exists(candidateFilename) then
            real_filename = candidateFilename
            break
        end
    end
    
    if not real_filename then return end
    
    local extension = string.gmatch(real_filename, "%.(%a+)$")() or "_default"
    
    return real_filename, script_extension_handlers[extension]
end

server.find_script = find_script

function server.script(filename)
    
    local filename, scriptRunner = find_script(filename)
    
    if loaded_scripts[filename] then
        return nil
    end
    
    loaded_scripts[filename] = true
    
    if not scriptRunner then
        error(string.format("Unrecognized file extension for script \"%s\".", filename))
    end
    
    return scriptRunner(filename)
end

server.load_once = server.script
script = server.script
load_once = server.load_once

function server.unload_module(name)
    
    local control = loaded_modules[name]
    if not control then error(string.format("module \"%s\" not found", name)) end
    
    if control.unload then
        control.unload()
    end
    
    for _, handlerId in ipairs(control.event_handlers) do
        server.cancel_handler(handlerId)
    end
    
    for _, handlerId in ipairs(control.event_signals) do
        server.cancel_event_signal(handlerId)
    end
    
    for _, handlerId in ipairs(control.timers) do
        server.cancel_timer(handlerId)
    end
    
    loaded_modules[name] = nil
    loaded_scripts[control.filename] = nil
    
    collectgarbage()
    
    if control.successful_startup then
        signal_unloaded(name, control.filename)
        server.log_status("Unloaded module " .. name)
    end
end

local function unload_all_modules()
    for name in pairs(loaded_modules) do 
        catch_error(server.unload_module, name, true)
    end
end

local function load_module(name)

    local event_handlers = {}
    local event_signals = {}
    local timers = {}
    
    environment.server.event_handler = function(name, handler)
        local handlerId = server.event_handler(name, handler)
        event_handlers[#event_handlers + 1] = handlerId
        return handlerId
    end
    
    environment.server.create_event_signal = function(name)
        local trigger, signalId = server.create_event_signal(name)
        event_signals[#event_signals + 1] = signalId
        return trigger, signalId
    end
    
    local function timerFunction(timerFunction, countdown, handler)
        local handlerId = timerFunction(countdown, handler)
        timers[#timers + 1] = handlerId
        return handlerId
    end
    
    environment.server.sleep = function(countdown, handler)
        return timerFunction(server.sleep, countdown, handler)
    end
    
    environment.server.interval = function(countdown, handler)
        return timerFunction(server.interval, countdown, handler)
    end
    
    local filename, runScript = find_script(name)
    
    if loaded_scripts[filename] then
        return
    end
    
    if not filename then
        error(string.format("module \"%s\" not found", name))
    end
    
    if not runScript then
        error(string.format("module \"%s\" is an unknown script type", name))
    end
    
    local success, control = return_catch_error(runScript, filename)
    
    if not success then
        control = nil
    end
    
    control = control or {}
    control.filename = filename
    control.event_handlers = event_handlers
    control.event_signals = event_signals
    control.timers = timers
    control.successful_startup = success
    
    environment.server.event_handler = nil
    environment.server.create_event_signal = nil
    environment.server.sleep = nil
    environment.server.interval = nil
    
    loaded_scripts[filename] = true
    loaded_modules[name] = control
    
    if not success then
        server.unload_module(name)
        return
    end
    
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

local temporary_modules = {}

function server.load_temporary_module(name)
    
    server.module(name)
    
    if loaded_modules[name] then
        temporary_modules[#temporary_modules + 1] = name
    end
end

server.event_handler("mapchange", function()
    
    for _, name in ipairs(temporary_modules) do
        server.unload_module(name)
    end
    
    temporary_modules = {}
end)

server.event_handler("started", load_modules_now)
server.event_handler("shutdown", unload_all_modules)
