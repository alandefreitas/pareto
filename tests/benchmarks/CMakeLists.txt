#######################################################
### Google benchmark                                ###
#######################################################
CPMAddPackage(
        NAME benchmark
        GITHUB_REPOSITORY google/benchmark
        VERSION 1.5.1
        OPTIONS
        "BENCHMARK_ENABLE_TESTING Off"
        "LLVMAR_EXECUTABLE ON"
        "LLVMNM_EXECUTABLE ON"
        "LLVMRANLIB_EXECUTABLE ON"
        "GCC_AR ON"
        "GCC_RANLIB ON"
        "BENCHMARK_ENABLE_INSTALL OFF"
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
