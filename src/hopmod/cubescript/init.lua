local init_library, error_message = loadfile("cubescript_library.lua")
if not init_library then error(error_message) end
set_env_table(init_library())

