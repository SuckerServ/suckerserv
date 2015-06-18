--[[

    	A player command to unspec all players

]]

return function()
    server.unspecall(true)
    server.msg("unspecall_command")
end
