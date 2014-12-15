require "net"

local UPDATE = 60*60*1000

local function close_connection(client, callback, error_message)
    client:close()
    callback(error_message)
end

local function cleargbans(hostname)
    for ipmask, vars in pairs(server.ip_vars()) do
        if vars.is_gban and vars.ban_admin == hostname then server.unban(ipmask) end
    end
end

local function addban(args, hostname)
    local ip = check_ip(args)
    if ip[1] ~= 0 then
        server.log_error(ip[1])
        return
    else
        ip = ip[2]
        local n = #ip
        local mask = n*8
        ip = table.concat(ip, ".") .. string.rep(".0", 4-n) .. "/" .. mask
        server.ban(ip, nil, hostname, "global ban", nil, true)
    end
end

local function readmasterinput(client, callback, hostname, banlist)
    client:async_read_until("\n", function(line, error_message)

        if not line then
            close_connection(client, callback, not banlist and (error_message or "failed to read reply from server"))
            return
        end

        local command, args = line:match("([^ ]+)%s*(.*)\n")

        if command == "succreg" then
            readmasterinput(client, callback, hostname, true)
        elseif command == "cleargbans" then
            cleargbans(hostname)
            readmasterinput(client, callback, hostname, true)
        elseif command == "addgban" then
            addban(args, hostname)
            readmasterinput(client, callback, hostname, true)
        elseif command == "failreg" then
            close_connection(client, callback, args or "master server rejected registration")
        else
            close_connection(client, callback, "master server sent unknown reply")
        end
    end)
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
            
            readmasterinput(client, callback, hostname)
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
