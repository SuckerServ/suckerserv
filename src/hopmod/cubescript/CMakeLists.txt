cmake_minimum_required(VERSION 2.6)
project(cubescript)

find_package(Lua51)

add_definitions(-Wall)
include_directories(${LUA_INCLUDE_DIR})

set(CUBESCRIPT_SOURCES 
    cubescript.cpp
    lua_command_stack.cpp
    lua/pcall.cpp)

add_library(cubescript STATIC ${CUBESCRIPT_SOURCES})
target_link_libraries(cubescript ${LUA_LIBRARIES})

#add_executable(test-cubescript test.cpp)
#target_link_libraries(test-cubescript cubescript)

add_executable(repl repl.cpp)
target_link_libraries(repl cubescript -lreadline)

