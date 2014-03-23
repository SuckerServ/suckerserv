
set(GEOIP_LIBRARY 
    geoip 
    CACHE INTERNAL "geoip library")

set(GEOIP_INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE INTERNAL "geoip include directory")
    
set(GEOIP_SOURCES
    GeoIP.c
    GeoIPCity.c
)

add_library(${GEOIP_LIBRARY} SHARED ${GEOIP_SOURCES})

install(TARGETS 
            ${GEOIP_LIBRARY}
        LIBRARY DESTINATION
            ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION
            ${INSTALL_ARCHIVE_DIR})

add_definitions(-DGEOIPDATADIR="~" -DPACKAGE_VERSION="1.6.0")

