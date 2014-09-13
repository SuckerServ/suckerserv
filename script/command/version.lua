
return function(cn)
    local version = server.version()
    local revision = server.revision()
    
    server.player_msg(cn, string.format("Running SuckerServ-v5. %s%s",
        green("Compiled at: " .. version),
        _if(revision > -1, blue(" Revision: " .. revision), "")
    ))
end
