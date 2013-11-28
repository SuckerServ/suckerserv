
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

    if server.player_ranking then 
        player_ranking = server.player_ranking(server.player_name(player)) 
    end

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

local function periodly_stats(query_backend, period, sendto, player)

    ranking = query_backend.player_ranking_by_period(server.player_name(player), period*60*60*24)
    row = query_backend.player_stats_by_period(server.player_name(player), period*60*60*24)
    if not row then
        server.player_msg(sendto, "No stats found.")
        return
    end

    if period == 1 then period = "Daily" elseif period == 7 then period = "Weekly" elseif period == 30 then period = "Monthly" end
    
    if sendto ~= player then
        server.player_msg(sendto, string.format("%s game stats for %s:", period, green(server.player_name(player))))
    end
    
    local kpd = round(row.frags / row.deaths, 2)
    local acc = round((row.hits / row.shots)*100)

    if not ranking then
        ranking = "Unknown" 
    end
    
    server.player_msg(sendto, string.format(server.stats_total_player_message,
        row.games,
        row.frags,
        row.deaths,
        kpd,
        acc,
        row.wins,
        ranking))
end

local function initialize(query_backend)
    
    if not query_backend or not validate(query_backend, {player_totals = "function"}) then
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

    stats_sub_command["daily"] = function(cn, player)
    
        player = tonumber(player)
        
        if player and not server.valid_cn(player) then
            player = nil
        end
       
        player = player or cn

        return periodly_stats(query_backend, 1, cn, player)
    end
    
    stats_sub_command["weekly"] = function(cn, player)
    
        player = tonumber(player)
        
        if player and not server.valid_cn(player) then
            player = nil
        end
       
        player = player or cn

        return periodly_stats(query_backend, 7, cn, player)
    end
    
    stats_sub_command["monthly"] = function(cn, player)
    
        player = tonumber(player)
        
        if player and not server.valid_cn(player) then
            player = nil
        end
       
        player = player or cn

        return periodly_stats(query_backend, 30, cn, player)
    end
end

return {initialize = initialize}

