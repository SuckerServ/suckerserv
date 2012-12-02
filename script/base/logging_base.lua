local _ = require "underscore"

local is_authserver = (server.is_authserver or 0) == 1

local filename = "server"
local admin = "admin"

if is_authserver then
    filename = "authserver"
end

local logfile = io.open("log/" .. filename .. ".log","a+")

function server.log(msg)
    assert(msg ~= nil)
    msg = server.filtertext(msg)
    logfile:write(os.date("[%a %d %b %X] ",os.time()))
    logfile:write(msg)
    logfile:write("\n")
    logfile:flush()
end

local logfile = io.open("log/" .. admin .. ".log","a+")

function admin_log(msg)
    assert(msg ~= nil)
    msg = server.filtertext(msg)
    logfile:write(os.date("[%a %d %b %X] ",os.time()))
    logfile:write(msg)
    logfile:write("\n")
    logfile:flush()
end


server.event_handler("shutdown", _.curry(logfile.close, logfile))

if not server.is_authserver then

    function server.log_status(msg)
        print(msg)
    end
    
    function server.log_error(error_message)
        
        if type(error_message) == "table" and type(error_message[1]) == "string" then
            error_message = error_message[1]
        end
        
        io.stderr:write(os.date("[%a %d %b %X] ",os.time()))
        io.stderr:write(error_message)
        io.stderr:write("\n")
        io.stderr:flush()
    end
end

