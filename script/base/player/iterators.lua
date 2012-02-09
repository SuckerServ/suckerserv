
local function client_iterator_generator(list)
    
    local function next_client(client_list, client)
       
        local index = nil
        if client then
            index = client.index
        end
        
        local key, cn = next(client_list, index)
        if not key then 
            return nil
        end
        
        local client_wrapper = server.Client(cn)
        client_wrapper.index = key
        
        return client_wrapper
    end
    
    return next_client, list, nil
end

function server.gspectators()
    return client_iterator_generator(server.spectators())
end

function server.gplayers()
    return client_iterator_generator(server.players())
end

function server.gclients()
    
    local all = server.players()
    
    for _, cn in pairs(server.spectators()) do
        table.insert(all, cn)
    end
    
    return client_iterator_generator(all)
end

function server.gbots()
    return client_iterator_generator(server.bots())
end

function server.gall()
    return client_iterator_generator(server.clients())
end

function server.gallplayers()
    
    local all = server.players()
    
    for _, cn in pairs(server.bots()) do
        table.insert(all, cn)
    end
    
    return client_iterator_generator(all)
end

function server.gteamplayers(team, with_specs, with_bots)
    
    local all = server.team_players(team)
    
    if with_specs then
        for _, cn in pairs(server.spectators()) do
            if server.player_team(cn) == team then
                table.insert(all, cn)
            end
        end
    end
    
    if with_bots then
        for _, cn in pairs(server.bots()) do
            if server.player_team(cn) == team then
                table.insert(all, cn)
            end
        end
    end
    
    return client_iterator_generator(all)
end

