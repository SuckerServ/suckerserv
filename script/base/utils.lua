local _ = require "underscore"

function mins(value)
    return value * 1000 * 60
end

function secs(value)
    return value * 1000
end

function coloured_text(colour_code, text)
    if text then
        return "\fs\f" .. colour_code .. text .. "\fr"
    else
        return "\fs\f" .. colour_code
    end
end

green    = _.curry(coloured_text, 0)
blue     = _.curry(coloured_text, 1)
yellow   = _.curry(coloured_text, 2)
red      = _.curry(coloured_text, 3)
grey     = _.curry(coloured_text, 4)
magenta  = _.curry(coloured_text, 5)
orange   = _.curry(coloured_text, 6)
white    = _.curry(coloured_text, 7)

-- Copied from http://lua-users.org/wiki/SimpleRound
function math.round(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end
round = math.round

function pack(...)
    return arg
end

function catch_error(chunk, ...)
    
    local pcall_results = pack(pcall(chunk, unpack(arg)))
    
    if not pcall_results[1] then
        server.log_error(pcall_results[2])
    end
    
    return pcall_results
end

function server.hashpassword(cn, password)
    return crypto.tigersum(string.format("%i %i %s", cn, server.player_sessionid(cn), password))
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
if not server.is_authserver then dofile("./script/base/utils/gamemode.lua") end
dofile("./script/base/utils/mysql.lua")
dofile("./script/base/utils/string.lua")
dofile("./script/base/utils/table.lua")
dofile("./script/base/utils/validate.lua")
dofile("./script/base/utils/deferred.lua")
dofile("./script/base/utils/event_emitter.lua")
dofile("./script/base/utils/network.lua")