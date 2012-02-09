
CREATE TABLE IF NOT EXISTS games (
    id                  INTEGER PRIMARY KEY,
    datetime            INTEGER DEFAULT 0,
    gamemode            TEXT DEFAULT "",
    mapname             TEXT DEFAULT "",
    duration            INTEGER DEFAULT 0,
    finished            BOOLEAN DEFAULT 0,
    players             INTEGER DEFAULT 0,
    bots                INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS teams (
    id                  INTEGER PRIMARY KEY,
    game_id             INTEGER REFERENCES games(id),
    name                TEXT,
    score               INTEGER DEFAULT 0,
    win                 BOOLEAN DEFAULT 0,
    draw                BOOLEAN DEFAULT 0
);

CREATE TABLE IF NOT EXISTS players (
    id                  INTEGER PRIMARY KEY,
    game_id             INTEGER REFERENCES games(id),
    team_id             INTEGER REFERENCES teams(id) DEFAULT 0,
    name                TEXT,
    ipaddr              TEXT,
    country             TEXT,
    score               INTEGER DEFAULT 0,
    frags               INTEGER DEFAULT 0,
    deaths              INTEGER DEFAULT 0,
    suicides            INTEGER DEFAULT 0,
    teamkills           INTEGER DEFAULT 0,
    hits                INTEGER DEFAULT 0,
    misses              INTEGER DEFAULT 0,
    shots               INTEGER DEFAULT 0,
    damage              INTEGER DEFAULT 0,
    damagewasted        INTEGER DEFAULT 0,
    timeplayed          INTEGER DEFAULT 0,
    finished            BOOLEAN DEFAULT 0,
    win                 BOOLEAN DEFAULT 0,
    rank                INTEGER DEFAULT 0,
    botskill            INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS playertotals (
    id                  INTEGER PRIMARY KEY,
    name                TEXT UNIQUE,
    ipaddr              TEXT,
    country             TEXT,
    first_game          TEXT,
    last_game           TEXT,
    frags               INTEGER DEFAULT 0,
    max_frags           INTEGER DEFAULT 0,
    deaths              INTEGER DEFAULT 0,
    suicides            INTEGER DEFAULT 0,
    teamkills           INTEGER DEFAULT 0,
    hits                INTEGER DEFAULT 0,
    misses              INTEGER DEFAULT 0,
    shots               INTEGER DEFAULT 0,
    damage              INTEGER DEFAULT 0,
    damagewasted        INTEGER DEFAULT 0,
    wins                INTEGER DEFAULT 0,
    games               INTEGER DEFAULT 0,
    withdraws           INTEGER DEFAULT 0,
    timeplayed          INTEGER DEFAULT 0
);

CREATE INDEX IF NOT EXISTS "player_name" ON players (name);
CREATE INDEX IF NOT EXISTS "player_ipaddr" ON players (ipaddr);
CREATE INDEX IF NOT EXISTS "game_id" ON players (game_id ASC);
CREATE INDEX IF NOT EXISTS "playertotals_by_name" ON playertotals (name ASC);

CREATE TRIGGER IF NOT EXISTS delete_game_trigger AFTER DELETE ON games
BEGIN
    DELETE FROM teams where game_id = old.id;
    DELETE FROM players where game_id = old.id;
END;

CREATE TRIGGER IF NOT EXISTS delete_player_trigger AFTER DELETE ON players
BEGIN
    UPDATE playertotals SET
        frags = frags - old.frags,
        max_frags = (select max(frags) from players where id = old.id),
        deaths = deaths - old.deaths,
        suicides = suicides - old.suicides,
        teamkills = teamkills - old.teamkills,
        hits = hits - old.hits,
        misses = misses - old.misses,
        shots = shots - old.shots,
        wins = wins - old.win,
        games = games - 1,
        withdraws = withdraws - (old.finished = 0)
        WHERE name = old.name;
END;

CREATE TRIGGER IF NOT EXISTS update_playertotals_trigger AFTER INSERT ON players
BEGIN
    INSERT OR IGNORE INTO playertotals (name,first_game) VALUES (new.name,strftime('%s','now'));
    UPDATE playertotals SET 
        ipaddr = new.ipaddr,
        country = new.country,
        last_game = strftime('%s','now'), 
        frags = frags + new.frags,
        max_frags = max(max_frags, new.frags),
        deaths = deaths + new.deaths,
        suicides = suicides + new.suicides,
        teamkills = teamkills + new.teamkills,
        hits = hits + new.hits,
        misses = misses + new.misses,
        shots = shots + new.shots,
        damage = damage + new.damage,
        damagewasted = damagewasted + new.damagewasted,
        wins = wins + (new.win and new.finished),
        games = games + 1,
        withdraws = withdraws + (new.finished = 0),
        timeplayed = timeplayed + new.timeplayed
        WHERE name = new.name;
END;

