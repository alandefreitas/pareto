#######################################################
### Google benchmark                                ###
#######################################################
# Force some options for the google benchmark library
if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DNDEBUG)
    set(LLVMAR_EXECUTABLE ON CACHE BOOL "Release mode" FORCE)
    set(LLVMNM_EXECUTABLE ON CACHE BOOL "Release mode" FORCE)
    set(LLVMRANLIB_EXECUTABLE ON CACHE BOOL "Release mode" FORCE)
    set(GCC_AR ON CACHE BOOL "Release mode" FORCE)
    set(GCC_RANLIB ON CACHE BOOL "Release mode" FORCE)
endif()
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Enable installation of benchmark" FORCE)

# Find or get the google benchmark library
# Note that CPM will try to find(benchmark) before downloading it
CPMAddPackage(
        NAME benchmark
        GITHUB_REPOSITORY google/benchmark
        VERSION 1.5.1
        OPTIONS
        "BENCHMARK_ENABLE_TESTING Off"
)

if (benchmark_ADDED)
    # patch google benchmark target
    set_target_properties(benchmark PROPERTIES CXX_STANDARD 17)
    target_compile_definitions(benchmark PUBLIC -DNDEBUG)
endif()


#######################################################
### Pareto front benchmarks                         ###
#######################################################
# run with "--benchmark_repetitions=30 --benchmark_display_aggregates_only=true --benchmark_out=data_structure_benchmark.csv --benchmark_out_format=csv"
add_executable(pareto_front_hv_benchmark hv_benchmark.cpp)
target_link_libraries(pareto_front_hv_benchmark PUBLIC pareto benchmark)
if (BUILD_LONG_TESTS)
    target_compile_definitions(pareto_front_hv_benchmark PUBLIC BUILD_LONG_TESTS)
endif()
if (MSVC)
    # Allow MSVC to compile such a large code
    target_compile_options(pareto_front_hv_benchmark PRIVATE /bigobj)
    # MSVC requires this flag if the code uses C++ exception handling
    target_compile_options(pareto_front_hv_benchmark PRIVATE /EHsc)
endif()

#######################################################
### Pareto archive benchmarks                       ###
#######################################################
add_executable(pareto_front_data_structure_benchmark data_structure_benchmark.cpp)
target_link_libraries(pareto_front_data_structure_benchmark PUBLIC pareto benchmark)
if (BUILD_LONG_TESTS)
    target_compile_definitions(pareto_front_data_structure_benchmark PUBLIC BUILD_LONG_TESTS)
endif()
if (MSVC)
    # Allow MSVC to compile such a large code
    target_compile_options(pareto_front_data_structure_benchmark PRIVATE /bigobj)
    # MSVC requires this flag if the code uses C++ exception handling
    target_compile_options(pareto_front_data_structure_benchmark PRIVATE /EHsc)
endif()

#######################################################
### Memory allocation benchmarks                    ###
#######################################################
# Memory allocators make a huge difference for sets. Not so much for other data structures.
# This experiment depends on some C++ features that many compilers don't implement
include(CheckIncludeFileCXX)
check_include_file_cxx(bit HAS_BIT)
if (HAS_BIT AND NOT MSVC)
    add_executable(pareto_front_memory_resource_benchmark monotonic_resource.cpp memory_resource_benchmark.cpp)
    target_link_libraries(pareto_front_memory_resource_benchmark PUBLIC pareto benchmark)
    if (MSVC)
        # Allow MSVC to compile such a large code
        target_compile_options(pareto_front_memory_resource_benchmark PRIVATE /bigobj)
        # MSVC requires this flag if the code uses C++ exception handling
        target_compile_options(pareto_front_memory_resource_benchmark PRIVATE /EHsc)
    endif()
endif()
