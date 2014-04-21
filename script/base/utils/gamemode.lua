server.event_handler("mapchange", function()
    gamemodeinfo = server.get_gamemode_info()
end)
gamemodeinfo = server.get_gamemode_info()

-- Array of game mode names, used by the map rotation module
gamemodes = {
    [ 1] = "ffa",
    [ 2] = "coop edit",
    [ 3] = "teamplay",
    [ 4] = "instagib",
    [ 5] = "insta team",
    [ 6] = "efficiency",
    [ 7] = "effic team",
    [ 8] = "tactics",
    [ 9] = "tac team",
    [10] = "capture",
    [11] = "regen capture",
    [12] = "ctf",
    [13] = "insta ctf",
    [14] = "protect",
    [15] = "insta protect",
    [16] = "hold",
    [17] = "insta hold",
    [18] = "effic ctf",
    [19] = "effic protect",
    [20] = "effic hold",
    [21] = "effic collect",
    [22] = "insta collect",
    [23] = "collect"
}

do
    local mode_aliases = {
	["coopedit"]		= "coop edit",
        ["coop"]		= "coop edit",
        ["tplay"]		= "teamplay",
        ["tffa"]		= "teamplay",
        ["insta"]		= "instagib",
        ["instagibteam"]	= "insta team",
        ["instateam"]		= "insta team",
        ["iteam"]		= "insta team",
        ["effic"]		= "efficiency",
        ["efficiencyteam"]	= "effic team",
        ["efficteam"]		= "effic team",
        ["eteam"]		= "effic team",
        ["tac"]			= "tactics",
        ["tacticsteam"]		= "tac team",
        ["tacteam"]		= "tac team",
        ["tteam"]		= "tac team",
        ["cap"]			= "capture",
        ["regencapture"]	= "regen capture",
        ["regencap"]		= "regen capture",
        ["regen"]		= "regen capture",
        ["instagibctf"]		= "insta ctf",
        ["instactf"]		= "insta ctf",
        ["ictf"]		= "insta ctf",
        ["instagibprotect"]	= "insta protect",
        ["instaprotect"]	= "insta protect",
        ["iprotect"]		= "insta protect",
        ["instgibhold"]		= "insta hold",
        ["instahold"]		= "insta hold",
        ["ihold"]		= "insta hold",
        ["efficiencyctf"]	= "effic ctf",
        ["efficctf"]		= "effic ctf",
        ["ectf"]		= "effic ctf",
        ["efficiencyprotect"]	= "effic protect",
        ["efficprotect"]	= "effic protect",
        ["eprotect"]		= "effic protect",
        ["efficiencyhold"]	= "effic hold",
        ["effichold"]		= "effic hold",
        ["ehold"]		= "effic hold",
        ["effichold"]		= "effic hold",
        ["ecollect"]		= "effic collect",
        ["icollect"]	= "insta collect"
    }
    
    function server.parse_mode(mode)
        local available_modes = list_to_set(gamemodes)
        if not available_modes[mode] then mode = mode_aliases[mode] end
        return mode
    end
end

do
    local modes = {
        ["ffa"]                 = true,
        ["coop edit"]           = true,
        ["teamplay"]            = true,
        ["instagib"]            = true,
        ["insta team"]       = true,
        ["efficiency"]          = true,
        ["effic team"]     = true,
        ["tactics"]             = true,
        ["tac team"]        = true,
        ["capture"]             = true,
        ["regen capture"]       = true,
        ["ctf"]                 = true,
        ["insta ctf"]           = true,
        ["protect"]             = true,
        ["insta protect"]       = true,
        ["hold"]                = true,
        ["insta hold"]          = true,
        ["effic ctf"]      = true,
        ["effic protect"]  = true,
        ["effic hold"]     = true,
        ["insta collect"]       = true,
        ["collect"]             = true,
        ["effic collect"]  = true
    }
        
    function server.valid_gamemode(input)
        return modes[input] == true
    end
end
