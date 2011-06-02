--[[
    Auto Team Balancing Using Bots
    
    blocks teamswitchs against the balance
	teambalance_using_moveblock
]]

local using_moveblock = server.teambalance_using_moveblock == 1
local bot_skill_low = server.teambalance_bot_skill_low
local bot_skill_high = server.teambalance_bot_skill_high

if bot_skill_low and (bot_skill_high or 0) < bot_skill_low then
    bot_skill_high = bot_skill_low
end

local bots_added = 0
local has_flag = {}


local function addbots(num)
    
    if num == 0 then return end
    
    if bot_skill_low then
	for x = 1, num do server.addbot(math.random(bot_skill_low, bot_skill_high)) end
    else
	for x = 1, num do server.addbot(-1) end
    end
end

local function delbots(num)

    local bots = server.bots()
    local num = math.min(num, #bots)
    
    for x = 1, num do
	if not has_flag[127 + x] then server.delbot(bots[x]) end
    end
end

local function remove_bots()

    if bots_added > 0 then
        delbots(bots_added)
        bots_added = 0
    end
end

local function check_balance()

    if not gamemodeinfo.teams or server.mastermode > 0 then -- deactivate conditions
        remove_bots()
        return
    end
    
    local teams = server.team_sizes()
    local size_difference = math.abs((teams.good or 0) - (teams.evil or 0))
    local unbalanced = size_difference > 0
    
    if unbalanced then
        local change = size_difference - bots_added
        
        if change ~= 0 then
        
    	    local change_function = addbots
            if change < 0 then change_function = delbots end
            change_function(math.abs(change))
            
            bots_added = bots_added + change
        end
    else
        remove_bots()
    end
end


server.event_handler("disconnect",  check_balance)
server.event_handler("connect",     check_balance)
server.event_handler("spectator",   check_balance)
server.event_handler("reteam",      check_balance)

server.event_handler("mapchange", function()

    has_flag	= {}
    bots_added	= 0 -- the server disconnects all the bots at the end of the game
    check_balance() -- for one player case
end)

if using_moveblock
then
    server.event_handler("chteamrequest", function(cn, curteam, newteam)
    
	local teams = server.team_sizes()
	
	if (teams[curteam] or 0) > (teams[newteam] or 0) then
    	    server.player_msg(cn, red(string.format("Team change disallowed: \"%s\" team has enough players.", newteam)))
    	    return -1
	end
    end)
end

server.event_handler("setmastermode", function(cn, current, new)

    if new ~= "open" and bots_added then
        remove_bots()
        server.player_msg(cn, "Auto Team Balancing has been disabled. It will be re-enabled once the bots have been removed and/or the mastermode has been set to OPEN(0).")
    end
end)

server.event_handler("takeflag", function(cn)

    if is_enabled() and server.is_bot(cn) then
	has_flag[cn] = true
    end
end)

server.event_handler("scoreflag", function(cn)

    if is_enabled() and server.is_bot(cn) then
	has_flag[cn] = nil
	check_balance()
    end
end)

server.event_handler("dropflag", function(cn)

    if is_enabled() and server.is_bot(cn) then
	has_flag[cn] = nil
	check_balance()
    end
end)


local function unload_module()
    remove_bots()
end


check_balance() -- in case this module was loaded from #reload


return {unload = unload_module}
