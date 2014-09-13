
http_server_root["admin"] = http_server.resource({
    get = function(request)
        
        if web_admin.require_login(request) then
            return
        end
        
        http_response.send_file(request, "script/base/auth/server/web/public/admin.html", "text/html")
    end
})

