
function server.player_vars_table(key_function)
    
    local players_vars = {}
    
    local function vars(cn)
        
        local key = key_function(cn)
        
        local vars = players_vars[key]
        
        if not vars then
            vars = {}
            players_vars[key] = vars
        end
        
        return vars
    end
    
    local function clear(cn)
        
        if cn then
            players_vars[key_function(cn)] = nil
        else
            players_vars = {}
        end
        
    end
    
    return {
        vars = vars,
        clear = clear
    }
end
