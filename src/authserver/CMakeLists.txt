include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame
    ${CMAKE_CURRENT_SOURCE_DIR}/../hopmod
    ${ENET_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIRS}
    ${ASIO_INCLUDE_DIRS}
    ${FUNGU_INCLUDE_DIRS})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions(-DSTANDALONE -DASIO_STANDALONE)

add_executable(sauer_authserver 
	authserver.cpp
	core_bindings.cpp
	events.cpp
	../hopmod/scheduler.cpp
	../hopmod/lua.cpp)

target_link_libraries(sauer_authserver
    sauertools
    lua_modules
    ${ENET_LIBRARY}
    -lrt)

install(TARGETS
            sauer_authserver
       RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR})

