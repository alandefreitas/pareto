cmake_minimum_required(VERSION 3.14)
project(pareto_front VERSION 1.1.0)
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
option(BUILD_PYTHON_BINDING "Compile the Python binding" ${MASTER_PROJECT})
message("BUILD_PYTHON_BINDING=${BUILD_PYTHON_BINDING}")
option(BUILD_TESTS "Compile the tests" ${MASTER_PROJECT})
option(BUILD_INSTALLER "Build an installation package" OFF)
option(BUILD_PACKAGE "Build an installation package" OFF)

# How to build
option(BUILD_LONG_TESTS "Build the Data Structure Benchmark (It takes very long)" OFF)
message("BUILD_LONG_TESTS=${BUILD_LONG_TESTS}")
option(BUILD_BOOST_TREE "Include R-Tree using Boost Geometry (Boost dependency)" OFF)
option(MAX_NUM_DIMENSIONS_PYTHON "Max number of compile-time dimensions for fronts in the Python bindings. Higher dimensions will be set at runtime." 10)
if (NOT MAX_NUM_DIMENSIONS_PYTHON)
    set(MAX_NUM_DIMENSIONS_PYTHON 10)
endif()
option(BUILD_BINDING_FOR_ALL_STRUCTURES "Make all data structures available in the Python bindings" ON)
message("MAX_NUM_DIMENSIONS_PYTHON=${MAX_NUM_DIMENSIONS_PYTHON}")

# Main library
add_subdirectory(sources)

# Python bindings
if (BUILD_PYTHON_BINDING)
    add_subdirectory(bindings)
endif()

# Examples
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests: unit tests and benchmarks
if (BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

# Packaging
if (BUILD_INSTALLER AND BUILD_PACKAGE)
    # https://cliutils.gitlab.io/modern-cmake/chapters/install/packaging.html
    include(CPack)
    set(CPACK_PACKAGE_NAME "pareto-front")
    set(CPACK_PACKAGE_VENDOR "https://github.com/alandefreitas")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Pareto Fronts and Archives for C++ and Python")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    include(CPack)
endif()