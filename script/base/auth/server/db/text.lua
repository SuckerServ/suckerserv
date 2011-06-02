local file
local mem = {}
local mem_case_insensitive_domains = {}

local msg = {}
msg.no_domain = "Domain, %s does not exist."
msg.domain = "Domain, %s already exists."
msg.no_user = "User, %s@%s does not exist."
msg.user = "User, %s@%s already exists."
msg.same = "%s is named %s."

local function write_db()

    local out = io.open(file, "w+")
    
    for domain, users in pairs(mem)
    do
	local msg = "__add_domain \"" .. domain .. "\""
	
	if mem_case_insensitive_domains[domain]
	then
	    msg = msg .. " \"case_insensitive\""
	end
	
	out:write(msg .. "\n")
	
	for name, key in pairs(users)
	do
	    out:write("__add_user \"" .. name .. "\" \"" .. domain .. "\" \"" .. key .. "\"\n")
	end
    end
    
    out:flush()
    out:close()
end


-- internal.load(filename)
--	nil, nil, err or domains_and_users, case_insensitive_domains, nil
--		domains_and_users[domain][name] = pubkey
--		case_insensitive_domains[domain] = true
local function external_load(filename)

    file = filename
    
    local domains_and_users = {}
    local case_insensitive_domains = {}
    
    function server.__add_domain(domain, case_insensitive)
    
	mem[domain] = {}
	domains_and_users[domain] = {}
	
	if case_insensitive
	then
	    mem_case_insensitive_domains[domain] = true
	    case_insensitive_domains[domain] = true
	end
    end
    
    function server.__add_user(name, domain, pubkey)

	mem[domain][name] = pubkey
	domains_and_users[domain][name] = pubkey
    end
    
--    os.execute("test -e " .. file .. " || touch " .. file)
    if server.file_exists(file)
    then
	server.execute_cubescript_file(file)
    else
	os.execute("touch " .. file .. " &")
    end
    
    return domains_and_users, case_insensitive_domains
end

-- internal.is_user(name, domain)
--       nil or true
local function external_is_user(name, domain)

    if mem[domain] and mem[domain][name]
    then
	return true
    end
    
    return
end

-- internal.add_user(name, domain, pubkey)
--       err or nil
local function external_add_user(name, domain, pubkey)

    if not mem[domain]
    then
	return string.format(msg.no_domain, domain)
    end
    
    if mem[domain][name]
    then
	return string.format(msg.user, name, domain)
    end
    
    mem[domain][name] = pubkey
    
    write_db()
    
    return
end

-- internal.del_user(name, domain)
--       err or nil
local function external_del_user(name, domain)

    if not mem[domain]
    then
	return string.format(msg.no_domain, domain)
    end
    
    if not mem[domain][name]
    then
	return string.format(msg.no_user, name, domain)
    end
    
    mem[domain][name] = nil
    
    write_db()
    
    return
end

-- internal.change_user_name(name, domain, new_name)
--       nil, err or pubkey, nil
local function external_change_user_name(name, domain, new_name)

    if name == new_name
    then
	return nil, string.format(msg.same, name, new_name)
    end
    
    if not mem[domain]
    then
	return nil, string.format(msg.no_domain, domain)
    end
    
    if not mem[domain][name]
    then
	return nil, string.format(msg.no_user, name, domain)
    end
    
    if mem[domain][new_name]
    then
	return nil, string.format(msg.user, new_name, domain)
    end
    
    mem[domain][new_name] = mem[domain][name]
    mem[domain][name] = nil
    
    write_db()
    
    return mem[domain][new_name]
end

-- internal.change_user_key(name, domain, pubkey)
--       err or nil
local function external_change_user_key(name, domain, new_pubkey)

    if not mem[domain]
    then
	return nil, string.format(msg.no_domain, domain)
    end
    
    if not mem[domain][name]
    then
	return nil, string.format(msg.no_user, name, domain)
    end
    
    mem[domain][name] = new_pubkey
    
    write_db()
    
    return
end

-- internal.change_user_domain(name, domain, new_domain)
--       nil, err or pubkey, nil
local function external_change_user_domain(name, domain, new_domain)

    if domain == new_domain
    then
	return nil, string.format(msg.same, domain, new_domain)
    end
    
    if not mem[domain]
    then
	return nil, string.format(msg.no_domain, domain)
    end
    
    if not mem[new_domain]
    then
	return nil, string.format(msg.no_domain, new_domain)
    end
    
    if not mem[domain][name]
    then
	return nil, string.format(msg.no_user, name, domain)
    end
    
    if mem[new_domain][name]
    then
	return nil, string.format(msg.user, name, new_domain)
    end
    
    mem[new_domain][name] = mem[domain][name]
    mem[domain][name] = nil
    
    write_db()
    
    return mem[new_domain][name]
end

-- internal.list_users(domain)
--       nil, err or users, nil
--               users[name] = pubkey
local function external_list_users(domain)

    if not mem[domain]
    then
	return nil, string.format(msg.no_domain, domain)
    end
    
    local users = {}
    
    for name, key in pairs(mem[domain])
    do
	users[name] = key
    end
    
    return users
end

-- internal.add_domain(domain, case_insensitive)
--       err or nil
local function external_add_domain(domain, case_insensitive)

    if mem[domain]
    then
	return string.format(msg.domain, domain)
    end
    
    mem[domain] = {}
    
    if case_insensitive
    then
	mem_case_insensitive_domains[domain] = true
    end
    
    write_db()
    
    return
end

-- internal.del_domain(domain)
--       nil, err or users, nil
--               users[name] = true
local function external_del_domain(domain)

    if not mem[domain]
    then
	return nil, string.format(msg.no_domain, domain)
    end
    
    local users = {}
    
    for name, _ in pairs(mem[domain] or {})
    do
        users[name] = true
    end
    
    mem[domain] = nil
    
    write_db()
    
    return users
end

-- internal.change_domain_name(domain, new_domain)
--       nil, err or users, nil
--               users[name] = pubkey
local function external_change_domain_name(domain, new_domain)

    if domain == new_domain
    then
	return nil, string.format(msg.same, domain, new_domain)
    end
    
    if not mem[domain]
    then
	return nil, string.format(msg.no_domain, domain)
    end
    
    if not mem[new_domain]
    then
	mem[new_domain] = {}
	mem_case_insensitive_domains[new_domain] = mem_case_insensitive_domains[domain]
    else
	if (mem_case_insensitive_domains[domain] and not mem_case_insensitive_domains[new_domain]) or (not mem_case_insensitive_domains[domain] and mem_case_insensitive_domains[new_domain])
	then
	    return nil, "New domain already exists and has not the same case sensitive setting."
	end
	
	for name, _ in pairs(mem[domain])
	do
	    if mem[new_domain][name]
	    then
		return nil, string.format(msg.user, name, new_domain)
	    end
	end
    end
    
    local users = {}
    
    for name, key in pairs(mem[domain])
    do
        users[name] = key
        mem[new_domain][name] = key
    end
    
    mem[domain] = nil
    
    write_db()
    
    return users
end

-- internal.change_domain_sensitivity(domain, case_insensitive)
--       nil, err or users, nil
--               users[name] = pubkey
local function external_change_domain_sensitivity(domain, case_insensitive)

    if not mem[domain]
    then
	return nil, string.format(msg.no_domain, domain)
    end
    
    if case_insensitive
    then
	mem_case_insensitive_domains[domain] = true
    else
	mem_case_insensitive_domains[domain] = nil
    end
    
    local users = mem[domain]
    
    write_db()
    
    return users
end

return {is_user = external_is_user,
    add_user = external_add_user,
    del_user = external_del_user,
    change_user_name = external_change_user_name,
    change_user_key = external_change_user_key,
    change_user_domain = external_change_user_domain,
    list_users = external_list_users,
    add_domain = external_add_domain,
    del_domain = external_del_domain,
    change_domain_name = external_change_domain_name,
    change_domain_sensitivity = external_change_domain_sensitivity,
    load = external_load}
