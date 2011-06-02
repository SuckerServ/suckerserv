-- frontend
-- server.add_user(name, domain, pubkey)
-- server.del_user(name, domain)
-- server.change_user_name(name, domain, new_name)
-- server.change_user_key(name, domain, pubkey)
-- server.change_user_domain(name, domain, new_domain)
-- server.list_domains()
-- server.list_users(domain)
-- server.add_domain(domain, case_insensitive)
-- server.del_domain(domain)
-- server.change_domain_name(domain, new_domain)
-- server.change_domain_sensitivity(domain, case_insensitive)
--
-- backend
-- internal.is_user(name, domain)
--	nil or true
-- internal.add_user(name, domain, pubkey)
--	err or nil
-- internal.del_user(name, domain)
--	err or nil
-- internal.change_user_name(name, domain, new_name)
--	nil, err or pubkey, nil
-- internal.change_user_key(name, domain, pubkey)
--	err or nil
-- internal.change_user_domain(name, domain, new_domain)
--	nil, err or pubkey, nil
-- internal.list_users(domain)
--	nil, err or users, nil
--		users[name] = pubkey
-- internal.add_domain(domain, case_insensitive)
--	err or nil
-- internal.del_domain(domain)
--	nil, err or users, nil
--		users[name] = true
-- internal.change_domain_name(domain, new_domain)
--	nil, err or users, nil
--		users[name] = pubkey
-- internal.change_domain_sensitivity(domain, case_insensitive)
--	nil, err or users, nil
--		users[name] = pubkey

function server.add_user(name, domain, pubkey)

    if string.len(name) > 15
    then
	server.log_error("add_user: " .. name .. " is too long. (Maximum 15 characters.)")
	return
    end
    
    if name == "unnamed" or name == "bot"
    then
	server.log_error("add_user: 'unnamed' and 'bot' are not allowed as usernames.")
	return
    end
    
    if not internal.domains[domain]
    then
	server.log_error("add_user: Domain, " .. domain .. " doesn't exist.")
	return
    end
    
    if internal.is_user(name, domain)
    then
	server.log_error("add_user: " .. name .. "@" .. domain .. " already exists.")
	return
    end
    
    local err = internal.add_user(name, domain, pubkey)
    if err
    then
	server.log_error("add_user (backend): " .. err)
	return
    end
    
    if internal.case_insensitive_domains[domain]
    then
	server.adduser(string.lower(name), domain, pubkey)
    end
    
    server.adduser(name, domain, pubkey)
    
    server.log("Added: " .. name .. "@" .. domain .. ".")
end

function server.del_user(name, domain)

    if not internal.domains[domain]
    then
	server.log_error("del_user: Domain, " .. domain .. " does not exist.")
	return
    end
    
    if not internal.is_user(name, domain)
    then
	server.log_error("del_user: " .. name .. "@" .. domain .. " does not exist.")
	return
    end
    
    local err = internal.del_user(name, domain)
    if err
    then
	server.log_error("del_user (backend): " .. err)
	return
    end
    
    if internal.case_insensitive_domains[domain]
    then
	if not (name == string.lower(name))	-- prevent double removes
	then
	    server.deleteuser(string.lower(name), domain)
	end
    end
    
    server.deleteuser(name, domain)
    
    server.log("Deleted: " .. name .. "@" .. domain .. ".")
end

function server.change_user_name(name, domain, new_name)

    if name == new_name
    then
	server.log_error("change_user_name: User is already named " .. name .. ".")
	return
    end
    
    if not internal.domains[domain]
    then
	server.log_error("change_user_name: Domain, " .. domain .. " does not exist.")
	return
    end
    
    if not internal.is_user(name, domain)
    then
	server.log_error("change_user_name: " .. name .. "@" .. domain .. " does not exist.")
	return
    end
    
    if internal.is_user(new_name, domain)
    then
	server.log_error("change_user_name: " .. new_name .. "@" .. domain .. " already exists.")
	return
    end
    
    local pubkey, err = internal.change_user_name(name, domain, new_name)
    if err
    then
	server.log_error("change_user_name (backend): " .. err)
	return
    end
    
    if internal.case_insensitive_domains[domain]
    then
	if not (name == string.lower(name))
	then
	    server.deleteuser(string.lower(name), domain)
	end
	server.adduser(string.lower(new_name), domain, pubkey)
    end
    
    server.deleteuser(name, domain)
    server.adduser(new_name, domain, pubkey)
    
    server.log("Renamed: " .. name .. "@" .. domain .. " to " .. new_name .. "@" .. domain .. ".")
end

function server.change_user_key(name, domain, pubkey)

    if not internal.domains[domain]
    then
	server.log_error("change_user_key: Domain, " .. domain .. " does not exist.")
	return
    end
    
    if not internal.is_user(name, domain)
    then
	server.log_error("change_user_key: " .. name .. "@" .. domain .. " does not exist.")
	return
    end
    
    local err = internal.change_user_key(name, domain, pubkey)
    if err
    then
	server.log_error("change_user_key (backend): " .. err)
	return
    end
    
    if internal.case_insensitive_domains[domain]
    then
	if not (name == string.lower(name))
	then
	    server.deleteuser(string.lower(name), domain)
	end
	server.adduser(string.lower(new_name), domain, pubkey)
    end
    
    server.deleteuser(name, domain)
    server.adduser(name, domain, pubkey)
    
    server.log("Changed the pubkey of " .. name .. "@" .. domain .. ".")
end

function server.change_user_domain(name, domain, new_domain)

    if domain == new_domain
    then
	server.log_error("change_user_domain: User is already in " .. domain .. ".")
        return
    end
    
    if not internal.domains[domain]
    then
        server.log_error("change_user_domain: Domain, " .. domain .. " does not exist.")
        return
    end
    
    if not internal.domains[new_domain]
    then
        server.log_error("change_user_domain: Domain, " .. new_domain .. " does not exist.")
        return
    end
    
    if not internal.is_user(name, domain)
    then
	server.log_error("change_user_domain: " .. name .. "@" .. domain .. " does not exist.")
	return
    end
    
    if internal.is_user(name, new_domain)
    then
	server.log_error("change_user_domain: " .. name .. "@" .. new_domain .. " already exist.")
	return
    end
    
    local pubkey, err = internal.change_user_domain(name, domain, new_domain)
    if err
    then
	server.log_error("change_user_domain (backend): " .. err)
	return
    end
    
    if internal.case_insensitive_domains[domain]
    then
	if not (name == string.lower(name))
	then
	    server.deleteuser(string.lower(name), domain)
	end
	server.adduser(string.lower(name), new_domain, pubkey)
    end
    server.deleteuser(name, domain)
    server.adduser(name, new_domain, pubkey)
    
    server.log("Changed the domain of " .. name .. "@" .. domain .. " to " .. name .. "@" .. new_domain .. ".")
end

function server.list_domains()

    server.log("Available domains are:")
    
    for domain, _ in pairs(internal.domains)
    do
	local msg = domain
	
	if internal.case_insensitive_domains[domain]
	then
	    msg = msg .. " (prepared for case insensitive name lookups)"
	end
	
	server.log(msg)
    end
end

function server.list_users(domain)

    if not internal.domains[domain]
    then
	server.log_error("list_users: Domain, " .. domain .. " does not exist.")
	return
    end
    
    local users, err = internal.list_users(domain)
    if err
    then
	server.log_error("list_users (backend): " .. err)
	return
    end
    
    if #users == "0"
    then
	server.log("No users in " .. domain .. ".")
    else
	server.log("Users in " .. domain .. ":")
	
	for name, _ in pairs(users)
	do
	    server.log(name)
	end
    end
end

function server.add_domain(domain, case_insensitive)

    if internal.domains[domain]
    then
	server.log_error("add_domain: Domain, " .. domain .. " already exists.")
	return
    end
    
    if case_insensitive and tostring(case_insensitive) == "0"
    then
	case_insensitive = nil
    end
    
    local err = internal.add_domain(domain, case_insensitive)
    if err
    then
	server.log_error("add_domain (backend): " .. err)
	return
    end
    
    internal.domains[domain] = true
    
    local msg = "Added domain: " .. domain
    if case_insensitive
    then
	internal.case_insensitive_domains[domain] = true
	
	msg = msg .. " (prepared for case insensitive name lookups)."
    end
    
    server.log(msg)
end

function server.del_domain(domain)

    if not internal.domains[domain]
    then
	server.log_error("del_domain: Domain, " .. domain .. " does not exist.")
	return
    end
    
    local users, err = internal.del_domain(domain)
    if err
    then
	server.log_error("del_domain (backend): " .. err)
	return
    end
    
    if internal.case_insensitive_domains[domain]
    then
	for name, _ in pairs(users)
	do
	    if not (name == string.lower(name))
	    then
		server.deleteuser(string.lower(name), domain)
	    end
	end
	
	internal.case_insensitive_domains[domain] = nil
    end
    
    for name, _ in pairs(users)
    do
        server.deleteuser(name, domain)
    end
    
    internal.domains[domain] = nil
    
    server.log("Deleted domain: " .. domain .. ".")
end

function server.change_domain_name(domain, new_domain)

    if domain == new_domain
    then
	server.log_error("change_user_domain: Domain is already named " .. domain .. ".")
        return
    end
    
    if not internal.domains[domain]
    then
	server.log_error("change_domain_name: Domain, " .. domain .. " does not exist.")
	return
    end
    
    if internal.domains[new_domain]
    then
	server.log_error("change_domain_name: Domain, " .. new_domain .. " already exists.")
	return
    end
    
    local users, err = internal.change_domain_name(domain, new_domain)
    if err
    then
	server.log_error("change_domain_name (backend): " .. err)
	return
    end
    
    if internal.case_insensitive_domains[domain]
    then
	for name, key in pairs(users)
	do
	    if not (name == string.lower(name))
	    then
		server.deleteuser(string.lower(name), domain)
	    end
	    server.adduser(string.lower(name), new_domain, key)
	end
	
	internal.case_insensitive_domains[domain] = nil
	internal.case_insensitive_domains[new_domain] = true
    end
    
    for name, key in pairs(users)
    do
        server.deleteuser(name, domain)
        server.adduser(name, new_domain, key)
    end
    
    internal.domains[domain] = nil
    internal.domains[new_domain] = true
    
    server.log("Renamed domain, " .. domain .. " to " .. new_domain .. ".")
end

function server.change_domain_sensitivity(domain, case_insensitive)

    if not internal.domains[domain]
    then
	server.log_error("change_domain_sensitivity: Domain, " .. domain .. " does not exist.")
	return
    end
    
    if not ((not case_insensitive and not internal.case_insensitive_domains[domain]) or (case_insensitive and internal.case_insensitive_domains[domain]))
    then
	local users, err = internal.change_domain_sensitivity(domain, case_insensitive)
	if err
	then
	    server.log_error("change_domain_sensitivity (backend): " .. err)
	    return
	end
	
	if not case_insensitive
	then
	    for name, key in pairs(users)
	    do
		if not (name == string.lower(name))
		then
		    server.deleteuser(string.lower(name), domain)
		end
		server.deleteuser(name, domain)
		server.adduser(name, domain, key)
	    end
	    
	    internal.case_insensitive_domains[domain] = nil
	    
	    server.log("Set domain, " .. domain .. " to case sensitive.")
	else
	    for name, key in pairs(users)
	    do
		server.deleteuser(name, domain)
		server.adduser(string.lower(name), domain, key)
		server.adduser(name, domain, key)
	    end
	    
	    internal.case_insensitive_domains[domain] = true
	    
	    server.log("Set domain, " .. domain .. " to case insensitive.")
	end
    end
end
