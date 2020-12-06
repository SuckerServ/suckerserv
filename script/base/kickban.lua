local VERY_LONG_BANTIME = 63072000
local SHORT_BANTIME = 14400

local kick_signal = server.create_event_signal("kick")
local temporary_bans = {}

local function kick_banned_players(ipmask, bantime, admin, reason)

    for _, cn in ipairs(server.clients()) do
        if net.ipmask(server.player_iplong(cn)) == net.ipmask(ipmask) and server.player_priv_code(cn) < server.PRIV_ADMIN then
            server.disconnect(cn, server.DISC_KICK, reason)
            kick_signal(cn, bantime, admin or "", reason or "")
        end
    end
end

function server.kick(cn, bantime, admin, reason)

    if server.player_isbot(cn) then
        server.delbot(cn)
        return
    end

    if not bantime then
        bantime = SHORT_BANTIME
    else
        if bantime == -1 then
            bantime = VERY_LONG_BANTIME
        end
    end

    server.ban(server.player_iplong(cn), bantime, admin, reason, server.player_name(cn))
end

function server.ban(ipmask, bantime, admin, reason, name, gban)

    if not bantime or bantime == -1 then
        bantime = VERY_LONG_BANTIME
    end

    reason = reason or ""

    server.set_ip_var(ipmask, "ban_name", name)
    server.set_ip_var(ipmask, "ban_time", os.date())
    server.set_ip_var(ipmask, "ban_expire", os.time() + bantime)
    server.set_ip_var(ipmask, "ban_admin", admin)
    server.set_ip_var(ipmask, "ban_reason", reason)
    server.set_ip_var(ipmask, "is_gban", gban or false)

    if bantime <= SHORT_BANTIME and not gban then
        table.insert(temporary_bans, ipmask)
    end

    kick_banned_players(ipmask, bantime, admin, reason)
end

function server.unban(ipmask)

    local log_message = "Removing ban on " .. net.ipmask(ipmask):to_string()

    server.log(log_message)
    server.log_status(log_message)

    server.set_ip_var(ipmask, "ban_name", nil)
    server.set_ip_var(ipmask, "ban_expire", nil)
    server.set_ip_var(ipmask, "ban_admin", nil)
    server.set_ip_var(ipmask, "ban_reason", nil)
    server.set_ip_var(ipmask, "ban_time", nil)
    server.set_ip_var(ipmask, "is_gban", nil)
end

function server.clearbans()
    for ipmask, vars in pairs(server.ip_vars()) do
        if not (vars.is_gban or false) then server.unban(ipmask) end
    end

    temporary_bans = {}

    server.msg("clearbans")
end

local function is_banned(ipmask, reserved_slot)
    local ban = server.ip_vars(ipmask)
    local bantime = ban.ban_expire
    if (ban.ignore_gban or false) then
        return false
    end
    if bantime and not reserved_slot then
        if bantime > os.time() then
            return true
        else
            server.unban(ipmask)
            return false
        end
    end
end

server.is_banned = is_banned

server.event_handler("connecting", function(cn, hostname, name, password, reserved_slot)
    if server.player_priv_code(cn) ~= server.PRIV_NONE then return end
    if is_banned(hostname, reserved_slot) then
        return -1
    end
end)

server.event_handler("clearbans_request", server.clearbans)

server.event_handler("kick_request", function(admin_cn, admin_name, bantime, target, reason)
    if server.player_priv_code(admin_cn) > server.player_priv_code(target) then
        server.kick(target, bantime, admin_name, reason)
    else
        server.player_msg(admin_cn, "command_permission_denied")
    end
end)

server.event_handler("started", function()
    
    -- Don't run on server reload
    if server.uptime > 1 then return end 
    
    local bancount = 0
    
    -- Remove expired bans
    for _, ban in pairs(server.ip_var_instances("ban_expire")) do
        
        local ipmask = ban[1]
        local expire = ban[2]
        
        if expire - SHORT_BANTIME <= os.time() then
            server.unban(ipmask)
        else
            bancount = bancount + 1
        end
    end
    
    if bancount > 0 then
        server.log_status(string.format("Ban count: %i", bancount))
    end
end)
