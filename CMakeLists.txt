cmake_minimum_required(VERSION 3.10)

project(SuckerServ)

include(GNUInstallDirs)

set(CMAKE_MODULE_PATH 
        ${CMAKE_MODULE_PATH} 
        ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

add_definitions(-Wall -Wextra -Wno-unused-parameter)

add_subdirectory(src)

