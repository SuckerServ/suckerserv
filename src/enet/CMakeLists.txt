project(enet C)

option(USE_CRC32 "Whether to use CRC32 packet verification")

if(USE_CRC32)
    add_definitions(-DUSE_CRC32)
endif(USE_CRC32)

include(CheckFunctionExists)
include(CheckTypeSize)

check_function_exists(gethostbyaddr_r GETHOSTBYADDR_R_FUNCTION_FOUND)
check_function_exists(gethostbyname_r GETHOSTBYNAME_R_FUNCTION_FOUND)
check_function_exists(poll            POLL_FUNCTION_FOUND)
check_function_exists(fcntl           FCNTL_FUNCTION_FOUND)
check_function_exists(inet_pton       INET_PTON_FUNCTION_FOUND)
check_function_exists(inet_ntop       INET_NTOP_FUNCTION_FOUND)

if(GETHOSTBYADDR_R_FUNCTION_FOUND)
    add_definitions(-DHAS_GETHOSTBYADDR_R)
endif(GETHOSTBYADDR_R_FUNCTION_FOUND)

if(GETHOSTBYNAME_R_FUNCTION_FOUND)
    add_definitions(-DHAS_GETHOSTBYNAME_R)
endif(GETHOSTBYNAME_R_FUNCTION_FOUND)

if(POLL_FUNCTION_FOUND)
    add_definitions(-DHAS_POLL)
endif(POLL_FUNCTION_FOUND)

if(FCNTL_FUNCTION_FOUND)
    add_definitions(-DHAS_FCNTL)
endif(FCNTL_FUNCTION_FOUND)

if(INET_PTON_FUNCTION_FOUND)
    add_definitions(-DHAS_INET_PTON)
endif(INET_PTON_FUNCTION_FOUND)

if(INET_NTOP_FUNCTION_FOUND)
    add_definitions(-DHAS_INET_NTOP)
endif(INET_NTOP_FUNCTION_FOUND)

set(CMAKE_EXTRA_INCLUDE_FILES sys/types.h sys/socket.h)
check_type_size(socklen_t HAS_SOCKLEN_T)
set(CMAKE_EXTRA_INCLUDE_FILES)

if(HAS_SOCKLEN_T)
    add_definitions(-DHAS_SOCKLEN_T)
endif(HAS_SOCKLEN_T)

set(ENET_LIBRARY 
    enet 
    CACHE INTERNAL "enet library")

set(ENET_INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    CACHE INTERNAL "enet public include directories")

IF(NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG") 
    add_definitions( -O2)
ENDIF(NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG")

set(ENET_SOURCES
	host.c
	list.c
	callbacks.c
	compress.c
	packet.c
	peer.c
	protocol.c
	unix.c
	win32.c)

include_directories(${ENET_INCLUDE_DIRS})

add_library(${ENET_LIBRARY} STATIC ${ENET_SOURCES})

install(TARGETS 
            ${ENET_LIBARY} 
       LIBRARY DESTINATION
            ${INSTALL_LIBRARY_DIR} 
       ARCHIVE DESTINATION
            ${INSTALL_LIBRARY_DIR})

