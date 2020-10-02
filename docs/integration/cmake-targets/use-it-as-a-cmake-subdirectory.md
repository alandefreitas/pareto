---
layout: default
title: Use it as a CMake subdirectory
nav_order: 2
has_children: false
parent: CMake targets
grand_parent: Integration
has_toc: false
---
# Use it as a CMake subdirectory

You can use pareto-front directly in CMake projects without installing it. Check if you have [Cmake](http://cmake.org) 3.14+ installed:

```bash
cmake -version
```

Clone the whole project
 
```bash
git clone https://github.com/alandefreitas/pareto-front/
```

and add the subdirectory to your CMake project:

```cmake
add_subdirectory(pareto-front)
```

When creating your executable, link the library to the targets you want:

```
add_executable(my_target main.cpp)
target_link_libraries(my_target PUBLIC pareto-front)
```

Add this header to your source files:

```cpp
#include <pareto_front/front.h>
```

Or to use Pareto archives:

```cpp
#include <pareto_front/archives.h>
```

However, it's always recommended to look for ParetoFront with `find_package` before including it as a subdirectory. Otherwise, we can get [ODR errors](https://en.wikipedia.org/wiki/One_Definition_Rule) in larger projects. 




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
