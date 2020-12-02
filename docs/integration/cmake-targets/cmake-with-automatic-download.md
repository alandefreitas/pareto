---
layout: default
title: CMake with Automatic Download
nav_order: 3
has_children: false
parent: CMake targets
grand_parent: Integration
has_toc: false
---
# CMake with Automatic Download

Check if you have [Cmake](http://cmake.org) 3.14+ installed:

```bash
cmake -version
```

Install [CPM.cmake](https://github.com/TheLartians/CPM.cmake) and then:

```cmake
CPMAddPackage(
    NAME ParetoFront
    GITHUB_REPOSITORY alandefreitas/pareto
    GIT_TAG origin/master # or whatever tag you want
)
# ...
target_link_libraries(my_target PUBLIC pareto)
```

Then add this header to your source files:

```cpp
#include <pareto_front/front.h>
```

Or to use Pareto archives:

```cpp
#include <pareto_front/archives.h>
```

However, it's always recommended to look for ParetoFront with `find_package` before including it as a subdirectory. You can use:

```
option(CPM_USE_LOCAL_PACKAGES "Try `find_package` before downloading dependencies" ON)
```

to let CPM.cmake do that for you. Otherwise, we can get [ODR errors](https://en.wikipedia.org/wiki/One_Definition_Rule) in larger projects.




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
