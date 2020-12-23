---
layout: default
title: Other build systems
nav_order: 4
has_children: false
parent: Integration
has_toc: false
---
# Other build systems

If you want to use it in another build system you can either install the library (Section [*Installing*](#installing)) or you have to somehow rewrite the build script.

If you want to rewrite the build script, your project needs to 1) include the headers and compile all source files in the [`sources`](https://github.com/alandefreitas/pareto/blob/master/) directory, and 2) link the dependencies described in [`sources/CMakeLists.txt`](https://github.com/alandefreitas/pareto/blob/master/).

Then add this header to your source files:

```cpp
#include <pareto_front/front.h>
```

Or to use Pareto archives:

```cpp
#include <pareto_front/archives.h>
```




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
