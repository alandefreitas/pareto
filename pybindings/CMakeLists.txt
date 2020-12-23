#######################################################
### Pybind11 for Python bindings                    ###
#######################################################
# Find or get pybind11
# Note that CPM will try to "find_package" before downloading it
# See the option CPM_USE_LOCAL_PACKAGES in ROOT/CMakeLists.txt
# This is important: see One Definition Rule (ODR)
CPMAddPackage(
        NAME pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11/
        VERSION 2.6.1
        OPTIONS
        "PYBIND11_TEST OFF"
        "PYBIND11_INSTALL OFF"
)

#######################################################
### Create bindings                                 ###
#######################################################
# Create python bindings
pybind11_add_module(pareto_python pareto_python.cpp)
target_link_libraries(pareto_python PRIVATE pareto)
set_target_properties(pareto_python PROPERTIES OUTPUT_NAME pareto)

# Change visibility
# https://www.rapidtables.com/code/linux/gcc/gcc-o.html
# https://pybind11.readthedocs.io/en/stable/faq.html#someclass-declared-with-greater-visibility-than-the-type-of-its-field-someclass-member-wattributes
if (NOT MSVC)
    target_compile_options(pareto_python PRIVATE -fvisibility=hidden)
endif()
target_bigobj_options(pareto_python)
target_exception_options(pareto_python)

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

    install(TARGETS pareto_python
            COMPONENT "Python_Library"
            LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES}
            ARCHIVE DESTINATION ${PYTHON_SITE_PACKAGES}
    )
endif()