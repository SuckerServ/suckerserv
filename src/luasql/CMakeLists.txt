set(MYSQL_DIR "/usr/local/mysql" CACHE STRING "Directory of MySQL installation")

set(LUASQL_MYSQL_LIBRARY luasql_mysql CACHE INTERNAL "luasql_mysql library")
set(LUASQL_POSTGRES_LIBRARY luasql_postgres CACHE INTERNAL "luasql_postgres library")

include_directories(${LUA_INCLUDE_DIRS} ${MYSQL_DIR}/include /usr/include/mysql)
link_directories(${MYSQL_DIR}/lib)

set(SOURCES
    luasql.c
    ls_mysql.c)

add_library(${LUASQL_MYSQL_LIBRARY} SHARED luasql.c ls_mysql.c)
target_link_libraries(${LUASQL_MYSQL_LIBRARY} -lz -lmysqlclient ${LUA_LIBRARY})

install(TARGETS 
            ${LUASQL_MYSQL_LIBRARY}
        LIBRARY DESTINATION 
            ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION
            ${INSTALL_ARCHIVE_DIR})

# Find PostgreSQL
find_package(PostgreSQL)

IF(POSTGRESQL_FOUND)
	include_directories(${LUA_INCLUDE_DIRS} ${POSTGRESQL_INCLUDE_DIRS})

	add_library(${LUASQL_POSTGRES_LIBRARY} SHARED luasql.c ls_postgres.c)
	target_link_libraries(${LUASQL_POSTGRES_LIBRARY} ${LUA_LIBRARY} ${POSTGRESQL_LIBRARIES})

	install(TARGETS
			${LUASQL_POSTGRES_LIBRARY}
		LIBRARY DESTINATION
			${INSTALL_LIBRARY_DIR}
		ARCHIVE DESTINATION
			${INSTALL_ARCHIVE_DIR})

ENDIF()
