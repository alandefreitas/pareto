---
layout: default
title: Installing
nav_order: 3
has_children: false
parent: Integration
has_toc: false
---
# Installing

Get one of binary packages from the [release section](https://github.com/alandefreitas/pareto/releases). These file names have the following syntax:

* Python Binary <OS>
    * This is only the binary for Python.
    * Copy this file to your site-packages directory or to your project directory.
    * No need to `pip install`
* pareto-< version >-< OS >.< package extension >
    * These packages contain the Python bindings and the C++ library.
* Binary Packages < OS >
    * These files contain all packages for a given OS.

If using one the installers, make sure you install the Python bindings to your site-packages directory (this is the
default directory for most packages). You can find your site-packages directory with:

```bash
python -c "from distutils.sysconfig import get_python_lib; print(get_python_lib());"
```

These binaries refer to the last release version. If you need a more recent version of pareto, you can download
the [binary packages from the CI artifacts](https://github.com/alandefreitas/pareto/actions?query=workflow%3APareto+event%3Apush)
or build the library [from the source files](#build-from-source).

Once the package is installed, you can use the Python library with

```
import pareto
```

or link your C++ program to the library and include the directories where you installed pareto.

Unless you changed the default options, the C++ library is likely to be in `/usr/local/` (Linux / Mac OS)
or `C:/Program Files/` (Windows). The installer will try to find the directory where you usually keep your libraries but
that's not always perfect.

CMake should be able to locate the `ParetoConfig.cmake` script automatically if you installed the library
under `/usr/local/` (Linux / Mac OS). 

!!! warning "find_package on windows"
    There is no easy default directory for `find_package` on windows. You have to [set it](https://stackoverflow.com/questions/21314893/what-is-the-default-search-path-for-find-package-in-windows-using-cmake) yourself.



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
