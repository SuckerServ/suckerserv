require "luasql_mysql"

local servername
local connection
local open_settings
local queue = {}

local function readWholeFile(filename)
    local file, err = io.open(filename)
    if not file then error(err) end
    return file:read("*a")
end

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

    local schema = readWholeFile(settings.schema)

    for statement in string.gmatch(schema, "CREATE TABLE[^;]+") do
        local cursor, err = execute_statement(statement)
        if not cursor then error(err) end
    end
    
    local triggers = readWholeFile(settings.triggers)
    
    for statement in string.gmatch(triggers, "CREATE TRIGGER[^~]+") do
        local cursor, err = execute_statement(statement)
        if not cursor then error(err) end
    end
    
end

local function open(settings)
    
    connection = luasql.mysql():connect(settings.database, settings.username, settings.password, settings.hostname, settings.port)
    
    if not connection then
        server.log_error(string.format("couldn't connect to MySQL server at %s:%s", settings.hostname, settings.port))
        return nil
    end
    
    servername = settings.servername
    
    if settings.install then
        install_db(connection, settings)
    end

    open_settings = settings
    
    return true
end

local function insert_game(game)

    local insert_game_sql = [[INSERT INTO games (servername, datetime, duration, gamemode, mapname, players, bots, finished) 
        VALUES ('%s', from_unixtime(%i), %i, '%s', '%s', %i, %i, %i)]]
    
    if not execute_statement(string.format(
        insert_game_sql,
        escape_string(servername),
        game.datetime,
        game.duration,
        escape_string(game.mode),
        escape_string(game.map),
        game.players,
        game.bots,
        game.finished and 1 or 0)) then return nil end
    
    local cursor = execute_statement("SELECT last_insert_id()")
    if not cursor then return nil end
    return cursor:fetch()
end

local function insert_team(game_id, team)
    
    local insert_team_sql = "INSERT INTO teams (game_id, name, score, win, draw) VALUES (%i, '%s', %i, %i, %i)"
    
    if not execute_statement(string.format(
        insert_team_sql,
        game_id,
        escape_string(team.name),
        team.score,
        team.win and 1 or 0,
        team.draw and 1 or 0)) then return nil end
    
    local cursor = execute_statement("SELECT last_insert_id()")
    if not cursor then return nil end
    return cursor:fetch()
end

local function insert_player(game_id, player)
    
    local insert_player_sql = [[INSERT INTO players 
        (game_id, team_id, name, ipaddr, country, score, frags, deaths, suicides, teamkills, hits, misses, shots, damage, damagewasted, timeplayed, finished, win, rank, botskill)
        VALUES(%i, %i, '%s', '%s', '%s', %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i)]]
    
    local cursor = execute_statement(string.format(
        insert_player_sql,
        game_id, 
        player.team_id or 0, 
        escape_string(player.name),
        player.ipaddr,
        player.country,
        player.score,
        player.frags,
        player.deaths,
        player.suicides,
        player.teamkills,
        player.hits,
        player.misses,
        player.shots,
        player.damage,
        player.damagewasted,
        player.timeplayed,
        player.finished and 1 or 0,
        player.win and 1 or 0,
        player.rank,
        player.botskill))
    
    return cursor ~= nil
end

local function mysql_commit_game(game, players, teams)
    
    if not connection and not open(open_settings) then
       return false
    end
    
    if not execute_statement("START TRANSACTION") then 
        return false
    end

    local game_id = insert_game(game)

    if not game_id then 
        return false
    end

    for i, team in ipairs(teams or {}) do
        local team_id = insert_team(game_id, team)
        
        if not team_id then
            return false
        end
        
        for id, player in pairs(players) do
            if player.team == team_name then player.team_id = team_id end
        end
    end

    for id, player in pairs(players) do
        if not insert_player(game_id, player) then 
            return false
        end
    end

    if not execute_statement("COMMIT") then
        return false
    end
    
    return true
end

local function commit_game(game, players, teams)
    
    local function queue_current_game()
        queue[#queue+1] = {game, players, teams}
    end
    
    while #queue > 0 do
        if mysql_commit_game(queue[1][1], queue[1][2], queue[1][3]) then
            table.remove(queue, 1)
        else
            queue_current_game()
            return
        end
    end
    
    if not mysql_commit_game(game, players, teams) then
        queue_current_game()
    end
end

local function player_totals(name)
	player_totals = execute_statement(string.format("SELECT * FROM playertotals WHERE name = '%s'", name))
	row = player_totals:fetch ({}, "a")
    return row
end

local function find_names_by_ip(ip, exclude_name)
    find_names_by_ip = execute_statement(string.format("SELECT DISTINCT name FROM players WHERE ipaddr = '%s' ORDER BY name ASC", ip))
    local names = {}
	row = find_names_by_ip:fetch ({}, "a")
    while row do
        if not exclude_name or exclude_name ~= row.name then
            names[#names + 1] = row.name
        end
		row = find_names_by_ip:fetch (row, "a")
    end
    return names
end

local function player_ranking(player_name)
    local sql = [[SELECT rank
                    FROM (SELECT count(1)+1 AS rank
                            FROM playertotals 
                            WHERE playertotals.frags > (SELECT frags FROM playertotals WHERE name = '%s'))T
                    WHERE (SELECT frags
                            FROM playertotals
                            WHERE name = '%s') > 0]]
    local player_ranking = execute_statement(string.format(sql, escape_string(player_name), escape_string(player_name)))
    return player_ranking:fetch()
end

local function player_ranking_by_period(player_name, period)
    local sql = [[SELECT rank
                    FROM (SELECT COUNT(1)+1 rank
                        FROM (SELECT SUM(frags) as frags
                            FROM players 
                            INNER JOIN games ON players.game_id=games.id 
                            WHERE UNIX_TIMESTAMP(games.datetime) BETWEEN UNIX_TIMESTAMP()-%d AND UNIX_TIMESTAMP() 
                            GROUP BY name )T
 
                        WHERE frags>(SELECT SUM(frags) 
                            FROM players 
                            INNER JOIN games ON players.game_id=games.id 
                            WHERE UNIX_TIMESTAMP(games.datetime) BETWEEN UNIX_TIMESTAMP()-%d AND UNIX_TIMESTAMP() AND name = '%s'
                            GROUP BY name))A

                    WHERE (SELECT SUM(frags) 
                        FROM players 
                        INNER JOIN games ON players.game_id=games.id 
                        WHERE UNIX_TIMESTAMP(games.datetime) BETWEEN UNIX_TIMESTAMP()-%d AND UNIX_TIMESTAMP() AND name = '%s'
                        GROUP BY name)>0]]

    local player_ranking = execute_statement(string.format(sql, period, period, escape_string(player_name), period, escape_string(player_name)))
    return player_ranking:fetch()
end

local function player_stats_by_period(name, period)
  
    sql = [[SELECT name, sum(frags) AS frags, sum(deaths) AS deaths, sum(teamkills) AS teamkills, sum(suicides) AS suicides, sum(hits) AS hits, sum(shots) AS shots, sum(win) AS wins, sum(timeplayed) AS timeplayed, count(*) AS games, ipaddr
            FROM players, games
            WHERE games.id = players.game_id AND UNIX_TIMESTAMP(games.datetime) > UNIX_TIMESTAMP() - %d AND name = '%s'
            GROUP BY name
            ORDER BY sum(frags) DESC ]]

	player_stats_by_period = execute_statement(string.format(sql, period, escape_string(name)))
	row = player_stats_by_period:fetch ({}, "a")
    return row
end

return {open = open, commit_game = commit_game, player_totals = player_totals, find_names_by_ip = find_names_by_ip, player_ranking = player_ranking, player_stats_by_period = player_stats_by_period, player_ranking_by_period = player_ranking_by_period}
