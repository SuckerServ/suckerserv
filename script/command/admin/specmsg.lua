--[[
        A player command to send a message to all spectators
]]

local usage = "#specmsg <text>"

return function(cn, ...)

    local text = table.concat(arg, " ")
    
    if not text then
        return false, usage
    end

    for client in server.gspectators() do
        client:msg(string.format(server.specmsg_command_message, server.player_name(cn), cn, text))
    end
end