--[[
	A player command to change the current map time
]]

local usage = "#changetime <minutes> [<seconds>]"

return function(cn, minutes, seconds)

    if not minutes then
        return false, usage
    end

    minutes = tonumber(minutes)
    seconds = tonumber(seconds) or 0
    
    if not minutes or minutes < 0 then
        return false, "invalid minutes value"
    end
    
    if seconds < 0 or seconds > 60 then
        return false, "invalid seconds value"
    end
    
    server.changetime((minutes*60*1000)+(seconds*1000))
end
