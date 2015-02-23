--[[
    Always start the game fair 
    By Patrick Oberdorf
]]

local player_active = {}
local validFunction = false

server.event_handler("mapchange", function()
    player_active = {}
    validFunction = true
    server.pausegame(true)
    server.msg(blue("-> Waiting until all Players loaded the Map."))
end)

server.event_handler("maploaded", function(cn)
    local allactive=true
    if validFunction then
    
        player_active[cn]=true
        for i, cn_ in ipairs(server.players()) do
            if player_active[cn_] == nil then
                allactive=false
            end
        end
        
        if allactive then
            local countdown = 4
            server.interval(1000, function()
                allactive=false
                countdown = countdown - 1
                server.msg(blue(string.format("-> %i...", countdown)))
                    if countdown == 0 then
                        validFunction = false
                        server.pausegame(false)
                        server.msg(blue("-> GO!"))
                        return -1
                    end
            end)
        end
    end
end)
