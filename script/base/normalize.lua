
if not server.default_game_on_empty then
    return
end

if not map_rotation then
    error("normalize module depends on the map_rotation object")
    return
end

if not server.valid_gamemode(server.parse_mode(server.default_gamemode)) then
    error(string.format("Error in the normalize module: %s is not a valid gamemode", server.default_gamemode))
    return
end

local DEFAULT_GAMEMODE = server.parse_mode(server.default_gamemode)

local function changemap()
    local nextmap = map_rotation.get_map_name(DEFAULT_GAMEMODE)
    server.changemap(nextmap, DEFAULT_GAMEMODE)
end

server.event_handler("disconnect", function(cn)
-- playercount includes speccount
    if server.playercount == 0 and server.player_isbot(cn) == false and server.gamemode ~= DEFAULT_GAMEMODE then
         changemap()
    end
end)

-- Server started
if #server.map == 0 then
    changemap()
end

