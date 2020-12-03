#######################################################
### Pybind11 for Python bindings                    ###
#######################################################
# These bindings are very complicated because they
# involve lots of heavy templates.
# If you're learning pybind11, this is not the place
# to start.

# Find or get pybind11
# Note that CPM will try to "find_package" before downloading it
# See the option CPM_USE_LOCAL_PACKAGES in ROOT/CMakeLists.txt
# This is important: see One Definition Rule (ODR)
CPMAddPackage(
        NAME pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11/
        VERSION 2.6.1
        OPTIONS "PYBIND11_TEST OFF" "PYBIND11_INSTALL OFF"
)

#######################################################
### Generate source files for bindings              ###
#######################################################
# We are going to explicitly instantiate all templates we use in the bindings
# This is fundamental for faster compile times
# https://isocpp.org/wiki/faq/cpp11-language-templates#extern-templates
# For each function or template we need in the bindings, we
# automatically generate a header file with extern template and
# a source file instantiating this template.

# The file binding_n_dimensional.h is the one with the template
# function to create bindings for a front with N dimensions and
# a given data structure. This is the template function that
# uses the instantiations the files above create.
# To make compilation faster, we generate source code to explicitly
# instantiate this template on a number of other .cpp files.
# Each file instantiates the front and archive bindings for a given
# dimension and data structure. This speeds up compilation and saves
# us some memory.
set(EXPLICIT_TRANSLATION_UNITS)

# python_instantiations.h is a automatically generated
# header with extern templates for all data structures
# for fronts and archives
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/python_instantiations.h
        "#ifndef PARETO_FRONT_PYTHON_INSTANTIATIONS_H\n#define PARETO_FRONT_PYTHON_INSTANTIATIONS_H\n\n#include <pareto/front.h>\n#include <pareto/archive.h>\n#include <pybind11/pybind11.h>\n#include \"custom_equality.h\"\n\nnamespace pareto {\n"
        )

# binding_n_dimensional_extern_templates.h is a automatically generated
# header with extern templates for binding_n_dimensional.h
# binding_n_dimensional.h includes it
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h
        "#ifndef PARETO_FRONT_BINDING_N_DIMENSIONAL_EXTERN_TEMPLATES\n#define PARETO_FRONT_BINDING_N_DIMENSIONAL_EXTERN_TEMPLATES\n\n"
        )

# for each data structure (don't even consider boost anymore)
foreach (TREE_TYPE vector quad kd r r_star)
    # for each possible dimension
    foreach (DIMENSION RANGE 0 ${MAX_NUM_DIMENSIONS_PYTHON})
        # if this is the first time we are looking at this dimension
        if (TREE_TYPE STREQUAL vector)
            # extern templates for point and query box
            file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/python_instantiations.h
                    "    extern template class point<double, ${DIMENSION}>;\n    extern template class query_box<double, ${DIMENSION}>;\n"
                    )

            # instantiation for point
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/point_instantiation/double_point_${DIMENSION}_instantiation.cpp
                    "#include <pareto/point.h>\n\nnamespace pareto {\n    template class point<double, ${DIMENSION}>;\n}\n"
                    )
            list(APPEND EXPLICIT_TRANSLATION_UNITS ${CMAKE_CURRENT_BINARY_DIR}/point_instantiation/double_point_${DIMENSION}_instantiation.cpp)

            # instantiation for query box
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/query_box_instantiation/double_query_box_${DIMENSION}_instantiation.cpp
                    "#include <pareto/query/query_box.h>\n\nnamespace pareto {\n    template class query_box<double, ${DIMENSION}>;\n}\n"
                    )
            list(APPEND EXPLICIT_TRANSLATION_UNITS ${CMAKE_CURRENT_BINARY_DIR}/query_box_instantiation/double_query_box_${DIMENSION}_instantiation.cpp)

        endif()
        # if we are expected to generate bindings for this dimension and structure
        if (BUILD_BINDING_FOR_ALL_STRUCTURES OR (TREE_TYPE STREQUAL kd) OR (TREE_TYPE STREQUAL vector))
            # Generate a translation unit that instantiates this binding generation function
            configure_file(
                    translation_unit.cpp.in
                    ${CMAKE_CURRENT_BINARY_DIR}/binding_instantiation/binding_${TREE_TYPE}_${DIMENSION}_dimensional.cpp
            )
            list(APPEND EXPLICIT_TRANSLATION_UNITS ${CMAKE_CURRENT_BINARY_DIR}/binding_instantiation/binding_${TREE_TYPE}_${DIMENSION}_dimensional.cpp)

            # Append extern template for function in header
            file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h
                    "extern template void binding_for_N_dimensional<${DIMENSION},pareto::${TREE_TYPE}_tree_tag>(py::module &m, bool binding_default);\n"
                    )

            # Append extern templates for classes in header
            file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/python_instantiations.h
                    "    extern template class ${TREE_TYPE}_tree<double, ${DIMENSION}, pybind11::object>;\n    extern template class front<double, ${DIMENSION}, pybind11::object, ${TREE_TYPE}_tree_tag>;\n    extern template class archive<double, ${DIMENSION}, pybind11::object, ${TREE_TYPE}_tree_tag>;\n\n"
                    )

            # Generate a translation unit that instantiates the tree object
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${TREE_TYPE}_instantiation/double_object_${TREE_TYPE}_tree_${DIMENSION}_instantiation.cpp
                    "#include <pybind11/pybind11.h>\n#include \"custom_equality.h\"\n#include <pareto/tree/${TREE_TYPE}_tree.h>\n\nnamespace pareto {\n    template class ${TREE_TYPE}_tree<double, ${DIMENSION}, pybind11::object>;\n}\n"
                    )
            list(APPEND EXPLICIT_TRANSLATION_UNITS ${CMAKE_CURRENT_BINARY_DIR}/${TREE_TYPE}_instantiation/double_object_${TREE_TYPE}_tree_${DIMENSION}_instantiation.cpp)

            # Generate a translation unit that instantiates the front object
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/front_instantiation/double_object_front_${TREE_TYPE}_tree_${DIMENSION}_instantiation.cpp
                    "#include <pybind11/pybind11.h>\n#include \"custom_equality.h\"\n#include <pareto/front.h>\n\nnamespace pareto {\n    template class front<double, ${DIMENSION}, pybind11::object, ${TREE_TYPE}_tree_tag>;\n}\n"
                    )
            list(APPEND EXPLICIT_TRANSLATION_UNITS ${CMAKE_CURRENT_BINARY_DIR}/front_instantiation/double_object_front_${TREE_TYPE}_tree_${DIMENSION}_instantiation.cpp)

            # Generate a translation unit that instantiates the front object
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/archive_instantitation/double_object_archive_${TREE_TYPE}_tree_${DIMENSION}_instantiation.cpp
                    "#include <pybind11/pybind11.h>\n#include \"custom_equality.h\"\n#include <pareto/archive.h>\n\nnamespace pareto {\n    template class archive<double, ${DIMENSION}, pybind11::object, ${TREE_TYPE}_tree_tag>;\n}\n"
                    )
            list(APPEND EXPLICIT_TRANSLATION_UNITS ${CMAKE_CURRENT_BINARY_DIR}/archive_instantitation/double_object_archive_${TREE_TYPE}_tree_${DIMENSION}_instantiation.cpp)
        endif()
    endforeach()
endforeach()
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h
        "\n#endif // PARETO_FRONT_BINDING_N_DIMENSIONAL_EXTERN_TEMPLATES\n"
        )
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/python_instantiations.h
        "}\n\n#endif // PARETO_FRONT_PYTHON_INSTANTIATIONS_H\n"
        )

#######################################################
### Create bindings                                 ###
#######################################################
# Create python bindings
pybind11_add_module(pyfront
        # header file with extern template for the instantiations
        ${CMAKE_CURRENT_BINARY_DIR}/python_instantiations.h

        # custom equality comparison for py::object
        custom_equality.h
        custom_equality.cpp

        # template function to create one binding
        binding_n_dimensional.h

        # header with extern template for all functions
        ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h

        # instantiation of the function that creates strings from tags
        tag_to_string.cpp

        # .cpp files with template instantiations creating the bindings
        ${EXPLICIT_TRANSLATION_UNITS}

        # library that calls binding_n_dimensional
        pyfront.cpp
        )
target_link_libraries(pyfront PRIVATE pareto pybind11)

# Change visibility
# https://www.rapidtables.com/code/linux/gcc/gcc-o.html
# https://pybind11.readthedocs.io/en/stable/faq.html#someclass-declared-with-greater-visibility-than-the-type-of-its-field-someclass-member-wattributes
if (NOT MSVC)
    target_compile_options(pyfront PRIVATE -fvisibility=hidden)
endif()

# Try to make the final library a little smaller because
# There are lots of template instantiations in there
if (NOT MSVC)
    target_compile_options(pyfront PRIVATE -Os)
else()
    target_compile_options(pyfront PRIVATE /Os)
endif()

if (MSVC)
    # Allow MSVC to compile such a large code
    target_compile_options(pyfront PRIVATE /bigobj)
    # MSVC requires this flag if the code uses C++ exception handling
    target_compile_options(pyfront PRIVATE /EHsc)
endif()

# Pass the flag to ignore the extra data structures
if (BUILD_BINDING_FOR_ALL_STRUCTURES)
    target_compile_options(pyfront PRIVATE -DBUILD_BINDING_FOR_ALL_STRUCTURES)
endif()

# Pass the definition indicating the number of dimensions we want
target_compile_definitions(pyfront PRIVATE MAX_NUM_DIMENSIONS_PYTHON=${MAX_NUM_DIMENSIONS_PYTHON})

# Allow explicit instantiations from ${CMAKE_CURRENT_BINARY_DIR} to find binding_n_dimensional.h
target_include_directories(pyfront PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Allow binding_n_dimensional.h to include ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h
target_include_directories(pyfront PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#######################################################
### Installer                                       ###
#######################################################
# Install the python bindings
if (BUILD_INSTALLER)
    # If MSVC, CPack cannot create packages with
    # default python directory. The default directory
    # will be CMAKE_INSTALL_LIBDIR at first.
    # The user is responsible for setting the best
    # directory.
    # Find python and the site-packages directory
    # Find the python executable
    if (PYTHON_EXECUTABLE)
        # pybind11 should have found it already
        message("PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
    else()
        set(PYTHON_EXECUTABLE python)
    endif()

    # Python one-line script to find the site-packages directory
    # relative to ${CMAKE_INSTALL_PREFIX}. This script will try
    # to find a relative path first because cpack generators
    # cannot use absolute paths.
    # https://stackoverflow.com/questions/7287996/python-get-relative-path-from-comparing-two-absolute-paths
    set(PYTHON_SITE_PACKAGES_SCRIPT "from distutils.sysconfig import get_python_lib; import os; print(os.path.relpath(get_python_lib(), '${CMAKE_INSTALL_PREFIX}'));")

    # 1st attempt: Try to find the python site-packages directory
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "${PYTHON_SITE_PACKAGES_SCRIPT}" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (PYTHON_SITE_PACKAGES AND (MSVC OR WIN32))
        # the packages in windows cannot start with "..\"
        message("PYTHON_SITE_PACKAGES_ORIGINAL=${PYTHON_SITE_PACKAGES}")
        if (PYTHON_SITE_PACKAGES MATCHES "^\\.\\.\\\\.*$")
            set(PYTHON_SITE_PACKAGES ${CMAKE_INSTALL_LIBDIR})
            message("PYTHON_SITE_PACKAGES_CHANGED_TO=${PYTHON_SITE_PACKAGES}")
        endif()
    endif()

    # 2st attempt
    if (NOT PYTHON_SITE_PACKAGES)
        execute_process(COMMAND python3 -c "${PYTHON_SITE_PACKAGES_SCRIPT}" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    if (PYTHON_SITE_PACKAGES AND (MSVC OR WIN32))
        # the packages in windows cannot start with "..\"
        message("PYTHON_SITE_PACKAGES_ORIGINAL=${PYTHON_SITE_PACKAGES}")
        if (PYTHON_SITE_PACKAGES MATCHES "^\\.\\.\\\\.*$")
            set(PYTHON_SITE_PACKAGES ${CMAKE_INSTALL_LIBDIR})
            message("PYTHON_SITE_PACKAGES_CHANGED_TO=${PYTHON_SITE_PACKAGES}")
        endif()
    endif()

    # 3st attempt
    if (NOT PYTHON_SITE_PACKAGES)
        execute_process(COMMAND python2 -c "${PYTHON_SITE_PACKAGES_SCRIPT}" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    if (PYTHON_SITE_PACKAGES AND (MSVC OR WIN32))
        # the packages in windows cannot start with "..\"
        message("PYTHON_SITE_PACKAGES_ORIGINAL=${PYTHON_SITE_PACKAGES}")
        if (PYTHON_SITE_PACKAGES MATCHES "^\\.\\.\\\\.*$")
            set(PYTHON_SITE_PACKAGES ${CMAKE_INSTALL_LIBDIR})
            message("PYTHON_SITE_PACKAGES_CHANGED_TO=${PYTHON_SITE_PACKAGES}")
        endif()
    endif()

    # Use the default lib directory if cannot find site-packages
    if (NOT PYTHON_SITE_PACKAGES)
        set(PYTHON_SITE_PACKAGES ${CMAKE_INSTALL_LIBDIR})
    endif()

    message("PYTHON_SITE_PACKAGES=${PYTHON_SITE_PACKAGES}")
    message("CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

    install(TARGETS pyfront
            COMPONENT "Python_Library"
            LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES}
            ARCHIVE DESTINATION ${PYTHON_SITE_PACKAGES}
            )
endif()