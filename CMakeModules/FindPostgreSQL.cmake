# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#	Note:
#		Searching headers and libraries is very simple and is NOT as powerful as scripts
#		distributed with CMake, because LuaDist defines directories to search for.
#		Everyone is encouraged to contact the author with improvements. Maybe this file
#		becomes part of CMake distribution sometimes.

# - Find PostgreSQL
# Find the native PostgreSQL headers and libraries.
#
# POSTGRESQL_INCLUDE_DIRS	- where to find libpq-fe.h, etc.
# POSTGRESQL_LIBRARIES	- List of libraries when using PostgreSQL.
# POSTGRESQL_FOUND	- True if PostgreSQL found.

# Look for the header file.
#~ FIND_PATH(POSTGRESQL_INCLUDE_DIR NAMES libpq-fe.h)
	FIND_PATH(POSTGRESQL_INCLUDE_DIR postgres.h
		/usr/include/server
		/usr/include/pgsql/server
		/usr/local/include/pgsql/server
		/usr/include/postgresql
		/usr/include/postgresql/server
		/usr/include/postgresql/*/server
		/usr/local/include/postgresql
		/usr/local/include/postgresql/server
		/usr/local/include/postgresql/*/server
		$ENV{ProgramFiles}/PostgreSQL/*/include/server
		$ENV{SystemDrive}/PostgreSQL/*/include/server
	)


# Look for the library.
#FIND_LIBRARY(POSTGRESQL_LIBRARY NAMES pq)

	FIND_LIBRARY(POSTGRESQL_LIBRARY NAMES pq libpq
		PATHS
		/usr/lib
		/usr/local/lib
		/usr/lib/postgresql
		/usr/lib64
		/usr/local/lib64
		/usr/lib64/postgresql
		$ENV{ProgramFiles}/PostgreSQL/*/lib/ms
		$ENV{SystemDrive}/PostgreSQL/*/lib/ms
	)

# Handle the QUIETLY and REQUIRED arguments and set POSTGRESQL_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(POSTGRESQL DEFAULT_MSG POSTGRESQL_LIBRARY POSTGRESQL_INCLUDE_DIR)

# Copy the results to the output variables.
IF(POSTGRESQL_FOUND)
	SET(POSTGRESQL_LIBRARIES ${POSTGRESQL_LIBRARY})
	SET(POSTGRESQL_INCLUDE_DIRS ${POSTGRESQL_INCLUDE_DIR})
ELSE(POSTGRESQL_FOUND)
	SET(POSTGRESQL_LIBRARIES)
	SET(POSTGRESQL_INCLUDE_DIRS)
ENDIF(POSTGRESQL_FOUND)

MARK_AS_ADVANCED(POSTGRESQL_INCLUDE_DIRS POSTGRESQL_LIBRARIES)
