cmake_minimum_required(VERSION 3.10)

project(fungu CXX)

set(FUNGU_INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    CACHE INTERNAL "public include directories for the fungu libraries")

add_subdirectory(src)

