# - Find MMDB
# Find the MMDB libraries
#
#  This module defines the following variables:
#     MMDB_FOUND       - True if MMDB_INCLUDE_DIR & MMDB_LIBRARY are found
#     MMDB_LIBRARIES   - Set when MMDB_LIBRARY is found
#     MMDB_INCLUDE_DIRS - Set when MMDB_INCLUDE_DIR is found
#
#     MMDB_INCLUDE_DIR - where to find maxminddb.h.
#     MMDB_LIBRARY     - the MMDB library
#

#=============================================================================
# Copyright 2014 piernov <piernov@piernov.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(MMDB_INCLUDE_DIR NAMES maxminddb.h)

find_library(MMDB_LIBRARY NAMES maxminddb)

# handle the QUIETLY and REQUIRED arguments and set MMDB_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MMDB DEFAULT_MSG MMDB_LIBRARY MMDB_INCLUDE_DIR)

if(MMDB_FOUND)
  set(MMDB_LIBRARIES ${MMDB_LIBRARY})
  set(MMDB_INCLUDE_DIRS ${MMDB_INCLUDE_DIR})
endif()

mark_as_advanced(MMDB_INCLUDE_DIR MMDB_LIBRARY)
