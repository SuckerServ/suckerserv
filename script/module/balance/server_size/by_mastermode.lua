local max_players = server.maxplayers

local total_max_players	= server.resize_server_mastermode_size
local resize_mastermode	= server.resize_server_mastermode_using_mastermode

local mm_map = {
    open	= 0,
    veto	= 1,
    locked	= 2,
    private	= 3
}


local function resize(mm)

    if mm == resize_mastermode
    then
        server.maxplayers = total_max_players
    else
        server.maxplayers = max_players
    end
end


server.event_handler("setmastermode", function(_, _, new)

    resize(mm_map[new])
end)

server.event_handler("mapchange", function()

    resize(server.mastermode)
end)

server.event_handler("disconnect", function()

    resize(server.mastermode)
end)

server.event_handler("started", function()

    resize(server.mastermode)
end)


return {unload = function()

    server.maxplayers = max_players
end}
