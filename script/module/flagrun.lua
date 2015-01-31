--[[
    Save flagrun time
    //piernov
]]

local using_mysql = (server.stats_use_mysql == 1)
local query_backend_name = server.stats_query_backend

local luasql = require("luasql_mysql")

local flagruns = {}
local queue = {}

local mysql_schema = [[
CREATE TABLE IF NOT EXISTS  `flagruns` (
  `id`              bigint(11) NOT NULL AUTO_INCREMENT,
  `mapname`            varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `playername`            varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `time`            int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE(mapname)
);
]]

local function escape_string(s)
	s = string.gsub(s, "\\", "\\\\")
	s = string.gsub(s, "%\"", "\\\"")
	s = string.gsub(s, "%'", "\\'")
	return s
end

local function execute_statement(statement)
    local cursor, errorinfo = connection:execute(statement)
    if not cursor then
        server.log_error(string.format("Buffering stats data because of a MySQL stats failure: %s", errorinfo))
        connection:close()
        connection = nil
    end
    return cursor
end

local function install_db(connection, settings)
    for statement in string.gmatch(mysql_schema, "CREATE TABLE[^;]+") do
        local cursor, err = execute_statement(statement)
        if not cursor then error(err) end
    end
end

local function mysql_open(settings)

    connection = luasql.mysql():connect(settings.database, settings.username, settings.password, settings.hostname, settings.port)

    if not connection then
        error(string.format("couldn't connect to MySQL server at %s:%s", settings.hostname, settings.port))
    end

    servername = settings.servername

    if settings.install then
        install_db(connection, settings)
    end

    open_settings = settings

    return true
end

local function insert_flagrun(flagrun)

    if not flagruns[flagrun.mapname] then
        local insert_flagrun_sql = [[INSERT INTO flagruns (mapname, playername, time)
            VALUES ('%s', '%s', '%i')]]
        if not execute_statement(string.format(
            insert_flagrun_sql,
            escape_string(flagrun.mapname),
            escape_string(flagrun.playername),
            flagrun.time)) then return nil end

    else
        local insert_flagrun_sql = [[UPDATE flagruns SET playername = '%s', time = '%i'
            WHERE mapname = '%s']]
        if not execute_statement(string.format(
            insert_flagrun_sql,
            escape_string(flagrun.playername),
            flagrun.time,
            escape_string(flagrun.mapname))) then return nil end
    end

    flagruns[flagrun.mapname] =  {flagrun.playername, flagrun.time}

    local cursor = execute_statement("SELECT last_insert_id()")
    if not cursor then return nil end
    return cursor:fetch()
end

local function mysql_commit_flagrun(flagrun)
    
    if not connection and not mysql_open(open_settings) then
       return false
    end
    
    if not execute_statement("START TRANSACTION") then 
        return false
    end

    local flagrun_id = insert_flagrun(flagrun)

    if not flagrun_id then 
        return false
    end

    if not execute_statement("COMMIT") then
        return false
    end
    
    return true
end

local function commit_flagrun(flagrun)
    
    local function queue_current_flagrun()
        queue[#queue+1] = flagrun
    end
    
    while #queue > 0 do
        if mysql_commit_flagrun(queue[1]) then
            table.remove(queue, 1)
        else
            queue_current_flagrun()
            return
        end
    end
    
    if not mysql_commit_flagrun(flagrun) then
        queue_current_flagrun()
    end
end

local function load_flagruns(map)
    local load_flagruns_query = execute_statement(string.format("SELECT playername, time FROM flagruns WHERE mapname = '%s'", map))
    row = load_flagruns_query:fetch ({}, "a")
    return {row.playername, tonumber(row.time)}
end

if using_mysql then
    local ret = catch_error(mysql_open, {
        hostname    = server.stats_mysql_hostname,
        port        = server.stats_mysql_port,
        username    = server.stats_mysql_username,
        password    = server.stats_mysql_password,
        database    = server.stats_mysql_database,
        install     = server.stats_mysql_install == "true",
        servername  = server.stats_servername
    })
end

local scoreflag = server.event_handler("scoreflag", function(cn, team, score, timetrial)
    if gamemodeinfo.ctf and timetrial > 0 then
        local playername = server.player_name(cn)
        local mapname = server.map
        if not flagruns[server.map] or timetrial < flagruns[server.map][2] then
            commit_flagrun({ mapname = mapname, playername = playername, time = timetrial})
        end
        server.msg(string.format(server.flagrun_message, playername, timetrial/1000, flagruns[mapname][1], flagruns[mapname][2]/1000))
    end
end)

local mapchange = server.event_handler("mapchange", function(map)
    flagruns = {}
    if gamemodeinfo.ctf then
        flagruns[map] = load_flagruns(map)
    end
end)

local started = server.event_handler("started", function()
    flagruns = {}
    if gamemodeinfo.ctf then
        flagruns[server.map] = load_flagruns(server.map)
    end
end)
