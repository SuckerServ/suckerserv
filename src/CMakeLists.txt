#### Dependencies ####

## Enet ##
find_package(ENet 1.3.13)

if(ENET_FOUND)
    set(ENET_INCLUDE_DIRS ${ENET_INCLUDE_DIR})    
else(ENET_FOUND)
    add_subdirectory(enet)
endif(ENET_FOUND)

## LuaJIT##
option(LUA_USE_LUAJIT "Use LuaJIT instead of default Lua" ON)
unset(Lua_FOUND CACHE)
unset(LUA_INCLUDE_DIR CACHE)
unset(LUA_LIBRARY CACHE)
find_package(LuaJIT REQUIRED)

set(LUA_INCLUDE_DIRS ${LUA_INCLUDE_DIR})
set(LUA_LIBRARY ${LUA_LIBRARY})

## Asio ##
find_package(Asio)

if(NOT ASIO_FOUND)
    execute_process(
        COMMAND git submodule update --init src/asio
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
    set(ASIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/asio/asio/include)
endif()

## OpenSSL ##
find_package(OpenSSL)

option(WITH_OPENSSL "Enable SSL support" ON)

## MaxmindDB ##
find_package(MMDB)

if(NOT MMDB_FOUND)
    execute_process(
        COMMAND git submodule update --init src/libmaxminddb
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

    set(MMDB_LIBRARY
        maxminddb
        CACHE INTERNAL "mmdb library")

    set(MMDB_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/libmaxminddb/include
        ${CMAKE_CURRENT_BINARY_DIR}/maxminddb)

    set(MMDB_SOURCES
        libmaxminddb/src/maxminddb.c
    )

    SET_SOURCE_FILES_PROPERTIES(libmaxminddb/src/maxminddb.c PROPERTIES COMPILE_FLAGS -std=c99)
    SET_SOURCE_FILES_PROPERTIES(libmaxminddb/src/maxminddb.c PROPERTIES COMPILE_DEFINITIONS PACKAGE_VERSION="1.0.1")

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/libmaxminddb/include/maxminddb_config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/maxminddb/maxminddb_config.h
    )
    
    add_library(${MMDB_LIBRARY} SHARED ${MMDB_SOURCES})
    
    install(TARGETS
        ${MMDB_LIBRARY}
        LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR})
endif()

option(WITH_MMDB "Enable MMDB support" ON)

if(WITH_MMDB)
    add_definitions(-DWITH_MMDB)
endif(WITH_MMDB)

## LuaTZ ##

execute_process(
    COMMAND git submodule update --init src/luatz
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

FILE(GLOB LuaTZ_FILES "${CMAKE_CURRENT_SOURCE_DIR}/luatz/luatz/*.lua")

install(FILES
    ${LuaTZ_FILES}
    DESTINATION ${PROJECT_SOURCE_DIR}/script/package/luatz)


add_subdirectory(fungu)
add_subdirectory(lsqlite3)
add_subdirectory(hopmod/standalone/luapp)
add_subdirectory(luasql)
#add_subdirectory(authserver)


include(CheckLibraryExists)

check_library_exists(pthread pthread_self "" PTHREAD_EXISTS)
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    check_library_exists(rt clock_gettime "" RTLIB_EXISTS)
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(WITH_OPENSSL AND OPENSSL_FOUND AND OPENSSL_INCLUDE_DIR)
    set(LUA_NET_SSL_SOURCES
        hopmod/lua/modules/net/ssl_context.cpp
        hopmod/lua/modules/net/ssl_tcp_stream.cpp)
    add_definitions(-DKSSL_DEBUG)
else(WITH_OPENSSL AND OPENSSL_FOUND AND OPENSSL_INCLUDE_DIR)
    add_definitions(-DDISABLE_SSL -DWITHOUT_OPENSSL)
endif(WITH_OPENSSL AND OPENSSL_FOUND AND OPENSSL_INCLUDE_DIR)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/fpsgame
    ${CMAKE_CURRENT_SOURCE_DIR}/hopmod
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ENET_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIRS}
    ${ASIO_INCLUDE_DIRS}
    ${FUNGU_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${MMDB_INCLUDE_DIRS})

add_definitions(-fsigned-char -DSTANDALONE -DREVISION=$ENV{REVISION} -DASIO_STANDALONE)


if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    add_definitions( -g)
else(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    add_definitions( -O3)
endif(CMAKE_BUILD_TYPE STREQUAL "DEBUG")

set(SAUERTOOLS_SOURCES
    shared/tools.cpp
    shared/stream.cpp
    shared/crypto.cpp
    )

add_library(sauertools STATIC ${SAUERTOOLS_SOURCES})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(sauertools -lrt)
else(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_library(coreServices CoreServices)
    target_link_libraries(sauertools ${coreServices})
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

target_link_libraries(sauertools -lz ${ENET_LIBRARY})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(LUA_MODULES_SOURCES
    hopmod/lib/md5.c
    hopmod/utils/time.cpp
    hopmod/utils/files.cpp
    hopmod/utils/hopmod.cpp
    hopmod/net/address.cpp
    hopmod/net/address_mask.cpp
    hopmod/net/address_prefix.cpp
    hopmod/lua/event.cpp
    hopmod/lua/push_function.cpp
    hopmod/lua/library_extensions.cpp
    hopmod/lua/pcall.cpp
    hopmod/lua/error_handler.cpp
    hopmod/lua/push.cpp
    hopmod/lua/register_class.cpp
    hopmod/lua/modules/module.cpp
    hopmod/lua/modules/crypto.cpp
    hopmod/lua/modules/mmdb.cpp
    hopmod/lua/modules/filesystem.cpp
    hopmod/lua/modules/net/module.cpp
    hopmod/lua/modules/net/tcp_socket.cpp
    hopmod/lua/modules/net/weak_ref.cpp
    hopmod/lua/modules/net/ipmask.cpp
    hopmod/lua/modules/net/file_stream.cpp
    hopmod/lua/modules/net/tcp_acceptor.cpp
    hopmod/lua/modules/net/resolver.cpp
    ${LUA_NET_SSL_SOURCES}
    hopmod/httpserver/filesystem_resource.cpp
    hopmod/httpserver/lua_module.cpp )

add_library(lua_modules STATIC ${LUA_MODULES_SOURCES})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(lua_modules -pthread)
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

target_link_libraries(lua_modules 
    sauertools
    ${LUA_LIBRARY}
    ${MMDB_LIBRARY}
    ${FUNGU_STRING_LIBRARY}
    ${FUNGU_HTTP_LIBRARY}
    ${OPENSSL_LIBRARIES})


set(GAME_SERVER_SOURCES
    engine/server.cpp
    fpsgame/server.cpp
    hopmod/netbans.cpp
    hopmod/startup.cpp
    hopmod/scheduler.cpp
    hopmod/lua.cpp
    hopmod/core_bindings.cpp
    hopmod/events.cpp)

add_executable(sauer_server ${GAME_SERVER_SOURCES})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(sauer_server -lrt)
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

target_link_libraries(sauer_server 
    -lz
    sauertools
    lua_modules
    ${ENET_LIBRARY})

add_executable(keygen hopmod/standalone/keygen.cpp)
target_link_libraries(keygen sauertools)


add_executable(monitor hopmod/standalone/monitor.cpp hopmod/lib/program_arguments.cpp)


install(TARGETS 
    sauer_server
    monitor
    RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR} 
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR} 
    ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR})

install(TARGETS 
    keygen
    RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}/utils)
