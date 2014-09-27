local env = {}
setmetatable(env, {__index = _G})

env["exec_search_paths"] = {}

local function add_exec_search_path(path)
    table.insert(env.exec_search_paths, path)
end

local function find_script(filename, function_level)

    function is_readable(filename)
        local file = io.open(filename)
        if file then
            file:close()
            return true
        else return false end
    end

    if is_readable(filename) then
        return filename
    end

    -- Absolute filename
    if string.sub(filename, 1, 1) == "/" then
        return
    end

    function get_current_dir(function_level)
        function_level = function_level or 1
        function_level = function_level + 2
        local source = debug.getinfo(function_level).source
        local source_type = string.sub(source, 1, 1)
        if source_type == "@" then
            local dirname = string.match(string.sub(source, 2), "^(.*)(/[^/]*)$")
            return dirname
        end
    end

    local current_dir = get_current_dir(function_level)
    if current_dir then
        local absolute_filename = current_dir .. "/" .. filename
        if is_readable(absolute_filename) then
            return absolute_filename
        end
    end

    local search_paths = env.exec_search_paths
    for _, path in pairs(search_paths) do
        local absolute_filename = path .. "/" .. filename
        if is_readable(absolute_filename) then
            return absolute_filename
        end
    end
end

local function exec(filename, function_level)

    function_level = function_level or 2

    local absolute_filename = find_script(filename, function_level)

    if not absolute_filename then
        error(string.format("cannot execute '%s': file not found", filename))
    end

    filename = absolute_filename

    local file_type = string.match(filename, "[^.]*$")

    local pcall_results = (function(...) return {...} end)(pcall(dofile, filename))

    if pcall_results[1] == false then
        error(pcall_results[2], 0)
    end

    table.remove(pcall_results, 1)
    return unpack(pcall_results)
end

local function pexec(filename)
    return (function(...) return {...} end)(pcall(exec, filename, 3))
end

local function exec_if_found(filename)

    filename = find_script(filename, 1)

    if not filename then
        return
    end

    exec(filename, 2)
end

local function eval_lua(str)
    local chunk, error_message = load(str)
    if not chunk then
        error(error_message)
    end
    return chunk()
end

return {
	add_exec_search_path = add_exec_search_path,
	find_script = find_script,
	exec = exec,
	pexec = pexec,
	exec_if_found = exec_if_found,
	eval_lua = eval_lua
}
