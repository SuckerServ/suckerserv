local selection_index = 0
local implementation
local known_maps = {}

local function reset_known_maps()

    known_maps = {}
    
    for _, gamemode in ipairs(gamemodes) do
        known_maps[gamemode] = {}
    end
end

local function set_implementation(new_implementation)

    reset_known_maps()
    
    local current = implementation
    implementation = new_implementation
    return current
end

local function has_implementation()
    return implementation
end

local function has_selection_functions()
    return has_implementation() and implementation.get_gamemode and implementation.get_map_name
end

local function reload_maprotation()

    if not has_implementation then
        return
    end
    
    return implementation.reload()
end

local function get_map_rotation(gamemode)

    if not (has_implementation() and implementation.get_map_rotation) then
        error("missing implementation function: get_map_rotation")
    end
    
    return implementation.get_map_rotation(selection_index, gamemode)
end

local function get_map_name(gamemode)

    if not (has_implementation() and implementation.get_map_name) then
        error("missing implementation function: get_map_name")
    end
    
    local map_name = implementation.get_map_name(selection_index, gamemode)

    if map_name == server.map then
        selection_index = selection_index + 1
        map_name = implementation.get_map_name(selection_index, gamemode)
    end
    
    return map_name
end

local function get_gamemode(index)
    
    if not (has_implementation() and implementation.get_gamemode) then
        error("missing implementation function: get_gamemode")
    end
    
    return implementation.get_gamemode(selection_index)
end

-- Helper function
local function is_known_map(request_map, gamemode)
    
    if not (has_implementation() and implementation.get_map_rotation) then
        error("missing implementation function: get_map_rotation")
    end
    
    gamemode = gamemode or server.gamemode
    
    if known_maps[gamemode][request_map] then
        return true
    end
    
    local maps = implementation.get_map_rotation()[gamemode]
    
    for _, map_name in ipairs(maps) do
        if map_name == request_map then
            known_maps[gamemode][map_name] = true
            return true
        end
    end
    
    return false
end

server.event_handler("setnextgame", function()
    
    if not has_selection_functions() then
        return
    end
    
    local gamemode = get_gamemode()
    local map = get_map_name(gamemode)
    
    if not (gamemode and map) then
        return
    end
    
    server.next_mode = gamemode
    server.next_map = map
     
    selection_index = selection_index + 1
end)

-- Helper function
local function load_from_lists(var_prefix)
    
    local parse = server.parse_list
    
    local maps = {}
    
    -- Load the set of maps from the map list variables into the local maps table
    for _, gamemode in ipairs(gamemodes) do
    
        local varname = var_prefix .. gamemode .. "_maps"
        
        local list = server[varname]
        if not list then
            error("Expected a list of maps to exist at " .. varname)
        end
        
        local provisional = parse(list)
        local rotation = {}
        
        for _, mapname in pairs(provisional) do
            
            if supported_maps[mapname] then
                rotation[#rotation + 1] = mapname
            else
                server.log_error(string.format("Excluding unknown map %s in %s maps rotation.", mapname, varname))
            end
        end
        
        maps[gamemode] = rotation
    end
    
    return maps
end

map_rotation = {
    set_implementation = set_implementation,
    reload = reload_maprotation,
    get_map_rotation = get_map_rotation,
    get_map_name = get_map_name,
    get_gamemode = get_gamemode,
    load_from_lists = load_from_lists,
    is_known_map = is_known_map
}

server.reload_maprotation = map_rotation.reload
server.is_known_map = map_rotation.is_known_map

