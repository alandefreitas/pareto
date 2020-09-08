# - Trase library
# This is a package for svg plots
# Future work: replace trase with matplotplusplus
CPMAddPackage(
        NAME trase
        GITHUB_REPOSITORY trase-cpp/trase
        GIT_TAG 607404cf4b36e0d7fcbe10ee2a2078de7c01d194
        VERSION 0.0.1
        OPTIONS
            "trase_BUILD_OPENGL OFF"
)

# Hypervolume computation in C
# http://lopez-ibanez.eu/hypervolume
# This is bundled because it's not on a git repository.
# It could go offline at anytime.
add_library(hv pareto_front/hv-2.0rc2/hv.h pareto_front/hv-2.0rc2/hv.c)
add_compile_definitions(hv VARIANT=4)
set_target_properties(hv PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

if (BUILD_BOOST_TREE)
    # - Geometry package
    # This is a boost library we only use for the benchmarks
    find_package(Boost REQUIRED)
endif()

find_package(Threads)

# Pareto front container
add_library(pareto_front INTERFACE)
target_include_directories(pareto_front INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pareto_front INTERFACE hv trase Threads::Threads)

# Retroactively make trase -fPIC
set_target_properties(trase PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

if (BUILD_BOOST_TREE)
    target_include_directories(pareto_front INTERFACE ${Boost_INCLUDE_DIR})
    target_compile_definitions(pareto_front INTERFACE BUILD_BOOST_TREE)
endif()