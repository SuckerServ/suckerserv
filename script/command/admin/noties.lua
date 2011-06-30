--[[
    A player command to enable/ disable no ties module

]]


local msg_info = "#noties [0|off|1|on]"


return function(cn, option)

    if not server.no_ties
    then
	return false, "no_tie module is not loaded."
    end
    
    if not option
    then
	return false, msg_info
    end
    
    if (option == "0") or (option == "off")
    then
	server.no_ties()
	server.player_msg(cn, string.format(server.noties_disabled_message))
    elseif (option == "1") or (option == "on")
    then
	server.no_ties(true)
	server.player_msg(cn, string.format(server.noties_enabled_message))
    else
	return false, msg_info
    end
end
