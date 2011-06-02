--[[
    
    Base module for restricting map votes
    
    Copyright (C) 2009 Graham Daws
    
    MAINTAINER
        Graham
    
    GUIDELINES
        * Transfer global configuration variables to local scope variables
]]

local deny_mapvote = (server.allow_mapvote == 0)
local deny_modevote = (server.allow_modevote == 0)
local deny_unknown_map = (server.mapvote_disallow_unknown_map == 1)
local deny_excluded_map = (server.mapvote_disallow_excluded_map == 1)
local allowed_modes = list_to_set(server.parse_list(server["allowed_gamemodes"]))

local function mapvote(cn, map, mode)

    if server.player_priv_code(cn) == server.PRIV_ADMIN then
        return
    end
    
    if deny_mapvote then
        server.player_msg(cn, red("Map voting is disabled."))
        return -1
	end
    
    if not allowed_modes[mode] then
        server.player_msg(cn, string.format(red("Vote rejected: disallowed game mode"), yellow(mode)))
        return -1
    end
    
    if mode == "coop edit" then
        return
    end
    
    if deny_unknown_map and not supported_maps[map] then
        server.player_msg(cn, string.format(red("Vote rejected: %s is an unknown map"), yellow(map)))
        return -1
    end
    
    if deny_excluded_map and not map_rotation.is_known_map(map, mode) then
        server.player_msg(cn, string.format(red("Vote rejected: %s is not in the %s map rotation"), yellow(map), yellow(mode)))
        return -1
    end
end

server.event_handler("mapvote", mapvote)
