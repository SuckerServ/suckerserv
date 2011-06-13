-- (c) 2011 Thomas
-- Description: Functions for loading map items, flags and bases statically 

local MAPINFO_PATH = "data/mapinfo/"
local map = ""

local function file_exists(mapfile)
    local file, err = io.open(mapfile)
    if err then 
        server.log(string.format("no mapinfo-file found for map %s (%s)", server.map, mapfile))
        return false
    end
    return true
end

function server.load_map_items()

    local map_item_filepath = string.format(MAPINFO_PATH .. "items/%s.items", map)

    if not file_exists(map_item_filepath) then
        return
    end
    
    local items = strSplit(server.read_whole_file(map_item_filepath), "\n")
	
    local i = 1
    while i < #items do
        local item = strSplit(items[i], ":")
        server.add_item(item[1], item[2])
        i = i + 1
    end
	
end

function server.load_map_flags()

    local mode = server.gamemode
	
    local is_hold = false
    local is_ctf = false
	
    if string.find(mode, "hold") then is_hold = true end
    if string.find(mode, "ctf") or string.find(mode, "protect") then is_ctf = true end
	
    if not is_hold and not is_ctf then 
        return 
    end
	
    local map_flag_filepath = string.format(MAPINFO_PATH .. "flags/%s.%sflags", map, _if(is_hold, "hold", ""))
    if not file_exists(map_flag_filepath) then
        return
    end
    local flags = strSplit(server.read_whole_file(map_flag_filepath), "\n")
	
    local i = 1
    while i < #flags do
	
        local flag = strSplit(flags[i], ":")
        local num = i - 1
        local team = flag[1]
        local x = flag[2] -- unused
        local y = flag[3]
        local z = flag[4] -- unused
		
        if is_hold then
		
            team = -1
            x = flag[1] -- unused
            y = flag[2]
            z = flag[3] -- unused
			
           end
		
        server.add_flag(num, team, y)
		
        i = i + 1
		
    end
	
    return is_hold
	
end

function server.load_map_bases()

	
    if not string.find(server.gamemode, "capture") then 
        return false
    end
    
    local map_base_filepath = string.format(MAPINFO_PATH .. "bases/%s.bases", map)
    if not file_exists(map_base_filepath) then
        return
    end
    local bases = strSplit(server.read_whole_file(map_base_filepath), "\n")
        
    local i = 1
    while i < #bases do
	
        local base = strSplit(bases[i], ":")

        local type = base[1]
        local x = base[2] 
        local y = base[3]
        local z = base[4] 
                
        server.add_base(type, x, y, z)
                
        i = i + 1
		
    end
	
    return true
	
end





server.event_handler("mapchange", function()

    map = string.sub(server.map, 0, _if(string.len(server.map) < 100, string.len(server.map), 100))

    server.load_map_items()
	
    if server.load_map_flags() then
        server.prepare_hold_mode()
    end
	
    if server.load_map_bases() then
        server.prepare_capture_mode()
    end
	
end)