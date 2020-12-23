add_executable(example_spatial3d spatial3d.cpp)
target_link_libraries(example_spatial3d pareto)
target_exception_options(example_spatial3d)

add_executable(example_front3d front3d.cpp)
target_link_libraries(example_front3d pareto)
target_exception_options(example_front3d)

add_executable(example_archive3d archive3d.cpp)
target_link_libraries(example_archive3d pareto)
target_exception_options(example_archive3d)

if (Matplot++_FOUND)
    add_executable(example_matplotpp_example matplotpp_example.cpp)
    target_link_libraries(example_matplotpp_example PRIVATE Matplot++::matplot pareto)
    target_exception_options(example_matplotpp_example)
endif()