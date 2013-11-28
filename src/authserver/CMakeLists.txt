include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame
    ${CMAKE_CURRENT_SOURCE_DIR}/../hopmod
    ${ENET_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIRS}
    ${FUNGU_INCLUDE_DIRS})

add_definitions(-DSTANDALONE)

add_executable(sauer_authserver 
	authserver.cpp
	core_bindings.cpp
	../hopmod/signals.cpp
	../hopmod/events.cpp
	../hopmod/lua.cpp)
target_link_libraries(sauer_authserver
    sauertools
    lua_modules
    ${ENET_LIBRARY}
    ${BOOST_THREAD_LIBRARY}
    -lrt)

install(TARGETS
            sauer_authserver
       RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR})

