--[[
        A player command to send a message to all spectators
]]

local usage = "#specmsg <text>"

return function(cn, ...)

    local text = table.concat({...}, " ")
    
    if not text then
        return false, usage
    end

    for client in server.gspectators() do
        client:msg("specmsg", {name = server.player_name(cn), cn = cn, msg = text})
    end
end
