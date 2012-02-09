set(BOOST_SYSTEM_LIBRARY boost_system CACHE INTERNAL "boost.system library")
set(BOOST_SYSTEM_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../ CACHE INTERNAL "boost.system include directories")

add_definitions(-DBOOST_SYSTEM_STATIC_LINK=1)
include_directories(${BOOST_SYSTEM_INCLUDE_DIRS})

add_library(${BOOST_SYSTEM_LIBRARY} STATIC src/error_code.cpp)

install(TARGETS 
            ${BOOST_SYSTEM_LIBRARY}
        LIBRARY DESTINATION
            ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION 
            ${INSTALL_ARCHIVE_DIR})

