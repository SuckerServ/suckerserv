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
    server.msg(orange("--[ Waiting until all Players loaded the Map."))
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
            server.pausegame(false)
            server.msg(orange("--[ GO!"))
            validFunction = false
        end
    end
end)
