require "http_server"
require "Json"

http_server_root["ip-vars"] = http_server.resource({
    get = function(request)
        
        if web_admin.require_backend_login(request) then
            return
        end
        
        http_response.send_json(request, server.ip_vars())
    end
})

