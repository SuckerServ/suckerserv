--[[
    alternative team balancing with bots
    
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

local bot_skill_low = server.teambalance_bot_skill_low
local bot_skill_high = server.teambalance_bot_skill_high

if (bot_skill_high or 0) < bot_skill_low then
	bot_skill_high = bot_skill_low
end

local old_botbalance = server.botbalance

if old_botbalance == 1
then
    server.botbalance = 0
end

local team_map = {
    good	= "evil",
    evil	= "good"
}

local is_intermission

local has_flag = {}


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

    return _if((gamemodeinfo.teams and server.mastermode == 0 and not is_intermission), true, nil)
end

local function addbots(num)

    if num == 0
    then
	return
    end
    
    if bot_skill_low
    then
	for x = 1, num
	do
	    server.addbot(math.random(bot_skill_low, bot_skill_high))
	end
    else
	for x = 1, num
	do
	    server.addbot(-1)
	end
    end
end

local function delbots(num)

    if num == 0 or server.botcount == 0
    then
	return
    end
    
    local bots = server.bots()
    
    for x = 1, num
    do
	if not has_flag[127 + x]
	then
	    server.delbot(bots[x])
	end
    end
end

local function remove_all_bots()

    delbots(server.botcount)
end

local function check_balance(time)

    if (server.playercount - server.speccount) == 1
    then
        local bots = server.botcount
	
	if bots == 0
	then
	    addbots(1)
	    
	elseif bots > 1
	then
	    delbots(bots - 1)
	end
    else
	if not time
	then
    	    time = 500
	end
	
	server.sleep(time, function()
	
	    if is_unbalanced()
	    then
		local fuller = fuller_team()
		local other = team_map[fuller]
		local diff = (server.team_size(fuller) - 1) - server.team_size(other, nil, true)
		
		if not (diff == 0)
		then
		    local change_function = addbots
		    
		    if diff < 0 then
			change_function = delbots
		    end
		    
		    change_function(math.abs(diff))
		end
	    else
		remove_all_bots()
	    end
	end)
    end
end

server.event_handler("spectator", function(cn, joined)

    if is_enabled()
    then
	local pcount = server.playercount - server.speccount
	
	if joined == 1
	then
	    if pcount == 0
	    then
		remove_all_bots()
	    else
		check_balance(5000)
	    end
	    
	elseif pcount == 1
	then
	    local bots = server.botcount
	    
	    if bots == 0
	    then
		addbots(1)
		
	    elseif bots > 1 then
		delbots(bots - 1)
	    end
	    
	elseif is_unbalanced()
	then
	    if using_player_moving_balancer_when_leaving_spec
	    then
		local fuller = fuller_team()
		
		if server.player_team(cn) == fuller
		then
		    server.changeteam(cn, team_map[fuller])
		    server.player_msg(cn, "You switched the team for balance.")
		end
	    end
	    
	    check_balance()
	end
    end
end)

server.event_handler("chteamrequest", function(cn, old, new)

    if is_enabled()
    then
	if not team_map[new]
	then
	    server.player_msg(cn, red("Only teams good and evil are allowed."))
	    return (-1)
	    
	elseif not (server.player_status_code(cn) == 5) and using_moveblock
	then
	    local sizes = server.team_sizes()
	    
	    if (math.abs(((sizes[old] or 0) - 1) - ((sizes[new] or 0) + 1))) > 1
	    then
		server.player_msg(cn, red(string.format("Team change disallowed: \"%s\" team has enough players.", new)))
		return (-1)
	    end
	end
    end
end)

server.event_handler("reteam", function(cn, old, new)

    if is_enabled() and not (server.player_status_code(cn) == 5)
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
		server.changeteam(cn, team_map[fuller])
		server.player_msg(cn, "You switched the team for balance")
	    end
	end
    end)
end

server.event_handler("finishedgame", function()

    is_intermission = nil
    has_flag = {}
end)

server.event_handler("intermission", function()

    is_intermission = true
    has_flag = {}
    
    if is_enabled()
    then
	remove_all_bots()
    end
end)

server.event_handler("mapchange", function()

    if is_enabled()
    then
	check_balance(10000)
    end
end)

server.event_handler("connect", function(cn)

    if is_enabled() and not server.is_bot(cn)
    then
	check_balance()
    end
end)

server.event_handler("disconnect", function(cn)

    if is_enabled() and not server.is_bot(cn)
    then
	check_balance(10000)
    end
end)

server.event_handler("setmastermode", function(cn, cur, new)

    if is_enabled()
    then
	if not (new == "open")
	then
	    server.player_msg(cn, "Auto Team Balancing has been disabled. It will be re-enabled once the bots have been removed and/or the mastermode has been set to OPEN(0).")
	    remove_all_bots()
	    
	elseif not (cur == "open")
	then
	    check_balance()
	end
    end
end)

server.event_handler("takeflag", function(cn)

    if is_enabled() and server.is_bot(cn)
    then
	has_flag[cn] = true
    end
end)

server.event_handler("scoreflag", function(cn)

    if is_enabled() and server.is_bot(cn)
    then
	has_flag[cn] = nil
	check_balance()
    end
end)

server.event_handler("dropflag", function(cn)

    if is_enabled() and server.is_bot(cn)
    then
	has_flag[cn] = nil
	check_balance()
    end
end)


check_balance(1000) -- in case this module was loaded from #reload


return {unload = function()

    remove_all_bots()
    
    is_intermission = nil
    has_flag = {}
    
    server.botbalance = old_botbalance
end}
