
set(FUNGU_HTTP_LIBRARY fungu_http
    CACHE INTERNAL "fungu_http library")

set(SOURCES
    connection.cpp
    header.cpp
    info.cpp
    request.cpp
    request_line.cpp
    response.cpp
    status.cpp)

include_directories(${FUNGU_INCLUDE_DIRS} ${BOOST_INCLUDE_DIR})

add_library(${FUNGU_HTTP_LIBRARY} STATIC ${SOURCES})

target_link_libraries(${FUNGU_HTTP_LIBRARY} ${FUNGU_STRING_LIBRARY} ${FUNGU_SCRIPT_LIBRARY} -pthread ${BOOST_SYSTEM_LIBRARY})

install(TARGETS 
            ${FUNGU_HTTP_LIBRARY}
        LIBRARY DESTINATION
            ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION
            ${INSTALL_ARCHIVE_DIR})

