#[=======================================================================[.rst:

FindPMR
This module looks for the C++17 standard library's PMR utilities.

#]=======================================================================]

if(TARGET std::pmr)
    # This module has already been processed. Don't do it again.
    return()
endif()

cmake_minimum_required(VERSION 3.10)

include(CMakePushCheckState)
include(CheckIncludeFileCXX)

# If we're not cross-compiling, try to run test executables.
# Otherwise, assume that compile + link is a sufficient check.
if(CMAKE_CROSSCOMPILING)
    include(CheckCXXSourceCompiles)
    macro(_cmcm_check_cxx_source code var)
        check_cxx_source_compiles("${code}" ${var})
    endmacro()
else()
    include(CheckCXXSourceRuns)
    macro(_cmcm_check_cxx_source code var)
        check_cxx_source_runs("${code}" ${var})
    endmacro()
endif()

cmake_push_check_state()

set(CMAKE_REQUIRED_QUIET ${PMR_FIND_QUIETLY})

# All of our tests required C++17 or later
set(CMAKE_CXX_STANDARD 17)

# Check include
check_include_file_cxx("memory_resource" _CXX_PMR_HAVE_HEADER)
mark_as_advanced(_CXX_PMR_HAVE_HEADER)

# We have hope that PMR is there so initialize variables
if(_CXX_PMR_HAVE_HEADER)
    set(_have_pmr TRUE)
    set(_pmr_header memory_resource)
    set(_pmr_namespace std::pmr)
else()
    set(_have_pmr FALSE)
endif()

# Initialize results
# We don't bother to look for experimental pmr because vendors (like GCC) have pmr implemented
# in <memory_resource> and some others (like Clang) have not implemented in <memory_resource> nor in
# <experimental/memory_resource>
set(CXX_PMR_HAVE_PMR_HEADER ${_have_pmr} CACHE BOOL "TRUE if we have the C++ pmr headers")
set(CXX_PMR_HEADER ${_pmr_header} CACHE STRING "The header that should be included to obtain the pmr APIs")
set(CXX_PMR_NAMESPACE ${_pmr_namespace} CACHE STRING "The C++ namespace that contains the pmr APIs")

set(_found FALSE)

if(CXX_PMR_HAVE_PMR_HEADER)
    # We have some pmr library available.
    # Try to compile something
    string(CONFIGURE [[
        #include <cstdlib>
        #include <map>
        #include <@CXX_PMR_HEADER@>

        int main() {
            std::map<int,int,std::less<>,std::pmr::polymorphic_allocator<std::pair<const int, int>>> m;
            m.emplace(2,3);
            return 0;
        }
    ]] code @ONLY)

    # Try to run it and get result
    _cmcm_check_cxx_source("${code}" CXX_PMR_RUN_SUCCESSFULLY)

    if(CXX_PMR_RUN_SUCCESSFULLY)
        add_library(std::pmr INTERFACE IMPORTED)
        set_property(TARGET std::pmr APPEND PROPERTY INTERFACE_COMPILE_FEATURES cxx_std_17)
        set(_found TRUE)
    endif()
endif()

cmake_pop_check_state()

set(PMR_FOUND ${_found} CACHE BOOL "TRUE if we can run a program using std::pmr" FORCE)

if(PMR_FIND_REQUIRED AND NOT PMR_FOUND)
    message(FATAL_ERROR "Cannot run simple program using std::pmr")
endif()