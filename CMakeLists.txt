cmake_minimum_required(VERSION 3.14)
project(pareto_front)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
include(cmake/CPM.cmake)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(MASTER_PROJECT ON)
else()
    set(MASTER_PROJECT OFF)
endif()

message("MASTER_PROJECT=${MASTER_PROJECT}")

add_subdirectory(sources)

if (MASTER_PROJECT)
    add_subdirectory(examples)
    add_subdirectory(tests)
    add_subdirectory(bindings)
endif()