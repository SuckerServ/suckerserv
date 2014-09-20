# - Find Asio
# Find the Asio libraries
#
#  This module defines the following variables:
#     ASIO_FOUND       - True if ASIO_INCLUDE_DIR are found
#     ASIO_INCLUDE_DIRS - Set when ASIO_INCLUDE_DIR is found
#
#     ASIO_INCLUDE_DIR - where to find maxminddb.h.
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

find_path(ASIO_INCLUDE_DIR NAMES asio.hpp)

# handle the QUIETLY and REQUIRED arguments and set ASIO_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Asio DEFAULT_MSG ASIO_INCLUDE_DIR)

if(ASIO_FOUND)
  set(ASIO_INCLUDE_DIRS ${ASIO_INCLUDE_DIR})
endif()

mark_as_advanced(ASIO_INCLUDE_DIR)
