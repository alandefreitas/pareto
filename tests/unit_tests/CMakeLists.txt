#######################################################
### Catch2 - Unit tests                             ###
#######################################################
# Find or get Catch2
# Note that CPM will try to "find_package(Catch2)" before downloading it
# See the option CPM_USE_LOCAL_PACKAGES in ROOT/CMakeLists.txt
# This is important: see One Definition Rule (ODR)
CPMAddPackage(
        NAME Catch2
        GITHUB_REPOSITORY catchorg/Catch2
        VERSION 2.12.1
        OPTIONS
            "CATCH_USE_VALGRIND OFF"
            "CATCH_BUILD_TESTING OFF"
            "CATCH_BUILD_EXAMPLES OFF"
            "CATCH_BUILD_EXTRA_TESTS OFF"
            "CATCH_ENABLE_COVERAGE OFF"
            "CATCH_ENABLE_WERROR ON"
            "CATCH_INSTALL_DOCS OFF"
            "CATCH_INSTALL_HELPERS OFF"
)
include(${Catch2_SOURCE_DIR}/contrib/Catch.cmake)

#######################################################
### Test points and query boxes                     ###
#######################################################
# Create explicit template instantiations in a separate target
# This is fundamental for faster compile times
# to avoid duplicate instantiations in all tests
# Compiling the tests take forever without these
# This is especially important for the python bindings
# Many platforms cannot even create the bindings without these
# https://isocpp.org/wiki/faq/cpp11-language-templates#extern-templates
add_library(tests_point_and_query_box_instantiation
        instantiation/test_instantiations.h
        instantiation/double_point_instantiations.cpp
        instantiation/double_query_box_instantiations.cpp
)
target_link_libraries(tests_point_and_query_box_instantiation PUBLIC pareto)
target_exception_options(tests_point_and_query_box_instantiation)
target_bigobj_options(tests_point_and_query_box_instantiation)

# Create test with the tests_point_and_query_box_instantiation
add_executable(unit_test_point point.cpp)
target_link_libraries(unit_test_point PUBLIC tests_point_and_query_box_instantiation Catch2)
target_longtests_definitions(unit_test_point)
target_exception_options(unit_test_point)
target_bigobj_options(unit_test_point)
catch_discover_tests(unit_test_point)

#######################################################
### Test memory allocators                          ###
#######################################################
add_executable(unit_test_memory memory.cpp)
target_link_libraries(unit_test_memory PUBLIC pareto Catch2)
target_longtests_definitions(unit_test_memory)
target_exception_options(unit_test_memory)
target_bigobj_options(unit_test_memory)
catch_discover_tests(unit_test_memory)

#######################################################
### Test tree data structures                       ###
#######################################################
# Create explicit template instantiations for trees
add_library(tests_tree_instantiation
        instantiation/test_instantiations.h
        instantiation/double_unsigned_kd_tree_instantiations.cpp
        instantiation/double_unsigned_quad_tree_instantiations.cpp
        instantiation/double_unsigned_r_star_tree_instantiations.cpp
        instantiation/double_unsigned_r_tree_instantiations.cpp
        instantiation/double_unsigned_vector_tree_instantiations.cpp
    )
target_link_libraries(tests_tree_instantiation PUBLIC tests_point_and_query_box_instantiation)
target_exception_options(tests_tree_instantiation)
target_bigobj_options(tests_tree_instantiation)

# Create test with the tests_tree_instantiation
add_executable(unit_test_trees trees.cpp)
target_link_libraries(unit_test_trees PUBLIC tests_tree_instantiation Catch2)
target_exception_options(unit_test_trees)
target_bigobj_options(unit_test_trees)
target_longtests_definitions(unit_test_trees)
target_msvc_compile_options(unit_test_trees /wd4305)
catch_discover_tests(unit_test_trees)

#######################################################
### Test Pareto fronts (fuzz)                       ###
#######################################################
# Create explicit template instantiations for fronts
add_library(tests_front_instantiation
        instantiation/test_instantiations.h
        instantiation/double_unsigned_front_instantiations.cpp
        instantiation/double_unsigned_front_kd_instantiations.cpp
        instantiation/double_unsigned_front_quad_instantiations.cpp
        instantiation/double_unsigned_front_r_instantiations.cpp
        instantiation/double_unsigned_front_r_star_instantiations.cpp
        instantiation/double_unsigned_front_vector_instantiations.cpp
    )
target_link_libraries(tests_front_instantiation PUBLIC tests_tree_instantiation)
target_exception_options(tests_front_instantiation)
target_bigobj_options(tests_front_instantiation)

# Create test with the tests_front_instantiation
add_executable(unit_test_front_fuzz front_fuzz.cpp)
target_link_libraries(unit_test_front_fuzz PUBLIC tests_front_instantiation Catch2)
target_longtests_definitions(unit_test_front_fuzz)
target_exception_options(unit_test_front_fuzz)
target_bigobj_options(unit_test_front_fuzz)
target_pedantic_options(unit_test_front_fuzz)
catch_discover_tests(unit_test_front_fuzz)

#######################################################
### Test Pareto fronts (interface)                  ###
#######################################################
add_executable(unit_test_front_interface front_interface.cpp)
target_link_libraries(unit_test_front_interface PUBLIC pareto Catch2)
target_longtests_definitions(unit_test_front_interface)
target_exception_options(unit_test_front_interface)
target_bigobj_options(unit_test_front_interface)
target_pedantic_options(unit_test_front_interface)
catch_discover_tests(unit_test_front_interface)

#######################################################
### Test Pareto archives                            ###
#######################################################
# Create explicit template instantiations for archives
add_library(tests_archive_instantiation
        instantiation/test_instantiations.h
        instantiation/double_unsigned_archive_instantiations.cpp
        instantiation/double_unsigned_archive_kd_instantiations.cpp
        instantiation/double_unsigned_archive_quad_instantiations.cpp
        instantiation/double_unsigned_archive_r_instantiations.cpp
        instantiation/double_unsigned_archive_r_star_instantiations.cpp
        instantiation/double_unsigned_archive_vector_instantiations.cpp
    )
target_link_libraries(tests_archive_instantiation PUBLIC tests_front_instantiation)
target_exception_options(tests_archive_instantiation)
target_bigobj_options(tests_archive_instantiation)

# Create test with the tests_archive_instantiation
add_executable(unit_test_pareto_archive archive.cpp)
target_link_libraries(unit_test_pareto_archive PUBLIC tests_archive_instantiation Catch2)
target_longtests_definitions(unit_test_pareto_archive)
target_exception_options(unit_test_pareto_archive)
target_bigobj_options(unit_test_pareto_archive)
catch_discover_tests(unit_test_pareto_archive)
