cmake_minimum_required(VERSION 3.10)

project(SuckerServ VERSION 6)

include(GNUInstallDirs)

set(CMAKE_MODULE_PATH 
        ${CMAKE_MODULE_PATH} 
        ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

# Build revision
if (NOT DEFINED SUCKERSERV_REVISION)
	if(EXISTS "${PROJECT_SOURCE_DIR}/.git/")
		execute_process(
			COMMAND git rev-list HEAD --count
			OUTPUT_VARIABLE GIT_REVISION
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		execute_process(
			COMMAND git name-rev --name-only HEAD
			OUTPUT_VARIABLE GIT_BRANCH
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		execute_process(
			COMMAND git config branch.${GIT_BRANCH}.remote
			OUTPUT_VARIABLE GIT_REMOTE
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		execute_process(
			COMMAND git config remote.${GIT_REMOTE}.url
			OUTPUT_VARIABLE GIT_URL
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		string(REGEX REPLACE ".*/(.+)/.+$" "\\1" GIT_REPO "${GIT_URL}") # Extract the user from a Github repo URL
		set(SUCKERSERV_REVISION "${GIT_REVISION}")
		set(SUCKERSERV_SOURCE_URL "${GIT_REPO}/${GIT_BRANCH}")
	else()
		set(SUCKERSERV_REVISION "${PROJECT_VERSION}")
	endif()
endif()
message("${PROJECT_NAME} release: ${SUCKERSERV_REVISION} ${SUCKERSERV_SOURCE_URL}")

add_definitions(-Wall -Wextra -Wno-unused-parameter)

add_subdirectory(src)

