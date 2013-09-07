local Json = require "Json"

local function includeHeaders(response, headers)
    for name, value in pairs(headers or {}) do
        response:header(name, value)
    end
end

local function send_html(request, html)
    
    local response = http_server.response(request, 200)
    
    response:header("Content-Type", "text/html")
    response:set_content_length(#html)
    response:send_header()
    
    if #html > 0 then
        response:send_body(html)
    end
end

local function send_json(request, tab)
    
    local output = Json.Encode(tab)
    
    local response = http_server.response(request, 200)
    response:header("Content-Type", "application/json")
    response:set_content_length(#output)
    response:send_header()
    
    if #output > 0 then
        response:send_body(output)
    end
end

local function send_file(request, filename, contentType)
    
    local file, errorMessage = io.open(filename, "r")
    if not file then error(string.format("couldn't open file: %s", errorMessage)) end
    local content = file:read("*a")
    file:close()
    
    local response = http_server.response(request, 200)
    response:header("Content-Type", contentType)
    response:set_content_length(#content)
    response:send_header()
    if #content > 0 then
        response:send_body(content)
    end
end

local function send_error(request, code, errorMessage, headers)
    local response = http_server.response(request, code)
    response:header("Content-Type", "text/plain")
    response:set_content_length(#errorMessage)
    includeHeaders(response, headers)
    response:send_header()
    
    if #errorMessage > 0 then
        response:send_body(errorMessage)
    end
end

local function redirect(request, absoluteUrl, headers)
    local response = http_server.response(request, 303)
    response:set_content_length(0)
    response:header("Location", absoluteUrl)
    includeHeaders(response, headers)
    response:send_header()
end

http_response = {
    send_file = send_file,
    send_json = send_json,
    send_html = send_html,
    send_error = send_error,
    redirect = redirect
}

return http_response
