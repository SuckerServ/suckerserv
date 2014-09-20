function server.name_to_cn(name)
    
    if not name then
        return
    end
    
    name = string.lower(name)
    
    local full_matches = 0
    local first_matching_cn
    
    for client in server.gclients() do
        if string.lower(client:name()) == name then
            first_matching_cn = first_matching_cn or client.cn
            full_matches = full_matches + 1 
        end
    end
    
    if full_matches == 1 then
        
        return first_matching_cn
        
    elseif full_matches == 0 then
        
        local substring_matches = {}
        
        for p in server.gclients() do
            local candidate = p:name()
            
            if string.find(string.lower(candidate), name) then
                table.insert(substring_matches, {name=candidate,cn=p.cn})
            end
        end
        
        if #substring_matches == 1 then
            return substring_matches[1].cn
        end
        
        return nil, substring_matches
        
    elseif full_matches > 1 then
        return nil, full_matches
    end
    
end

function server.disambiguate_name_list(cn, name)
    
    local message = ""
    
    name = string.lower(name)
    
    for client in server.gclients() do
        local client_name = client:name()
        if name == string.lower(client_name) then
            message = message .. string.format("%i %s\n", client.cn, client_name)
        end
    end
    
    server.player_msg(cn, message)
end

function server.similar_name_list(cn, names)
    local message = ""
    for i, player in pairs(names) do
        message = message .. string.format("%i %s\n", player.cn, player.name)
    end
    server.player_msg(cn, message)
end

function server.name_to_cn_list_matches(cn,name)
    local lcn, info = server.name_to_cn(name)
    if not lcn then
        if type(info) == "number" then  -- Multiple name matches
            server.player_msg(cn, red(string.format("There are %i players here matching that name:", info)))
            server.disambiguate_name_list(cn,name)
        elseif #info == 0 then  -- no matches
            server.player_msg(cn, red("There are no players found matching that name."))
        else    -- Similar matches
            server.player_msg(cn, red("There are no players found matching that name, but here are some similar names:"))
            server.similar_name_list(cn, info)
        end
        return nil
    else
        return lcn
    end
end

function server.is_bot(cn)
    return cn > 127
end

function server.is_teamkill(player1, player2)
    if not gamemodeinfo.teams then 
        return false
    end
    if server.player_team(player1) == server.player_team(player2) then 
        return true
    end
    return false
end

function server.valid_cn(cn)
    local cn = tonumber(cn)
    return server.player_sessionid(cn or -1) ~= -1 and not server.player_is_spy(cn or -1)
end

function server.specall()
    for player in server.gplayers() do 
        player:spec()
    end
end

function server.unspecall()
    for spectator in server.gspectators() do 
        spectator:unspec()
    end
end

function print_displaynamelist(clientnums)
    local names = {}
    for _, cn in ipairs(clientnums) do
        table.insert(names, server.player_displayname(cn))
    end
    return print_list(unpack(names))
end

