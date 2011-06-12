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
mapbattle.timeout = 30000
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
    if playing == countmaps then playing = 0 end
    local nextmap = mapvar[playing+num]
    return nextmap or default_map
end

function mapbattle.winner(map1, map2)
    if votes1 < votes2 then
        server.msg(red(">>> ") .. blue("Winner: ") .. green(map2))
        return map2
    else
        server.msg(red(">>> ") .. blue("Winner: ") .. green(map1))
        return map1
    end
end

mapbattle.mode.DEFAULT = function (map1, map2, gamemode) 
    server.sleep(2000, function()
        server.changemap(map1, gamemode)
    end)
end

mapbattle.mode.MAPBATTLE = function (map1, map2, gamemode)
    server.sleep(1000, function()
        mapa = map1
        mapb = map2
        mode = gamemode
        server.msg(red(">>> ") .. blue("Vote for map ") .. green(map1) .. blue(" or ") .. green(map2) .. blue(" with 1 or 2"))

        server.sleep(mapbattle.timeout, function()
            if not map_changed then
                server.changemap(mapbattle.winner(mapa, mapb), gamemode)
                map_changed = true
            end
        end)

    end)
end

server.event_handler("intermission", function() 
        server.pausegame(1)
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
    if text == tostring(1) or text == tostring(map1) then
        if has_voted[cn] == true then
            server.player_msg(cn, red(">>> You have voted already"))
            return -1
        end
        votes1 = votes1 + 1
        voted = voted + 1
        has_voted[cn] = true
        server.msg(red(">>> ") .. green(server.player_displayname(cn)) .. " voted for " .. blue(mapa))

    elseif text == tostring(2) or text == tostring(map2) then
        if has_voted[cn] == true then
            server.player_msg(cn, red(">>> You have voted already"))
            return -1
        end
        votes2 = votes2 + 1
        voted = voted + 1
        has_voted[cn] = true
        server.msg(red(">>> ") .. green(server.player_displayname(cn)) .. " voted for " .. blue(mapb))
    end

    if voted > (#server.clients()/1.5) or voted == (#server.clients()/1.5) then
        if map_changed == true then return end
        map_changed = true
        server.changemap(mapbattle.winner(mapa, mapb), mode)
    end
end)
