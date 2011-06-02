
set(FUNGU_SCRIPT_LIBRARY 
    fungu_script
    CACHE INTERNAL "fungu.script library")

set(SOURCES
    type.cpp
    constructs.cpp
    env.cpp
    execute.cpp
    objects.cpp
    corelib.cpp
    lua.cpp)

add_definitions(-DFUNGU_WITH_LUA)

include_directories(
    ${FUNGU_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIRS})

add_library(${FUNGU_SCRIPT_LIBRARY} STATIC ${SOURCES})

target_link_libraries(${FUNGU_SCRIPT_LIBRARY} ${FUNGU_STRING_LIBRARY} ${LUA_LIBRARY})

