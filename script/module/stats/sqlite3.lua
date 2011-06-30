require "sqlite3utils"

local db, insert_game, insert_team, insert_player, select_player_totals, select_names_by_ip, select_player_ranking

local function open(settings)

    db = sqlite3.open(settings.filename)
    if not db then return nil, db:error_message() end
    
    sqlite3utils.create_missing_tables(settings.schemafile, db, settings.filename)
    
    function server.stats_sqlite_reinstall_triggers()
        sqlite3utils.reinstallTriggers(settings.schemafile, db)
    end
    
    if settings.exclusive_locking == 1 then
        sqlite3utils.set_sqlite3_exclusive_locking(db)
    end
    
    sqlite3utils.set_sqlite3_synchronous_pragma(db, settings.synchronous)
    
    insert_game = db:prepare[[
        INSERT INTO games (
            datetime,
            duration,
            gamemode,
            mapname,
            players,
            bots,
            finished
        )
        VALUES (
            :datetime,
            :duration,
            :mode,
            :map,
            :players,
            :bots,
            :finished
       )
    ]]
    
    if not insert_game then 
        return nil, db:error_message()
    end
    
    insert_team = db:prepare[[
        INSERT INTO teams (
            game_id,
            name,
            score,
            win,
            draw
        ) 
        VALUES (
            :gameid,
            :name,
            :score,
            :win,
            :draw
       )
    ]]
    
    if not insert_team then 
        return nil, db:error_message()
    end
    
    insert_player = db:prepare[[
        INSERT INTO players (
            game_id,
            team_id,
            name,
            ipaddr,
            country,
            score,
            frags,
            deaths,
            suicides,
            teamkills,
            hits,
            shots,
            damage,
            damagewasted,
            timeplayed,
            finished,
            win,
            rank,
            botskill
        )
        VALUES(
            :game_id,
            :team_id,
            :name,
            :ipaddr,
            :country,
            :score,
            :frags,
            :deaths,
            :suicides,
            :teamkills,
            :hits,
            :shots,
            :damage,
            :damagewasted,
            :timeplayed,
            :finished,
            :win,
            :rank,
            :botskill
       )
    ]]
    
    if not insert_player then 
        return nil, db:error_message()
    end

    select_player_totals = db:prepare("SELECT * FROM playertotals WHERE name = :name")
    if not select_player_totals then return nil, db:error_message() end

    select_names_by_ip = db:prepare("SELECT DISTINCT name FROM players WHERE ipaddr = :ipaddr ORDER BY name ASC")
    if not select_names_by_ip then return nil, db:error_message() end

    select_player_ranking = db:prepare("SELECT name FROM playertotals ORDER BY frags DESC")
    if not select_player_ranking  then return nil, db:error_message() end

    server.stats_db_absolute_filename = os.getcwd() .. "/" .. settings.filename
    
    return true
end

local function commit_game(game, players, teams)
    
    db:exec("BEGIN TRANSACTION")
    
    insert_game:bind_names(game)
    insert_game:step()
    insert_game:reset()
    local game_id = db:last_insert_rowid()
    
    local team_id = {}
    
    for _, team in ipairs(teams or {}) do
        
        team.gameid = game_id
        
        insert_team:bind_names(team)
        insert_team:step()
        insert_team:reset()
        
        team_id[team.name] = db:last_insert_rowid()
    end
    
    for _, player in pairs(players) do
        
        player.game_id = game_id
        
        if player.team then
            player.team_id = team_id[player.team]
        end
        
        insert_player:bind_names(player)
        insert_player:step()
        insert_player:reset()
    end
    
    db:exec("COMMIT TRANSACTION")
    
end

local function player_totals(name)
    select_player_totals:bind_names{name = name}
    row = sqlite3utils.first_row(select_player_totals)
    return row
end

local function find_names_by_ip(ip, exclude_name)
    local names = {}
    select_names_by_ip:bind_names{ipaddr = ip}
    for row in select_names_by_ip:nrows() do
        if not exclude_name or exclude_name ~= row.name then
            names[#names + 1] = row.name
        end
    end
    return names
end

local function player_ranking(player_name)
    local names = {}
    for row in select_names_by_ip:nrows() do
        if not exclude_name or exclude_name ~= row.name then
            names[#names + 1] = row.name
        end
    end
    for rank,name in pairs(names) do
        if name == player_name then return tostring(rank) end
    end
end

return {
    open                = open, 
    commit_game         = commit_game,
    player_totals       = player_totals,
    find_names_by_ip    = find_names_by_ip,
    player_ranking      = player_ranking
}

