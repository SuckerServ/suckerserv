add_subdirectory(enet)
add_subdirectory(lua)
add_subdirectory(boost)
add_subdirectory(fungu)
add_subdirectory(lsqlite3-7)
add_subdirectory(authserver)
add_subdirectory(luasql)

include(CheckLibraryExists)

check_library_exists(GeoIP GeoIP_open "" GEOIP_EXISTS)
check_library_exists(z gzopen "" ZLIB_EXISTS)
check_library_exists(pthread pthread_self "" PTHREAD_EXISTS)
check_library_exists(rt clock_gettime "" RTLIB_EXISTS)

if(NOT ${GEOIP_EXISTS})
    message(FATAL_ERROR "missing required library: GeoIP")
endif(NOT ${GEOIP_EXISTS})

if(NOT ${ZLIB_EXISTS})
    message(FATAL_ERROR "missing required library: ZLIB")
endif(NOT ${ZLIB_EXISTS})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/fpsgame
    ${CMAKE_CURRENT_SOURCE_DIR}/hopmod
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/authserver
    ${ENET_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIRS}
    ${FUNGU_INCLUDE_DIRS})

add_definitions(-Wall -fsigned-char -DSTANDALONE -DFUNGU_WITH_LUA -DHOPMOD_GAMESERVER_EVENTS ${LSQLITE3_DEFINITIONS})

set(SCRIPTING_LIBRARIES ${LUA_LIBRARY} ${FUNGU_STRING_LIBRARY} ${FUNGU_SCRIPT_LIBRARY})

set(SAUERTOOLS_SOURCES
    shared/tools.cpp
    shared/stream.cpp
    shared/crypto.cpp
    hopmod/utils.cpp
    hopmod/net/address.cpp
    hopmod/net/address_mask.cpp
    hopmod/net/address_prefix.cpp)

add_library(sauertools STATIC ${SAUERTOOLS_SOURCES})

set(LUA_MODULES_SOURCES
    hopmod/lua/crypto.cpp
    hopmod/md5.c
    hopmod/lua/net.cpp
    hopmod/lua/timer.cpp
    hopmod/lua/cubescript.cpp
    hopmod/lua/geoip.cpp
    hopmod/lua/filesystem.cpp
    hopmod/lua/packlib.c)

add_library(lua_modules STATIC ${LUA_MODULES_SOURCES})
target_link_libraries(lua_modules -lGeoIP -pthread sauertools ${SCRIPTING_LIBRARIES})

add_library(serverscripting STATIC hopmod/scripting.cpp hopmod/signals.cpp)
target_link_libraries(serverscripting lua_modules ${BOOST_SIGNALS_LIBRARY} ${BOOST_SYSTEM_LIBRARY} ${SCRIPTING_LIBRARIES})

add_library(scriptpipe STATIC hopmod/script_pipe.cpp)
target_link_libraries(scriptpipe serverscripting)

SET(GAME_SERVER_SOURCES
    engine/server.cpp
    fpsgame/server.cpp
    hopmod/startup.cpp
    hopmod/scripting.cpp
    hopmod/script_bindings.cpp
    hopmod/script_pipe.cpp
    hopmod/parse_player_command.cpp
    hopmod/scheduler.cpp
    hopmod/playerid.cpp
    hopmod/restarter.cpp
    hopmod/httpserver/serverexec.cpp
    hopmod/httpserver/filesystem_resource.cpp
    hopmod/httpserver/lua_module.cpp)

add_definitions(${LSQLITE3_DEFINITIONS})
add_executable(sauer_server ${GAME_SERVER_SOURCES})
target_link_libraries(sauer_server sauertools serverscripting lua_modules ${BOOST_THREAD_LIBRARY} ${LSQLITE3_LIBRARY} scriptpipe ${ENET_LIBRARY} ${FUNGU_HTTP_LIBRARY} -lz -lrt)

add_executable(keygen hopmod/keygen.cpp)
target_link_libraries(keygen sauertools)

add_executable(luapp hopmod/luapp.cpp)
target_link_libraries(luapp ${LUA_LIBRARY} ${BOOST_SYSTEM_LIBRARY} lua_modules -lrt)

add_executable(monitor hopmod/monitor.cpp hopmod/program_arguments.cpp)

install(TARGETS 
            sauer_server
            sauertools
            lua_modules
            serverscripting
            scriptpipe
            monitor
        RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR} 
        LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR} 
        ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR})

install(TARGETS 
            keygen
            luapp
        RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}/utils)

