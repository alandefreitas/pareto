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

# Create explicit template instantiations in a target
# This is fundamental for faster compile times
# to avoid duplicate instantiations in all tests
# The tests take forever without these
# This is especially important for the python bindings
# Many platforms cannot create the bindings without these
# https://isocpp.org/wiki/faq/cpp11-language-templates#extern-templates
add_library(tests_point_instantiation
        instantiation/test_instantiations.h
        instantiation/double_point_instantiations.cpp
)
target_link_libraries(tests_point_instantiation PUBLIC pareto_front)

add_library(tests_query_box_instantiation
        instantiation/test_instantiations.h
        instantiation/double_query_box_instantiations.cpp
    )
target_link_libraries(tests_query_box_instantiation PUBLIC tests_point_instantiation)

add_library(tests_tree_instantiation
        instantiation/test_instantiations.h
        instantiation/double_unsigned_kd_tree_instantiations.cpp
        instantiation/double_unsigned_quad_tree_instantiations.cpp
        instantiation/double_unsigned_r_star_tree_instantiations.cpp
        instantiation/double_unsigned_r_tree_instantiations.cpp
        instantiation/double_unsigned_vector_tree_instantiations.cpp
    )
target_link_libraries(tests_tree_instantiation PUBLIC tests_query_box_instantiation)

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

add_executable(unit_test_point point.cpp)
target_link_libraries(unit_test_point PUBLIC tests_point_instantiation Catch2)
if (BUILD_LONG_TESTS)
    target_compile_definitions(unit_test_point PUBLIC BUILD_LONG_TESTS)
endif()
catch_discover_tests(unit_test_point)

add_executable(unit_test_trees trees.cpp)
target_link_libraries(unit_test_trees PUBLIC tests_tree_instantiation Catch2)
if (BUILD_LONG_TESTS)
    target_compile_definitions(unit_test_trees PUBLIC BUILD_LONG_TESTS)
endif()
if(MSVC)
    # Because there is so much in the file
    target_compile_options(unit_test_trees PRIVATE /bigobj)
    target_compile_options(unit_test_trees PUBLIC /wd4305)
endif()
catch_discover_tests(unit_test_trees)

add_executable(unit_test_pareto_front fronts.cpp)
target_link_libraries(unit_test_pareto_front PUBLIC tests_front_instantiation Catch2)
if (BUILD_LONG_TESTS)
    target_compile_definitions(unit_test_pareto_front PUBLIC BUILD_LONG_TESTS)
endif()

if(MSVC)
    # Because there is so much in the file
    target_compile_options(unit_test_pareto_front PRIVATE /bigobj)
    target_compile_options(unit_test_pareto_front PUBLIC /wd4305)
endif()
catch_discover_tests(unit_test_pareto_front)

add_executable(unit_test_pareto_archive archive.cpp)
target_link_libraries(unit_test_pareto_archive PUBLIC tests_archive_instantiation Catch2)
if (BUILD_LONG_TESTS)
    target_compile_definitions(unit_test_pareto_archive PUBLIC BUILD_LONG_TESTS)
endif()
if(MSVC)
    # Because there is so much in the file
    target_compile_options(unit_test_pareto_archive PRIVATE /bigobj)
    target_compile_options(unit_test_pareto_archive PUBLIC /wd4305)
endif()
catch_discover_tests(unit_test_pareto_archive)