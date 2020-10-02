---
layout: default
title: Limitations
nav_order: 8
has_children: false
has_toc: false
---
# Limitations

One limitation of the library interface is that the most efficient data structures have their dimension defined at compile-time. This is not an inconvenience to C++ users but it makes it impossible to have Python bindings for all front dimensions defined at compile-time. 

Our solution was to include the fronts with <img src="https://render.githubusercontent.com/render/math?math=m=1,...,10"> dimensions set at compile-time in our Python bindings. For each data structure, we provide a extra implementation that has its dimension determined at runtime. The Python bindings fallback to this special implementation whenever <img src="https://render.githubusercontent.com/render/math?math=m>10">. 

This value can be altered when compiling the library:
 
```bash
mkdir build
cmake -version
cmake .. -DMAX_NUM_DIMENSIONS_PYTHON=20 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2"
cmake --build . -j 2 --config Release
```

 Change the value of `MAX_NUM_DIMENSIONS_PYTHON` to whatever number of dimensions you need in compile-time. The higher this value, the larger the final binary. However, you shouldn't usually need to change this value. The rationale is that, due to the [curse of dimensionality](https://en.wikipedia.org/wiki/Curse_of_dimensionality), if you have a problem with more than 10 dimensions, if you have bigger problems than whether the dimension is set at compile or run-time. 




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
