cmake_minimum_required(VERSION 3.14)
project(pareto_front)

set(CMAKE_CXX_STANDARD 17)

# https://github.com/TheLartians/CPM.cmake#snippets
include(cmake/CPM.cmake)

# Library
add_subdirectory(sources)

add_subdirectory(examples)
