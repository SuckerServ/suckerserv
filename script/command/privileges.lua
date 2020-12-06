--[[
	A player command to list players privileges
]]

return function(cn)
    for p in server.gplayers() do
        if server.player_priv_code(p.cn) > server.PRIV_NONE then
            server.player_msg(cn, "player_privileges_list", {name = p:name(), cn = p.cn, priv = p:priv()})
        end
    end
end
