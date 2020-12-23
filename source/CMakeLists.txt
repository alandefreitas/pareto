#######################################################
### Dependencies                                    ###
#######################################################
# - Geometry package
# This is a boost library we only use for the benchmarks
# This containers type is now deprecated and will be removed from
# later versions
if (BUILD_BOOST_TREE)
    find_package(Boost REQUIRED)
endif()

# Look for pthread
find_package(Threads QUIET)

# Look for PMR headers and test if they are working
find_package(PMR)

#######################################################
### Library                                         ###
#######################################################
add_library(pareto INTERFACE)

target_include_directories(pareto
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(pareto INTERFACE Threads::Threads)

#######################################################
### Apply options                                   ###
#######################################################
# Set warning levels
if (BUILD_WITH_PEDANTIC_WARNINGS)
    if (MSVC)
        target_compile_options(pareto INTERFACE /W4 /WX)
    else()
        # -Wno-self-assign-overloaded has a false positive that prevents pybind11
        # from working
        target_compile_options(pareto INTERFACE -Wall -Wextra -pedantic -Werror -Wno-self-assign-overloaded)
    endif()
endif()

# Set macro to include PMR
if (PMR_FOUND)
    target_compile_definitions(pareto INTERFACE BUILD_PARETO_WITH_PMR)
endif()

# Set macro to include Boost.Geometry
if (BUILD_BOOST_TREE)
    target_include_directories(pareto INTERFACE ${Boost_INCLUDE_DIR})
    target_compile_definitions(pareto INTERFACE BUILD_BOOST_TREE BOOST_ALLOW_DEPRECATED_HEADERS)
    # Ignore the errors in Boost.Geometry
    target_compile_options(pareto INTERFACE -Wno-unused-parameter -Wno-deprecated-declarations)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(pareto INTERFACE -Wno-error=class-memaccess)
    endif()
endif()

if (BUILD_PRECOMPILED_HEADERS)
    cotire(pareto)
endif()

if (MSVC)
    # MSVC requires this flag if the code uses C++ exception handling
    target_compile_options(pareto INTERFACE /EHsc)
endif()

#######################################################
### Installer                                       ###
#######################################################
if(BUILD_INSTALLER)
    # Install pareto targets
    # Because pareto is INTERFACE, this installs no artifacts
    # but includes pareto in the EXPORT
    install(TARGETS pareto
            EXPORT ParetoTargets
            COMPONENT "CPP_Library"
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )

    # Install headers
    install(DIRECTORY ${PARETO_ROOT_DIR}/source/
            COMPONENT "CPP_Library"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.h"
            )

    # Install cmake script that imports the targets
    install(EXPORT ParetoTargets
            FILE ParetoTargets.cmake
            NAMESPACE ParetoTargets::
            COMPONENT "CPP_Library"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pareto
            )
endif()

