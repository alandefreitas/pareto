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
if (BUILD_BINDING_FOR_ALL_STRUCTURES)
    target_compile_options(pyfront PUBLIC -DBUILD_BINDING_FOR_ALL_STRUCTURES)
endif()
target_compile_definitions(pyfront PUBLIC MAX_NUM_DIMENSIONS_PYTHON=${MAX_NUM_DIMENSIONS_PYTHON})

if(MSVC)
    # Because there is so much in the file
    target_compile_options(pyfront PRIVATE /bigobj)
    target_compile_options(pyfront PUBLIC /wd4305)
endif()
