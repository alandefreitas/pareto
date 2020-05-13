cmake_minimum_required(VERSION 3.14)
project(pareto_front)
set(CMAKE_CXX_STANDARD 17)
include(cmake/CPM.cmake)

add_subdirectory(sources)
add_subdirectory(examples)
add_subdirectory(bindings)
