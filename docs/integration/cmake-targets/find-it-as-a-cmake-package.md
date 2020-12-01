---
layout: default
title: Find it as a CMake Package
nav_order: 1
has_children: false
parent: CMake targets
grand_parent: Integration
has_toc: false
---
# Find it as a CMake Package

If you have the library installed, you can call

```cmake
find_package(ParetoFront)
```

from your CMake build script. 

When creating your executable, link the library to the targets you want:

```
add_executable(my_target main.cpp)
target_link_libraries(my_target PUBLIC pareto_front)
```

Add this header to your source files:

```cpp
#include <pareto_front/front.h>
```

Or to use Pareto archives:

```cpp
#include <pareto_front/archives.h>
```




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
