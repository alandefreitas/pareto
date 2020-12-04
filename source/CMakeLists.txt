#######################################################
### Dependencies                                    ###
#######################################################
# - Trase library
# This is a package for svg plots
# We only use this for the documentation
# Future work: replace trase with matplotplusplus
if (BUILD_FRONTS_WITH_TRASE)
    CPMAddPackage(
            NAME trase
            GITHUB_REPOSITORY trase-cpp/trase
            GIT_TAG 607404cf4b36e0d7fcbe10ee2a2078de7c01d194
            VERSION 0.0.1
            OPTIONS
            "trase_BUILD_OPENGL OFF"
    )
endif()

# Hypervolume computation in C
# http://lopez-ibanez.eu/hypervolume
# This is bundled because it's not on a git repository.
# It could go offline at anytime.
add_library(hv pareto/hv-2.0rc2/hv.h pareto/hv-2.0rc2/hv.c)
target_compile_definitions(hv PRIVATE VARIANT=4)
set_target_properties(hv PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

if (BUILD_BOOST_TREE)
    # - Geometry package
    # This is a boost library we only use for the benchmarks
    find_package(Boost REQUIRED)
endif()

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
    target_compile_options(pareto INTERFACE -Wno-\#pragma-messages -Wno-unused-parameter)
endif()

if (BUILD_FRONTS_WITH_TRASE)
    target_link_libraries(pareto INTERFACE trase)
    # Retroactively make trase -fPIC
    set_target_properties(trase PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
    target_compile_definitions(pareto INTERFACE BUILD_FRONTS_WITH_TRASE)
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

