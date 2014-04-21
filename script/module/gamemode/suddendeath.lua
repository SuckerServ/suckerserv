--[[
    suddendeath mode: game will not end while there is a tie
    so far there will be a tie when timelimit is reached, the next score will clearify and stop the game
    
    command function: suddendeath(enable)
    to enable (enable = true)/ disable (enable = nil)
]]

local default_enabled = server.suddendeath_enabled_by_default == 1

local is_enabled
local is_active

local mode = {}

-- ffa:		frag, sucide events
-- capture:	scoreupdate event
-- ctf:		scoreflag event
-- protect:	scoreflag, resetflag events
local mode_map = {
    ["ffa"]			= "ffa",
    ["teamplay"]		= "ffa",
    ["tactics"]			= "ffa",
    ["tac team"]		= "ffa",
    ["efficiency"]		= "ffa",
    ["effic team"]		= "ffa",
    ["instagib"]		= "ffa",
    ["insta team"]		= "ffa",
    ["ctf"]			= "ctf",
    ["insta ctf"]		= "ctf",
    ["effic ctf"]		= "ctf",
    ["hold"]			= "ctf",
    ["insta hold"]		= "ctf",
    ["effic hold"]		= "ctf",
    ["protect"]			= "protect",
    ["insta protect"]		= "protect",
    ["effic protect"]		= "protect",
    ["capture"]			= "capture",
    ["regen capture"]		= "capture"
}

local msg_start = "Suddendeath - next score wins!"


local function clean()

    mode = {}
    is_active = nil
    is_enabled = nil
end

local function is_tie()

    local scores = {}
    
    if gamemodeinfo.teams
    then
	for _, team in pairs(server.teams())
	do
	    local score = server.team_score(team)
	    
	    if scores[score]
	    then
		return true
	    end
	    
	    scores[score] = team
	end
    else
	for p in server.gplayers()
	do
	    local score = p:score()
	    
	    if scores[score]
	    then
		return true
	    end
	    
	    scores[score] = p.cn
	end
    end
    
    return
end

local function event_mode_ffa()

    if is_active and mode.ffa and not is_tie()
    then
	server.changetime(0)
    end
end


server.event_handler("frag", event_mode_ffa)

server.event_handler("suicide", event_mode_ffa)

server.event_handler("scoreflag", function()

    if is_active and (mode.ctf or mode.protect) and not is_tie()
    then
	server.changetime(0)
    end
end)

server.event_handler("resetflag", function()

    if is_active and mode.protect and not is_tie()
    then
	server.changetime(0)
    end
end)

server.event_handler("scoreupdate", function()

    if is_active and mode.capture and not is_tie()
    then
	server.changetime(0)
    end
end)

server.event_handler("timeupdate", function(mins, secs)

    if (mins == 0) and (secs == 0) and is_enabled and not is_active and not gamemodeinfo.edit
    then
	if is_tie()
	then
	    server.changetime(60000)
	    server.msg(msg_start)
	    mode[mode_map[server.gamemode]] = true
	    is_active = true
	end
    end
end)

server.event_handler("finishedgame", clean)

server.event_handler("mapchange", function()

    is_enabled = default_enabled
end)

server.event_handler("connect", function()

    if server.playercount == 1
    then
        is_enabled = default_enabled
    end
end)


server.suddendeath = function(enable)

    clean()
    
    if not enable
    then
	is_enabled = nil
    else
	is_enabled = true
    end
end


return {unload = function()
    clean()
end}
