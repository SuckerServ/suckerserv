local clans = {}
local tags = {}

local failure_message = {
    [auth.request_status.REQUEST_FAILED]	= "You are using a clan tag in your name which is reserved by the clan '%s'.",
    [auth.request_status.CHALLENGE_FAILED]	= "The server failed to authenticate you for the use of the reserved clan tag."
}
failure_message[auth.request_status.RESPONSE_FAILED]	= failure_message[auth.request_status.CHALLENGE_FAILED]
failure_message[auth.request_status.TIMEOUT]		= failure_message[auth.request_status.CHALLENGE_FAILED]


function server.is_clan_registrated(tag)

    if tag
    then
	return tags[string.lower(tag)]
    end
    
    return
end


function clan(description)

    if not validate(description, {name="string", tag_pattern="string"}) then
        error("invalid clan information")
    end
    
    if description.auth
    then
        local server_id = auth.directory.make_server_id(description.auth.server[1])
        
        auth.directory.server{
            id		= server_id,
            hostname	= description.auth.server[1],
            port	= description.auth.server[2]
        }
        
        auth.directory.domain{
            server	= server_id,
            id		= description.auth.domain
        }
        
        description.auth_domain	= description.auth.domain
        description.server	= nil
    else
        if description.auth_domain
        then
            if not auth.directory.get_domain(description.auth_domain)
            then
                error(string.format("Unknown auth domain for clan tag protection entry '%s'.", description.name))
            end
        else
            error(string.format("Missing auth information for clan tag protection entry '%s'.", description.name))
        end
    end
    
    description.tag_pattern = string.lower(description.tag_pattern)
    
    table.insert(clans, description)
    
    tags[description.tag_pattern] = true
end

local function check_name(cn)

    if not server.is_bot(cn)
    then
	local name	= string.lower(server.player_name(cn))
	local sid	= server.player_sessionid(cn)
	
	for _, clan in pairs(clans)
	do
    	    if string.match(name, clan.tag_pattern)
    	    then
        	auth.send_request(cn, clan.auth_domain, function(cn, user_id, domain, status)
    		
    		    if not (sid == server.player_sessionid(cn))
    		    then
    			return
    		    end
    		    
            	    if status == auth.request_status.SUCCESS
            	    then
                	server.log(string.format("%s(%i) authenticated as '%s' to use reserved clan tag.", server.player_name(cn), cn, user_id))
                    	server.player_vars(cn).reserved_tag = clan.tag_pattern
            	    end
            	    
            	    if status == auth.request_status.SUCCESS or status == auth.request_status.CANCELLED
            	    then
            		return
            	    end
            	    
            	    server.player_msg(cn, " ")
            	    server.player_msg(cn, red(string.format(failure_message[status], clan.name)))
            	    
            	    server.player_rename(cn, "unnamed", true)
            	    
            	    server.player_msg(cn, server.clantag_rename_message)
        	end)
    	    end
	end
    end
end


server.event_handler("connect", check_name)

server.event_handler("rename", check_name)

exec_if_found("conf/clans.lua")

return {unload = function()
    server.is_clan_registrated = nil
    for p in server.gall()
    do
	p:vars().reserved_tag = nil
    end
end}
