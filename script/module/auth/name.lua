local domain		= server.name_reservation_domain

if not auth.directory.get_domain(domain)
then
    server.log_error("Name protection auth doesn't work, but is required: unknown domain " .. tostring(domain))
    return
end

local MIN_CACHE_TIME	= 3 * 60 * 60 * 1000

local failure_message = {
    [auth.request_status.REQUEST_FAILED]	= "You are using a reserved name.",
    [auth.request_status.CHALLENGE_FAILED]	= "The server failed to authenticate you for the use of the reserved name.",
    ["WRONG_KEY"]				= "You authenticated with a key for another user."
}
failure_message[auth.request_status.RESPONSE_FAILED]	= failure_message[auth.request_status.CHALLENGE_FAILED]
failure_message[auth.request_status.TIMEOUT]		= failure_message[auth.request_status.CHALLENGE_FAILED]

local name_cache = {}


server.name_protection_loaded = true


local function rename(cn)

    server.player_rename(cn, "unnamed", true)
    
    server.player_msg(cn, "You have used a reserved name of another player. Server renamed you to 'unnamed'.")
    server.player_msg(cn, " ")
end

local function request_key(cn)

    local sid = server.player_sessionid(cn)
    
    auth.send_request(cn, domain, function(cn, user_id, domain, status)
    
	if not (sid == server.player_sessionid(cn))
	then
	    return
	end
	
        local v = server.player_vars(cn)
	
	if status == auth.request_status.SUCCESS
	then
	    if v.nameprotect_wanted_authname == string.lower(user_id)
	    then
		v.reserved_name = user_id
		server.log(string.format("%s(%i) authenticated as '%s' to use reserved name.", server.player_name(cn), cn, user_id))
		return
	    else
		status = "WRONG_KEY"
	    end
	end
	
	if status == auth.request_status.CANCELLED
	then
	    return
	end
	
	server.player_msg(cn, " ")
	server.player_msg(cn, red(failure_message[status]))
	
	rename(cn)
    end)
end

local function check_name(cn)

    local v = server.player_vars(cn)
    
    v.nameprotect_wanted_authname = string.lower(server.player_name(cn))
    
    if v.reserved_name and not (string.lower(v.reserved_name) == v.nameprotect_wanted_authname)
    then
        if name_cache[v.nameprotect_wanted_authname]
        then
    	    rename(cn)
	else
	    auth.query_id(v.nameprotect_wanted_authname, domain, function(result)
	    
		if result
		then
		    name_cache[server.player_vars(cn).nameprotect_wanted_authname] = server.uptime
		    
		    rename(cn)
		end
	    end)
	end
	
    elseif name_cache[v.nameprotect_wanted_authname]
    then
        request_key(cn)
    else
        auth.query_id(v.nameprotect_wanted_authname, domain, function(result)
	
	    if result
	    then
		name_cache[server.player_vars(cn).nameprotect_wanted_authname] = server.uptime
		
		request_key(cn)
	    end
	end)
    end
end


server.event_handler("connect", check_name)

server.event_handler("rename", check_name)

server.event_handler("disconnect", function(cn)

    local v = server.player_vars(cn)
    
    v.nameprotect_wanted_authname	= nil
    v.reserved_name			= nil
end)

server.event_handler("maintenance", function()

    local cur_time = server.uptime
    
    for name, time in pairs(name_cache)
    do
	if (cur_time - time) > MIN_CACHE_TIME
	then
	    name_cache[name] = nil
	end
    end
end)


return {unload = function()

    server.name_protection_loaded = nil
    
    for p in server.gall()
    do
	p:vars().nameprotect_wanted_authname	= nil
	p:vars().reserved_name			= nil
    end
end}
