#######################################################
### Google benchmark                                ###
#######################################################
CPMAddPackage(
        NAME benchmark
        GITHUB_REPOSITORY google/benchmark
        VERSION 1.5.0
        OPTIONS
        "BENCHMARK_ENABLE_TESTING Off"
)

if (benchmark_ADDED)
    # patch google benchmark target
    set_target_properties(benchmark PROPERTIES CXX_STANDARD 17)
    target_compile_options(benchmark PRIVATE -Wno-error=deprecated-declarations)
endif()

#######################################################
### PMR benchmarks                                  ###
#######################################################
add_executable(pmr_benchmark pmr_benchmark.cpp)
target_link_libraries(pmr_benchmark PRIVATE pareto benchmark)
target_bigobj_options(pmr_benchmark)
target_exception_options(pmr_benchmark)

#######################################################
### Data structures + Pareto benchmarks             ###
#######################################################
# run with "./containers_benchmark --benchmark_repetitions=30 --benchmark_display_aggregates_only=true --benchmark_out=containers_benchmark.json --benchmark_out_format=json"
add_executable(containers_benchmark containers_benchmark.cpp)
target_link_libraries(containers_benchmark PRIVATE pareto benchmark)
target_bigobj_options(containers_benchmark)
target_exception_options(containers_benchmark)
if (BUILD_LONG_TESTS)
    target_compile_definitions(containers_benchmark PRIVATE BUILD_LONG_TESTS)
endif()

# Application to plot these benchmarks
if (Matplot++_FOUND)
    CPMAddPackage(NAME nlohmann_json VERSION 3.9.1 URL https://github.com/nlohmann/json/releases/download/v3.9.1/include.zip  URL_HASH SHA256=6bea5877b1541d353bd77bdfbdb2696333ae5ed8f9e8cc22df657192218cad91)
    if(nlohmann_json_ADDED)
        add_library(nlohmann_json INTERFACE)
        target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
        add_executable(plot_container_benchmark plot_containers_benchmark.cpp)
        target_link_libraries(plot_container_benchmark PRIVATE Matplot++::matplot nlohmann_json)
        target_exception_options(plot_container_benchmark)
    endif()
endif()

#######################################################
### Hypervolume benchmarks                          ###
#######################################################
add_executable(hypervolume_benchmark hypervolume_benchmark.cpp)
target_link_libraries(hypervolume_benchmark PRIVATE pareto benchmark)
target_bigobj_options(hypervolume_benchmark)
target_exception_options(hypervolume_benchmark)
if (BUILD_LONG_TESTS)
    target_compile_definitions(hypervolume_benchmark PRIVATE BUILD_LONG_TESTS)
endif()

if (BUILD_BOOST_TREE)
    target_compile_definitions(pareto INTERFACE BUILD_BOOST_TREE)
    target_compile_options(pareto INTERFACE -Wno-\#pragma-messages -Wno-unused-parameter -Wno-deprecated-declarations)
endif()
