require "http_server"
require "Json"

local function get_team_info(team)
    local result = {}
    result.score = server.team_score(team);
    return result
end

local function teams()
    local result = {}
    for _, teamname in ipairs(server.teams()) do
        result[teamname] = get_team_info(teamname)
    end
    return result;
end

http_server_root["teams"] = http_server.resource({
    get = function(request)
        http_response.send_json(request, teams())
    end
})

