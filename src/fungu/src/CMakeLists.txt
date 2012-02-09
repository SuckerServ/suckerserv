include_directories(${FUNGU_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS})

set(FUNGU_STRING_LIBRARY fungu_string 
    CACHE INTERNAL "fungu_string library")

add_library(${FUNGU_STRING_LIBRARY} STATIC string.cpp)

add_subdirectory(net)

install(TARGETS
            ${FUNGU_STRING_LIBRARY}
        ARCHIVE DESTINATION
            ${INSTALL_ARCHIVE_DIR})

