project(lsqlite3 C)

include(CheckLibraryExists)
check_library_exists(sqlite3 sqlite3_open "" SQLITE3_FOUND)

if(${SQLITE3_FOUND})

    set(LSQLITE3_LIBRARY
        lsqlite3
        CACHE INTERNAL "lsqlite3 library")

    add_library(${LSQLITE3_LIBRARY} STATIC lsqlite3.c)
    target_link_libraries(${LSQLITE3_LIBRARY} ${LUA_LIBRARY} -lsqlite3)
    include_directories(${LUA_INCLUDE_DIRS})
    
    set(LSQLITE3_DEFINITIONS 
        -DHAS_LSQLITE3 
        CACHE INTERNAL "lsqlite3 preprocessor definitions")
    
    install(TARGETS 
                ${LSQLITE3_LIBRARY}
            LIBRARY DESTINATION 
                ${INSTALL_LIBRARY_DIR}
            ARCHIVE DESTINATION
                ${INSTALL_ARCHIVE_DIR})

endif(${SQLITE3_FOUND})

