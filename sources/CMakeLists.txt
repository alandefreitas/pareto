# Pareto front has 3 dependencies:
# - Geometry package
#       This is a boost library. We include a subset of Boost with
#       only with libraries we need.
# - Trase library
#       This package will be downloaded automatically.
# - hv-2.0rc2/hv.h for calculating hyper-volumes
#       This is included because it's not on a git repository.

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
add_library(hv pareto_front/hv-2.0rc2/hv.h pareto_front/hv-2.0rc2/hv.c)
add_compile_definitions(hv VARIANT=4)

# Pareto front container
add_library(pareto_front INTERFACE)
target_include_directories(pareto_front INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pareto_front INTERFACE hv trase)