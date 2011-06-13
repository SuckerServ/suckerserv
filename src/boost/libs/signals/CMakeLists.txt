set(BOOST_SIGNALS_LIBRARY boost_signals CACHE INTERNAL "boost.signals library")
set(BOOST_SIGNALS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../ CACHE INTERNAL "boost.signals include directories")

include_directories(${BOOST_SIGNALS_INCLUDE_DIRS})
add_definitions(-DBOOST_SIGNALS_DYN_LINK=1)

set(BOOST_SIGNALS_SOURCES
    src/connection.cpp
    src/named_slot_map.cpp
    src/signal_base.cpp
    src/slot.cpp
    src/trackable.cpp)

add_library(${BOOST_SIGNALS_LIBRARY} SHARED ${BOOST_SIGNALS_SOURCES})

install(TARGETS 
            ${BOOST_SIGNALS_LIBRARY}
        LIBRARY DESTINATION
            ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION 
            ${INSTALL_ARCHIVE_DIR})

