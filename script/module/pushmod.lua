--[[
	Push gamemode module, inspired from netman87's rocketmod
]]--

local hitpush_multiplier = 16
local ammos = {
	200,
	200,
	200,
	200,
	200,
	200,
}

local function reset(cn)
        server.player_vars(cn).lasthitactor = nil
        server.player_vars(cn).lasthittime = nil
        server.player_vars(cn).falling = nil
	server.player_vars(cn).killed = nil
	server.player_vars(cn).lasthitgun = nil
	server.player_vars(cn).lasthitdamage = nil
end

server.event_handler("spawn", function(cn)
        for gun, ammo in pairs(ammos) do
                server.player_change_ammo(cn, gun, ammo)
        end

	reset(cn)
end)


server.event_handler("damage", function(client, actor, damage, gun, x, y, z)
	if server.player_vars(client).killed then
		return
	end
        if client ~= actor then
          server.player_vars(client).lasthitactor = actor
          server.player_vars(client).lasthittime = server.gamemillis + server.player_ping(client) + 10
          server.player_vars(client).lasthitgun = gun
          server.player_vars(client).lasthitdamage = damage
        end
        server.player_vars(client).falling = 0
        server.hitpush(client, actor, damage * hitpush_multiplier, gun, x, y, z)
        return -1
end)

server.event_handler("suicide", function(cn)
	if server.player_vars(cn).lasthitactor and (server.gamemillis - server.player_vars(cn).lasthittime) <= 10000 then
		server.player_vars(cn).killed = true
		server.player_dodamage(cn, server.player_vars(cn).lasthitactor, server.player_vars(cn).lasthitdamage, server.player_vars(cn).lasthitgun, 10, 10, 10)
		return -1
	end
end)

