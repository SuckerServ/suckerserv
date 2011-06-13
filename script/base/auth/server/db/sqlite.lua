require "sqlite3utils"
require "crypto"

local databases = {}
local base_dir
local sqlite_synchronous = ((server.sqlite3_synchronous or 0) == 1)
local sqlite_exclusive_locking = ((server.sqlite3_exclusive_locking or 0) == 1)

local msg = {}
msg.no_domain = "Domain, %s does not exist."
msg.domain = "Domain, %s already exists."
msg.no_user = "User, %s@%s does not exist."
msg.user = "User, %s@%s already exists."
msg.same = "%s is named %s."
msg.sqlite_no_select = "Fetching %s failed."


local function create_db()

    local name = crypto.md5sum(os.date("%s%N"))
    while server.file_exists(base_dir .. "/" .. name .. ".sqlite")
    do
	name = crypto.md5sum(name .. os.date("%s%N"))
    end
    
    local file = base_dir .. "/" .. name .. ".sqlite"
    
    local db, err = sqlite3.open(file)
    
    if not db
    then
	return nil, err
    end
    
    sqlite3utils.create_missing_tables("./script/base/auth/server/db/sqlite_schema.sql", db, file)
    
    db:close()
    
    return file
end

local function open_db(domain)

    if databases[domain]
    then
	local err
	databases[domain].handler, err = sqlite3.open(databases[domain].file)
	if not databases[domain].handler
	then
	    return err
	end
	
	if sqlite_synchronous
	then
	    sqlite3utils.set_sqlite3_synchronous_pragma(databases[domain].handler, sqlite_synchronous)
	end
	if sqlite_exclusive_locking
	then
	    sqlite3utils.set_sqlite3_exclusive_locking(databases[domain].handler)
	end
	
	databases[domain].insert = {}
	databases[domain].insert.user = databases[domain].handler:prepare("INSERT INTO users (name, domain_id, pubkey) VALUES(:name, :domain_id, :pubkey)")
	if not databases[domain].insert.user
        then
    	    return databases[domain].handler:error_message()
	end
	databases[domain].insert.domain = databases[domain].handler:prepare("INSERT INTO domains (name, case_insensitive) VALUES (:name, :case_insensitive)")
	if not databases[domain].insert.domain
        then
    	    return databases[domain].handler:error_message()
	end
	
	databases[domain].delete = {}
	databases[domain].delete.user = databases[domain].handler:prepare("DELETE FROM users WHERE domain_id = :domain_id AND name = :name")
	if not databases[domain].delete.user
        then
    	    return databases[domain].handler:error_message()
	end
	
	databases[domain].update = {}
	databases[domain].update.user_name = databases[domain].handler:prepare("UPDATE users SET name = :new_name WHERE domain_id = :domain_id AND name = :name")
	if not databases[domain].update.user_name
        then
    	    return databases[domain].handler:error_message()
	end
	databases[domain].update.user_key = databases[domain].handler:prepare("UPDATE users SET pubkey = :new_pubkey WHERE domain_id = :domain_id AND name = :name")
	if not databases[domain].update.user_key
        then
    	    return databases[domain].handler:error_message()
	end
	databases[domain].update.domain_case_insensitive = databases[domain].handler:prepare("UPDATE domains SET case_insensitive = :case_insensitive WHERE id = :domain_id")
	if not databases[domain].update.domain_case_insensitive
        then
    	    return databases[domain].handler:error_message()
	end
	
	databases[domain].select = {}
	databases[domain].select.domain_id = databases[domain].handler:prepare("SELECT id FROM domains WHERE name = :name")
        if not databases[domain].select.domain_id
        then
    	    return databases[domain].handler:error_message()
	end
	databases[domain].select.domain_case_insensitive = databases[domain].handler:prepare("SELECT case_insensitive FROM domains WHERE id = :domain_id")
        if not databases[domain].select.domain_case_insensitive
        then
    	    return databases[domain].handler:error_message()
	end
	databases[domain].select.user_pubkey = databases[domain].handler:prepare("SELECT pubkey FROM users WHERE domain_id = :domain_id AND name = :name")
	if not databases[domain].select.user_pubkey
        then
    	    return databases[domain].handler:error_message()
	end
	databases[domain].select.user = databases[domain].handler:prepare("SELECT * FROM users WHERE domain_id = :domain_id AND name = :name")
	if not databases[domain].select.user
        then
    	    return databases[domain].handler:error_message()
	end
	databases[domain].select.users = databases[domain].handler:prepare("SELECT name, pubkey FROM users WHERE domain_id = :domain_id")
	if not databases[domain].select.users
        then
    	    return databases[domain].handler:error_message()
	end
	databases[domain].select.users_no_key = databases[domain].handler:prepare("SELECT name FROM users WHERE domain_id = :domain_id")
	if not databases[domain].select.users_no_key
        then
    	    return databases[domain].handler:error_message()
	end
    else
	return "No information about the database."
    end
    
    return
end

local function domain_id(domain)

    if databases[domain]
    then
	databases[domain].select.domain_id:bind_names{["name"] = domain}
	local tab = sqlite3utils.first_row(databases[domain].select.domain_id)
	databases[domain].select.domain_id:reset()
	if tab
	then
	    return tab.id
	end
    end
    
    return
end

local function is_user(name, domain_id, domain)

    databases[domain].select.user:bind_names{["domain_id"] = domain_id, ["name"] = name}
    local tab = sqlite3utils.first_row(databases[domain].select.user)
    databases[domain].select.user:reset()
    if tab
    then
        return true
    end
    
    return
end

local function pubkey(name, domain_id, domain)

    databases[domain].select.user_pubkey:bind_names{["domain_id"] = domain_id, ["name"] = name}
    local tab = sqlite3utils.first_row(databases[domain].select.user_pubkey)
    databases[domain].select.user_pubkey:reset()
    if tab
    then
	return tab.pubkey
    end
    
    return
end

local function case_insensitive(domain_id, domain)

    databases[domain].select.domain_case_insensitive:bind_names{["domain_id"] = domain_id}
    local tab = sqlite3utils.first_row(databases[domain].select.domain_case_insensitive)
    databases[domain].select.domain_case_insensitive:reset()
    if tab
    then
	return tab.case_insensitive
    end
    
    return
end

local function list_users(domain_id, domain, no_key)

    local users = {}
    
    if no_key
    then
	databases[domain].select.users_no_key:bind_names{["domain_id"] = domain_id}
	for tab in databases[domain].select.users_no_key:nrows()
	do
	    users[tab.name] = true
	end
	databases[domain].select.users_no_key:reset()
    else
	databases[domain].select.users:bind_names{["domain_id"] = domain_id}
	for tab in databases[domain].select.users:nrows()
	do
	    users[tab.name] = tab.pubkey
	end
	databases[domain].select.users:reset()
    end
    
    return users
end

local function add_domain(domain, case_insensitive)

    local file, err = create_db()
    if err
    then
	return err
    end
    
    databases[domain] = {}
    databases[domain].file = file
    
    err = open_db(domain)
    if err
    then
        return err
    end
    
    local case_in = 0
    if case_insensitive
    then
	case_in = 1
    end
    
    databases[domain].handler:exec("BEGIN TRANSACTION")
    databases[domain].insert.domain:bind_names{["name"] = domain, ["case_insensitive"] = case_in}
    databases[domain].insert.domain:step()
    databases[domain].insert.domain:reset()
    databases[domain].handler:exec("COMMIT TRANSACTION")
    
    return
end

local function del_domain(domain)

    databases[domain].handler:close()
    os.execute("rm -f " .. databases[domain].file .. " &")
    databases[domain] = nil
    
    return
end

local function add_user(name, domain_id, pubkey, domain)

    databases[domain].handler:exec("BEGIN TRANSACTION")
    databases[domain].insert.user:bind_names{["name"] = name, ["domain_id"] = domain_id, ["pubkey"] = pubkey}
    databases[domain].insert.user:step()
    databases[domain].insert.user:reset()
    databases[domain].handler:exec("COMMIT TRANSACTION")
    
    return
end

local function del_user(name, domain_id, domain)

    databases[domain].handler:exec("BEGIN TRANSACTION")
    databases[domain].delete.user:bind_names{["domain_id"] = domain_id, ["name"] = name}
    databases[domain].delete.user:step()
    databases[domain].delete.user:reset()
    databases[domain].handler:exec("COMMIT TRANSACTION")
    
    return
end


-- internal.load(dir)
--	nil, nil, err or domains_and_users, case_insensitive_domains, nil
--		domains_and_users[domain][name] = pubkey
--		case_insensitive_domains[domain] = true
local function external_load(dir)

    base_dir = dir
    
    if server.dir_exists(base_dir)
    then
	local filesystem = require "filesystem"
	
	for filetype, filename in filesystem.dir(base_dir)
	do
    	    local fullfilename = base_dir .. "/" .. filename
    	    
    	    if filetype == 0 and filename:match(".sqlite$")
    	    then
        	local db, err = sqlite3.open(fullfilename)
        	if not db
        	then
        	    return nil, nil, err
    		end
    		
    		for tab in db:nrows("SELECT name FROM domains")
    		do
    		    if databases[tab.name]
    		    then
    			return nil, nil, "Double use of domain " .. tab.name
    		    end
    		    
    		    databases[tab.name] = {}
		    databases[tab.name].file = fullfilename
		end
		
		db:close()
    	    end
    	end
    else
	os.execute("mkdir -p " .. base_dir .. " &")
    end
    
    local domains_and_users = {}
    local case_insensitive_domains = {}
    
    for domain, db in pairs(databases)
    do
	local err = open_db(domain)
	if err
	then
	    return nil, nil, err
	end
	
	domains_and_users[domain] = {}
	
	for tab in db.handler:nrows(string.format("SELECT case_insensitive FROM domains WHERE name = '%s'", domain))
    	do
	    if tab.case_insensitive == "1"
	    then
		case_insensitive_domains[domain] = true
	    end
	end
	
	for tab in db.handler:nrows(string.format("SELECT users.name as name, users.pubkey as pubkey FROM users INNER JOIN domains ON users.domain_id = domains.id WHERE domains.name = '%s'", domain))
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
	if is_user(name, did, domain)
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
    
    if is_user(name, did, domain)
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
    
    if not is_user(name, did, domain)
    then
	return string.format(msg.no_user, name, domain)
    end
    
    return del_user(name, did, domain)
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
    
    if not is_user(name, did, domain)
    then
	return nil, string.format(msg.no_user, name, domain)
    end
    
    if is_user(new_name, did, domain)
    then
	return nil, string.format(msg.user, new_name, domain)
    end
    
    databases[domain].handler:exec("BEGIN TRANSACTION")
    databases[domain].update.user_name:bind_names{["domain_id"] = did, ["name"] = name, ["new_name"] = new_name}
    databases[domain].update.user_name:step()
    databases[domain].update.user_name:reset()
    databases[domain].handler:exec("COMMIT TRANSACTION")
    
    local key = pubkey(new_name, did, domain)
    if not key
    then
	return nil, string.format(msg.sqlite_no_select, "pubkey")
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
    
    if not is_user(name, did, domain)
    then
	return string.format(msg.no_user, name, domain)
    end
    
    databases[domain].handler:exec("BEGIN TRANSACTION")
    databases[domain].update.user_key:bind_names{["domain_id"] = did, ["name"] = name, ["new_pubkey"] = new_pubkey}
    databases[domain].update.user_key:step()
    databases[domain].update.user_key:reset()
    databases[domain].handler:exec("COMMIT TRANSACTION")
    
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
    
    if not is_user(name, did, domain)
    then
	return nil, string.format(msg.no_user, name, domain)
    end
    
    if is_user(name, new_did, new_domain)
    then
	return nil, string.format(msg.user, name, new_domain)
    end
    
    local key = pubkey(name, did, domain)
    if not key
    then
	return nil, string.format(msg.sqlite_no_select, "pubkey")
    end
    
    del_user(name, did, domain)
    add_user(name, new_did, key, new_domain)
    
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
    
    return list_users(did, domain)
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
    
    local users = list_users(did, domain, "no_key")
    
    local err = del_domain(domain)
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
    
    local users = list_users(did, domain)
    
    local new_did = domain_id(new_domain)
    if not new_did
    then
	local domain_case = case_insensitive(did, domain)
	if not domain_case
	then
	    return nil, string.format(msg.sqlite.no_select, "case_insensitive")
	else
	    if tonumber(domain_case) == 1
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
	    return nil, string.format(msg.sqlite_no_select, "domain_id")
	end
    else
	local domain_case = case_insensitive(did, domain)
	if not domain_case
	then
	    return nil, string.format(msg.sqlite_no_select, "case_insensitive")
	else
	    if tonumber(domain_case) == 1
	    then
		domain_case = true
	    else
		domain_case = false
	    end
	end
	
	local new_domain_case = case_insensitive(new_did, domain)
	if not new_domain_case
	then
	    return nil, string.format(msg.sqlite_no_select, "case_insensitive")
	else
	    if tonumber(new_domain_case) == 1
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
	    if is_user(name, new_did, new_domain)
	    then
		return nil, string.format(msg.user, name, new_domain)
	    end
	end
    end
    
    for name, key in pairs(users)
    do
        add_user(name, new_did, key, new_domain)
    end
    
    del_domain(domain)
    
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
    
    local users = list_users(did, domain)
    
    local case_in = 0
    if case_insensitive
    then
	case_in = 1
    end
    
    databases[domain].handler:exec("BEGIN TRANSACTION")
    databases[domain].update.domain_case_insensitive:bind_names{["domain_id"] = did, ["case_insensitive"] = case_in}
    databases[domain].update.domain_case_insensitive:step()
    databases[domain].update.domain_case_insensitive:reset()
    databases[domain].handler:exec("COMMIT TRANSACTION")
    
    return users
end


server.event_handler("shutdown", function()

    for _, db in pairs(databases)
    do
	db.handler:close()
	db.handler = nil
    end
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
