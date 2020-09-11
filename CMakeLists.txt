#######################################################
### Pareto fronts                                   ###
#######################################################
# Project information
cmake_minimum_required(VERSION 3.14)
project(pareto_front VERSION 1.1.0)
set(CMAKE_CXX_STANDARD 17)

# CMake dependencies for installer
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Include CPM to find or download packages
# Note that CPM will try to find the packages locally before downloading them
# This avoids ODR problems
option(CPM_USE_LOCAL_PACKAGES "Try `find_package` before downloading dependencies" ON)

include(cmake/CPM.cmake)
# Check if this a master project or a subdirectory of another project
if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(MASTER_PROJECT ON)
else()
    set(MASTER_PROJECT OFF)
endif()
message("MASTER_PROJECT=${MASTER_PROJECT}")
set(PARETOFRONTS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#######################################################
### Options                                         ###
#######################################################
# What to build
option(BUILD_EXAMPLES "Compile the examples" ${MASTER_PROJECT})
option(BUILD_PYTHON_BINDING "Compile the Python binding" ${MASTER_PROJECT})
option(BUILD_TESTS "Compile the tests" ${MASTER_PROJECT})
option(BUILD_INSTALLER "Build an installation package" ${MASTER_PROJECT})
option(BUILD_PACKAGE "Build an installation package" ${MASTER_PROJECT})

# How to build
option(BUILD_LONG_TESTS "Build the Data Structure Benchmark (It takes very long)" ON)

option(BUILD_BOOST_TREE "Include R-Tree using Boost Geometry (Boost dependency)" OFF)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(DEBUG_MODE ON)
    set(NOT_DEBUG_MODE OFF)
else()
    set(DEBUG_MODE OFF)
    set(NOT_DEBUG_MODE ON)
endif()
if (DEBUG_MODE)
    set(DEFAULT_MAX_NUM_DIMENSIONS_PYTHON 3)
else()
    set(DEFAULT_MAX_NUM_DIMENSIONS_PYTHON 10)
endif()
option(MAX_NUM_DIMENSIONS_PYTHON "Max number of compile-time dimensions for fronts in the Python bindings. Higher dimensions will be set at runtime." ${DEFAULT_MAX_NUM_DIMENSIONS_PYTHON})
if (NOT MAX_NUM_DIMENSIONS_PYTHON)
    set(MAX_NUM_DIMENSIONS_PYTHON 10)
endif()
message("MAX_NUM_DIMENSIONS_PYTHON=${MAX_NUM_DIMENSIONS_PYTHON}")

option(BUILD_BINDING_FOR_ALL_STRUCTURES "Make all data structures available in the Python bindings" ON)

option(BUILD_FRONTS_WITH_TRASE "Trase includes functions to plot pareto fronts. These were only used for the documentation." OFF)

option(OPTIMIZE_RELEASE_CODE_BY_DEFAULT "Turn on optimization flags by default if we are in Release" ON)
if (OPTIMIZE_RELEASE_CODE_BY_DEFAULT)
    if (MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    endif()
endif()

option(BUILD_WITH_PEDANTIC_WARNINGS "Use pedantic warnings. This is useful for developers because many of these warnings will be in continuous integration anyway." ${DEBUG_MODE})
option(BUILD_WITH_UTF8 "Accept utf-8 in MSVC by default." ON)
if (BUILD_WITH_UTF8 AND MSVC)
    set(CMAKE_CXX_FLAGS "/utf-8")
endif()

# Hack to check for min in Windows.h
# http://www.suodenjoki.dk/us/archive/2010/min-max.htm
include(CheckSymbolExists)
check_symbol_exists(min "Windows.h" HAVE_WINDOWS_MINMAX)
if (HAVE_WINDOWS_MINMAX)
    add_compile_definitions(NOMINMAX)
endif()

#######################################################
### Libraries                                       ###
#######################################################
# Main library
add_subdirectory(sources)

# Python bindings
if (BUILD_PYTHON_BINDING)
    add_subdirectory(bindings)
endif()

#######################################################
### Examples and tests                              ###
#######################################################
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

#######################################################
### Installer                                       ###
#######################################################
# Create and install pareto-fronts-config.cmake
if (BUILD_INSTALLER)
    # https://cliutils.gitlab.io/modern-cmake/chapters/install/installing.html
    # Set variable where the cmake config is
    set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/ParetoFront)

    # Create ParetoFrontConfigVersion.cmake and install it
    write_basic_package_version_file(
            ParetoFrontConfigVersion.cmake
            VERSION ${PACKAGE_VERSION}
            COMPATIBILITY AnyNewerVersion
    )

    # Create ParetoFrontConfig.cmake from ParetoFrontConfig.cmake.in
    # ParetoFrontConfig.cmake will include the ParetoFrontTargets.cmake file
    # We could have just renamed ParetoFrontTargets.cmake to ParetoFrontConfig.cmake
    # But ParetoFrontConfig.cmake allows us to include extra dependencies
    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/ParetoFrontConfig.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/ParetoFrontConfig.cmake
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ParetoFront)

    # Install the file ParetoFrontConfig.cmake
    install(FILES
                ${CMAKE_CURRENT_BINARY_DIR}/ParetoFrontConfig.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/ParetoFrontConfigVersion.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ParetoFront)
endif()

#######################################################
### Packages                                        ###
#######################################################
if (BUILD_INSTALLER AND BUILD_PACKAGE)
    # Set the cpack variables
    # https://cliutils.gitlab.io/modern-cmake/chapters/install/packaging.html

    # The most common cpack variables
    set(CPACK_PACKAGE_NAME "pareto-front")
    set(CPACK_PACKAGE_VENDOR "https://github.com/alandefreitas")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Pareto Fronts and Archives for C++/Python")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

    # Set CPACK_SOURCE_IGNORE_FILES with files source packages shouldn't install
    # We get these from .gitignore to avoid redundancy
    FILE(READ .gitignore GITIGNORE_CONTENTS)
    STRING(REGEX REPLACE ";" "\\\\;" GITIGNORE_CONTENTS "${GITIGNORE_CONTENTS}")
    STRING(REGEX REPLACE "\n" ";" GITIGNORE_CONTENTS "${GITIGNORE_CONTENTS}")
    set(CPACK_SOURCE_IGNORE_FILES ${GITIGNORE_CONTENTS})

    # Always include CPack at last
    include(CPack)
endif()