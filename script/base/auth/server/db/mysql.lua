require "luasql_mysql"

local connection = {}
connection.settings = {}

local msg = {}
msg.no_domain = "Domain, %s does not exist."
msg.domain = "Domain, %s already exists."
msg.no_user = "User, %s@%s does not exist."
msg.user = "User, %s@%s already exists."
msg.same = "%s is named %s."
msg.mysql = {}
msg.mysql.unknown = "Connection to the mysql database failed."
msg.mysql.no_insert = "Inserting %s failed."
msg.mysql.no_delete = "Deleting %s failed."
msg.mysql.no_update = "Updating %s failed."
msg.mysql.no_select = "Fetching %s failed."


local function domain_id(domain)

    local tab = mysql.row(mysql.exec(connection, string.format("SELECT id FROM domains WHERE name = '%s'", mysql.escape_string(domain))))
    if tab
    then
        return tab.id
    end
    
    return
end

local function is_user(name, domain_id)

    local tab = mysql.row(mysql.exec(connection, string.format("SELECT name FROM users WHERE domain_id = %i AND name = '%s'", domain_id, mysql.escape_string(name))))
    if tab
    then
        return true
    end
    
    return
end

local function pubkey(name, domain_id)

    local tab = mysql.row(mysql.exec(connection, string.format("SELECT pubkey FROM users WHERE domain_id = %i AND name = '%s'", domain_id, mysql.escape_string(name))))
    if tab
    then
        return tab.pubkey
    end
    
    return
end

local function list_users(domain_id, no_key)

    local users = {}
    
    if no_key
    then
	for tab in mysql.rows(mysql.exec(connection, string.format("SELECT name FROM users WHERE domain_id = %i", domain_id)))
	do
	    users[tab.name] = true
	end
    else
	for tab in mysql.rows(mysql.exec(connection, string.format("SELECT name, pubkey FROM users WHERE domain_id = %i", domain_id)))
	do
	    users[tab.name] = tab.pubkey
	end
    end
    
    return users
end

local function add_domain(domain, case_insensitive)

    local case_in = 0
    if case_insensitive
    then
	case_in = 1
    end
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return msg.mysql.unknown
    end
    
    local cursor = mysql.exec(connection, string.format("INSERT INTO domains (name, case_insensitive) VALUES ('%s', %i)", mysql.escape_string(domain), case_in))
    if not cursor
    then
	return string.format(msg.mysql.no_insert, domain)
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return msg.mysql.unknown
    end
    
    return
end

local function add_user(name, domain_id, pubkey, domain)

    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return msg.mysql.unknown
    end
    
    local cursor = mysql.exec(connection, string.format("INSERT INTO users (name, domain_id, pubkey) VALUES('%s', %i, '%s')", mysql.escape_string(name), domain_id, mysql.escape_string(pubkey)))
    if not cursor
    then
	return string.format(msg.mysql.no_insert, string.format("%s@%s", name, domain))
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return msg.mysql.unknown
    end
    
    return
end

local function del_domain(domain_id, domain)

    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return msg.mysql.unknown
    end
    
    local cursor = mysql.exec(connection, string.format("DELETE FROM domains WHERE id = %i", domain_id))
    if not cursor
    then
	return string.format(msg.mysql.no_delete, domain)
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return msg.mysql.unknown
    end
    
    return
end


-- internal.load(host, port, db, user, pass, install_db)
--	nil, nil, err or domains_and_users, case_insensitive_domains, nil
--		domains_and_users[domain][name] = pubkey
--		case_insensitive_domains[domain] = true
local function external_load(host, port, db, user, pass, install_db)

    connection.settings.hostname = host
    connection.settings.port = port
    connection.settings.database = db
    connection.settings.username = user
    connection.settings.password = pass
    connection.settings.schema = "./script/base/auth/server/db/mysql_schema.sql"
    connection.settings.triggers = "./script/base/auth/server/db/mysql_triggers.sql"
    
    if not mysql.open(connection, install_db)
    then
	return nil, nil, msg.mysql.unknown
    end
    
    local domains_and_users = {}
    local case_insensitive_domains = {}
    
    for tab in mysql.rows(mysql.exec(connection, "SELECT name, case_insensitive FROM domains"))
    do
        domains_and_users[tab.name] = {}
        
        if tab.case_insensitive == "1"
        then
    	    case_insensitive_domains[tab.name] = true
    	end
    end
    
    for domain, _ in pairs(domains_and_users)
    do
	for tab in mysql.rows(mysql.exec(connection, string.format("SELECT users.name as name, users.pubkey as pubkey FROM users, domains WHERE domains.name = '%s'", mysql.escape_string(domain))))
	do
    	    domains_and_users[domain][tab.name] = tab.pubkey
    	end
    end
    
    return domains_and_users, case_insensitive_domains
end

-- internal.is_user(name, domain)
--       nil or true
local function external_is_user(name, domain)

    local did = domain_id(domain)
    if did
    then
	if is_user(name, did)
	then
	    return true
	end
    end
    
    return
end

-- internal.add_user(name, domain, pubkey)
--       err or nil
local function external_add_user(name, domain, pubkey)

    local did = domain_id(domain)
    if not did
    then
	return string.format(msg.no_domain, domain)
    end
    
    if is_user(name, did)
    then
	string.format(msg.user, name, domain)
    end
    
    return add_user(name, did, pubkey, domain)
end

-- internal.del_user(name, domain)
--       err or nil
local function external_del_user(name, domain)

    local did = domain_id(domain)
    if not did
    then
	return string.format(msg.no_domain, domain)
    end
    
    if not is_user(name, did)
    then
	return string.format(msg.no_user, name, domain)
    end
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return msg.mysql.unknown
    end
    
    local cursor = mysql.exec(connection, string.format("DELETE FROM users WHERE domain_id = %i AND name = '%s'", did, mysql.escape_string(name)))
    if not cursor
    then
        return string.format(msg.mysql.no_delete, string.format("%s@%s", name, domain))
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return msg.mysql.unknown
    end
    
    return
end

-- internal.change_user_name(name, domain, new_name)
--       nil, err or pubkey, nil
local function external_change_user_name(name, domain, new_name)

    if name == new_name
    then
	return nil, string.format(msg.same, name, new_name)
    end
    
    local did = domain_id(domain)
    if not did
    then
	return nil, string.format(msg.no_domain, domain)
    end
    
    if not is_user(name, did)
    then
	return nil, string.format(msg.no_user, name, domain)
    end
    
    if is_user(new_name, did)
    then
	return nil, string.format(msg.user, new_name, domain)
    end
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return nil, msg.mysql.unknown
    end
    
    local cursor = mysql.exec(connection, string.format("UPDATE users SET name = '%s' WHERE domain_id = %i AND name = '%s'", mysql.escape_string(new_name), did, mysql.escape_string(name)))
    if not cursor
    then
	return nil, string.format(msg.mysql.no_update, string.format("%s@%s", name, domain))
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return nil, msg.mysql.unknown
    end
    
    local key = pubkey(new_name, did)
    if not key
    then
	return nil, string.format(msg.mysql.no_select, "pubkey")
    end
    
    return key
end

-- internal.change_user_key(name, domain, pubkey)
--       err or nil
local function external_change_user_key(name, domain, new_pubkey)

    local did = domain_id(domain)
    if not did
    then
        return string.format(msg.no_domain, domain)
    end
    
    if not is_user(name, did)
    then
	return string.format(msg.no_user, name, domain)
    end
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return msg.mysql.unknown
    end
    
    local cursor = mysql.exec(connection, string.format("UPDATE users SET pubkey = '%s' WHERE domain_id = %i AND name = '%s'", mysql.escape_string(new_pubkey), did, mysql.escape_string(name)))
    if not cursor
    then
        return string.format(msg.mysql.no_update, string.format("%s@%s", name, domain))
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return msg.mysql.unknown
    end
    
    return
end

-- internal.change_user_domain(name, domain, new_domain)
--       nil, err or pubkey, nil
local function external_change_user_domain(name, domain, new_domain)

    if domain == new_domain
    then
	return nil, string.format(msg.same, domain, new_domain)
    end
    
    local did = domain_id(domain)
    if not did
    then
        return nil, string.format(msg.no_domain, domain)
    end
    
    local new_did = domain_id(new_domain)
    if not new_did
    then
        return nil, string.format(msg.no_domain, new_domain)
    end
    
    if not is_user(name, did)
    then
	return nil, string.format(msg.no_user, name, domain)
    end
    
    if is_user(name, new_did)
    then
	return nil, string.format(msg.user, name, new_domain)
    end
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return nil, msg.mysql.unknown
    end
    
    local cursor = mysql.exec(connection, string.format("UPDATE users SET domain_id = %i WHERE domain_id = %i AND name = '%s'", new_did, did, mysql.escape_string(name)))
    if not cursor
    then
	return nil, string.format(msg.mysql.no_update, string.format("%s@%s", name, domain))
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return nil, msg.mysql.unknown
    end
    
    local key = pubkey(name, new_did)
    if not key
    then
	return nil, string.format(msg.mysql.no_select, "pubkey")
    end
    
    return key
end

-- internal.list_users(domain)
--       nil, err or users, nil
--               users[name] = pubkey
local function external_list_users(domain)

    local did = domain_id(domain)
    if not did
    then
        return nil, string.format(msg.no_domain, domain)
    end
    
    return list_users(did)
end

-- internal.add_domain(domain, case_insensitive)
--       err or nil
local function external_add_domain(domain, case_insensitive)

    local did = domain_id(domain)
    if did
    then
	return string.format(msg.domain, domain)
    end
    
    return add_domain(domain, case_insensitive)
end

-- internal.del_domain(domain)
--       nil, err or users, nil
--               users[name] = true
local function external_del_domain(domain)

    local did = domain_id(domain)
    if not did
    then
        return nil, string.format(msg.no_domain, domain)
    end
    
    local users = list_users(did, "no_key")
    
    local err = del_domain(did, domain)
    if err
    then
	return nil, err
    end
    
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
    
    local did = domain_id(domain)
    if not did
    then
	return nil, string.format(msg.no_domain, domain)
    end
    
    local users = list_users(did)
    
    local new_did = domain_id(new_domain)
    if not new_did
    then
	local domain_case = mysql.row(mysql.exec(connection, string.format("SELECT case_insensitive FROM domains WHERE id = %i", did)))
	if not domain_case
	then
	    return nil, string.format(msg.mysql.no_select, "case_insensitive")
	else
	    if domain_case.case_insensitive == "1"
	    then
		domain_case = true
	    else
		domain_case = nil
	    end
	end
	
	local add_domain_err = add_domain(new_domain, domain_case)
	if add_domain_err
	then
	    return nil, add_domain_err
	end
	
	new_did = domain_id(new_domain)
	if not new_did
	then
	    return nil, string.format(msg.mysql.no_select, "domain_id")
	end
    else
	local domain_case = mysql.row(mysql.exec(connection, string.format("SELECT case_insensitive FROM domains WHERE id = %i", did)))
	if not domain_case
	then
	    return nil, string.format(msg.mysql.no_select, "case_insensitive")
	else
	    if domain_case.case_insensitive == "1"
	    then
		domain_case = true
	    else
		domain_case = false
	    end
	end
	
	local new_domain_case = mysql.row(mysql.exec(connection, string.format("SELECT case_insensitive FROM domains WHERE id = %i", new_did)))
	if not new_domain_case
	then
	    return nil, string.format(msg.mysql.no_select, "case_insensitive")
	else
	    if new_domain_case.case_insensitive == "1"
	    then
		new_domain_case = true
	    else
		new_domain_case = false
	    end
	end
	
	if (domain_case and not new_domain_case) or (not domain_case and new_domain_case)
        then
            return nil, "New domain already exists and has not the same case sensitive setting."
        end
        
	for name, _ in pairs(users)
	do
	    if is_user(name, new_did)
	    then
		return nil, string.format(msg.user, name, new_domain)
	    end
	end
    end
    
    for name, key in pairs(users)
    do
        add_user(name, new_did, key, new_domain)
    end
    
    del_domain(did, domain)
    
    return users
end

-- internal.change_domain_sensitivity(domain, case_insensitive)
--       nil, err or users, nil
--               users[name] = pubkey
local function external_change_domain_sensitivity(domain, case_insensitive)

    local did = domain_id(domain)
    if not did
    then
        return nil, string.format(msg.no_domain, domain)
    end
    
    local users = list_users(did)
    
    local case_in = 0
    if case_insensitive
    then
	case_in = 1
    end
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return nil, msg.mysql.unknown
    end
    
    local cursor = mysql.exec(connection, string.format("UPDATE domains SET case_insensitive = %i WHERE id = %i", case_in, did))
    if not cursor
    then
	return nil, string.format(msg.mysql.no_update, domain)
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return nil, msg.mysql.unknown
    end
    
    return users
end


server.event_handler("shutdown", function()

    mysql.close(connection)
end)


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
