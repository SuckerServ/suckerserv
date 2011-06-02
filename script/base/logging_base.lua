local is_authserver = (server.is_authserver or 0) == 1

local filename = "server"

if is_authserver then
    filename = "authserver"
end

local logfile = io.open("log/" .. filename .. ".log","a+")

function server.log(msg)
    assert(msg ~= nil)
    logfile:write(os.date("[%a %d %b %X] ",os.time()))
    logfile:write(msg)
    logfile:write("\n")
    logfile:flush()
end

server.event_handler("shutdown", bind(logfile.close, logfile))

if not server.is_authserver then

    function server.log_status(msg)
        print(msg)
    end
    
    function server.log_error(msg)
        assert(msg ~= nil)
        io.stderr:write(os.date("[%a %d %b %X] ",os.time()))
        io.stderr:write(msg)
        io.stderr:write("\n")
        io.stderr:flush()
    end
end

