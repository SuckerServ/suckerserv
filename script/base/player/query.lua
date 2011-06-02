
local players_collection_methods = {

    msg = function(player, text)
        player:msg(text)
    end,
    
    each = function(player, callback)
        callback(player)
    end,
    
    method = function(...)
    
        local player = arg[1]
        local player_function_name = arg[2]
        
        table.remove(arg, 2)
        
        if type(player_function_name) == "function" then
            player_function_name(unpack(arg))
        else
            player[player_function_name](unpack(arg))
        end
    end
}

local function players_collection(collection)
    local object = {}
    local players = collection
    setmetatable(object, {__index = function(_, key)
        local method = players_collection_methods[key]
        if not method then return nil end
        return function(...)
            for _, player in ipairs(players) do
                method(player, unpack(arg))
            end
            return object
        end
    end})
    return object
end

local players_select_predicates = {
    team = function(cn, teamname) return server.player_team(cn) == teamname end,
    bots = function(cn) return server.player_isbot(cn) end,
    spectators = function(cn) return server.player_status_code(cn) == server.SPECTATOR end,
    admins = function(cn) return server.player_priv_code(cn) == server.PRIV_ADMIN end,
    masters = function(cn) return server.player_priv_code(cn) == server.PRIV_MASTER end,
    all = function() return true end
}

local function players_selector()
    local object = {}
    local predicates = {}
    setmetatable(object, {
        __index = function(_, key)
            local method = players_select_predicates[key]
            
            if not method then
                
                local selected_players = {}
                
                for _, cn in ipairs(server.clients()) do
                    for _, predicate in ipairs(predicates) do
                        if predicate(cn) then
                            selected_players[#selected_players + 1] = server.new_player_object(cn)
                        end
                    end
                end
                
                return players_collection(selected_players)[key]
            end

            return function(...)
                predicates[#predicates + 1] = function(cn) return method(cn, unpack(arg)) end
                return object
            end
        end
    })
    return object
end

query = {}
setmetatable(query, {__index = function(_, key) return players_selector()[key] end})

function query_add_collection_method(name, func)
    player_collections_method[name] = func
end

function query_add_select_predicate(name, func)
    player_select_predicates[name] = func
end
