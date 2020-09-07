CPMAddPackage(
    NAME pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11/
    VERSION 2.5.0
    OPTIONS "PYBIND11_TEST OFF" "PYBIND11_INSTALL OFF"
)

pybind11_add_module(pyfront pyfront.cpp)
target_link_libraries(pyfront PUBLIC pareto_front)

# https://www.rapidtables.com/code/linux/gcc/gcc-o.html
# https://pybind11.readthedocs.io/en/stable/faq.html#someclass-declared-with-greater-visibility-than-the-type-of-its-field-someclass-member-wattributes
target_compile_options(pyfront PRIVATE -fvisibility=hidden -Os)
target_compile_definitions(pyfront PUBLIC MAX_NUM_DIMENSIONS_PYTHON=${MAX_NUM_DIMENSIONS_PYTHON})