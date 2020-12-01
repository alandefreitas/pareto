add_executable(example_front2d front2d.cpp)
target_link_libraries(example_front2d pareto)
target_exception_options(example_front2d)

add_executable(example_archive2d archive2d.cpp)
target_link_libraries(example_archive2d pareto)
target_exception_options(example_archive2d)

add_executable(example_front3d front3d.cpp)
target_link_libraries(example_front3d pareto)
target_exception_options(example_front3d)

add_executable(example_archive3d archive3d.cpp)
target_link_libraries(example_archive3d pareto)
target_exception_options(example_archive3d)

add_executable(example_curse_sets curse_sets.cpp)
target_link_libraries(example_curse_sets pareto)
target_exception_options(example_curse_sets)

find_package(Matplot++ QUIET)
if (Matplot++_FOUND)
    add_executable(example_matplotpp matplotpp_example.cpp)
    target_link_libraries(example_matplotpp pareto Matplot++::matplot)
    target_exception_options(example_matplotpp)
endif()
