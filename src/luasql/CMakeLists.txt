set(MYSQL_DIR "/usr/local/mysql" CACHE STRING "Directory of MySQL installation")

set(LUASQL_MYSQL_LIBRARY luasql_mysql CACHE INTERNAL "luasql_mysql library")

include_directories(${LUA_INCLUDE_DIRS} ${MYSQL_DIR}/include /usr/include/mysql)
link_directories(${MYSQL_DIR}/lib)

set(SOURCES
    src/luasql.c
    src/ls_mysql.c)

add_library(${LUASQL_MYSQL_LIBRARY} SHARED src/luasql.c src/ls_mysql.c)
target_link_libraries(${LUASQL_MYSQL_LIBRARY} -lz -lmysqlclient ${LUA_LIBRARY})

install(TARGETS 
            ${LUASQL_MYSQL_LIBRARY}
        LIBRARY DESTINATION 
            ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION
            ${INSTALL_ARCHIVE_DIR})

