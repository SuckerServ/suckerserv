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

local real_changeteam = core.changeteam
core.changeteam = function(cn, team, suicide)
    if suicide == nil then suicide = true end
    real_changeteam(cn, team, suicide)
end

