--[[

  A player command to lower time when mapsucks ratio is reached
  By piernov <piernov@piernov.org>

  May 25 2013 (gear4): shortened one-line "if"'s, localised and shortened local statements, optimised (?) matches for "sucks" etc.

]]

local mapsucks = 0
local plural_time, plural_players, conjugate = "", "", ""
local ratio, lower_time = server.mapsucks_ratio, server.mapsucks_lower_time

function server.mapsucks_vote(cn)
  if not server.player_vars(cn).votesucks then
    server.player_vars(cn).votesucks = true
    mapsucks = mapsucks + 1

    server.player_msg(cn, "mapsucks", { nb = (mapsucks - 1) })
    server.msg("mapsucks_announce", { name = server.player_displayname(cn) })

    if mapsucks > (#server.players() / ratio) then
      server.changetime(lower_time*60*1000)

      plural_time = (lower_time > 1) and "s" or ""
      plural_players = (#server.players() > 1) and "s" or ""
      conjugate = (#server.players() > 1) and "" or "s"

      server.msg("mapsucks_timelowered", { time = lower_time, ts = plural_time, nb = mapsucks, total = #server.players(), ps = plural_players, vs = conjugate })
    end
    return
  else
    server.player_msg(cn, "mapbattle_vote_already")
    return
  end
end

server.event_handler("text", function(cn, text)
  if text:lower():match("sucks") and not text:lower():match("[#!@]mapsucks") then 
    server.player_msg(cn, "mapsucks_analysetext")
  end
end)

server.event_handler("connect", function(cn)
  server.player_vars(cn).votesucks = false
end)

server.event_handler("mapchange", function(map, mode)
  mapsucks = 0
end)
