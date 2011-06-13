--[[--------------------------------------------------------------------------
--
--    A script to record a demo of every game
--
--]]--------------------------------------------------------------------------

local is_recording


local function start_recording(map, mode)

    if gamemodeinfo.edit or (server.playercount < 2) or is_recording
    then
        return
    end
    
    mode = string.gsub(mode, " ", "_")
    
    server.recorddemo(string.format("log/demo/%s.%s.%s.dmo", os.date("!%y_%m_%d.%H_%M"), mode, map))
    server.msg(server.demo_recording_message)
    
    is_recording = true
end

server.event_handler("mapchange", start_recording)

server.event_handler("connect", function(cn)

    if server.playercount == 2
    then
	start_recording(server.map, server.gamemode)
    end
end)

server.event_handler("disconnect", function(cn, reason)

    if (server.playercount == 0) and is_recording
    then
	server.stopdemo()
	is_recording = nil
    end
end)

server.event_handler("finishedgame", function()

    is_recording = nil
end)


return {unload = function()

    is_recording = nil
end}
