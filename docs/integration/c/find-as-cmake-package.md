---
layout: default
title: Find as CMake package
nav_order: 5
has_children: false
parent: C++
grand_parent: Integration
has_toc: false
---
# Find as CMake package

If you are using CMake and have the library installed on your system, you can then find Pareto with the usual `find_package` command:

```cmake
find_package(Pareto REQUIRED)
# ...
target_link_libraries(my_target PUBLIC pareto)
```

Your target will be able to see the pareto headers now.

!!! warning "find_package on windows"
    There is no easy default directory for find_package on windows. You have to [set it](https://stackoverflow.com/questions/21314893/what-is-the-default-search-path-for-find-package-in-windows-using-cmake) yourself. 



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
