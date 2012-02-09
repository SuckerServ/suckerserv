--[[

    	A player command to unspec all players

]]

return function()
    server.unspecall(true)
    server.msg(server.unspecall_command_message)
end
