CREATE TRIGGER `delete_game_trigger` AFTER DELETE ON `games`
FOR EACH ROW BEGIN
    DELETE FROM teams where game_id = OLD.id;
    DELETE FROM players where game_id = OLD.id;
END ~

CREATE TRIGGER `update_playertotals_trigger` AFTER INSERT ON `players`
FOR EACH ROW BEGIN
    INSERT IGNORE INTO playertotals (name, first_game) VALUES(new.name, now());
    UPDATE playertotals SET 
        ipaddr = new.ipaddr,
        country = new.country,
        first_game = COALESCE(first_game, now()),
        last_game = now(), 
        frags = frags + new.frags,
        max_frags = GREATEST(max_frags, new.frags),
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
END ~

CREATE TRIGGER `delete_player_record_trigger` AFTER DELETE ON `players`
FOR EACH ROW BEGIN
    UPDATE playertotals SET
        frags = frags - old.frags,
        max_frags = (select max(frags) from players where name = old.name),
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
END ~
