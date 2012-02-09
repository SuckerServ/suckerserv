
local servers = {LOCAL = {}}
local domains = {}

local function get_domain(id)
    return domains[id]
end

local function get_user(domain, id)
    if not domain.users then return nil end
    return domain.users[id]
end

local function check_add_table(description, required_fields)
    
    if type(description) ~= "table" then 
        error("expected table")
    end
    
    for _, v in ipairs(required_fields) do
        if not description[v] then
            error("missing " .. v)
        end
    end
end

local function add_server(description)
    check_add_table(description, {"id"})
    description.remote = true
    servers[description.id] = description
end

local function add_domain(description)
    
    check_add_table(description, {"id"})
    
    local server = servers[description.server]
    if not server then error("server not found") end

    description.server = server
    
    if not description.server.remote then
        description.users = {}
        description.get_user = get_user
    end
    
    domains[description.id] = description
    
end

local function add_user(description)
    
    check_add_table(description, {"domain", "id", "public_key"})
    
    local domain = domains[description.domain]
    if not domain then error("domain not found") end
    
    if not domain.users then
        error("cannot add user to remote domain")
    end
    
    domain.users[description.id] = {public_key = description.public_key}
    
end

auth.directory = {}
auth.directory.server = add_server
auth.directory.domain = add_domain
auth.directory.user = add_user
auth.directory.get_domain = get_domain

function auth.directory.make_server_id(id)
    if servers[id] then
       return auth.directory.make_server_id(id .. "_") 
    else 
        return id
    end
end
