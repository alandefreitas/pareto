---
layout: default
title: Embed with CPM.cmake
nav_order: 4
has_children: false
parent: C++
grand_parent: Integration
has_toc: false
---
# Embed with CPM.cmake

[CPM.cmake](https://github.com/TheLartians/CPM.cmake) is a nice wrapper around the CMake FetchContent function. Install [CPM.cmake](https://github.com/TheLartians/CPM.cmake) and then use this command to add Pareto to your build script:

```cmake
CPMAddPackage(
        NAME Pareto
        GITHUB_REPOSITORY alandefreitas/pareto
        GIT_TAG origin/master # or whatever tag you want
)
# ...
target_link_libraries(my_target PUBLIC pareto)
```

Your target will be able to see the pareto headers now.



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
