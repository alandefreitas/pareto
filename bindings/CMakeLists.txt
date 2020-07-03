CPMAddPackage(
    NAME pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11/
    VERSION 2.5.0
)

pybind11_add_module(pyfront pyfront.cpp)
target_link_libraries(pyfront PUBLIC pareto_front)
# https://www.rapidtables.com/code/linux/gcc/gcc-o.html
# https://pybind11.readthedocs.io/en/stable/faq.html#someclass-declared-with-greater-visibility-than-the-type-of-its-field-someclass-member-wattributes
target_compile_options(pyfront PRIVATE -fvisibility=hidden -Os)