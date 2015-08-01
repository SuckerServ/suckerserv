find_package(ENet 1.3.13)

if(ENET_FOUND)
    set(ENET_INCLUDE_DIRS ${ENET_INCLUDE_DIR})    
else(ENET_FOUND)
    add_subdirectory(enet)
endif(ENET_FOUND)


if(NOT (CMAKE_MAJOR_VERSION LESS 2) AND NOT (CMAKE_MINOR_VERSION LESS 8))
    find_package(Lua5.2)
endif(NOT (CMAKE_MAJOR_VERSION LESS 2) AND NOT (CMAKE_MINOR_VERSION LESS 8))

if(LUA5.2_FOUND)
    set(LUA_INCLUDE_DIRS ${LUA5.2_INCLUDE_DIR})
    set(LUA_LIBRARY ${LUA5.2_LIBRARY})
else(LUA5.2_FOUND)
    add_subdirectory(lua)
endif(LUA5.2_FOUND)

find_package(Boost 1.49.0 COMPONENTS system thread)

if(Boost_FOUND)
    set(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    set(BOOST_SYSTEM_LIBRARY ${Boost_SYSTEM_LIBRARY})
    set(BOOST_THREAD_LIBRARY ${Boost_THREAD_LIBRARY})
else(Boost_FOUND)
    add_subdirectory(boost)
endif(Boost_FOUND)


find_package(OpenSSL)


find_package(GeoIP)

option(WITH_GEOIP "Enable GeoIP support" ON)

if(WITH_GEOIP)
    add_definitions(-DWITH_GEOIP)
    if(GeoIP_FOUND)
        set(GEOIP_INCLUDE_DIRS ${GeoIP_INCLUDE_DIRS})
        set(GEOIP_LIBRARY ${GeoIP_LIBRARY})
    else(GeoIP_FOUND)
        add_subdirectory(geoip)
    endif(GeoIP_FOUND)
endif(WITH_GEOIP)


add_subdirectory(fungu)
add_subdirectory(lsqlite3)
add_subdirectory(hopmod/standalone/luapp)
add_subdirectory(luasql)
add_subdirectory(authserver)


include(CheckLibraryExists)

check_library_exists(pthread pthread_self "" PTHREAD_EXISTS)
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    check_library_exists(rt clock_gettime "" RTLIB_EXISTS)
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

option(WITH_OPENSSL "Enable SSL support" ON)

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
    ${BOOST_INCLUDE_DIRS}
    ${FUNGU_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${GEOIP_INCLUDE_DIRS})

add_definitions(-Wall -fsigned-char -DSTANDALONE -DREVISION=$ENV{REVISION})


if(NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    add_definitions( -O3)
endif(NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG")

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


set(LUA_MODULES_SOURCES
    hopmod/lib/md5.c
    hopmod/utils.cpp
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
    hopmod/lua/modules/timer.cpp
    hopmod/lua/modules/geoip.cpp
    hopmod/lua/modules/filesystem.cpp
    hopmod/lua/modules/packlib.c
    hopmod/lua/modules/cubescript.cpp
    hopmod/lua/modules/net/module.cpp
    hopmod/lua/modules/net/tcp_socket.cpp
    hopmod/lua/modules/net/weak_ref.cpp
    hopmod/lua/modules/net/ipmask.cpp
    hopmod/lua/modules/net/file_stream.cpp
    hopmod/lua/modules/net/tcp_acceptor.cpp
    hopmod/lua/modules/net/resolver.cpp
    ${LUA_NET_SSL_SOURCES}
    hopmod/cubescript/cubescript.cpp
    hopmod/cubescript/lua_command_stack.cpp
    hopmod/cubescript/lua/pcall.cpp
    hopmod/httpserver/filesystem_resource.cpp
    hopmod/httpserver/lua_module.cpp )

add_library(lua_modules STATIC ${LUA_MODULES_SOURCES})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(lua_modules -pthread)
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

target_link_libraries(lua_modules 
    sauertools
    ${LUA_LIBRARY}
    ${GEOIP_LIBRARY}
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
    hopmod/events.cpp
    hopmod/signals.cpp)

add_executable(sauer_server ${GAME_SERVER_SOURCES})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(sauer_server -lrt)
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

target_link_libraries(sauer_server 
    -lz
    sauertools
    lua_modules
    ${BOOST_THREAD_LIBRARY}
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

install(FILES 
    hopmod/cubescript/cubescript_library.lua 
    DESTINATION ${PROJECT_SOURCE_DIR}/script/package)

