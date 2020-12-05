#######################################################
### Dependencies                                    ###
#######################################################
# Hypervolume computation in C
# http://lopez-ibanez.eu/hypervolume
# This is bundled because it's not on a git repository.
# It could go offline at anytime.
add_library(hv pareto/hv-2.0rc2/hv.h pareto/hv-2.0rc2/hv.c)
target_compile_definitions(hv PRIVATE VARIANT=4)
set_target_properties(hv PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# - Geometry package
# This is a boost library we only use for the benchmarks
# This tree type is now deprecated and will be removed from
# later versions
if (BUILD_BOOST_TREE)
    find_package(Boost REQUIRED)
endif ()

find_package(Threads)

#######################################################
### Library                                         ###
#######################################################
add_library(pareto INTERFACE)

target_include_directories(pareto
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(pareto INTERFACE hv Threads::Threads)
# Set warning levels to about the same level for MSVC, GCC, and Clang
if (BUILD_WITH_PEDANTIC_WARNINGS)
    if (MSVC)
        target_compile_options(pareto INTERFACE /W4 /WX)
    else()
        # -Wno-self-assign-overloaded has a false positive that prevents pybind11
        # from working
        target_compile_options(pareto INTERFACE -Wall -Wextra -pedantic -Werror -Wno-self-assign-overloaded)
    endif()
endif()

if (BUILD_BOOST_TREE)
    target_include_directories(pareto INTERFACE ${Boost_INCLUDE_DIR})
    target_compile_definitions(pareto INTERFACE BUILD_BOOST_TREE)
    target_compile_options(pareto INTERFACE -Wno-\#pragma-messages -Wno-unused-parameter -Wno-deprecated-declarations)
endif()

#######################################################
### Installer                                       ###
#######################################################
if(BUILD_INSTALLER)
    # Install hv and pareto targets
    # Because pareto is INTERFACE, this installs no artifacts
    # but includes pareto in the EXPORT
    install(TARGETS hv pareto
            EXPORT ParetoFrontTargets
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
    install(EXPORT ParetoFrontTargets
            FILE ParetoFrontTargets.cmake
            NAMESPACE ParetoFrontTargets::
            COMPONENT "CPP_Library"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ParetoFront
            )
endif()

