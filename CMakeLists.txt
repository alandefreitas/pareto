cmake_minimum_required(VERSION 3.14)
project(pareto_front)
set(CMAKE_CXX_STANDARD 17)
include(cmake/CPM.cmake)

# Is this a master project or a subdirectory of another project
if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(MASTER_PROJECT ON)
else()
    set(MASTER_PROJECT OFF)
endif()
message("MASTER_PROJECT=${MASTER_PROJECT}")

# What to build
option(BUILD_EXAMPLES "Compile the examples" ${MASTER_PROJECT})
option(BUILD_PYTHON_BINDINGS "Compile the Python binding" ${MASTER_PROJECT})
option(BUILD_TESTS "Compile the tests" ${MASTER_PROJECT})

# How to build
option(BUILD_LONG_TESTS "Build the Data Structure Benchmark (It takes very long)" OFF)
option(BUILD_BOOST_TREE "Include R-Tree using Boost Geometry (Boost dependency)" OFF)
option(MAX_NUM_DIMENSIONS_PYTHON "Max number of compile-time dimensions for fronts in the Python bindings. Higher dimensions will be set at runtime." 10)
message("BUILD_LONG_TESTS=${BUILD_LONG_TESTS}")
message("MAX_NUM_DIMENSIONS_PYTHON=${MAX_NUM_DIMENSIONS_PYTHON}")

# Main library
add_subdirectory(sources)

# Python bindings
if (BUILD_PYTHON_BINDINGS)
    add_subdirectory(bindings)
endif()

# Examples
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests: unit tests and benchmarks
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()