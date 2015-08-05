--[[
    passive balancer
    
    doesn't move players (except leaving spectators) and doesn't add bots
    
    disallow other teams than good and evil [not optional]
    blocks teamswitchs against the balance
	teambalance_using_moveblock
    reteams spectator leaving players, when it helps to balance
	teambalance_using_player_moving_balancer_when_leaving_spec
    reteams dead players, that cry balance, while they are in the fuller team
	teambalance_using_text_addin
]]

local using_moveblock = server.teambalance_using_moveblock == 1
local using_player_moving_balancer_when_leaving_spec = server.teambalance_using_player_moving_balancer_when_leaving_spec == 1
local using_text_addin = server.teambalance_using_text_addin == 1

local old_botbalance = server.botbalance

if old_botbalance == 0
then
    server.botbalance = 1
end

local team_map = {
    good	= "evil",
    evil	= "good"
}


local function fuller_team()

    local sizes = server.team_sizes()
    
    return _if(((sizes.good or 0) > (sizes.evil or 0)), "good", "evil")
end

local function team_diff()

    local sizes = server.team_sizes()
    
    return (math.abs((sizes.good or 0) - (sizes.evil or 0)))
end

local function is_unbalanced()

    return _if((team_diff() > 1), true, nil)
end

local function is_enabled()

    return _if((gamemodeinfo.teams and server.mastermode == 0), true, nil)
end


if using_player_moving_balancer_when_leaving_spec
then
    server.event_handler("spectator", function(cn, joined)
    
	if is_enabled() and joined == 0 and is_unbalanced()
	then
	    local fuller = fuller_team()
	    
	    if server.player_team(cn) == fuller
	    then
		server.changeteam(cn, team_map[fuller])
		server.player_msg(cn, string.format(server.balance_switched_message))
	    end
	end
    end)
end


server.event_handler("chteamrequest", function(cn, old, new)

    if is_enabled()
    then
	if not team_map[new]
	then
	    server.player_msg(cn, string.format(server.balance_allowed_teams_message))
	    return (-1)
	    
	elseif not (server.player_status_code(cn) == 5) and using_moveblock
	then
	    local sizes = server.team_sizes()
	    
	    if (math.abs(((sizes[old] or 0) - 1) - ((sizes[new] or 0) + 1))) > 1
	    then
		server.player_msg(cn, string.format(server.balance_disallow_message, new))
		return (-1)
	    end
	end
    end
end)

if using_text_addin
then
    server.event_handler("text", function(cn, text)
    
	if is_enabled() and is_unbalanced()
	then
	    local fuller = fuller_team()
	    
	    if server.player_team(cn) == fuller and server.player_status(cn) == "dead" and (string.match(text, "balance") or string.match(text, "BALANCE") or string.match(text, "teams") or string.match(text, "TEAMS"))
	    then
		server.changeteam(cn, team_map[fuller])
		server.player_msg(cn, string.format(server.balance_switched_message))
	    end
	end
    end)
end


return {unload = function()

    server.botbalance = old_botbalance
end}
