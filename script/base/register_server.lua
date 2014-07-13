require "net"

local UPDATE = 60*60*1000

local function close_connection(client, callback, error_message)
    client:close()
    callback(error_message)
end

local function register_server(hostname, port, gameport, callback)

    local client = net.tcp_client()
    
    if #server.serverip > 0 then
        client:bind(server.serverip, 0)
    end
    
    client:async_connect(hostname, port, function(error_message)
        
        if error_message then
            close_connection(client, callback, error_message)
            return
        end
        
        client:async_send(string.format("regserv %i\n", gameport), function(error_message)
            
            if error_message then
                close_connection(client, callback, error_message)
                return
            end
            
            client:async_read_until("\n", function(line, error_message)
                
                if not line then
                    close_connection(client, callback, error_message or "failed to read reply from server")
                    return
                end
                
                local command, reason = line:match("([^ ]+)([ \n]*.*)\n")
                
                if command == "succreg" then
                    close_connection(client, callback)
                elseif command == "failreg" then
                    close_connection(client, callback, reason or "master server rejected registration")
                else
                    close_connection(client, callback, "master server sent unknown reply")
                end
            end)
        end)
    end)
end

local function update()

    if server.publicserver == 1 then
        for _, entry in ipairs(server.parse_list(server.masterservers)) do
            local fields = server.parse_list(entry)
            if #fields == 2 then
                register_server(fields[1], fields[2], server.serverport, function(error_message)
                    if error_message then
                        server.log_error("Master server error: " .. error_message)
                    else
                        server.log_status("Server registration succeeded.")
                    end
                end)
            end
        end
    end
end

server.interval(UPDATE, update)
update()

