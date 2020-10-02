add_executable(example_front2d front2d.cpp)
target_link_libraries(example_front2d pareto_front)
if (MSVC)
    # MSVC requires this flag if the code uses C++ exception handling
    target_compile_options(example_front2d PRIVATE /EHsc)
endif()


add_executable(example_archive2d archive2d.cpp)
target_link_libraries(example_archive2d pareto_front)
if (MSVC)
    # MSVC requires this flag if the code uses C++ exception handling
    target_compile_options(example_archive2d PRIVATE /EHsc)
endif()


add_executable(example_front3d front3d.cpp)
target_link_libraries(example_front3d pareto_front)
if (MSVC)
    # MSVC requires this flag if the code uses C++ exception handling
    target_compile_options(example_front3d PRIVATE /EHsc)
endif()


add_executable(example_archive3d archive3d.cpp)
target_link_libraries(example_archive3d pareto_front)
if (MSVC)
    # MSVC requires this flag if the code uses C++ exception handling
    target_compile_options(example_archive3d PRIVATE /EHsc)
endif()

find_package(Matplot++)
if (Matplot++_FOUND)
    add_executable(example_matplotpp matplotpp_example.cpp)
    target_link_libraries(example_matplotpp pareto_front Matplot++::matplot)
    if (MSVC)
        # MSVC requires this flag if the code uses C++ exception handling
        target_compile_options(example_matplotpp PRIVATE /EHsc)
    endif()
endif()


