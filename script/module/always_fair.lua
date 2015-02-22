--[[
    Always start the game fair 
    By Patrick Oberdorf
]]


server.event_handler("mapchange", function()
      server.pausegame(true)
      server.msg(orange("--[ Waiting until all Players loaded the Map."))
end)

server.event_handler("maploaded", function()
      server.pausegame(false)
      server.msg(orange("--[ GO!"))
end)
