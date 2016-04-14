set(LSQLITE3_LIBRARY lsqlite3 CACHE INTERNAL "lsqlite3 library")

include(CheckLibraryExists)
check_library_exists(sqlite3 sqlite3_open "" SQLITE3_FOUND)

if(SQLITE3_FOUND)
    include_directories(${LUA_INCLUDE_DIRS})

    add_library(${LSQLITE3_LIBRARY} SHARED lsqlite3.c)
    target_link_libraries(${LSQLITE3_LIBRARY} ${LUA_LIBRARY} -lsqlite3)

    if(LUA_USE_LUAJIT)
        SET(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")
    endif()
    
    install(TARGETS 
                ${LSQLITE3_LIBRARY}
            LIBRARY DESTINATION 
                ${INSTALL_LIBRARY_DIR}
            ARCHIVE DESTINATION
                ${INSTALL_ARCHIVE_DIR})

endif(SQLITE3_FOUND)
