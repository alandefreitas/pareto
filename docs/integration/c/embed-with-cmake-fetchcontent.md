---
layout: default
title: Embed with CMake FetchContent
nav_order: 3
has_children: false
parent: C++
grand_parent: Integration
has_toc: false
---
# Embed with CMake FetchContent

FetchContent is a CMake command to automatically download the repository:

```cmake
include(FetchContent)

FetchContent_Declare(pareto
        GIT_REPOSITORY https://github.com/alandefreitas/pareto
        GIT_TAG origin/master # or whatever tag you want
)

FetchContent_GetProperties(pareto)
if(NOT pareto_POPULATED)
    FetchContent_Populate(pareto)
    add_subdirectory(${pareto_SOURCE_DIR} ${pareto_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# ...
target_link_libraries(my_target PRIVATE pareto)
```

Your target will be able to see the pareto headers now.



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
