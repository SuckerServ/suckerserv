local sqlite3 = require "lsqlite3"

local _M = {}

function first_row(statement)
    statement:reset()
    if statement:step() ~= sqlite3.ROW then
        return nil
    end
    return statement:get_named_values()
end

function create_missing_tables(schemafilename, db, dbfilename)
    
    local schemafile, err = io.open(schemafilename)
    if not schemafile then return nil, err end
    
    local schema = sqlite3.open_memory()
    schema:exec("BEGIN TRANSACTION")
    schema:exec(schemafile:read("*a"))
    schema:exec("COMMIT TRANSACTION")
    
    db:exec("BEGIN TRANSACTION")
    
    for row in schema:rows("SELECT name, type, sql FROM sqlite_master") do
    
        local row_name = row[1]
        local row_type = row[2]
        local row_sql = row[3]
    
        local search_for_table,perr = db:prepare("SELECT count(*) as count FROM sqlite_master WHERE name = ?")
        search_for_table:bind_values(row_name)
        
        local table_count,perr = first_row(search_for_table)
        if table_count.count == 0 then
            
            print(string.format("Updating database %s: adding %s %s.", dbfilename, row_name, row_type))
            
            db:exec(row_sql)
            
        else -- table found, check columns
            
            local existing_cols = {}
            
            for table_column in db:nrows(string.format("PRAGMA table_info(%s)", row_name)) do
                local table_column_name = table_column[2]
                
                existing_cols[table_column.name] = table_column
            end
            
            for table_column in schema:nrows(string.format("PRAGMA table_info(%s)", row_name)) do
                
                if not existing_cols[table_column.name] then
                    
                    local column_def = table_column.name .. " " .. table_column.type
                    
                    if table_column.pk == 1 then
                        column_def = column_def .. " PRIMARY KEY"
                    else
                        if table_column.notnull == 1 then
                            column_def = column_def .. " NOT NULL"
                        elseif table_column.dflt_value then
                            column_def = column_def .. " DEFAULT " .. table_column.dflt_value
                        end
                    end
                    
                    print(string.format("Updating %s table in database %s: adding %s column.", row_name, db.filename, table_column.name))
                    
                    db:exec(string.format("ALTER TABLE %s ADD COLUMN %s", row_name, column_def))
                    
                end
            end
        end
    end
    
    db:exec("COMMIT TRANSACTION")
end

--[[
function BROKEN_reinstall_triggers(schemafilename, db)
    
    schemafile,err = io.open(schemafilename)
    if not schemafile then return nil,err end
    
    local schema = sqlite3.open_memory()
    schema:exec("BEGIN TRANSACTION")
    schema:exec(schemafile:read("*a"))
    schema:exec("COMMIT TRANSACTION")

    db:exec("BEGIN TRANSACTION")
    
    for trigger in schema:rows("SELECT * FROM sqlite_master WHERE type = 'trigger'") do
        db:exec("DROP TRIGGER IF EXISTS " .. trigger.name)
        db:exec(trigger.sql)
    end
    
    db:exec("COMMIT TRANSACTION")
    
end
]]

function set_sqlite3_synchronous_pragma(db, value)

    local accepted = {[0] = true, [1] = true, [2] = true, ["OFF"] = true, ["NORMAL"] = true, ["FULL"] = true}
    
    if not accepted[value] then
        error("Unrecognised value set for stats_sqlite_synchronous variable.")
    end
    
    db:exec("PRAGMA synchronous = " .. value)
end

function set_sqlite3_exclusive_locking(db)

    db:exec("PRAGMA locking_mode=EXCLUSIVE")
end

_M.first_row = first_row
_M.create_missing_tables = create_missing_tables
_M.set_sqlite3_synchronous_pragma = set_sqlite3_synchronous_pragma
_M.set_sqlite3_exclusive_locking = set_sqlite3_exclusive_locking

return _M
