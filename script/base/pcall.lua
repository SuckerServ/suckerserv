--[[
    Overload the pcall function with a function that uses an error function that gives 
    more informative error messages.
    
    In Hopmod the source error message is wrapped in a table which is used to indicate to the outer
    pcall error handlers that the error message was been completely formed. This error handling 
    convention exists in Hopmod to stop stack traces being repeatedly output.
]]

local function xpcall_error_handler(error_message)
    if type(error_message) ~= "table" then
        return {debug.traceback(error_message, 2)}
    else
        return error_message
    end
end

native_pcall = pcall

pcall = function(func, ...)
    local outer_arg = {...}
    return xpcall(function() return func(table.unpack(outer_arg)) end, xpcall_error_handler)
end

