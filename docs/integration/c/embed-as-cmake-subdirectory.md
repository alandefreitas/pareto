---
layout: default
title: Embed as CMake subdirectory
nav_order: 2
has_children: false
parent: C++
grand_parent: Integration
has_toc: false
---
# Embed as CMake subdirectory

You can use pareto directly in CMake projects as a subproject. 

Clone the whole project inside your own project:

```bash
git clone https://github.com/alandefreitas/pareto/
```

and add the subdirectory to your CMake script:

```cmake
add_subdirectory(pareto)
```

When creating your executable, link the library to the targets you want:

```cmake
add_executable(my_target main.cpp)
target_link_libraries(my_target PRIVATE pareto)
```

Your target will be able to see the pareto headers now.



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
