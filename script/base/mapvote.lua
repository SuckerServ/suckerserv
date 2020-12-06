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
local allowed_modes = list_to_set(server.allowed_gamemodes)

local function mapvote(cn, map, mode)

    if server.player_priv_code(cn) == server.PRIV_ADMIN then
        return
    end
    
    if deny_mapvote then
        server.player_msg(cn, "mapvote_disabled")
        return -1
	end
    
    if not allowed_modes[mode] then
        server.player_msg(cn, "mapvote_disallowed_gamemode", {mode = mode})
        return -1
    end
    
    if mode == "coop edit" then
        return
    end
    
    if deny_unknown_map and not supported_map(map) then
        server.player_msg(cn, "mapvote_rejected_unknownmap", {map = map})
        return -1
    end
    
    if deny_excluded_map and not map_rotation.is_known_map(map, mode) then
        server.player_msg(cn, "mapvote_outside_maprotation", {map = map, mode = mode})
        return -1
    end
end

server.event_handler("mapvote", mapvote)
