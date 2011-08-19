
local methods = {
    msg             = function(obj, text) return server.player_msg(obj.cn, text) end,
    kick            = function(obj, ...) return server.kick(obj.cn, unpack(arg)) end,
    disconnect      = function(obj, ...) return server.disconnect(obj.cn, unpack(arg)) end,
    name            = function(obj) return server.player_name(obj.cn) end,
    displayname     = function(obj) return server.player_displayname(obj.cn) end,
    team            = function(obj) return server.player_team(obj.cn) end,
    priv            = function(obj) return server.player_priv(obj.cn) end,
    priv_code       = function(obj) return server.player_priv_code(obj.cn) end,
    id              = function(obj) return server.player_id(obj.cn) end,
    ping            = function(obj) return server.player_ping(obj.cn) end,
    lag             = function(obj) return server.player_lag(obj.cn) end,
    ip              = function(obj) return server.player_ip(obj.cn) end,
    iplong          = function(obj) return server.player_iplong(obj.cn) end,
    status          = function(obj) return server.player_status(obj.cn) end,
    status_code     = function(obj) return server.player_status_code(obj.cn) end,
    frags           = function(obj) return server.player_frags(obj.cn) end,
    score           = function(obj) return server.player_score(obj.cn) end,
    deaths          = function(obj) return server.player_deaths(obj.cn) end,
    suicides        = function(obj) return server.player_suicides(obj.cn) end,
    teamkills       = function(obj) return server.player_teamkills(obj.cn) end,
    damage          = function(obj) return server.player_damage(obj.cn) end,
    damagewasted    = function(obj) return server.player_damagewasted(obj.cn) end,
    maxhealth       = function(obj) return server.player_maxhealth(obj.cn) end,
    health          = function(obj) return server.player_health(obj.cn) end,
    gun             = function(obj) return server.player_gun(obj.cn) end,
    hits            = function(obj) return server.player_hits(obj.cn) end,
    misses          = function(obj) return server.player_misses(obj.cn) end,
    shots           = function(obj) return server.player_shots(obj.cn) end,
    accuracy        = function(obj) return server.player_accuracy(obj.cn) end,
    accuracy2       = function(obj) return server.player_accuracy2(obj.cn) end,
    timeplayed      = function(obj) return server.player_timeplayed(obj.cn) end,
    win             = function(obj) return server.player_win(obj.cn) end,
    slay            = function(obj) return server.player_slay(obj.cn) end,
    changeteam      = function(obj,newteam) return server.changeteam(obj.cn,newteam) end,
    bots            = function(obj) return server.player_bots(obj.cn) end,
    rank            = function(obj) return server.player_rank(obj.cn) end,
    isbot           = function(obj) return server.player_isbot(obj.cn) end,
    mapcrc          = function(obj) return server.player_mapcrc(obj.cn) end,
    connection_time = function(obj) return server.player_connection_time(obj.cn) end,
    force_spec      = function(obj) return server.force_spec(obj.cn) end,
    spec            = function(obj) return server.spec(obj.cn) end,
    unspec          = function(obj) return server.unspec(obj.cn) end,
    setadmin        = function(obj) return server.setadmin(obj.cn) end,
    setmaster       = function(obj) return server.setmaster(obj.cn) end,
    set_invadmin    = function(obj) return server.set_invisible_admin(obj.cn) end,
    set_invisible_admin = function(obj) return server.set_inivisible_admin(obj.cn) end,
    vars            = function(obj) return server.player_vars(obj.cn) end,
    pos             = function(obj) return server.player_pos(obj.cn) end,
    valid           = function(obj) return server.player_sessionid(obj.cn) == obj.sessionid end
}

function server.Client(cn)
    local object = {}
    object.cn = cn
    object.sessionid = server.player_sessionid(cn)
    setmetatable(object, {__index = methods})
    return object
end

