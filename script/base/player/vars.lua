require "journal"

local IPMASK_VARS_FILE = "log/player_vars_journal"

local ipmask_vars = {}
local ipmask_vars_by_ipmask = net.ipmask_table()
local ipmask_vars_by_name = {}
local player_session_vars = {}

local journal_file = journal.writer_open(IPMASK_VARS_FILE)

local function load_vars()
    
    ipmask_vars = {}
    ipmask_vars_by_ipmask = net.ipmask_table()
    ipmask_vars_by_name = {}
    
    local writes, error_message = journal.load(IPMASK_VARS_FILE)
    
    if not writes then
        io.stderr:write(string.format("failed to load stored player variables: %s\n", error_message))
        return
    end
    
    local set_ip_var = server.set_ip_var
    
    for _, write in pairs(writes) do
        set_ip_var(write[1], write[2], write[3], true)
    end
end

function server.set_ip_var(ipmask, name, value, dont_commit)
    
    ipmask = net.ipmask(ipmask):to_string() -- Normalized ipmask string representation
    
    local vars = ipmask_vars[ipmask]
    if not vars then
        vars = {}
        ipmask_vars[ipmask] = vars
        ipmask_vars_by_ipmask[ipmask] = vars
    end
    
    local existing_value = vars[name]
    
    if value == existing_value then
        return
    end
    
    -- Update the ipmask_vars_by_name table
    if existing_value then
        if not value then
            local vars_by_name = ipmask_vars_by_name[name]
            for index, instance in pairs(vars_by_name) do
                if instance[1] == ipmask then
                    table.remove(vars_by_name, index)
                end
            end
        end
    else
        if value then
            local vars_by_name = ipmask_vars_by_name[name]
            if not vars_by_name then
                vars_by_name = {}
                ipmask_vars_by_name[name] = vars_by_name
            end
            vars_by_name[#vars_by_name + 1] = {ipmask, value}
        end
    end
        
    vars[name] = value
    
    -- Remove the ipmask key when the vars table becomes empty
    if not value then
        if next(vars) == nil then
            ipmask_vars[ipmask] = nil
            ipmask_vars_by_ipmask[ipmask] = nil   
        end
    end
    
    if not dont_commit then
        journal_file:write(ipmask, name, value)
    end
end

function server.ip_vars(ipmask)
    
    if not ipmask then
        return ipmask_vars
    end
    
    local matches = ipmask_vars_by_ipmask[ipmask]
    local vars = {}
    for _, match in ipairs(matches) do
        for key, value in pairs(match) do
            vars[key] = value
        end
    end
    return vars
end

function server.ip_var_instances(name)
    return ipmask_vars_by_name[name] or {}
end

function server.player_set_session_var(cn, name, value)
    local session_id = server.player_sessionid(cn)
    local session_vars = player_session_vars[session_id]
    if not session_vars then
        session_vars = {}
        player_session_vars[session_id] = session_vars
    end
    session_vars[name] = value
end

function server.player_vars(cn)
    
    local id = server.player_id(cn)
    if id == -1 then error("invalid cn") end
    
    local ip_vars = server.ip_vars(server.player_ip(cn))
    
    local session_vars = player_session_vars[server.player_sessionid(cn)] or {}
    setmetatable(session_vars, {__index = ip_vars})
    
    return session_vars
end

load_vars()

server.event_handler("disconnect", function(cn)
    player_session_vars[server.player_sessionid(cn)] = {}
end)

