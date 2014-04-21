local max_players = server.maxplayers

local size_gamemode = {
    ["regen capture"]		= server.resize_server_gamemode_size_regen_capture	or max_players,
    ["capture"]			= server.resize_server_gamemode_size_capture		or max_players,
    ["effic ctf"]		= server.resize_server_gamemode_size_effic_ctf		or max_players,
    ["insta ctf"]		= server.resize_server_gamemode_size_insta_ctf		or max_players,
    ["ctf"]			= server.resize_server_gamemode_size_ctf		or max_players,
    ["effic protect"]		= server.resize_server_gamemode_size_effic_protect	or max_players,
    ["insta protect"]		= server.resize_server_gamemode_size_insta_protect	or max_players,
    ["protect"]			= server.resize_server_gamemode_size_protect		or max_players,
    ["effic hold"]		= server.resize_server_gamemode_size_effic_hold		or max_players,
    ["insta hold"]		= server.resize_server_gamemode_size_insta_hold		or max_players,
    ["hold"]			= server.resize_server_gamemode_size_hold		or max_players,
    ["teamplay"]		= server.resize_server_gamemode_size_teamplay		or max_players,
    ["ffa"]			= server.resize_server_gamemode_size_ffa		or max_players,
    ["effic team"]		= server.resize_server_gamemode_size_effic_team		or max_players,
    ["efficiency"]		= server.resize_server_gamemode_size_effic		or max_players,
    ["tac team"]		= server.resize_server_gamemode_size_tactics_team	or max_players,
    ["tactics"]			= server.resize_server_gamemode_size_tactics		or max_players,
    ["insta team"]		= server.resize_server_gamemode_size_insta_team		or max_players,
    ["instagib"]		= server.resize_server_gamemode_size_insta		or max_players,
    ["coop edit"]		= server.resize_server_gamemode_size_coop_edit		or max_players
}


local function resize()

    server.sleep(500, function()
    
	server.maxplayers = size_gamemode[server.gamemode]
    end)
end


server.event_handler("mapchange", resize)

server.event_handler("disconnect", resize)

server.event_handler("started", resize)


return {unload = function()

    server.maxplayers = max_players
end}
