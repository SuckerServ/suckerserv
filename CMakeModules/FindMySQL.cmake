#--------------------------------------------------------
# Copyright (C) 1995-2007 MySQL AB
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
#
# There are special exceptions to the terms and conditions of the GPL
# as it is applied to this software. View the full text of the exception
# in file LICENSE.exceptions in the top-level directory of this software
# distribution.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# The MySQL Connector/ODBC is licensed under the terms of the
# GPL, like most MySQL Connectors. There are special exceptions
# to the terms and conditions of the GPL as it is applied to
# this software, see the FLOSS License Exception available on
# mysql.com.

##########################################################################


#-------------- FIND MYSQL_INCLUDE_DIR ------------------
FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
		$ENV{MYSQL_INCLUDE_DIR}
		$ENV{MYSQL_DIR}/include
		/usr/include/mysql
		/usr/local/include/mysql
		/opt/mysql/mysql/include
		/opt/mysql/mysql/include/mysql
		/opt/mysql/include
		/opt/local/include/mysql5
		/usr/local/mysql/include
		/usr/local/mysql/include/mysql
		$ENV{ProgramFiles}/MySQL/*/include
		$ENV{SystemDrive}/MySQL/*/include)

#----------------- FIND MYSQL_LIBRARY -------------------
FIND_LIBRARY(MYSQL_LIBRARY NAMES mysqlclient libmysqlclient
			 PATHS
			 $ENV{MYSQL_DIR}/libmysql_r/.libs
			 $ENV{MYSQL_DIR}/lib
			 $ENV{MYSQL_DIR}/lib/mysql
			 /usr/lib64/mysql
			 /usr/lib/mysql
			 /usr/local/lib/mysql
			 /usr/local/mysql/lib
			 /usr/local/mysql/lib/mysql
			 /opt/local/mysql5/lib
			 /opt/local/lib/mysql5/mysql
			 /opt/mysql/mysql/lib/mysql
			 /opt/mysql/lib/mysql)

IF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
	SET(MYSQL_FOUND TRUE)
	SET(MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR})
	SET(MYSQL_LIBRARIES ${MYSQL_LIBRARY})

	MESSAGE(STATUS "MySQL Include dir: ${MYSQL_INCLUDE_DIRS}  library: ${MYSQL_LIBRARIES}")
ELSE (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
	MESSAGE(STATUS "Cannot find MySQL. Include dir: ${MYSQL_INCLUDE_DIR}  library: ${MYSQL_LIBRARY}")
ENDIF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)