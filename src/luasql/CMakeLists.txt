set(LUASQL_MYSQL_LIBRARY luasql_mysql CACHE INTERNAL "luasql_mysql library")
set(LUASQL_POSTGRES_LIBRARY luasql_postgres CACHE INTERNAL "luasql_postgres library")

# Find PostgreSQL
find_package(PostgreSQL)

IF(POSTGRESQL_FOUND)
	include_directories(${LUA_INCLUDE_DIRS} ${POSTGRESQL_INCLUDE_DIRS})

	add_library(${LUASQL_POSTGRES_LIBRARY} SHARED luasql.c ls_postgres.c)
	target_link_libraries(${LUASQL_POSTGRES_LIBRARY} ${LUA_LIBRARY} ${POSTGRESQL_LIBRARIES})

	install(TARGETS ${LUASQL_POSTGRES_LIBRARY})

ENDIF(POSTGRESQL_FOUND)

# Find MySQL
find_package(MySQL)

IF(MYSQL_FOUND)
	include_directories(${LUA_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS})

	add_library(${LUASQL_MYSQL_LIBRARY} SHARED luasql.c ls_mysql.c)
	target_link_libraries(${LUASQL_MYSQL_LIBRARY} ${LUA_LIBRARY} ${MYSQL_LIBRARIES})

	install(TARGETS ${LUASQL_MYSQL_LIBRARY})

ENDIF(MYSQL_FOUND)
