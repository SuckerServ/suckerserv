--[[

  A player command to lower time when mapsucks ratio is reached
  By piernov <piernov@piernov.org>

  May 25 2013 (gear4): shortened one-line "if"'s, localised and shortened local statements, optimised (?) matches for "sucks" etc., 

]]

local mapsucks = {}
local plural_time, plural_players, conjugate
local ratio, lower_time = server.mapsucks_ratio, server.mapsucks_lower_time

server.event_handler("text", function(cn, text)
  if text:lower():match("sucks") and not text:lower():match("[#!@]mapsucks") then 
    server.player_msg(cn, server.mapsucks_analysetext_message)
  end
end)

server.event_handler("connect", function(cn)
  cn_id = server.player_id(cn)
  if not mapsucks[cn_id] then return end
  mapsucks[cn_id] = cn
end)

server.event_handler("mapchange", function(map, mode)
  mapsucks = {}
end)

return function(cn)
  for _,player in pairs(server.players()) do
    if cn == player then
      cn_id = server.player_id(cn)

      if not mapsucks[cn_id] then
        mapsucks[cn_id] = cn
        mapsucks_size = table_size(mapsucks)

        server.player_msg(cn, string.format(server.mapsucks_message, (mapsucks_size - 1)))
        server.msg(string.format(server.mapsucks_announce_message, server.player_displayname(cn)))

        if mapsucks_size > (#server.players() / ratio) then
          server.changetime(lower_time*60*1000)

          plural_time = (lower_time > 1) and "s" or ""
          plural_players = (#server.players() > 1) and "s" or ""
          conjugate = (#server.players() > 1) and "" or "s"

          server.msg(string.format(server.mapsucks_timelowered_message, lower_time, plural_time, mapsucks_size, #server.players(), plural_players, conjugate))
        end
        return
      else
        server.player_msg(cn, server.mapbattle_vote_already)
        return
      end
    end
  end

  server.player_msg(cn, server.mapbattle_cant_vote_message)
  return -1
end
