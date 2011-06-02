set(BOOST_THREAD_LIBRARY boost_thread CACHE INTERNAL "boost.thread library")
set(BOOST_THREAD_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../ CACHE INTERNAL "boost.thread include directories")

add_definitions(-DBOOST_SYSTEM_STATIC_LINK=1 -DBOOST_THREAD_BUILD_LIB=1)
include_directories(${BOOST_SYSTEM_INCLUDE_DIRS})

if (WIN32)
  set(THREAD_SOURCES src/win32/thread.cpp src/win32/exceptions.cpp src/win32/tss_dll.cpp
                     src/win32/tss_pe.cpp)
else (WIN32)
  set(THREAD_SOURCES src/pthread/thread.cpp src/pthread/exceptions.cpp src/pthread/once.cpp)
endif (WIN32)

add_library(${BOOST_THREAD_LIBRARY} STATIC ${THREAD_SOURCES})

install(TARGETS 
            ${BOOST_THREAD_LIBRARY}
        LIBRARY DESTINATION
            ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION 
            ${INSTALL_ARCHIVE_DIR})

