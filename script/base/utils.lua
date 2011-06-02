-- Copied from http://lua-users.org/wiki/SimpleRound
function math.round(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end
round = math.round

function pack(...)
    return arg
end

function identity(...)
    return unpack(arg)
end

function return_catch_error(fun, ...)

    local returnvals = pack(pcall(fun, unpack(arg)))
        
    if returnvals[1] == false and returnvals[2] then
        server.log_error(returnvals[2])
    end
    
    return unpack(returnvals, 1, table.maxn(returnvals))
end

function catch_error(fun, ...)
    local returnvals = pack(return_catch_error(fun, unpack(arg)))
    table.remove(returnvals, 1)
    return unpack(returnvals, 1, table.maxn(returnvals))
end

function server.eval_lua(str)
    local func, err = loadstring(str)
    if not func then error(err) end
    return func()
end

function server.hashpassword(cn, password)
	return crypto.tigersum(string.format("%i %i %s", cn, server.player_sessionid(cn), password))
end

bind_placeholder = {

    is_bind_placeholder = true,
    
    create = function(index)
        local obj = {index=index}
        setmetatable(obj, bind_placeholder)
        return obj
    end,
    
    detected = function(obj)
        local mt = getmetatable(obj)
        return mt and mt.is_bind_placeholder
    end
}

_1 = bind_placeholder.create(1)
_2 = bind_placeholder.create(2)
_3 = bind_placeholder.create(2)
_4 = bind_placeholder.create(4)
_5 = bind_placeholder.create(5)
_6 = bind_placeholder.create(6)

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

function _if(expr, true_value, false_value)
    if expr then
        return true_value
    else
        return false_value
    end
end

dofile("./script/base/utils/apps.lua")
dofile("./script/base/utils/file.lua")
dofile("./script/base/utils/gamemode.lua")
dofile("./script/base/utils/mysql.lua")
dofile("./script/base/utils/string.lua")
dofile("./script/base/utils/table.lua")

