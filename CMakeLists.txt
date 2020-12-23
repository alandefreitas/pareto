#######################################################
### Pareto fronts                                   ###
#######################################################
# Project information
cmake_minimum_required(VERSION 3.16)
project(pareto VERSION 1.2.0)
set(CMAKE_CXX_STANDARD 17)
set(PARETO_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# CMake dependencies for installer
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(cmake/functions.cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Include CPM to find or download packages
# Note that CPM will try to find the packages locally before downloading them
# This avoids ODR problems
option(CPM_USE_LOCAL_PACKAGES "Try `find_package` before downloading dependencies" ON)
include(cmake/CPM.cmake)


#######################################################
### Options                                         ###
#######################################################
# Check if this a master project or a subdirectory of another project
set_master_project_vars()
# Check if we are building in DEBUG mode
set_debug_booleans()

# What to build
option(BUILD_EXAMPLES "Compile the examples" ${MASTER_PROJECT})
option(BUILD_PYTHON_BINDING "Compile the Python binding" ${MASTER_PROJECT})
option(BUILD_TESTS "Compile the tests" ${MASTER_PROJECT})
option(BUILD_TARGET_WITH_ALL_TESTS "Compile the tests" OFF)
option(BUILD_BENCHMARKS "Compile the benchmarks" ON)
option(BUILD_MATPLOT_TARGETS "Compile the targets that depend on matplot++" ON)
option(BUILD_INSTALLER "Build an installation package" ${MASTER_PROJECT})
option(BUILD_PACKAGE "Build an installation package" ${MASTER_PROJECT})

# How to build tests
option(BUILD_LONG_TESTS "Build the Data Structure Benchmark (It takes very long)" ON)
option(BUILD_BOOST_TREE "Include R-Tree using Boost.Geometry (Boost dependency). Deprecated: see pareto/boost_tree.h" OFF)
option(BUILD_PRECOMPILED_HEADERS "Build with address, thread, and undefined sanitizers" OFF)

# What compile options to use
option(BUILD_WITH_PEDANTIC_WARNINGS "Use pedantic warnings. Developers should leave this ON." ${DEBUG_MODE})
option(THREADS_PREFER_PTHREAD_FLAG "The use of the -pthread compiler and linker flag is preferred" ON)
option(BUILD_WITH_UTF8 "Accept utf-8 in MSVC by default." ON)
option(BUILD_WITH_ALL_SANITIZERS "Build with address, thread, and undefined sanitizers" ${DEBUG_MODE})

# Apply UFT-8
if (BUILD_WITH_UTF8 AND MSVC)
    set(CMAKE_CXX_FLAGS "/utf-8")
endif ()

# Apply satinizers
if (BUILD_WITH_ALL_SANITIZERS)
    add_all_sanitizers()
endif()

# Include cotire if pre-compiled headers will be used
if (BUILD_PRECOMPILED_HEADERS)
    include(cotire)
endif()

# Hack to check for min in Windows.h
# http://www.suodenjoki.dk/us/archive/2010/min-max.htm
include(CheckSymbolExists)
check_symbol_exists(min "Windows.h" HAVE_WINDOWS_MINMAX)
if (HAVE_WINDOWS_MINMAX)
    add_compile_definitions(NOMINMAX)
endif ()

#######################################################
### Global external libraries                       ###
#######################################################
# We might need matplot++ for examples or tests
if ((BUILD_EXAMPLES OR BUILD_TESTS) AND BUILD_MATPLOT_TARGETS)
    find_package(Matplot++ QUIET)
endif ()

#######################################################
### Libraries                                       ###
#######################################################
# Main library
add_subdirectory(source)

# Python bindings
if (BUILD_PYTHON_BINDING)
    add_subdirectory(pybindings)
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
# Create and install pareto-config.cmake
if (BUILD_INSTALLER)
    # https://cliutils.gitlab.io/modern-cmake/chapters/install/installing.html
    # Set variable where the cmake config is
    set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Pareto)

    # Create ParetoConfigVersion.cmake and install it
    write_basic_package_version_file(
            ParetoConfigVersion.cmake
            VERSION ${PACKAGE_VERSION}
            COMPATIBILITY AnyNewerVersion
    )

    # Create ParetoConfig.cmake from ParetoConfig.cmake.in
    # ParetoConfig.cmake will include the ParetoTargets.cmake file
    # We could have just renamed ParetoTargets.cmake to ParetoConfig.cmake
    # But ParetoConfig.cmake allows us to include extra dependencies
    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/ParetoConfig.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/ParetoConfig.cmake
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pareto)

    # Install the file ParetoConfig.cmake
    install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/ParetoConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/ParetoConfigVersion.cmake
            COMPONENT "CPP_Library"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pareto
            )
endif()

#######################################################
### Packages                                        ###
#######################################################
if (BUILD_INSTALLER AND BUILD_PACKAGE)
    # Set the cpack variables
    # https://cliutils.gitlab.io/modern-cmake/chapters/install/packaging.html

    # The most common cpack variables
    set(CPACK_PACKAGE_NAME "pareto")
    set(CPACK_PACKAGE_VENDOR "https://github.com/alandefreitas")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Spatial Containers, Pareto Fronts and Pareto Archives")
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