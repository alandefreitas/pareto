# Get pybind11
find_package(pybind11 2.5 QUIET)
if (NOT pybind11_FOUND)
    CPMAddPackage(
            NAME pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11/
            VERSION 2.5.0
            OPTIONS "PYBIND11_TEST OFF" "PYBIND11_INSTALL OFF"
    )
endif()

# Create explicit template instantiations in a target
# We use .cpp files to instantiate all types in namespace pareto
# This is fundamental for faster compile times
# to avoid duplicate instantiations
# This is especially important for the python bindings
# Many platforms cannot create the bindings without these
# https://isocpp.org/wiki/faq/cpp11-language-templates#extern-templates
add_library(pyfront_point_instantiation
        instantiation/python_instantiations.h
        instantiation/double_point_instantiations.cpp
        )
target_link_libraries(pyfront_point_instantiation PUBLIC pareto_front pybind11)
target_include_directories(pyfront_point_instantiation PUBLIC ${PYBIND11_INCLUDE_DIR})
target_compile_options(pyfront_point_instantiation PRIVATE -fvisibility=hidden -Os)

add_library(pyfront_query_box_instantiation
        instantiation/python_instantiations.h
        instantiation/double_query_box_instantiations.cpp
        )
target_link_libraries(pyfront_query_box_instantiation PUBLIC pyfront_point_instantiation)
target_compile_options(pyfront_query_box_instantiation PRIVATE -fvisibility=hidden -Os)

add_library(pyfront_tree_instantiation
        instantiation/python_instantiations.h
        instantiation/double_object_kd_tree_instantiations.cpp
        instantiation/double_object_quad_tree_instantiations.cpp
        instantiation/double_object_r_star_tree_instantiations.cpp
        instantiation/double_object_r_tree_instantiations.cpp
        instantiation/double_object_vector_tree_instantiations.cpp
        )
target_link_libraries(pyfront_tree_instantiation PUBLIC pyfront_query_box_instantiation)
target_compile_options(pyfront_tree_instantiation PRIVATE -fvisibility=hidden -Os)

add_library(pyfront_front_instantiation
        instantiation/python_instantiations.h
        instantiation/double_object_front_instantiations.cpp
        instantiation/double_object_front_kd_instantiations.cpp
        instantiation/double_object_front_quad_instantiations.cpp
        instantiation/double_object_front_r_instantiations.cpp
        instantiation/double_object_front_r_star_instantiations.cpp
        instantiation/double_object_front_vector_instantiations.cpp
        )
target_link_libraries(pyfront_front_instantiation PUBLIC pyfront_tree_instantiation)
target_compile_options(pyfront_front_instantiation PRIVATE -fvisibility=hidden -Os)

add_library(pyfront_archive_instantiation
        instantiation/python_instantiations.h
        instantiation/double_object_archive_instantiations.cpp
        instantiation/double_object_archive_kd_instantiations.cpp
        instantiation/double_object_archive_quad_instantiations.cpp
        instantiation/double_object_archive_r_instantiations.cpp
        instantiation/double_object_archive_r_star_instantiations.cpp
        instantiation/double_object_archive_vector_instantiations.cpp
        )
target_link_libraries(pyfront_archive_instantiation PUBLIC pyfront_front_instantiation)
target_compile_options(pyfront_archive_instantiation PRIVATE -fvisibility=hidden -Os)

# The file binding_n_dimensional.h is the one with the template
# function to create bindings for a front with N dimensions and
# a given data structure.
# To make compilation faster, we explicitly instantiate this
# template on a number of other files. Each file instantiates
# the front and archive for a given dimension and data structure.
# This speeds up compilation and saves us some memory.
set(EXPLICIT_TRANSLATION_UNITS)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h
        "#ifndef PARETO_FRONT_BINDING_N_DIMENSIONAL_EXTERN_TEMPLATES\n#define PARETO_FRONT_BINDING_N_DIMENSIONAL_EXTERN_TEMPLATES\n\n"
        )
foreach (TREE_TYPE vector quad kd r r_star)
    foreach (DIMENSION RANGE 0 ${MAX_NUM_DIMENSIONS_PYTHON})
        message("TREE_TYPE=${TREE_TYPE} DIMENSION=${DIMENSION}")
        configure_file(
                translation_unit.cpp.in
                ${CMAKE_CURRENT_BINARY_DIR}/binding_${TREE_TYPE}_${DIMENSION}_dimensional.cpp
        )
        list(APPEND EXPLICIT_TRANSLATION_UNITS ${CMAKE_CURRENT_BINARY_DIR}/binding_${TREE_TYPE}_${DIMENSION}_dimensional.cpp)
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h
                "extern template void binding_for_N_dimensional<${DIMENSION},pareto::${TREE_TYPE}_tree_tag>(py::module &m, bool binding_default);\n"
        )
    endforeach()
endforeach()
message(EXPLICIT_TRANSLATION_UNITS=${EXPLICIT_TRANSLATION_UNITS})
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h
        "\n#endif // PARETO_FRONT_BINDING_N_DIMENSIONAL_EXTERN_TEMPLATES\n"
        )

# Create python bindings
pybind11_add_module(pyfront
        # template function to create one binding
        binding_n_dimensional.h
        # instantiation of the function that creates strings from tags
        tag_to_string.cpp
        # header with extern template for all functions
        ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h
        # template instantiations creating the bindings
        ${EXPLICIT_TRANSLATION_UNITS}
        # library that calls binding_n_dimensional
        pyfront.cpp
)
target_link_libraries(pyfront PUBLIC pyfront_archive_instantiation)

# https://www.rapidtables.com/code/linux/gcc/gcc-o.html
# https://pybind11.readthedocs.io/en/stable/faq.html#someclass-declared-with-greater-visibility-than-the-type-of-its-field-someclass-member-wattributes
target_compile_options(pyfront PRIVATE -fvisibility=hidden -Os)
if (BUILD_BINDING_FOR_ALL_STRUCTURES)
    target_compile_options(pyfront PUBLIC -DBUILD_BINDING_FOR_ALL_STRUCTURES)
endif()
target_compile_definitions(pyfront PUBLIC MAX_NUM_DIMENSIONS_PYTHON=${MAX_NUM_DIMENSIONS_PYTHON})
# Allow explicit instantiations from ${CMAKE_CURRENT_BINARY_DIR} to find binding_n_dimensional.h
target_include_directories(pyfront PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# Allot binding_n_dimensional.h to include ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h
target_include_directories(pyfront PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if(MSVC)
    # Because there is so much in the file
    target_compile_options(pyfront PRIVATE /bigobj)
    target_compile_options(pyfront PUBLIC /wd4305)
endif()

if (BUILD_INSTALLER)
    install(TARGETS pyfront
            EXPORT pyfrontTargets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            INCLUDES DESTINATION include
    )
endif()