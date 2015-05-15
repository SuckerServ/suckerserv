require "luasql_mysql"

local connection = {}
connection.settings = {}

local msg = {}
msg.no_domain = "Domain %s does not exist."
msg.domain = "Domain %s already exists."
msg.no_user = "User %s@%s does not exist."
msg.user = "User %s@%s already exists."
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

local function domain_user_id(name, did)

    local sql = [[SELECT user_id
                      FROM domains_users
                      JOIN users ON domains_users.user_id = users.id
                      WHERE users.name = '%s'
                      AND domains_users.domain_id = %i]]

    local tab = mysql.row(mysql.exec(connection, string.format(sql, mysql.escape_string(name), did)))

    if tab
    then
        return tab.user_id
    end

    return false
end

local function is_user(name, did)
    return domain_user_id(name, did) ~= false
end

local function user_user_id(name, pubkey)

    local sql = [[SELECT id
                      FROM users
                      WHERE name = '%s'
                      AND pubkey = '%s']]

    local tab = mysql.row(mysql.exec(connection, string.format(sql, mysql.escape_string(name), pubkey)))

    if tab
    then
        return tab.id
    end

    return false
end

local function pubkey(name, domain_id)
    local sql = [[SELECT users.pubkey
                    FROM users
					JOIN domains_users ON users.id = domains_users.user_id
					JOIN domains ON domains.id = domains_users.domain_id
                    WHERE users.name = '%s' AND domains.id = %i]]

    local tab = mysql.row(mysql.exec(connection, string.format(sql, mysql.escape_string(name), domain_id)))
    if tab
    then
        return tab.pubkey
    end
    
    return
end

local function privilege(name, domain_id)
    local sql = [[SELECT users.privilege
                    FROM users
					JOIN domains_users ON users.id = domains_users.user_id
					JOIN domains ON domains.id = domains_users.domain_id
                    WHERE users.name = '%s' AND domains.id = %i]]

    local tab = mysql.row(mysql.exec(connection, string.format(sql, mysql.escape_string(name), domain_id)))
    if tab
    then
        return tab.privilege
    end
    
    return
end

local function list_users(domain_id, no_key)

    local users = {}
    
    if no_key
    then
	    local sql = [[SELECT users.name
                    FROM users
					JOIN domains_users ON users.id = domains_users.user_id
					JOIN domains ON domains.id = domains_users.domain_id
                    WHERE domains.id = %i]]
		for tab in mysql.rows(mysql.exec(connection, string.format(sql, domain_id)))
		do
			users[tab.name] = true
		end
    else
	    local sql = [[SELECT domains_users.id, users.name, users.pubkey, domains_users.privilege
                    FROM users
					JOIN domains_users ON users.id = domains_users.user_id
					JOIN domains ON domains.id = domains_users.domain_id
                    WHERE domains.id = %i]]
		for tab in mysql.rows(mysql.exec(connection, string.format(sql, domain_id)))
		do
			users[tab.name] = {id = tab.id, pubkey = tab.pubkey, privilege = tab.privilege}
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

local function add_user(name, domain_id, pubkey, domain, priv)

    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return msg.mysql.unknown
    end

    local uid = user_user_id(name, pubkey)

    if not uid then
        local sql = [[INSERT INTO users (name, pubkey) 
                        VALUES('%s', '%s')]]
        local cursor = mysql.exec(connection, string.format(sql, mysql.escape_string(name), mysql.escape_string(pubkey)))
        if not cursor then
            return string.format(msg.mysql.no_insert, string.format("%s@%s", name, domain))
        end

        uid = "LAST_INSERT_ID()"
    end

    local sql = [[INSERT INTO domains_users (user_id, domain_id, privilege)
                    VALUES(%s, %i, '%s')]]
    local cursor = mysql.exec(connection, string.format(sql, uid, domain_id, priv))
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
    
    for tab in mysql.rows(mysql.exec(connection, "SELECT * FROM domains"))
    do
        domains_and_users[tab.name] = list_users(tab.id, false)
        
        if tab.case_insensitive == "1"
        then
    	    case_insensitive_domains[tab.name] = true
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
local function external_add_user(name, domain, pubkey, priv)

    local did = domain_id(domain)
    if not did
    then
	return string.format(msg.no_domain, domain)
    end
    
    if is_user(name, did)
    then
	string.format(msg.user, name, domain)
    end
    
    return add_user(name, did, pubkey, domain, priv)
end

-- internal.del_user(name, domain)
--       err or nil
local function external_del_user(name, domain)

    local did = domain_id(domain)
    if not did then
	return string.format(msg.no_domain, domain)
    end
    
    local uid = domain_user_id(name, did)
    if not uid then
        return string.format(msg.no_user, name, domain)
    end   
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return msg.mysql.unknown
    end

    local sql = [[DELETE
                      FROM domains_users
                      WHERE user_id = %i
                      AND domain_id = %i]]


    local cursor = mysql.exec(connection, string.format(sql, uid, did))
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
--       nil, nil, err or pubkey, privilege, nil
local function external_change_user_name(name, domain, new_name)

    if name == new_name
    then
	return nil, nil, string.format(msg.same, name, new_name)
    end
    
    local did = domain_id(domain)
    if not did
    then
	return nil, nil, string.format(msg.no_domain, domain)
    end
    
    if not is_user(name, did)
    then
	return nil, nil, string.format(msg.no_user, name, domain)
    end
    
    if is_user(new_name, did)
    then
	return nil, nil, string.format(msg.user, new_name, domain)
    end
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return nil, nil, msg.mysql.unknown
    end
	
	local sql = [[UPDATE users
					JOIN domains_users ON users.id = domains_users.user_id
					JOIN domains ON domains.id = domains_users.domain_id
					SET users.name = '%s'
                    WHERE domains.id = %i AND users.name = '%s']]
    local cursor = mysql.exec(connection, string.format(sql, mysql.escape_string(new_name), did, mysql.escape_string(name)))
    if not cursor
    then
	return nil, nil, string.format(msg.mysql.no_update, string.format("%s@%s", name, domain))
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return nil, nil, msg.mysql.unknown
    end
    
    local key = pubkey(new_name, did)
    if not key
    then
	return nil, nil, string.format(msg.mysql.no_select, "pubkey")
    end
	
    local priv = privilege(new_name, did)
    if not priv
    then
		return nil, nil, string.format(msg.mysql.no_select, "privilege")
    end
    
    return key, priv
end

-- internal.change_user_key(name, domain, pubkey)
--       nil, err or priv, nil
local function external_change_user_key(name, domain, new_pubkey)

    local did = domain_id(domain)
    if not did
    then
        return nil, string.format(msg.no_domain, domain)
    end
    
    if not is_user(name, did)
    then
	return nil, string.format(msg.no_user, name, domain)
    end
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return nil, msg.mysql.unknown
    end
    
	local sql = [[UPDATE users
				JOIN domains_users ON users.id = domains_users.user_id
				JOIN domains ON domains.id = domains_users.domain_id
				SET users.pubkey = '%s'
				WHERE domains.id = %i AND users.name = '%s']]
    local cursor = mysql.exec(connection, string.format(sql, mysql.escape_string(new_pubkey), did, mysql.escape_string(name)))
    if not cursor
    then
        return string.format(msg.mysql.no_update, string.format("%s@%s", name, domain))
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return nil, msg.mysql.unknown
    end
	
	local priv = privilege(new_name, did)
    if not priv
    then
		return nil, string.format(msg.mysql.no_select, "privilege")
    end
    
    return priv
end

-- internal.change_user_priv(name, domain, priv)
--       nil, err or pubkey, nil
local function external_change_user_priv(name, domain, new_priv)

    local did = domain_id(domain)
    if not did
    then
        return nil, string.format(msg.no_domain, domain)
    end
    
    if not is_user(name, did)
    then
	return nil, string.format(msg.no_user, name, domain)
    end
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return nil, msg.mysql.unknown
    end
    
	local sql = [[UPDATE users
				JOIN domains_users ON users.id = domains_users.user_id
				JOIN domains ON domains.id = domains_users.domain_id
				SET users.privilege = '%s'
				WHERE domains.id = %i AND users.name = '%s']]
    local cursor = mysql.exec(connection, string.format(sql, mysql.escape_string(priv), did, mysql.escape_string(name)))
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
		return nil, nil, string.format(msg.mysql.no_select, "pubkey")
    end
    
    return key
end

-- internal.change_user_domain(name, domain, new_domain)
--       nil, nil, err or pubkey, privilege, nil
local function external_change_user_domain(name, domain, new_domain)

    if domain == new_domain
    then
	return nil, nil, string.format(msg.same, domain, new_domain)
    end
    
    local did = domain_id(domain)
    if not did
    then
        return nil, nil, string.format(msg.no_domain, domain)
    end
    
    local new_did = domain_id(new_domain)
    if not new_did
    then
        return nil, nil, string.format(msg.no_domain, new_domain)
    end
    
    if not is_user(name, did)
    then
	return nil, nil, string.format(msg.no_user, name, domain)
    end
    
    if is_user(name, new_did)
    then
	return nil, nil, string.format(msg.user, name, new_domain)
    end
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return nil, nil, msg.mysql.unknown
    end
    
	local sql = [[UPDATE domains_users 
					SET domain_id = %i 
					WHERE domain_id = %i 
						AND (SELECT id
                            FROM users
                            WHERE name = '%s') = user_id]]

    local cursor = mysql.exec(connection, string.format(sql, new_did, did, mysql.escape_string(name)))
    if not cursor
    then
	return nil, nil, string.format(msg.mysql.no_update, string.format("%s@%s", name, domain))
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return nil, nil, msg.mysql.unknown
    end
    
    local key = pubkey(name, new_did)
    if not key
    then
	return nil, nil, string.format(msg.mysql.no_select, "pubkey")
    end
	
    local key = pubkey(name, new_did)
    if not key
    then
	return nil, nil, string.format(msg.mysql.no_select, "pubkey")
    end
	
    local priv = privilege(new_name, did)
    if not priv
    then
		return nil, nil, string.format(msg.mysql.no_select, "privilege")
    end
    
    return key, priv
end

-- internal.list_users(domain)
--       nil, err or users, nil
--               users[name] = pubkey
local function external_list_users(domain, no_key)
    if no_key == nil then no_key = true end

    local did = domain_id(domain)
    if not did
    then
        return nil, string.format(msg.no_domain, domain)
    end
    
    return list_users(did, no_key)
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
    
    local users = list_users(did, true)
    
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
    
    if not mysql.exec(connection, "START TRANSACTION")
    then 
        return nil, msg.mysql.unknown
    end
    
	local sql = [[UPDATE domains
					SET name = '%s'
					WHERE name = '%s']]

    local cursor = mysql.exec(connection, string.format(sql, new_domain, domain))
    if not cursor
    then
		return nil, string.format(msg.mysql.no_update, string.format("%s@%s", name, domain))
    end
    
    if not mysql.exec(connection, "COMMIT")
    then
        return nil, msg.mysql.unknown
    end
    
    return list_users(domain_id(new_domain), false)
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
    
    local users = list_users(did, false)
    
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
    change_user_priv = external_change_user_priv,
    change_user_domain = external_change_user_domain,
    list_users = external_list_users,
    add_domain = external_add_domain,
    del_domain = external_del_domain,
    change_domain_name = external_change_domain_name,
    change_domain_sensitivity = external_change_domain_sensitivity,
    load = external_load}
