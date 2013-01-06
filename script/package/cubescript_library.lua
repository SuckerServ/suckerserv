--[[
  Copyright (c) 2010 Graham Daws <graham.daws@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
]]

local env = {}
setmetatable(env, {__index = _G})

-- Private utility functions

local function parse_array(input_string, environment, silent_error)
    
    environment = environment or env
    
    input_string = (input_string or "") .. "\n"
    
    local parent = {}
    local root = {}
    local node = nil
    local start_size = 0
    
    local processing = {
        
        push_command = function()
            if node then
                local new_node = {}
                node[#node + 1] = new_node
                parent[#parent + 1] = node
                node = new_node
            else
                node = root
            end
            start_size = #node
        end,
        
        push_argument_symbol = function(id)
            if #node == start_size then
                node[#node + 1] = id
            else
                node[#node + 1] = env[id]
            end
        end,
        
        push_argument = function(value)
            node[#node + 1] = value
        end,
        
        pop_string = function()
            local value = node[#node]
            node[#node] = nil
            return value
        end,
        
        call = function(index)
            
            local function_calls = {
                ["@"] = environment["@"],
                ["$"] = environment["$"]
            }
            
            local function_call = function_calls[node[1]]
            
            if function_call then
                table.remove(node, 1)
                local p = parent[#parent]
                p[#p] = function_call(table.unpack(node))
            end
            
            node = parent[#parent]
            parent[#parent] = nil
        end
    }
    
    local error_message = cubescript.eval(input_string, cubescript.command_stack(processing))
    
    if error_message then
        if not silent_error then
            error(error_message .. "\n" .. debug.traceback())
        else
            return nil, error_message
        end
    end
    
    return root
end

-- Values

env["false"] = function() return false end
env["true"] = function() return true end
env["nil"] = function() return nil end

env["_false"] = env["false"]
env["_true"] = env["true"]
env["_nil"] = env["nil"]

env["$"] = function(property)
    if type(property) == "function" then
        return property()
    else
        return property
    end
end

env["get"] = env["$"]

-- Comparison

env["="] = function(a, b) return a == b end
env["!="] = function(a, b) return a ~= b end
env["<"] = function(a, b) return a < b end
env["<="] = function(a, b) return a <= b end
env[">"] = function(a, b) return a > b end
env[">="] = function(a, b) return a >= b end

env["equal"] = env["="]
env["not_equal"] = env["!="]
env["less_than"] = env["<"]
env["less_than_or_equal"] = env["<="]
env["greater_than"] = env[">"]
env["greater_than_or_equal"] = env[">="]

-- Boolean logic

env["!"] = function(a) return not a end
env["||"] = function(a, b) return a or b end
env["&&"] = function(a, b) return a and b end

env["_not"] = env["!"]
env["_or"] = env["||"]
env["_and"] = env["&&"]

-- Arithmetic

local function generic_arithmetic(binary_function)
    return function(...)
        local arg = {...}
        local result = binary_function(arg[1], arg[2])
        for i = 3, #arg do
            result = binary_function(result, arg[i])
        end
        return result
    end
end

env["+"] = generic_arithmetic(function(x, y) return x + (y or 0) end)
env["-"] = generic_arithmetic(function(x, y) return x - (y or 2*x) end)
env["*"] = generic_arithmetic(function(x, y) return x * (y or 1) end)
env["div"] = generic_arithmetic(function(x, y) return x / (y or 1) end)
env["mod"] = function(x, y) return x % y end
env["max"] = math.max
env["min"] = math.min
env["rnd"] = function(n) return math.random(0, n) end

env["add"] = env["+"]
env["sub"] = env["-"]
env["mul"] = env["*"]

-- Array (some of these functions also work on strings)

env["array"] = function(...)
    return {...}
end

env["parse_array"] = parse_array

env["len"] = function(object) return #object end
env["listlen"] = env["len"]

env["at"] = function(object, index)
    
    if type(object) == "string" then
        return parse_array(object)[index]
    end
    
    return object[index]
end

-- String

local function implode(pieces, glue)
    glue = glue or ""
    local output = ""
    for i = 1, #pieces do
        if i > 1 then
            output = output .. glue
        end
        output = output .. tostring(pieces[i])
    end
    return output
end

env["@"] = function(...)
    return implode({...})
end

env["substr"] = function(s, start, length)
    if length == 0 then return "" end
    return string.sub(s, start, start + length - 1)
end

env["strstr"] = function(s, substring)
   local position = string.find(s, substring, 1, true)
   if not position then return -1 end
   return position - 1
end

env["strreplace"] = function(s, find, replacement)

    local output = ""
    
    local find_length = #find
    
    local start_position = 1
    local insert_position = string.find(s, find, 1, true)

    while insert_position do
        
        output = output .. 
                 string.sub(s, start_position, insert_position - 1) ..
                 replacement
        
        start_position = insert_position + find_length
        insert_position = string.find(s, find, start_position, true)
    end
    
    output = output .. string.sub(s, start_position)
    
    return output
end

env["format"] = function(s, ...)
    local arg = {...}
    local output = string.gsub(s, "%%[%w_]+", function(id)
        id = string.sub(id, 2)
        local index = tonumber(id)
        if index and arg[index] then
            return arg[index]
        else
            if arg[1] and type(arg[1]) == "table" then
                return arg[1][id] or ""
            else
                return ""
            end
        end
    end)
    return output
end

env["strlen"] = string.len
env["strcmp"] = env["="]
env["strcat"] = env["@"]
env["concatword"] = env["@"]
env["concat"] = function(...) return implode({...}, " ") end
env["implode"] = implode

env["def"] = function(name, value)
   _G[name] = value
   return value
end

env["current_location"] = function() 
    return "stdin"
end

local generate_expression_code, generate_code

local function make_function(parameters, body)
    
    if type(body) ~= "string" or (parameter and not body) then
        return function() return body or parameter end
    end
    
    local ast = {
        {arguments = {
                {value = "func", is_variable = true}, 
                {value = parameters},
                {value = body}
            }
        }
    }
    
    local lua_code = generate_code(ast)
    
    local create_lua_function, error_message = load("return " .. lua_code,
        "function defined at " .. env.current_location())
    if not create_lua_function then error(error_message) end
    
    local func = create_lua_function()
    setfenv(func, env)
    return func
end

env["func"] = function(parameters, body)
    return make_function(parameters, body)
end

env["to_lua"] = function(parameters, body)

    local ast = {
        {arguments = {
                {value = "func", is_variable = true}, 
                {value = parameters},
                {value = body}
            }
        }
    }
    
    local lua_code = generate_code(ast)
    return lua_code
end

env["call"] = function(func, ...)
   return func(table.unpack({...})) 
end

env["if"] = function(condition, true_body, false_body)
    if make_function({}, condition)() then
        return make_function({}, true_body)()
    else
        return make_function({}, false_body)()
    end
end

env["_if"] = env["if"]

env["loop"] = function(counter_name, times, body)
    local body_function = make_function(counter_name, body)
    for i = 1, times do
        body_function(i)
    end
end

local function compatible_name(name)
    
    local translate = {
        ["false"]  = "_false",
        ["true"]   = "_true",
        ["nil"]    = "_nil",
        ["="]      = "equal",
        ["!="]     = "not_equal",
        ["<"]      = "less_than",
        ["<="]     = "less_than_or_equal",
        [">"]      = "greater_than",
        [">="]     = "greater_than_or_equal",
        ["!"]      = "_not",
        ["||"]     = "_or",
        ["&&"]     = "_and",
        ["+"]      = "add",
        ["-"]      = "sub",
        ["*"]      = "mul",
        ["if"]     = "_if",
        ["return"] = "_return",
        ["@"]      = "strcat",
        ["$"]      = "get"
    }
    
    name = translate[name] or name
    
    if string.match(name, "[^%w_.]") then
         error("invalid name '" ..name .. "'")
    end
    
    return name
end

env["compatible_name"] = compatible_name

local function create_ast(output)
    
    local node = output
    local parent = {}
    
    return cubescript.command_stack({
        
        push_command = function()
            node[#node + 1] = {arguments = {}}
            parent[#parent + 1] = node
            node = node[#node].arguments
        end,
        
        push_argument_symbol = function(id)
            node[#node + 1] = {value = id, is_variable = true}
        end,
        
        push_argument = function(value)
            node[#node + 1] = {value = value}
        end,
        
        pop_string = function()
            local value = node[#node].value
            node[#node] = nil
            return value
        end,
        
        call = function(index)
            node = parent[#parent]
            parent[#parent] = nil
        end
    })
end

function generate_expression_code(input)
    
    local function generate_argument_code(input, index)
        assert(input.arguments[index])
        input.arguments[index].parent = input
        -- FIXME .parent must always be reset to nil to avoid mem leak
        local output = generate_expression_code(input.arguments[index])
        input.arguments[index].parent = nil
        return output
    end
    
    local function print_value(value)
        if type(value) == "string" then
            
            value = string.gsub(value, "[%c\\\\\"]", function(char)
                return "\\" .. string.byte(char)
            end)
            
            return "\"" .. value .. "\""
        else
            return value
        end
    end
    
    if not input.arguments and input.value then
        if input.is_variable then
            return input.value
        else
            return print_value(input.value)
        end
    end
    
    for key, arg in pairs(input.arguments) do
        if arg.is_variable then
            input.arguments[key].value = compatible_name(arg.value)
        end
    end
    
    local function native_value(v)
        return function() return v end
    end
    
    local function function_call(input)
        local output = generate_argument_code(input, 1) .. "("
        for i = 2, #input.arguments do
            if i > 2 then output = output .. "," end
            output = output .. generate_argument_code(input, i)
        end
        output = output .. ")"
        return output
    end
    
    local function not_enough_args(input, n)
        return #input.arguments -1 < n
    end
    
    local function define_variable(input)
        if input.parent or not_enough_args(input, 2) then
            return function_call(input)
        end
        local output = "local "
        output = output .. input.arguments[2].value
        output = output .. "="
        output = output .. generate_argument_code(input, 3)
        return output
    end
    
    local function arthmetic_operation(operator)
        return function(input)
        
            if not_enough_args(input, 2) or not input.parent then
                return function_call(input)
            end
            
            local output = "("
            output = output .. generate_argument_code(input, 2)
            for i = 3, #input.arguments do
               output = output .. " " .. operator .. " " 
                        .. generate_argument_code(input, i)
            end
            output = output .. ")"
            return output 
        end
    end
    
    local function comparison_operation(operator)
        return function(input)
            
            if not_enough_args(input, 2) or not input.parent then
                return function_call(input)
            end
            
            local output = "("
            output = output .. generate_argument_code(input, 2)
            output = output .. " " .. operator .. " " 
                     .. generate_argument_code(input, 3)
            for i = 4, #input.arguments do
                output = output .. " and " 
                    .. generate_argument_code(input, i) .. " " .. operator
                    .. " " .. generate_argument_code(input, 2)
            end
            output = output .. ")"
            return output
        end
    end
    
    local function logic_operation(operator)
        return function(input)
            if not_enough_args(input, 2) or not input.parent then
                return function_call(input)
            end
            return generate_argument_code(input, 2) 
                .. " " .. operator .. " " .. generate_argument_code(input, 3)
        end
    end
    
    local function not_operation(input)
        if not_enough_args(input, 1) or not input.parent then
            return function_call(input)
        end
        return "not " .. generate_argument_code(input, 2)
    end
    
    local function return_statement(input)
        if input.parent then
            return "error(\"invalid return statement\")"
        end
        local output = "do return "
        if input.arguments[2] then
            output = output .. generate_argument_code(input, 2)
        end
        output = output .. " end"
        return output
    end
    
    local function if_statement(input)
        
        if input.parent or not_enough_args(input, 2) then
            return function_call(input)
        end
        
        local output = "if " 
            .. generate_argument_code(input, 2) .. " then\n" 
            .. generate_code(input.arguments[3].value)
        
        if input.arguments[4] then
            output = output .. "else\n" 
                     .. generate_code(input.arguments[4].value)
        end
        
        output = output .. "end"
        return output
    end
    
    local function loop(input)
        
        if not input.arguments[2].value or input.arguments[2].is_variable then
           return function_call(input) 
        end
        
        local output = "for " .. input.arguments[2].value 
        output = output .. " = 0, " .. input.arguments[3].value .. " do\n"
        output = output .. generate_code(input.arguments[4].value)
        output = output .. "end"
        return output
    end
    
    function define_function(input)
    
        if not_enough_args(input, 2) or input.arguments[2].is_variable then
            return function_call(input)
        end
        
        local output = "function("
        
        local parameters = parse_array(input.arguments[2].value)
        local first = true
        for _, name in pairs(parameters) do
            if not first then
                output = output .. ","
            else
                first = false
            end
            output = output .. name
        end
        
        output = output .. ")\n"
        output = output .. generate_code(input.arguments[3].value)
        output = output .. "end"
        
        return output
    end
    
    local templates = {
        _true = native_value("true"),
        _false = native_value("false"),
        _nil = native_value("nil"),
        def = define_variable,
        add = arthmetic_operation("+"),
        sub = arthmetic_operation("-"),
        mul = arthmetic_operation("*"),
        div = arthmetic_operation("/"),
        equal = comparison_operation("=="),
        not_equal = comparison_operation("~="),
        less_than = comparison_operation("<"),
        less_than_or_equal = comparison_operation("<="),
        greater_than = comparison_operation(">"),
        greater_than_or_equal = comparison_operation(">="),
        _not = not_operation,
        _or = logic_operation("or"),
        _and = logic_operation("and"),
        _return = return_statement,
        _if = if_statement,
        loop = loop,
        func = define_function
    }
    
    return (templates[input.arguments[1].value] or function_call)(input)
end

function generate_code(input)
    
    if type(input) == "string" then
        local tree = {}
        input = input .. "\n"
        cubescript.eval(input, create_ast(tree))
        return generate_code(tree)
    end
    
    local output = ""
    for _, value in pairs(input) do
        if #value.arguments > 0 then
            output = output .. generate_expression_code(value) .. "\n"
        end
    end
    return output
end

env["lua"] = dofile

local function execute_cubescript(filename)
    
    local file = io.open(filename)
    if not file then
        error("could not open file '" .. filename .. "'")
    end
    
    local expression = ""
    local line_number = 1
    local line_number_expression_start = 1
    
    local old_current_location = env.current_location
    
    local function cleanup()
        env.current_location = old_current_location
        file:close()
    end
    
    for line in file:lines() do
    
        expression = expression .. line .. "\n"
        
        if cubescript.is_complete_expression(expression) then
            
            env.current_location = function()
                return filename .. ":" .. line_number_expression_start
            end
            
            local error_message = cubescript.eval(expression, env)
            
            if error_message then
                
                cleanup()
                
                error({string.format("%s:%i: %s", 
                    filename, line_number, error_message)}, 0)
            end
            
            expression = ""
            line_number_expression_start = line_number + 1
        end
        
        line_number = line_number + 1
    end
    
    cleanup()
end

env["exec_cubescript"] = execute_cubescript

env["exec_type"] = {
    lua = dofile,
    conf = execute_cubescript,
    cs = execute_cubescript
}

env["exec_search_paths"] = {}

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

env["find_script"] = find_script

env["exec"] = function(filename, function_level)
    
    function_level = function_level or 2
    
    local absolute_filename = find_script(filename, function_level)
    
    if not absolute_filename then
        error(string.format("cannot execute '%s': file not found", filename))
    end
    
    filename = absolute_filename
    
    local file_type = string.match(filename, "[^.]*$")
    local executor = env.exec_type[file_type]
    
    if not executor then
        error(string.format("cannot execute '%s': unknown script type"), filename)
    end
    
    local pcall_results = (function(...) return {...} end)(pcall(executor, filename))
    
    if pcall_results[1] == false then
        error(pcall_results[2], 0)
    end
    
    table.remove(pcall_results, 1)
    return table.unpack(pcall_results)
end

local exec = env.exec

env["pexec"] = function(filename)
    return (function(...) return {...} end)(pcall(exec, filename, 3))
end

env["exec_if_found"] = function(filename)
    
    filename = find_script(filename, 1)
    
    if not filename then
        return
    end
    
    exec(filename, 2)
end

env["eval_lua"] = function(str)
    local chunk, error_message = load(str)
    if not chunk then 
        error(error_message)
    end
    return chunk()
end

return env

