# - Try to find enet
# Once done this will define
#
#  ENET_FOUND - system has enet
#  ENet_INCLUDE_DIR - the enet include directory
#  ENet_LIBRARY - the library needed to link against enet
#
# $ENETDIR is an environment variable used for finding enet.
#
#  Borrowed from The Mana World
#  http://themanaworld.org/
#
# Several changes and additions by Fabian 'x3n' Landau
# Lots of simplifications by Adrian Friedli and Reto Grieder
# Version checking by Reto Grieder
#                 > www.orxonox.net <
# Adaptations and simplifications for SuckerServ - piernov <piernov@piernov.org>

INCLUDE(FindPackageHandleStandardArgs)

FUNCTION(DETERMINE_VERSION _name _file)
  IF(EXISTS ${_file})
    FILE(READ ${_file} _file_content)
    SET(_parts ${ARGN})
    LIST(LENGTH _parts _parts_length)
    IF(NOT ${_parts_length} EQUAL 3)
      SET(_parts MAJOR MINOR PATCH)
    ENDIF(NOT ${_parts_length} EQUAL 3)

    FOREACH(_part ${_parts})
      STRING(REGEX MATCH "${_name}_VERSION_${_part} +([0-9]+)" _match ${_file_content})
      IF(_match)
        SET(${_name}_VERSION_${_part} ${CMAKE_MATCH_1})
        SET(${_name}_VERSION_${_part} ${CMAKE_MATCH_1} PARENT_SCOPE)
      ELSE(_match)
        SET(${_name}_VERSION_${_part} "0")
        SET(${_name}_VERSION_${_part} "0" PARENT_SCOPE)
      ENDIF(_match)
      IF("${_parts}" MATCHES "^${_part}") # First?
        SET(${_name}_VERSION "${${_name}_VERSION_${_part}}")
      ELSE("${_parts}" MATCHES "^${_part}")
        SET(${_name}_VERSION "${${_name}_VERSION}.${${_name}_VERSION_${_part}}")
      ENDIF("${_parts}" MATCHES "^${_part}")
    ENDFOREACH(_part)
    SET(${_name}_VERSION "${${_name}_VERSION}" PARENT_SCOPE)
  ENDIF(EXISTS ${_file})
ENDFUNCTION(DETERMINE_VERSION)

FIND_PATH(ENET_INCLUDE_DIR enet/enet.h
  PATHS $ENV{ENETDIR}
  PATH_SUFFIXES include
)
FIND_LIBRARY(ENET_LIBRARY
  NAMES enet
  PATHS $ENV{ENETDIR}
  PATH_SUFFIXES lib
)

# Only works for 1.2.2 and higher.
DETERMINE_VERSION(ENET ${ENET_INCLUDE_DIR}/enet/enet.h)

# Handle the REQUIRED argument and set ENET_FOUND
# Also check the the version requirements
IF(ENET_VERSION VERSION_LESS ENet_FIND_VERSION)
  SET(ENET_VERSION FALSE)
ELSE(ENET_VERSION VERSION_LESS ENet_FIND_VERSION)
  SET(ENET_VERSION TRUE)
ENDIF(ENET_VERSION VERSION_LESS ENet_FIND_VERSION)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ENet DEFAULT_MSG
  ENET_LIBRARY
  ENET_INCLUDE_DIR
  ENET_VERSION
)

MARK_AS_ADVANCED(
  ENET_INCLUDE_DIR
  ENET_LIBRARY
)
