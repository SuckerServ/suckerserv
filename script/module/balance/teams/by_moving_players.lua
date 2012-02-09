--[[
    player moving team balancing
    
    disallow other teams than good and evil [not optional]
    blocks teamswitchs against the balance
        teambalance_using_moveblock
    reteams spectator leaving players, when it helps to balance
        teambalance_using_player_moving_balancer_when_leaving_spec
    reteams dead players, that cry balance, while they are in the fuller team
        teambalance_using_text_addin
]]

local using_moveblock = server.teambalance_using_moveblock == 1
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

local is_intermission
local search_dead_player


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

    return _if((gamemodeinfo.teams and (server.mastermode == 0) and not is_intermission), true, nil)
end

local function move_player(cn, team)

    if server.player_team(cn) == team
    then
	server.changeteam(cn, team_map[team])
	server.player_msg(cn,string.format(server.balance_switched_message))
	
	search_dead_player = nil
	
	check_balance(5000)
    end
end

local function check_already_dead_players(team)

    for p in server.gteamplayers(team)
    do
	if p:status() == "dead"
	then
	    move_player(p.cn, team)
	    return
	end
    end
    
    search_dead_player = true
end

local function check_balance(option)

    if not option
    then
	option = 1000
    end
    
    server.sleep(option, function()
    
	if is_unbalanced()
	then
	    if not search_dead_player
	    then
		check_already_dead_player(fuller_team())
	    end
	    
	elseif search_dead_player
	then
	    search_dead_player = nil
	end
    end)
end


server.event_handler("frag", function(tcn, acn)

    if search_dead_player
    then
	move_player(tcn, fuller_team())
    end
end)

server.event_handler("suicide", function(cn)

    if search_dead_player
    then
	move_player(cn, fuller_team())
    end
end)

server.event_handler("spectator", function(cn, joined)

    if is_enabled()
    then
	if joined == 1
	then
	    check_balance()
	    
	elseif is_unbalanced()
	then
	    move_player(cn, fuller_team())
	end
    end
end)

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
	    
	    if (math.abs((sizes[old] - 1) - (sizes[new] + 1))) > 1
	    then
		server.player_msg(cn, string.format(server.balance_disallow_message, new))
		return (-1)
	    end
	end
    end
end)

server.event_handler("reteam", function(cn, old, new)

    if is_enabled()
    then
	check_balance()
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
		move_player(cn, fuller)
	    end
	end
    end)
end

server.event_handler("finishedgame", function()

    is_intermission = nil
    search_dead_player = nil
end)

server.event_handler("intermission", function()

    is_intermission = true
    search_dead_player = nil
end)

server.event_handler("mapchange", function(map, mode)

    if is_enabled()
    then
	check_balance(10000)
    end
end)

server.event_handler("disconnect", function(cn, reason)

    if is_enabled()
    then
	check_balance()
    end
end)


check_balance(10000) -- in case this module was loaded from #reload


return {unload = function()

    is_intermission = nil
    search_dead_player = nil
    
    server.botbalance = old_botbalance
end}
