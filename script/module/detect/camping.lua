local START_AVERAGE = 200
local MAX_CAMP_VALUE = 30

local average_distances = {}
local last_positions = {}

local enabled = false

server.event_handler("frag", function(target, actor)
    
    if not enabled then return end
    
    local actor_id = server.player_sessionid(actor)
    
    local x, y, z = server.player_pos(actor)
    local last_pos = last_positions[actor_id] or {x = x, y = y, z = z}

    local distance = math.sqrt((x - last_pos.x)^2 + (y - last_pos.y)^2 + (z - last_pos.z)^2)
    average_distances[actor_id] = ((average_distances[actor_id] or START_AVERAGE)/2) + (distance/2)
    
    last_positions[actor_id] = {x = x, y = y, z = z}
    
    if average_distances[actor_id] <= MAX_CAMP_VALUE then
        server.msg(string.format(server.camping_message, server.player_displayname(actor)))
        average_distances[actor_id] = nil
    end
end)

server.event_handler("mapchange", function()

    enabled = gamemodeinfo.ctf or gamemodeinfo.protect
    
    average_distances = {}
    last_positions = {}
end)
