local LIMIT = 0.25

local excluded_mastermode = {
    locked	= true,
    private	= true
}

local damage_handler = nil

local function create_damage_handler()

    if damage_handler then return end
    
    damage_handler = server.event_handler("damage", function(target, actor, damage, gun)
        if target ~= actor and server.player_team(actor) == server.player_team(target) then
            if server.player_teamkills(actor) >= (server.player_frags(actor) * LIMIT) then
                return -1
            end
        end
    end)
end

local function destroy_damage_handler()
    if not damage_handler then return end
    server.cancel_handler(damage_handler)
    damage_handler = nil
end

local function update_activation()

    local enable = create_damage_handler
    local disable = destroy_damage_handler

    if gamemodeinfo.teams and not excluded_mastermode[server.mastermode] then
        enable()
    else
        disable()
    end
end

server.event_handler("mapchange", update_activation)
server.event_handler("setmastermode", update_activation)

