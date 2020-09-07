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

add_executable(unit_test_point point.cpp)
target_link_libraries(unit_test_point PUBLIC pareto_front Catch2)
if (BUILD_LONG_TESTS)
    target_compile_definitions(unit_test_point PUBLIC BUILD_LONG_TESTS)
endif()

add_executable(unit_test_trees trees.cpp)
target_link_libraries(unit_test_trees PUBLIC pareto_front Catch2)
if (BUILD_LONG_TESTS)
    target_compile_definitions(unit_test_trees PUBLIC BUILD_LONG_TESTS)
endif()

add_executable(unit_test_pareto_front fronts.cpp)
target_link_libraries(unit_test_pareto_front PUBLIC pareto_front Catch2)
if (BUILD_LONG_TESTS)
    target_compile_definitions(unit_test_pareto_front PUBLIC BUILD_LONG_TESTS)
endif()

add_executable(unit_test_pareto_archive archive.cpp)
target_link_libraries(unit_test_pareto_archive PUBLIC pareto_front Catch2)
if (BUILD_LONG_TESTS)
    target_compile_definitions(unit_test_pareto_archive PUBLIC BUILD_LONG_TESTS)
endif()
