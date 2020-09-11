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
    VERSION 2.5.0
    OPTIONS "PYBIND11_TEST OFF" "PYBIND11_INSTALL OFF"
)

#######################################################
### Generate source files for bindings              ###
#######################################################
# We are going to explicitly instantiate all templates we use in the bindings
# This is fundamental for faster compile times
# https://isocpp.org/wiki/faq/cpp11-language-templates#extern-templates

# We only instantiate these data structures if BUILD_BINDING_FOR_ALL_STRUCTURES is ON
# These source files instantiate up to 10 dimensions with extern template
# Anything beyond that will be instantiated later in binding_n_dimensional.h.
# It would be possible to create a script to generate the source code
# depending on the number of dimensions. But we assume that if you
# have more than 10 dimensions in your problem, then you have bigger
# problems than that
if (BUILD_BINDING_FOR_ALL_STRUCTURES)
    set(pyfront_tree_instantiation_sources
            instantiation/double_object_quad_tree_instantiations.cpp
            instantiation/double_object_r_star_tree_instantiations.cpp
            instantiation/double_object_r_tree_instantiations.cpp
    )

    set(pyfront_front_instantiation_sources
            instantiation/double_object_front_quad_instantiations.cpp
            instantiation/double_object_front_r_instantiations.cpp
            instantiation/double_object_front_r_star_instantiations.cpp
    )

    set(pyfront_archive_instantiation_sources
            instantiation/double_object_archive_quad_instantiations.cpp
            instantiation/double_object_archive_r_instantiations.cpp
            instantiation/double_object_archive_r_star_instantiations.cpp
    )
endif()

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
        # if we are expected to generate bindings for this dimension and structure
        if (BUILD_BINDING_FOR_ALL_STRUCTURES OR (TREE_TYPE STREQUAL kd) OR (TREE_TYPE STREQUAL vector))
            # Generate a translation unit that instantiates this binding generation function
            message("Generating translation unit for binding_${TREE_TYPE}_${DIMENSION}_dimensional.cpp")
            configure_file(
                    translation_unit.cpp.in
                    ${CMAKE_CURRENT_BINARY_DIR}/binding_${TREE_TYPE}_${DIMENSION}_dimensional.cpp
            )
            list(APPEND EXPLICIT_TRANSLATION_UNITS ${CMAKE_CURRENT_BINARY_DIR}/binding_${TREE_TYPE}_${DIMENSION}_dimensional.cpp)
            file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h
                    "extern template void binding_for_N_dimensional<${DIMENSION},pareto::${TREE_TYPE}_tree_tag>(py::module &m, bool binding_default);\n"
            )
        endif()
    endforeach()
endforeach()
message(EXPLICIT_TRANSLATION_UNITS=${EXPLICIT_TRANSLATION_UNITS})
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/binding_n_dimensional_extern_templates.h
        "\n#endif // PARETO_FRONT_BINDING_N_DIMENSIONAL_EXTERN_TEMPLATES\n"
        )

#######################################################
### Create bindings                                 ###
#######################################################
# Create python bindings
pybind11_add_module(pyfront
        # header file with extern template for the instantiations we are going to use
        instantiation/python_instantiations.h

        # instantiate point
        instantiation/double_point_instantiations.cpp

        # instantiate query box
        instantiation/double_query_box_instantiations.cpp

        # custom equality comparison for py::object
        custom_equality.h
        custom_equality.cpp

        # instantiate trees
        instantiation/double_object_kd_tree_instantiations.cpp
        ${pyfront_tree_instantiation_sources}
        instantiation/double_object_vector_tree_instantiations.cpp

        # instantiate fronts
        instantiation/double_object_front_instantiations.cpp
        instantiation/double_object_front_kd_instantiations.cpp
        ${pyfront_front_instantiation_sources}
        instantiation/double_object_front_vector_instantiations.cpp

        # instantiate archives
        instantiation/double_object_archive_instantiations.cpp
        instantiation/double_object_archive_kd_instantiations.cpp
        ${pyfront_archive_instantiation_sources}
        instantiation/double_object_archive_vector_instantiations.cpp

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
target_link_libraries(pyfront PUBLIC pareto_front pybind11)

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
    # Find the python executable
    if (PYTHON_EXECUTABLE)
        # pybind11 should have found it already
        message("PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
    else()
        set(PYTHON_EXECUTABLE python)
    endif()

    # Python one-line script to find the site-packages directory
    # relative to ${CMAKE_INSTALL_PREFIX}. This script will try
    # to find a relative path first and use the absolute path
    # as a second option
    # https://stackoverflow.com/questions/7287996/python-get-relative-path-from-comparing-two-absolute-paths
    set(PYTHON_SITE_PACKAGES_SCRIPT "from distutils.sysconfig import get_python_lib; import os; print(os.path.relpath(get_python_lib(), '${CMAKE_INSTALL_PREFIX}') if os.path.commonpath(['${CMAKE_INSTALL_PREFIX}', get_python_lib()]) == '${CMAKE_INSTALL_PREFIX}' else get_python_lib())")

    # Try to find the python site-packages directory
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "${PYTHON_SITE_PACKAGES_SCRIPT}" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

    # 2st attempt
    if (NOT PYTHON_SITE_PACKAGES)
        execute_process(COMMAND python3 -c "${PYTHON_SITE_PACKAGES_SCRIPT}" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    # 3st attempt
    if (NOT PYTHON_SITE_PACKAGES)
        execute_process(COMMAND python2 -c "${PYTHON_SITE_PACKAGES_SCRIPT}" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    # Use the default lib directory if cannot find site-packages
    if (NOT PYTHON_SITE_PACKAGES)
        set(PYTHON_SITE_PACKAGES ${CMAKE_INSTALL_LIBDIR})
    endif()

    message("PYTHON_SITE_PACKAGES=${PYTHON_SITE_PACKAGES}")
    message("CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

    install(TARGETS pyfront
            COMPONENT "Python Library"
            LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES}
            ARCHIVE DESTINATION ${PYTHON_SITE_PACKAGES}
    )
endif()