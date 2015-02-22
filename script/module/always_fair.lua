--[[
    Always start the game fair 
    By Patrick Oberdorf
]]

player_active = {}

server.event_handler("mapchange", function()
    player_active = {}
    server.pausegame(true)
    server.msg(orange("--[ Waiting until all Players loaded the Map."))
end)

server.event_handler("maploaded", function(cn)
    local allactive=true
    
    player_active[cn]=true
    for i, cn_ in ipairs(server.players()) do
        if player_active[cn_] == nil then
            allactive=false
        end
    end
        
    if allactive then
        server.pausegame(false)
        server.msg(orange("--[ GO!"))
    end
end)
