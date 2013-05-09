
add_definitions(-DNO_EVENTS -DNO_CORE_TABLE)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${LUA_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIRS}
    ${FUNGU_INCLUDE_DIRS})

IF(NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG") 
    add_definitions( -O3)
ENDIF(NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG")

add_executable(luapp luapp.cpp ../../lua.cpp)

target_link_libraries(luapp ${LUA_LIBRARY} ${BOOST_SYSTEM_LIBRARY} ${FUNGU_STRING_LIBRARY} ${FUNGU_HTTP_LIBRARY} lua_modules)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(luapp -lrt)
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

install(TARGETS luapp RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}/utils)

