--[[
    Vote for map at intermission 
    By piernov -- Original script from hopmod-extentions, thanks to killme_nl
]]

local mapbattle = { mode = {} }
local votes1 = 0
local votes2 = 0
local has_voted = {}
local map_changed = false
local mapa = nil
local mapb = nil
local mode = nil
local voted = 0

mapbattle.selected = "MAPBATTLE" --mode on intermission
mapbattle.timeout = server.intermission_time - 1000
mapbattle.defaultmap = "ot"

function mapbattle.reset_votes()
    votes1 = 0
    votes2 = 0
    has_voted = {}
    map_changed = false
    mode = nil
    mapa = nil
    mapb = nil
    voted = 0
end

function mapbattle.get_next_map(num, mode)
    if mode == nil then mode = server.gamemode or "ffa" end
    maps =  map_rotation.get_map_rotation(mode)
    local mapvar = maps[mode]
    local playing = 0
    for k,v in pairs(mapvar) do
        if v == server.map then 
            playing = k
        end
    end
    local countmaps = #mapvar or 0
    if playing > countmaps-2 then playing = 0 end
    local nextmap = mapvar[playing+num]
    return nextmap or mapbattle.defaultmap
end

function mapbattle.winner(map1, map2)
    if votes1 < votes2 then
        server.msg(string.format(server.mapbattle_winner_message, map2))
        return map2
    else
        server.msg(string.format(server.mapbattle_winner_message, map1))
        return map1
    end
end

mapbattle.mode.DEFAULT = function (map1, map2, gamemode) 
    server.sleep(mapbattle.timeout, function()
        server.changemap(map1, gamemode)
    end)
end

mapbattle.mode.MAPBATTLE = function (map1, map2, gamemode)
    server.sleep(1000, function()
        mapa = map1
        mapb = map2
        mode = gamemode
        server.msg(string.format(server.mapbattle_vote_message, map1, map2))

        server.sleep(mapbattle.timeout, function()
            if not map_changed then
                server.changemap(mapbattle.winner(mapa, mapb), gamemode)
                map_changed = true
            end
        end)

    end)
end

server.event_handler("intermission", function() 
        mapbattle.reset_votes()
        local map1 = map_rotation.get_map_name(server.gamemode)
        local map2 = mapbattle.get_next_map(2)
        local intermission_mode = mapbattle.mode[mapbattle.selected] or mapbattle.mode.DEFAULT
        intermission_mode(map1, map2, server.gamemode)
end)

server.event_handler("mapchange", function()
    map_changed = true
end)

server.event_handler("text", function(cn, text)
    if map_changed then return end
    for _,player in pairs(server.players()) do
        if cn == player then
            if text == tostring(1) or text == tostring(map1) then
                if has_voted[cn] == true then
                    server.player_msg(cn, server.mapbattle_vote_already)
                    return -1
                end
                votes1 = votes1 + 1
                voted = voted + 1
                has_voted[cn] = true
                server.msg(string.format(server.mapbattle_vote_ok, server.player_displayname(cn), mapa))

            elseif text == tostring(2) or text == tostring(map2) then
                if has_voted[cn] == true then
                    server.player_msg(cn, server.mapbattle_vote_already)
                    return -1
                end
                votes2 = votes2 + 1
                voted = voted + 1
                has_voted[cn] = true
                server.msg(string.format(server.mapbattle_vote_ok, server.player_displayname(cn), mapb))
            end

            if voted > (#server.players()/1.5) or voted == (#server.players()/1.5) then
                if map_changed == true then return end
                map_changed = true
                server.changemap(mapbattle.winner(mapa, mapb), mode)
            end
            return
        end
    end
    server.player_msg(cn, server.mapbattle_cant_vote_message)
    return -1
end)
