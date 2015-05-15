local crypto = require "crypto"

local users = {}
local sessions = {}

local function createUsersTable()
    for _, entry in ipairs(server.web_admins) do
        local fields = {string.match(entry, "(%w+) (%w+) (%w+)")}
        if #fields >= 3 then
            local name = fields[1]
            local password_hash = fields[2]
            local salt = fields[3]
            users[name] = {
                password_hash = password_hash,
                salt = salt
            }
        end
    end
end

createUsersTable()

local function generateSessionKey()
    return crypto.sauerecc.generate_key_pair()
end

local function isLoggedIn(request)
    local params = http_request.parse_cookie(request:header("cookie"))
    local sessionId = params.id
    if not sessionId then return false end
    local sessionInfo = sessions[sessionId]
    if not sessionInfo then return false end
    if sessionInfo.ip ~= request:client_ip() then return false end
    return true
end

local function requireLogin(request)
    if request:client_ip() ~= "127.0.0.1" and not isLoggedIn(request) then
        http_response.redirect(request, "http://" .. request:host() .. "/login?return=" .. http_request.absolute_uri(request))
        return true
    end
    return false
end

local function requireBackendLogin(request)
    if request:client_ip() ~= "127.0.0.1" and not isLoggedIn(request) then
        http_response.send_error(request, 401, "You must be logged in to access this resource.\n",{["WWW-Authenticate"] = "HopmodWebLogin"})
        return true
    end
    return false
end

local function getSessionUsername(request)
    local params = http_request.parse_cookie(request:header("cookie"))
    local sessionId = params.id
    if not sessionId then return "root" end
    local sessionInfo = sessions[sessionId]
    if not sessionInfo then return "root" end
    return sessionInfo.username
end

web_admin = {
    require_login = requireLogin,
    require_backend_login = requireBackendLogin,
    get_session_username = getSessionUsername
}

local function tryLogin(username, password)
    local user = users[username]
    if not user then return false end
    return crypto.tigersum(user.salt .. password) == user.password_hash
end

local function getLoginFormHtml(attributes)
    
    local failed = ""
    local username = ""
    
    if attributes.failed then
        failed = "<p class=\"form-error\">Invalid username or password.</p>"
    end
    
    local html = [[
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
    <head>
        <title>SuckerServ Authserver Admin Login</title>
        <link rel="stylesheet" type="text/css" href="/static/presentation/screen.css" />
        <script type="text/javascript">
            window.onload=function(){document.forms.login.username.focus();}
        </script>
    </head>
    <body>
        <div id="loginpage">
        <h1>%s - <span id="app-title">Cube 2 Authserver Control Panel</span></h1>
        <form method="post" id="login-form" name="login" action="?return=%s">
            %s
            <p><label for="username">Username</label><input type="text" name="username" id="username" value="%s" /></p>
            <p><label for="password">Password</label><input type="password" name="password" id="password" /></p>
            <p><span class="label-space">&nbsp;</span><input type="submit" value="Login" /></p>
        </form>
        </div>
    </body>
</html>
    ]]
    
    return string.format(html, server.servername, attributes.returnUrl or "", failed, attributes.username or "")
end

http_server_root["login"] = http_server.resource({

    get = function(request)
        local uri_query_params = http_request.parse_query_string(request:uri_query() or "")
        http_response.send_html(request, getLoginFormHtml({failed=false, returnUrl = uri_query_params["return"]}))
    end,
    
    post = function(request)
        
        local referer = request:header("referer")
        
        local ctype = request:header("content-type")
        
        if ctype ~= "application/x-www-form-urlencoded" then
            http_response.send_error(request, 400, "expected application/x-www-form-urlencoded type content")
            return
        end
        
        request:async_read_content(function(content)
        
            local uri_query_params = http_request.parse_query_string(request:uri_query() or "")
            local params = http_request.parse_query_string(content)
            
            local missing_fields = not params.username or not params.password
            
            if missing_fields or tryLogin(params.username, params.password) == false then
                server.log(string.format("Failed web admin login by %s@%s", params.username, request:client_ip()))
                http_response.send_html(request, 
                    getLoginFormHtml({failed=true, 
                        returnUrl = uri_query_params["return"],
                        username = params.username
                    }))
                return
            end
            
            server.log(string.format("Successful web admin login by %s@%s", params.username, request:client_ip()))
            
            local sessionId = generateSessionKey()
            
            sessions[sessionId] = {
                ip = request:client_ip(),
                username = params.username
            }

            local cookie = http_request.build_query_string({id = sessionId})
            
            http_response.redirect(request, http_utils.return_url(request, uri_query_params["return"]), {["Set-Cookie"] = cookie})
        end)
    end
})

http_server_root["logout"] = http_server.resource({
    get = function(request)
        
    end
})
