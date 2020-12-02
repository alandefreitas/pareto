---
layout: default
title: Packages
nav_order: 1
has_children: false
parent: Integration
has_toc: false
---
# Packages

Get one of binary packages from the [release section](https://github.com/alandefreitas/pareto/releases). 

* Python Binary <OS>: this is only the binary for Python. Copy this file to your site-packages directory.
* pareto-<version>-<OS>.<package extension>: these packages contain the Python bindings and the C++ library.
* Binary Packages <OS>: these packages contain all packages for a given OS.

If using one the installers, make sure you install the Python bindings to your site-packages directory (this is the default directory for most packages). You can find your site-packages directory with:

```bash
python -c "from distutils.sysconfig import get_python_lib; print(get_python_lib());"
``` 

These binaries refer to the last release version. If you need a more recent version of pareto, you can download the [binary packages from the CI artifacts](https://github.com/alandefreitas/pareto/actions?query=workflow%3AParetoFront+event%3Apush) or build the library [from the source files](build-from-source.md). 

Once the package is installed, you can use the Python library with

```
import pyfront
```
 
or link your C++ program to the library and include the directories where you installed pareto. 

Unless you changed the default options, the C++ library is likely to be in `/usr/local/` (Linux / Mac OS) or `C:/Program Files/` (Windows). The installer will try to find the directory where you usually keep your  libraries but that's not always perfect.

If you are using CMake, you can then find pareto with the usual `find_package` command:

```cmake
find_package(ParetoFront REQUIRED)
# ...
target_link_libraries(my_target PUBLIC pareto_front)
```

CMake should be able to locate the `ParetoFrontConfig.cmake` script automatically if you installed the library under `/usr/local/` (Linux / Mac OS) or `C:/Program Files/` (Windows). Otherwise, you need to include your installation directory in `CMAKE_MODULE_PATH` first: 

```cmake
list(APPEND CMAKE_MODULE_PATH put/your/installation/directory/here)
find_package(ParetoFront REQUIRED)
# ...
target_link_libraries(my_target PUBLIC pareto_front)
```




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
