require "Json"
require "net"

local VARS_FILE = "log/vars"
local MINIMUM_UPDATE_INTERVAL = 1000 * 60 * 60

local variablesByIp = {}
local ipVariableNamesIndex = {}
local variablesByIpIndex = net.ipmask_table()
local variablesById = {}

for _, cn in ipairs(server.clients()) do
    local id = server.player_id(cn)
    variablesById[id] = {}
end

local lastsave = 0

local function checkValue(value)
    if type(value) == "function" or type(value) == "userdata" then error("cannot store value of a " .. type(value) .. " type") end
end

local function saveVars()
    
    local tmpfilename = VARS_FILE .. os.time()
    
    local file = io.open(tmpfilename, "w")
    if not file then
        server.log_error("Unable to save player vars")
    end
    file:write(Json.Encode(variablesByIp))
    file:close()
    
    os.remove(VARS_FILE .. ".bck")
    os.rename(VARS_FILE, VARS_FILE .. ".bck")
    os.remove(VARS_FILE)
    os.rename(tmpfilename, VARS_FILE)
end

local function loadVars()

    local file = io.open(VARS_FILE)
    if not file then return end
    variablesByIp = Json.Decode(file:read("*a")) or {}
    file:close()
    
    for key, value in pairs(variablesByIp) do
        if not empty(value) then
            variablesByIpIndex[key] = value
        else
            variablesByIp[key] = nil
        end
    end
end

function server.player_vars(cn)
    local id = server.player_id(cn)
    if id == -1 then error("invalid cn") end
    local vars = variablesById[id]
    return vars
end

function server.set_ip_var(ipmask, name, value)
    
    ipmask = net.ipmask(ipmask)
    
    local matches = variablesByIpIndex[ipmask]
    local vars = matches[#matches]
    
    ipmask = ipmask:to_string()
    
    if not vars then
        vars = {}
        variablesByIp[ipmask] = vars
        variablesByIpIndex[ipmask] = vars
    end
    
    vars[name] = value
    
    -- Add variable instance to the variable names index
    local instances = ipVariableNamesIndex[name]
    if not instances then
        instances = {}
        ipVariableNamesIndex[name] = instances
    end
    instances[#instances + 1] = {ipmask, value, vars}
    
    -- Set as player variable for matching players currently connected
    for client in server.gclients() do
        if net.ipmask(ipmask) == net.ipmask(client:iplong()) then
            variablesById[client:id()][name] = value
        end
    end
end

function server.ip_var_instances(name)
    return ipVariableNamesIndex[name]
end

function server.ip_vars(ipmask)

    if not ipmask then
        return variablesByIp
    end
    
    local matches = variablesByIpIndex[ipmask]
    local vars = {}
    for _, match in ipairs(matches) do
        for key, value in pairs(match) do
            vars[key] = value
        end
    end
    return vars
end

server.event_handler("connect", function(cn)

    local id = server.player_id(cn)
    variablesById[id] = variablesById[id] or {}
    
    local matches = variablesByIpIndex[net.ipmask(server.player_iplong(cn))]
    local vars = variablesById[id]
    for _, match in ipairs(matches) do
        for key, value in pairs(match) do
            vars[key] = value
        end
    end
end)

server.event_handler("renaming", function(cn, futureId)
    local currentId = server.player_id(cn)
    variablesById[futureId] = variablesById[currentId]
    variablesById[currentId] = nil
end)

server.event_handler("disconnect", function()
    if server.playercount == 0 and server.uptime - lastsave > MINIMUM_UPDATE_INTERVAL then
        saveVars()
        lastsave = server.uptime
    end
end)

server.event_handler("started", loadVars)
server.event_handler("shutdown", saveVars)
