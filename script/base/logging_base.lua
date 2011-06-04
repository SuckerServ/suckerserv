local is_authserver = (server.is_authserver or 0) == 1

local filename = "server"

if is_authserver then
    filename = "authserver"
end

local logfile = io.open("log/" .. filename .. ".log","a+")

function bind(func, ...)

    local bind_args = arg
    
    return function(...)
    
        local call_args = {}
        
        for index, value in pairs(bind_args) do
            if bind_placeholder.detected(value) then
                call_args[index] = arg[value.index]
            else
                call_args[index] = value
            end
        end
        
        return func(unpack(call_args, 1, table.maxn(bind_args)))
    end
end

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

