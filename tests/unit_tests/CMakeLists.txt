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
add_library(catch_main ${Catch2_SOURCE_DIR}/examples/000-CatchMain.cpp)
target_link_libraries(catch_main PUBLIC Catch2)
target_exception_options(catch_main)

#######################################################
### Concepts                                        ###
#######################################################
# Make sure objects respect the concepts defined by the STL
add_executable(ut_concepts concepts.cpp)
target_link_libraries(ut_concepts PUBLIC pareto catch_main)
target_longtests_definitions(ut_concepts)
target_exception_options(ut_concepts)
target_bigobj_options(ut_concepts)
catch_discover_tests(ut_concepts)

#######################################################
### Test points and query boxes                     ###
#######################################################
# Create test with the tests_point_and_query_box_instantiation
add_executable(ut_point point.cpp)
target_link_libraries(ut_point PUBLIC pareto catch_main)
target_longtests_definitions(ut_point)
target_exception_options(ut_point)
target_bigobj_options(ut_point)
catch_discover_tests(ut_point)

#######################################################
### Shor PMR test                                   ###
#######################################################
# Create test with the tests_point_and_query_box_instantiation
add_executable(ut_pmr_test pmr_test.cpp)
target_link_libraries(ut_pmr_test PUBLIC pareto catch_main)
target_longtests_definitions(ut_pmr_test)
target_exception_options(ut_pmr_test)
target_bigobj_options(ut_pmr_test)
catch_discover_tests(ut_pmr_test)

#######################################################
### Data structures                                 ###
#######################################################
if (BUILD_BOOST_TREE)
    set(TREETAGS implicit quad kd r r_star boost)
else()
    set(TREETAGS implicit quad kd r r_star)
endif()
foreach (TREETAG ${TREETAGS})
    # Create test with the tests_tree_instantiation
    add_executable(ut_${TREETAG}_tree container_fuzz.cpp)
    target_link_libraries(ut_${TREETAG}_tree PUBLIC pareto catch_main)
    target_compile_definitions(ut_${TREETAG}_tree PRIVATE ${TREETAG}_TREETAG)
    target_exception_options(ut_${TREETAG}_tree)
    target_bigobj_options(ut_${TREETAG}_tree)
    target_longtests_definitions(ut_${TREETAG}_tree)
    target_msvc_compile_options(ut_${TREETAG}_tree /wd4305)
    catch_discover_tests(ut_${TREETAG}_tree)
endforeach ()

#######################################################
### Test Pareto fronts (fuzz)                       ###
#######################################################
# Create test with the tests_front_instantiation
foreach (TREETAG ${TREETAGS})
    # Create test with the tests_tree_instantiation
    add_executable(ut_${TREETAG}_front_fuzz front_fuzz.cpp)
    target_link_libraries(ut_${TREETAG}_front_fuzz PUBLIC pareto catch_main)
    target_compile_definitions(ut_${TREETAG}_front_fuzz PRIVATE ${TREETAG}_TREETAG)
    target_longtests_definitions(ut_${TREETAG}_front_fuzz)
    target_exception_options(ut_${TREETAG}_front_fuzz)
    target_bigobj_options(ut_${TREETAG}_front_fuzz)
    target_pedantic_options(ut_${TREETAG}_front_fuzz)
    catch_discover_tests(ut_${TREETAG}_front_fuzz)
endforeach()

#######################################################
### Test Pareto fronts (interface)                  ###
#######################################################
add_executable(ut_front_interface front_interface.cpp)
target_link_libraries(ut_front_interface PUBLIC pareto catch_main)
target_longtests_definitions(ut_front_interface)
target_exception_options(ut_front_interface)
target_bigobj_options(ut_front_interface)
target_pedantic_options(ut_front_interface)
catch_discover_tests(ut_front_interface)

#######################################################
### Test Pareto archives                            ###
#######################################################
# Create test with the tests_archive_instantiation
foreach (TREETAG ${TREETAGS})
    add_executable(ut_${TREETAG}_archive_fuzz archive.cpp)
    target_link_libraries(ut_${TREETAG}_archive_fuzz PUBLIC pareto catch_main)
#    if (Matplot++_FOUND)
#        target_link_libraries(ut_${TREETAG}_archive_fuzz PRIVATE Matplot++::matplot pareto)
#        target_compile_definitions(ut_${TREETAG}_archive_fuzz PRIVATE INCLUDE_MATPLOT)
#    endif()
    target_compile_definitions(ut_${TREETAG}_archive_fuzz PRIVATE ${TREETAG}_TREETAG)
    target_longtests_definitions(ut_${TREETAG}_archive_fuzz)
    target_exception_options(ut_${TREETAG}_archive_fuzz)
    target_bigobj_options(ut_${TREETAG}_archive_fuzz)
    catch_discover_tests(ut_${TREETAG}_archive_fuzz)
endforeach()
