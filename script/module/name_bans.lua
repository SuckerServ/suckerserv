--[[
    Module to Prevent Players from using offensive names
    
    Add names to conf/name_bans.txt!
    
    (c) 2010 Thomas
    
    Changelog
        24/05/2011 - Tell the renamed client that they have been renamed (GD)
        24/05/2011 - Check file is open before reading (GD)
        24/05/2011 - Disable pattern matching (GD)
        24/05/2011 - Use underscore library
        
    TODO
        * Use binary search
]]

local _ = require "underscore"

local file = io.open("conf/name_bans.txt")
if not file then
    error("file not found: 'conf/name_bans.txt'")
end

local banned_names = _.map(string.gmatch(file:read("*a"), "[^ \n]+"), string.upper)

file:close()

local function is_banned_name(name)
    name = string.upper(name)
    return _.any(banned_names, function(banned_name)
        return string.find(name, banned_name, 1, true) ~= nil
    end)
end

server.event_handler("connecting", function(cn, ip, name)
    if is_banned_name(name) then
        return -1
    end
end)

server.event_handler("rename", function(cn, old, new)
    if is_banned_name(new) then
        server.player_rename(cn, "unnamed", true)
        server.player_msg(cn, red(string.format("'%s' is a banned name", new))
    end
end)

