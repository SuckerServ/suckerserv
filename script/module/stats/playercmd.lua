
local function total_stats(query_backend, sendto, player)
    
    row = query_backend.player_totals(server.player_name(player))
    if not row then
        server.player_msg(sendto, "No stats found.")
        return
    end
    
    if sendto ~= player then
        server.player_msg(sendto, string.format("Total game stats for %s:", green(server.player_name(player))))
    end
    
    local kpd = round(row.frags / row.deaths, 2)
    local acc = round((row.hits / row.shots)*100)

    if not player_ranking then
        player_ranking = "Unknown" 
    end
    
    server.player_msg(sendto, string.format(server.stats_total_player_message,
        row.games,
        row.frags,
        row.deaths,
        kpd,
        acc,
        row.wins,
        player_ranking))
end

local function initialize(query_backend)
    
    if not query_backend or not table.has_fields(query_backend, "player_totals") then
        server.log_error("Error in stats player command initialization: not given a usable query backend")
        return
    end
    
    stats_sub_command["total"] = function(cn, player)
    
        player = tonumber(player)
        
        if player and not server.valid_cn(player) then
            player = nil
        end
       
        player = player or cn

        return total_stats(query_backend, cn, player)
    end
end

return {initialize = initialize}

