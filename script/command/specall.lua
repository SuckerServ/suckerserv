--[[

    	A player command to spec all players

]]

return function()
    server.specall(true)
    server.msg(server.specall_command_message)
end
