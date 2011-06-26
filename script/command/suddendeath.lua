-- #suddendeath [0|off|1|on]
-- #sd
-- #nosd

local msg_info = "#suddendeath [0|off|1|on]"


local function cmd_suddendeath(cn, option)

    if not server.suddendeath
    then
	return false, "suddendeath module is not loaded."
    end
    
    if not option
    then
	return false, msg_info
    end
    
    if (option == "0") or (option == "off")
    then
        server.suddendeath()
        server.player_msg(cn, string.format(server.sd_disabled_message))
    elseif (option == "1") or (option == "on")
    then
        server.suddendeath(true)
        server.player_msg(cn, string.format(sd_enabled_message))
    else
	return false, msg_info
    end
end


return {
    suddendeath_cmd = cmd_suddendeath,
    sd_cmd = function(cn) cmd_suddendeath(cn, "on") end,
    nosd_cmd = function(cn) cmd_suddendeath(cn, "off") end
}
