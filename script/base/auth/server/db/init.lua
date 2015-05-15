internal = nil

local function use_db(db, arg1, arg2, arg3, arg4, arg5, arg6)

    if not internal
    then
	internal = {}
	internal = dofile("./script/base/auth/server/db/" .. db .. ".lua")
	
	local domains_and_users, case_insensitive_domains, err = internal.load(arg1, arg2, arg3, arg4, arg5, arg6)
	if err
	then
	    server.log_error(err)
	    return
	end
	
	internal.domains = {}
	internal.case_insensitive_domains = {}
	
	for domain, _ in pairs(case_insensitive_domains)
	do
	    internal.case_insensitive_domains[domain] = true
	end
	
	for domain, users in pairs(domains_and_users)
	do
	    internal.domains[domain] = true
	    
	    if internal.case_insensitive_domains[domain]
	    then
	        for name, key in pairs(users)
		do
			local lowered_name = string.lower(name)
			if lowered_name ~= name then
				server.adduser(lowered_name, domain, key.pubkey, key.privilege)
			end
		end
	    end
	    
	    for name, key in pairs(users)
	    do
		server.adduser(name, domain, key.pubkey, key.privilege)
	    end
	end
    else
	server.log_error("You can use only one database.")
	return
    end
end

function server.use_text_database(file)
    use_db("text", file)
end

function server.use_sqlite_database(dir)
    use_db("sqlite", dir)
end

function server.use_mysql_database(hostname, port, database, user, pass, install_db)
    use_db("mysql", hostname, port, database, user, pass, install_db)
end

