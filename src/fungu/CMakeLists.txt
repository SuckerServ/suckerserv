project(fungu CXX)

cmake_minimum_required(VERSION 2.6)

set(FUNGU_INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    CACHE INTERNAL "public include directories for the fungu libraries")

add_subdirectory(src)

