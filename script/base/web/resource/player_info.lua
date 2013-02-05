require "http_server"
require "Json"

local function player(p)

    local output = {}
    
    output.cn = p.cn
    output.sessionid = p.sessionid
    output.id = p:id()
    output.name = p:name()
    output.team = p:team()
    output.ip = p:ip()
    output.ping = p:ping()
    output.lag = p:lag()
    output.priv = p:priv()
    output.status = p:status()
    output.connection_time = p:connection_time()
    output.timeplayed = p:timeplayed()
    output.frags = p:frags()
    output.deaths = p:deaths()
    output.suicides = p:suicides()
    output.teamkills = p:teamkills()
    output.misses = p:misses()
    output.shots = p:shots()
    output.accuracy = p:accuracy2()
    
    if(output.status == "spectator") then
        output.team = nil
    end
    
    return output
end

local function players()
    local output = {}
    for p in server.gplayers() do
        table.insert(output, player(p))
    end
    return output
end

local function clients()

    local output = {}
    
    for _, cn in ipairs(server.clients()) do
        table.insert(output, player(server.Client(cn)))
    end
    
    return output
end

http_server_root["players"] = http_server.resource({
    get = function(request)
        http_response.send_json(request, players())
    end
})

http_server_root["clients"] = http_server.resource({
    resolve = function(cn)
        if server.player_sessionid(cn) == -1 then return nil end
        return http_server.resource({
            get = function(request)
                http_response.send_json(request, player(server.Client(cn)))
            end
        })
    end,
    
    get = function(request)
        http_response.send_json(request, clients())
    end
})
