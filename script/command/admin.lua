--[[

	A player command to raise privilege to admin

]]

local domains = table_unique(server.parse_list(server["admin_domains"]))

if not domains then
    server.log_error("admin command: no domains set")
    return
end

local function init() end
local function unload() end

local function run(cn)

    local sid = server.player_sessionid(cn)
    
    for _, domain in pairs(domains) do
	    auth.send_request(cn, domain, function(cn, user_id, domain, status)
	
	        if not (sid == server.player_sessionid(cn)) or 
	           not (status == auth.request_status.SUCCESS) then
		        return
	        end
	        
	        if server.player_priv_code(cn) > 0 then
		        server.unsetpriv(cn)
	        end
	        
	        server.setadmin(cn)
	        
	        server.msg(string.format(server.claimadmin_message, server.player_displayname(cn), user_id))
	        server.log(string.format("%s playing as %s(%i) used auth to claim admin.", user_id, server.player_name(cn), cn))
	    end)
    end
end

return {init = init, run = run, unload = unload}

