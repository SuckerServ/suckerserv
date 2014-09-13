http_server = require "http_server"

local LISTENER_PORT = server.serverport + 3

local filename = "log/sauer_server_http.port"

local listener, errorMessage = http_server.listener("0.0.0.0", LISTENER_PORT)

if not listener then
    server.log_error(string.format("Unable to start HTTP server on TCP 0.0.0.0:%i (%s)", LISTENER_PORT, errorMessage))
    return
end

http_server_root = {}

listener:set_root(http_server.resource({
    resolve = function(name)
        return http_server_root[name]
    end
}))

http_server_root.static = http_server.filesystem_resource("script/base/web/public", "index.htm")

exec("script/base/web/http/utils.lua")
exec("script/base/web/http/response.lua")
exec("script/base/web/http/request.lua")
exec("script/base/web/resource/login.lua")
exec("script/base/web/resource/admin.lua")
exec("script/base/web/resource/listener.lua")
exec("script/base/web/resource/serverexec.lua")
exec("script/base/web/resource/queryvars.lua")
exec("script/base/web/resource/calls.lua")
exec("script/base/web/resource/player_info.lua")
exec("script/base/web/resource/team_info.lua")
exec("script/base/web/resource/netstats.lua")
exec("script/base/web/resource/ipvars.lua")
exec("script/base/web/resource/error_log.lua")

local function startHttpServer()
    
    local started, errorMessage = listener:start(function(errorMessage)
        server.log_error("Error in the HTTP server listener: " .. errorMessage)
        server.log_status("Restarting HTTP server after an error occured")
        server.sleep(0, startHttpServer)
    end)
    
    if started then
        server.log_status(string.format("HTTP server listening on TCP 0.0.0.0:%s", LISTENER_PORT))
        local portfile = io.open(filename,"w")
        portfile:write(LISTENER_PORT)
        portfile:flush()
        portfile:close()
    end
end

startHttpServer()

local function unload()
    listener:stop()
    os.remove(filename)
    http_server_root = nil
end

return {unload = unload}
