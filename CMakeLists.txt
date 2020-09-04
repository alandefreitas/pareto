cmake_minimum_required(VERSION 3.14)
project(pareto_front)

set(CMAKE_CXX_STANDARD 17)

include(cmake/CPM.cmake)

option(BUILD_EXAMPLES "Compile the examples" ON)
option(BUILD_PYTHON_BINDINGS "Compile the Python binding" ON)
option(BUILD_TESTS "Compile the tests" ON)
option(BUILD_LONG_TESTS "Build the Data Structure Benchmark (It takes very long)" OFF)
option(BUILD_BOOST_TREE "Include R-Tree using Boost Geometry" OFF)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(MASTER_PROJECT ON)
else()
    set(MASTER_PROJECT OFF)
endif()

message("MASTER_PROJECT=${MASTER_PROJECT}")

add_subdirectory(sources)

if (MASTER_PROJECT)
    if (BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()
    if (BUILD_TESTS)
        add_subdirectory(tests)
    endif()
    if (BUILD_PYTHON_BINDINGS)
        add_subdirectory(bindings)
    endif()
endif()