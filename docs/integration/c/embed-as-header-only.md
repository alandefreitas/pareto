---
layout: default
title: Embed as header-only
nav_order: 1
has_children: false
parent: C++
grand_parent: Integration
has_toc: false
---
# Embed as header-only

Copy the files from the `source` directory of this project to your `include` directory.

If you want to use `std::pmr` allocators by default, set the macro `BUILD_PARETO_WITH_PMR` before including the files.

=== "C++"

    ```cpp
    #def BUILD_PARETO_WITH_PMR
    #include <pareto/front.h>
    ```

Each header in `pareto` represents a data structure.

!!! warning
    Make sure you have C++17+ installed



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
