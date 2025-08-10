
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

include_directories(${FUNGU_INCLUDE_DIRS} ${ASIO_INCLUDE_DIRS})

add_definitions(-DASIO_STANDALONE)

add_library(${FUNGU_HTTP_LIBRARY} STATIC ${SOURCES})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${FUNGU_HTTP_LIBRARY} -pthread)
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

target_link_libraries(${FUNGU_HTTP_LIBRARY} ${FUNGU_STRING_LIBRARY})

install(TARGETS ${FUNGU_HTTP_LIBRARY})
