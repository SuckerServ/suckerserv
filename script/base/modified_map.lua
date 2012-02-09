local function failed_action(cn)
    server.force_spec(cn)
    modified_clients[server.player_sessionid(cn)] = cn
end

server.event_handler("modmap", function(cn, map, crc)

    if map ~= server.map or server.gamemode == "coop edit" then
        return
    end

    server.log(string.format("%s(%i) is using a modified map (crc %s)", server.player_name(cn), cn, crc))  
    failed_action(cn)

end)