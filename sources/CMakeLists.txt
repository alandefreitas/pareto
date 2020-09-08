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
add_library(hv pareto_front/hv-2.0rc2/hv.h pareto_front/hv-2.0rc2/hv.c)
target_compile_definitions(hv PRIVATE VARIANT=4)
set_target_properties(hv PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

if (BUILD_BOOST_TREE)
    # - Geometry package
    # This is a boost library we only use for the benchmarks
    find_package(Boost REQUIRED)
endif()

find_package(Threads)

# Pareto front container
add_library(pareto_front INTERFACE)

target_include_directories(pareto_front
        INTERFACE $<BUILD_INTERFACE:${PARETOFRONTS_ROOT_DIR}/sources>
                  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_include_directories(pareto_front INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(pareto_front INTERFACE hv Threads::Threads)
if (BUILD_FRONTS_WITH_TRASE)
    target_link_libraries(pareto_front INTERFACE trase)
    # Retroactively make trase -fPIC
    set_target_properties(trase PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
    target_compile_definitions(pareto_front INTERFACE BUILD_FRONTS_WITH_TRASE)
endif()

if (BUILD_BOOST_TREE)
    target_include_directories(pareto_front INTERFACE ${Boost_INCLUDE_DIR})
    target_compile_definitions(pareto_front INTERFACE BUILD_BOOST_TREE)
endif()

# Install
if(BUILD_INSTALLER)
    # Install headers
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.h"
        )

    # Install targets
    install(TARGETS hv
            EXPORT pareto_frontTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )

    # Install cmake script
    install(EXPORT pareto_frontTargets
            FILE ParetoFrontsTargets.cmake
            NAMESPACE ParetoFronts::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ParetoFronts
        )
endif()

# Set warnings to the same level for MSVC
#if (MSVC)
#  target_compile_options(pareto_front INTERFACE /W4 /WX)
#else()
#  target_compile_options(pareto_front INTERFACE -Wall -Wextra -pedantic -Werror)
#endif()