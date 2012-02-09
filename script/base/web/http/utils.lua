
local function same_host(host, url)
    local found = string.match(url, string.format("://%s", host))
    return found ~= nil
end

local function return_url(request, url, failUrl)
    if not same_host(request:host(), url or "") then
        return failUrl or ("http://" .. request:host())
    else
        return url
    end
end

http_utils = {
    same_host = same_host,
    return_url = return_url
}

return http_utils
