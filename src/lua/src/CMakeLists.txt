
set(LUA_LIBRARY 
    lua 
    CACHE INTERNAL "lua library")

set(LUA_INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE INTERNAL "lua include directory")
    
set(LUA_SOURCES
    lapi.c      
    lcode.c   
    ldo.c    
    lgc.c     
    llex.c      
    loadlib.c   
    loslib.c   
    lstring.c  
    ltablib.c      
    lzio.c
    lauxlib.c   
    ldblib.c  
    ldump.c  
    linit.c   
    lmathlib.c  
    lobject.c   
    lparser.c  
    lstrlib.c  
    ltm.c      
    lundump.c  
    lbaselib.c  
    ldebug.c  
    lfunc.c  
    liolib.c  
    lmem.c
    lopcodes.c
    lstate.c
    ltable.c
    lvm.c
    lctype.c
    lbitlib.c
    lcorolib.c
)

add_library(${LUA_LIBRARY} SHARED ${LUA_SOURCES})

if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
    add_definitions(-DLUA_USE_LINUX)
    target_link_libraries(${LUA_LIBRARY} -ldl)
endif(${CMAKE_SYSTEM_NAME} MATCHES Linux)

if(${CMAKE_SYSTEM_NAME} MATCHES BSD)
    add_definitions(-DLUA_USE_POSIX -DLUA_USE_DLOPEN)
endif(${CMAKE_SYSTEM_NAME} MATCHES BSD)

install(TARGETS 
            ${LUA_LIBRARY}
        LIBRARY DESTINATION
            ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION
            ${INSTALL_ARCHIVE_DIR})

add_definitions(-DLUA_COMPAT_MODULE)