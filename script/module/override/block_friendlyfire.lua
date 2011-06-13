
server.event_handler("damage", function(target, actor, damage, gun)
    if gamemodeinfo.teams and target ~= actor and server.player_team(actor) == server.player_team(target) then
        return -1
    end
end)
