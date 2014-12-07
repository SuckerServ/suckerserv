--[[
    A player command to raise privilege to master
]]

local function init() end
local function unload() end

local function run(cn)

    local domains = table_unique(server.parse_list(server["master_domains"]))

    if not domains then
        server.log_error("master command: no domains set")
        return
    end

    local sid = server.player_sessionid(cn)

    for _, domain in pairs(domains) do
        auth.send_request(cn, domain, function(cn, user_id, domain, status)

            if not (sid == server.player_sessionid(cn)) or not (status == auth.request_status.SUCCESS) then
                return
            end
            
            local admin_present = server.master ~= -1 and server.player_priv_code(server.master) == server.PRIV_ADMIN
            
            if not admin_present and not server.current_master_global_authed then
                server.unsetmaster()
                server.setmaster(cn)
                
                server.msg(string.format(server.claimmaster_message, server.player_displayname(cn), user_id))
                server.log(string.format("%s playing as %s(%i) used auth to claim master.", user_id, server.player_name(cn), cn))
                server.admin_log(string.format("%s playing as %s(%i) used auth to claim master.", user_id, server.player_name(cn), cn))
            else
                server.player_msg(cn, string.format(server.master_already_message))
            end
        end)
    end
end

return {init = init, run = run, unload = unload}

