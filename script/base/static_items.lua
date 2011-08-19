-- (c) 2011 Thomas
-- Description: Functions for loading map items, flags and bases statically 

exec("data/map-info.lua")

local map = ""
local mode = ""

function server.load_map_items()

    local items = item_data[map]
    
    if items == nil then return end

    local item 
    for i, t in ipairs(items) do
        item = items[i]
        server.add_item(item[1], item[2])
    end
end

function server.load_map_flags()
	
    local is_hold = false
    local is_ctf = false
	
    if string.find(mode, "hold") then is_hold = true end
    if string.find(mode, "ctf") or string.find(mode, "protect") then is_ctf = true end
	
    if not is_hold and not is_ctf then 
        return 
    end
    
    local flags = _if(is_hold, hold_flag_data[map], flag_data[map])  
    
    if flags == nil then return end
    
    local flag 
    for i, t in ipairs(flags) do
        flag = flags[i]
        if is_hold then
            server.add_flag(i - 1, -1, flag[1], flag[2], flag[3])
        else
            server.add_flag(i - 1, flag[1], flag[2], flag[3], flag[4])
        end
        
    end
	
    return is_hold
	
end

function server.load_map_bases()
	
    if not string.find(server.gamemode, "capture") then 
        return false
    end
    
    local bases = capture_data[map]
    
    if bases == nil then return end

    local base 
    for i, t in ipairs(bases) do
        base = bases[i]
        server.add_base(base[1], base[2], base[3], base[4])
    end    
	
    return true
	
end

server.event_handler("mapchange", function(_map, _mode)
    map = _map
    mode = _mode

    server.load_map_items()
	
    if server.load_map_flags() then
        server.prepare_hold_mode()
    end
	
    if server.load_map_bases() then
        server.prepare_capture_mode()
    end
	
end)