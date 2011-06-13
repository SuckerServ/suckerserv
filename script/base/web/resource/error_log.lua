require "http_server"
require "Json"

http_server_root["error-log"] = http_server.resource({
    get = function(request)
        
        if web_admin.require_backend_login(request) then
            return
        end
        
        http_response.send_file(request, "log/error.log", "text/plain")
    end
})

