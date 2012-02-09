local map_time = server.default_maptime

local function changeTime()
    server.changetime(map_time)
end

server.event_handler("mapchange", changeTime)
server.event_handler("started", changeTime)

