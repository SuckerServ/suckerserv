--[[
	A player command to list players privileges
]]

return function(cn)
    for p in server.gall() do
        if server.player_priv_code(p.cn) > server.PRIV_NONE then
            server.player_msg(cn, string.format(server.player_privileges_list_message, p:name(), p.cn, p:priv()))
        end
    end
end