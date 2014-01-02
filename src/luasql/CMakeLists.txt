set(LUASQL_MYSQL_LIBRARY luasql_mysql CACHE INTERNAL "luasql_mysql library")

# Find MySQL
find_package(MySQL)

IF(MYSQL_FOUND)
        include_directories(${LUA_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS})

        add_library(${LUASQL_MYSQL_LIBRARY} SHARED luasql.c ls_mysql.c)
        target_link_libraries(${LUASQL_MYSQL_LIBRARY} ${LUA_LIBRARY} ${MYSQL_LIBRARIES})

        install(TARGETS
                        ${LUASQL_MYSQL_LIBRARY}
                LIBRARY DESTINATION
                        ${INSTALL_LIBRARY_DIR}
                ARCHIVE DESTINATION
                        ${INSTALL_ARCHIVE_DIR})

ENDIF(MYSQL_FOUND)
