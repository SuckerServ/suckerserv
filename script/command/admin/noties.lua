--[[
    A player command to enable/ disable no ties module
]]

local usage = "#noties [0|off|1|on]"

return function(cn, option)
    if not server.no_ties then
        return false, "no_tie module is not loaded."
    end
    
    if not option then
        return false, usage
    end
    
    if (option == "0") or (option == "off") then
        server.no_ties()
        server.player_msg(cn, "no ties module disabled.")
    elseif option == "1" or option == "on" then
        server.no_ties(true)
        server.player_msg(cn, "no ties module enabled.")
    else
        return false, usage
    end
end
