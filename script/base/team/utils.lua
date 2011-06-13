function server.teams_count()

    local size = 0
    
    for _, team in pairs(server.teams())
    do
	size = size + 1
    end
    
    return size
end

function server.team_size(team, with_specs, with_bots)

    if not team
    then
	return
    end
    
    if with_specs and with_bots
    then
	return server.teamsize(team)
    end
    
    local size = 0
    
    for p in server.gteamplayers(team, with_specs, with_bots)
    do
        size = size + 1
    end
    
    return size
end

function server.team_sizes(with_specs, with_bots)

    local teams = {}
    local greatest_teamsize = 0
    
    local function increment(t, key)
    
	local newvalue = (t[key] or 0) + 1
	
	t[key] = newvalue
	
	return newvalue
    end
    
    local function fill(player)
    
	greatest_teamsize = math.max(greatest_teamsize, increment(teams, player:team()))
    end
    
    if with_specs and with_bots
    then
	for p in server.gall()
	do
	    fill(p)
	end
	
    elseif with_specs
    then
	for p in server.gclients()
	do
	    fill(p)
	end
	
    elseif with_bots
    then
	for p in server.gallplayers()
	do
	    fill(p)
	end
    else
	for p in server.gplayers()
	do
	    fill(p)
	end
    end
    
    return teams, greatest_teamsize
end
