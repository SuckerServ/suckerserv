--[[

  A player command to lower time when mapsucks ratio is reached
  By piernov <piernov@piernov.org>

  May 25 2013 (gear4): shortened one-line "if"'s, localised and shortened local statements, optimised (?) matches for "sucks" etc.

]]

module("server.mapsucks", package.seeall)

local mapsucks = 0
local plural_time, plural_players, conjugate = "", "", ""
local ratio, lower_time = server.mapsucks_ratio, server.mapsucks_lower_time

function vote(cn)
  if not server.player_vars(cn).votesucks then
    server.player_vars(cn).votesucks = true
    mapsucks = mapsucks + 1

    server.player_msg(cn, string.format(server.mapsucks_message, (mapsucks - 1)))
    server.msg(string.format(server.mapsucks_announce_message, server.player_displayname(cn)))

    if mapsucks > (#server.players() / ratio) then
      server.changetime(lower_time*60*1000)

      plural_time = (lower_time > 1) and "s" or ""
      plural_players = (#server.players() > 1) and "s" or ""
      conjugate = (#server.players() > 1) and "" or "s"

      server.msg(string.format(server.mapsucks_timelowered_message, lower_time, plural_time, mapsucks, #server.players(), plural_players, conjugate))
    end
    return
  else
    server.player_msg(cn, server.mapbattle_vote_already)
    return
  end
end

server.event_handler("text", function(cn, text)
  if text:lower():match("sucks") and not text:lower():match("[#!@]mapsucks") then 
    server.player_msg(cn, server.mapsucks_analysetext_message)
  end
end)

server.event_handler("connect", function(cn)
  server.player_vars(cn).votesucks = false
end)

server.event_handler("mapchange", function(map, mode)
  mapsucks = 0
end)
