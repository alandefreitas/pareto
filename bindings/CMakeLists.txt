CPMAddPackage(
    NAME pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11/
    VERSION 2.5.0
)

pybind11_add_module(pyfront pyfront.cpp)
target_link_libraries(pyfront PUBLIC pareto_front)