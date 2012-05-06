CREATE LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION delegatr()
  RETURNS "trigger" AS '
BEGIN
        DELETE FROM teams where game_id = OLD.id;
        DELETE FROM players where game_id = OLD.id;
RETURN NEW;
END;
'
LANGUAGE plpgsql;

CREATE TRIGGER  delete_game_trigger  AFTER DELETE ON  games
FOR EACH ROW EXECUTE PROCEDURE delegatr();


CREATE OR REPLACE FUNCTION updplaytotals()
  RETURNS "trigger" AS '
  DECLARE
  curtime timestamp := now();
  BEGIN
    if exists(select 1 from playertotals where name = new.name) then
            INSERT INTO playertotals (name, first_game) VALUES(new.name, curtime);
    end if;
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
        wins = ( CASE WHEN new.win=1 and new.finished=1 THEN wins + 1 ELSE wins END ) ,
        games = games + 1,
        withdraws = ( CASE WHEN new.finished=0 THEN withdraws + 1 ELSE withdraws END ),
        timeplayed = timeplayed + new.timeplayed
        WHERE name = new.name;
RETURN NEW;
END;
'
LANGUAGE plpgsql;


CREATE TRIGGER  update_playertotals_trigger  AFTER INSERT ON  players
FOR EACH ROW EXECUTE PROCEDURE updplaytotals();

CREATE OR REPLACE FUNCTION deleplayrecords()
  RETURNS "trigger" AS '
BEGIN
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
RETURN NEW;
END;
'
LANGUAGE plpgsql;

CREATE TRIGGER  delete_player_record_trigger  AFTER DELETE ON  players
FOR EACH ROW EXECUTE PROCEDURE deleplayrecords();

