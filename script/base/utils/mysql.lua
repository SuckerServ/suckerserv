mysql = {}

-- Copied from http://lua-users.org/wiki/GenericInputAlgorithms
function mysql.rows(cursor)

    local row = {}
    
    return function()
	return cursor:fetch(row,'a')
    end
end

function mysql.row(cursor)

    local row = {}
    
    return cursor:fetch(row,'a')
end

-- Copied from http://lua-users.org/wiki/GenericInputAlgorithms
-- fixed typo
function mysql.col(fieldname, cursor)

    local row = {}
    
    return function()
        row = cursor:fetch(row,'a')
        
        if not row then
    	    return nil 
        else
    	    return row[fieldname]
        end
    end
end

function mysql.escape_string(s)

    s = string.gsub(s, "\\", "\\\\")
    s = string.gsub(s, "%\"", "\\\"")
    s = string.gsub(s, "%'", "\\'")
    
    return s
end

local function install_db(connection)

    local schema = server.read_whole_file(connection.settings.schema)
    
    for statement in string.gmatch(schema, "CREATE TABLE[^;]+")
    do
        local cursor, err = connection.handler:execute(statement)
        if not cursor
        then
    	    error(err)
    	end
    end
    
    local triggers = server.read_whole_file(connection.settings.triggers)
    
    for statement in string.gmatch(triggers, "CREATE TRIGGER[^~]+")
    do
        local cursor, err = connection.handler:execute(statement)
        if not cursor
        then
    	    error(err)
    	end
    end
end

function mysql.open(connection, install)

    require "luasql_mysql"
    
    connection.handler = luasql.mysql():connect(connection.settings.database, connection.settings.username, connection.settings.password, connection.settings.hostname, connection.settings.port)
    
    if not connection.handler
    then
        server.log_error(string.format("couldn't connect to MySQL server at %s:%s", connection.settings.hostname, connection.settings.port))
        return
    end
    
    if install
    then
        install_db(connection)
    end
    
    return true
end

function mysql.reopen(connection)

    if not connection.handler and not mysql.open(connection)
    then
       return
    end
    
    return true
end

function mysql.exec(connection, statement)

    if not mysql.reopen(connection)
    then
	return
    end
    
    local cursor, errorinfo = connection.handler:execute(statement)
    
    if not cursor
    then
        server.log_error(errorinfo)
	
	connection.handler:close()
    	connection.handler = nil
    end
    
    return cursor
end

function mysql.close(connection)

    connection.handler:close()
    connection.handler = nil
    
    return
end
