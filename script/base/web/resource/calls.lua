http_server_root["calls"] = http_server.resource({
    post = function(request)
        request:async_read_content(function(calls)
        
            if web_admin.require_backend_login(request) then
                return
            end
            
            calls = Json.Decode(calls)
            
            local response = {}
            
            for _, call in ipairs(calls) do
                 
                local func = server[call.name]

                if func then
                
                    local return_info = {}
                    
                    local return_values = pack(pcall(func, unpack(call.args)))
                    
                    local failed = return_values[1] == false
                    table.remove(return_values, 1)
                    
                    if failed then
                       return_info.error = true
                    end
                    
                    return_info.values = return_values
                    
                    response[call.return_id] = return_info
                end
            end
            
            http_response.send_json(request, response)
        end)
    end
})

