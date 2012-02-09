local validate_type, validate_table

function validate(subject, schema)

    local validators = {
        ["string"] = validate_type,
        ["table"] = validate_table,
        ["function"] = schema
    }
    
    local validator = validators[type(schema)]
    
    if not validator then
        return false, "cannot validate"
    end
    
    local ok, error_message, path = validator(subject, schema)
    
    if not ok then
        if path then
            error_message = string.format("%s field: %s", table.concat(path, "."), error_message)
        end
        return false, error_message
    end
    
    return true
end

function validate_type(subject, schema)
    
    if schema == "any" then
        return true
    end
    
    if type(subject) ~= schema then
       return false, string.format("expected %s type", schema)
    end
    
    return true
end

function validate_table(subject, schema)
    
    local ok, error_message = validate_type(subject, "table")
    if not ok then
        return false, error_message
    end
    
    for key, validator in pairs(schema) do
        
        if not subject[key] then
            return false, string.format("missing %s field", key)
        end
        
        local ok, error_message, path = validate(subject[key], validator)
        if not ok then
            if not path then path = {} end
            path[#path + 1] = key
            return false, error_message, path
        end
    end
    
    return true
end

