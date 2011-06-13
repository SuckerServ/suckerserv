--[[
	A player command to raise privilege to (invisble) master
]]

local domains = table_unique(server.parse_list(server["invmaster_domains"])) or 
                table_unique(server.parse_list(server["master_domains"]))

if not domains then
    server.log_error("invmaster command: no domains set")
    return
end

local function set_invisible_master(cn, name)

    server.set_invisible_master(cn)
    server.player_msg(cn, "Your rights have been raised to invisible-master.")
    
    if not name then
    	server.log(string.format("%s(%i) claimed (inv)master.", server.player_name(cn), cn))
    else
    	server.log(string.format("%s playing as %s(%i) used auth to claim (inv)master.", name, server.player_name(cn), cn))
    end
end

local function init() end
local function unload() end

local function run(cn, pw)

    if server.player_priv_code(cn) > 0 then
	    server.unsetpriv(cn)
	
    elseif pw then
    	if server.check_admin_password(pw) then
	        set_invisible_master(cn)
	    end
    else
	    local sid = server.player_sessionid(cn)
	    
	    for _, domain in pairs(domains) do
	        auth.send_request(cn, domain, function(cn, user_id, domain, status)
		        if not (sid ~= server.player_sessionid(cn)) or 
		           not (status == auth.request_status.SUCCESS) then
		            return
		        end
		        
		        set_invisible_master(cn, user_id)
	        end)
	    end
    end
end

return {init = init,run = run,unload = unload}

