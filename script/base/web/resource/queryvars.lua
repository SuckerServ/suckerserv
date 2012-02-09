
local nonvartype = {["function"] = true, ["userdata"] = true, ["thread"] = true}

http_server_root["queryvars"] = http_server.resource({
    post = function(request)
        request:async_read_content(function(vars)
        
            if web_admin.require_backend_login(request) then
                return
            end
            
            server.web_admin_session_username = web_admin.get_session_username(request)
            
            vars = Json.Decode(vars)
            
            local result = {}
            
            for _, varname in ipairs(vars) do
                local value = server[varname]
                if nonvartype[type(value)] then value = nil end
                result[varname] = value
            end
            
            http_response.send_json(request, result)
        end)
    end
})
