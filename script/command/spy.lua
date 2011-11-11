-- (C) 2011 by Thomas

local usage = "#spy [0|off|1|on]"

return function(cn, val)

    if server.player_priv_code(cn) < 2 then
        server.player_msg(cn, red("Command not found."))
        return
    end

    if not val then
        return false, usage
    end

    server.setspy(cn, val == 1)

end