---
layout: default
title: Dependencies
nav_order: 1
has_children: false
parent: Build from source
grand_parent: Integration
has_toc: false
---
# Dependencies

This section lists the dependencies you need before installing pareto-front from source. The build script will try to find all these dependencies for you:

* C++17
* CMake 3.14 or higher
* Python 3.6.9 or higher (for the Python bindings)


Instructions: Linux/Ubuntu/GCC
    
Check your GCC version

```bash
g++ --version
```

The output should have something like

```console
g++-8 (Ubuntu 8.4.0-1ubuntu1~18.04) 8.4.0
```

If you see a version before GCC-8, update it with

```bash
sudo apt update
sudo apt install gcc-8
sudo apt install g++-8
```

To update to any other version, like GCC-9 or GCC-10:

```bash
sudo apt install build-essential
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt install g++-10
```

Once you installed a newer version of GCC, you can link it to `update-alternatives`. For instance, if you have GCC-7 and GCC-10, you can link them with:

```bash
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 7
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 7
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 10
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 10
```

You can now use `update-alternatives` to set you default `gcc` and `g++`:

```bash
update-alternatives --config g++
update-alternatives --config gcc
```

Check your CMake version:

```bash
cmake --version
```

If it's older than CMake 3.14, update it with

```bash
sudo apt upgrade cmake
```

or download the most recent version from [cmake.org](https://cmake.org/).

[Later](build-the-examples.md) when running CMake, make sure you are using GCC-8 or higher by appending the following options:

```bash
-DCMAKE_C_COMPILER=/usr/bin/gcc-8 -DCMAKE_CXX_COMPILER=/usr/bin/g++-8
```

If building the Python bindings, check your Python version:

```bash
python3 --version
```

If it's older than Python 3.6.9, update it with `apt-get` or download the lastest release version from https://www.python.org/downloads/ . If using an installer, make sure you add the application directory to your PATH environment variable.




Instructions: Mac Os/Clang

Check your Clang version:

```bash
clang --version
```

The output should have something like

```console
Apple clang version 11.0.0 (clang-1100.0.33.8)
```

If you see a version before Clang 11, update XCode in the App Store or update clang with homebrew. 

Check your CMake version:

```bash
cmake --version
```

If it's older than CMake 3.14, update it with

```bash
sudo brew upgrade cmake
```

or download the most recent version from [cmake.org](https://cmake.org/).

If the last command fails because you don't have [Homebrew](https://brew.sh) on your computer, you can install it with

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```

or you can follow the instructions in [https://brew.sh](https://brew.sh).

If building the Python bindings, check your Python version:

```bash
python3 --version
```

If it's older than Python 3.6.9, update it

```bash
sudo brew upgrade python3
```

or download the latest release version from https://www.python.org/downloads/ . If using an installer, make sure you add the application directory to your PATH environment variable.




Instructions: Windows/MSVC
    
* Make sure you have a recent version of [Visual Studio](https://visualstudio.microsoft.com)
* Download Git from [https://git-scm.com/download/win](https://git-scm.com/download/win) and install it
* Download CMake from [https://cmake.org/download/](https://cmake.org/download/) and install it
* Download Python from [https://www.python.org/downloads/](https://www.python.org/downloads/) and install it

If using the Python installer, make sure you add the application directory to your PATH environment variable.







<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
