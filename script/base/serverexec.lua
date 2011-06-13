local MAX_TIME = 5
local READ_SIZE = 256

local file, error_message = os.open_fifo("serverexec")

if not file then
    server.log_error(error_message)
    return
end

file_stream = net.file_stream(file)

local function read_expression(existing_code, discard_time_limit)

    file_stream:async_read_some(READ_SIZE, function(new_code, error_message)
        
        if not new_code then
            server.log_error("serverexec read error:" .. error_message)
        end
        
        if discard_time_limit and os.time() > discard_time_limit then
            server.log_error("error in serverexec: discarding old incomplete code")
            existing_code = ""
        end
        
        code = (existing_code or "") .. new_code
        
        if not cubescript.is_complete_expression(code) then
            read_expression(code, os.time() + MAX_TIME)
            return
        end
        
        local error_message = cubescript.eval_string(code)
        
        if error_message then
            
            local code = "\n<!-- START CODE -->\n" .. code .. "<!-- END CODE -->"
            server.log_error("error in serverexec: " .. error_message .. code)
        end
        
        read_expression()
    end)
end

read_expression()

server.event_handler("shutdown", function()
    file_stream:close()
    os.remove("serverexec")
end)

