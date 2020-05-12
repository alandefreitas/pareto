# Geometry package depends on LOTs of other boost libraries
CPMAddPackage(
        NAME boost-cmake
        GITHUB_REPOSITORY Orphis/boost-cmake
        GIT_TAG d3951bc7f0b9d09005f92aedcf6acfc595f050ea
        VERSION 1.71.0
        FIND_PACKAGE_ARGUMENTS "COMPONENTS system"
)

# Pareto front container
add_library(pareto_front INTERFACE)
target_include_directories(pareto_front INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pareto_front INTERFACE Boost::boost)