--[[
    Most of these core function overloads are for extending the original
    functions to support default arguments.
]]

local real_changemap = core.changemap
core.changemap = function(map, mode, time)
    real_changemap(map, mode or "", time or -1)    
end

local real_recorddemo = core.recorddemo
core.recorddemo = function(filename)
    real_recorddemo(filename or "")
end

