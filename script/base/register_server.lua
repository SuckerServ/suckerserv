require "net"

local UPDATE = 60 * 60 * 1000 -- 1 hour interval between each register request
local REGISTER_TIMEOUT = 10 * 1000 -- 10 seconds timeout for master connection
local is_unload = false
local master_client = net.tcp_client()

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
            server.log_status("Master server registration succeeded.")
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

local function send_register_request(client, hostname, port, gameport, callback)
    client:async_send(string.format("regserv %i\n", gameport), function(error_message)
        if error_message then
            close_connection(client, callback, error_message)
            return
        end

        readmasterinput(client, callback, hostname)
    end)
end

local function register_server(client, hostname, port, gameport, callback)
    server.log_status("Attempting to register to master server: " .. hostname .. ":" .. port)

    -- Close possible existing connection before establishing a new one
    client:close()

    if #server.serverip > 0 then
        client:bind(server.serverip, 0)
    end

    -- Close connection after some time whatever happens, should be enough to get reply and not keep a socket open for nothing
    server.sleep(REGISTER_TIMEOUT, function()
        client:close()
    end)

    client:async_connect(hostname, port, function(error_message)
        if error_message then
            close_connection(client, callback, error_message)
            return
        end

        send_register_request(client, hostname, port, gameport, callback)
    end)
end

local function update()
    if is_unload then
        return -1
    end

    if server.publicserver == 1 then
        for _, fields in ipairs(server.masterservers) do
            if #fields == 2 then
                register_server(master_client, fields[1], fields[2], server.serverport, function(error_message)
                    if error_message then
                        server.log_error("Master server error: " .. error_message)
                    else
                        server.log_status("Master server connection terminated.")
                    end
                end)
            end
        end
    end
end

server.interval(UPDATE, update)
server.event_handler("started", update)

local function unload()
    is_unload = true
    master_client:cancel()
    master_client:close()
end

return {unload = unload}
