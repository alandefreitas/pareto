---
layout: default
title: Dependencies
nav_order: 1
has_children: false
parent: Building
grand_parent: Integration
has_toc: false
---
# Dependencies

**C++**

Update your C++ compiler to at least C++17:

=== "Ubuntu"

    ```bash
    # install GCC10
    sudo apt install build-essential
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo apt install gcc-10
    sudo apt install g++-10
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 10
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 10
    # Choose gcc-10 there as the default compiler
    update-alternatives --config g++
    ```

=== "Mac OS"

    ```bash
    # Download clang
    curl --output clang.tar.xz -L https://github.com/llvm/llvm-project/releases/download/llvmorg-11.0.0/clang+llvm-11.0.0-x86_64-apple-darwin.tar.xz
    mkdir clang
    tar -xvJf clang.tar.xz -C clang
    # Copy the files to use/local
    cd clang/clang+llvm-11.0.0-x86_64-apple-darwin
    sudo cp -R * /usr/local/
    # Make it your default compiler
    export CXX=/usr/local/bin/clang++
    ```

=== "Windows"

    Update your [Visual Studio Compiler](https://visualstudio.microsoft.com/).

**CMake**

Update your CMake to at least CMake 3.16+. You can check your CMake version with:

```bash
cmake --version
```

If you need to update it, then

=== "Ubuntu + apt"

    ```bash
    sudo apt upgrade cmake
    ```

=== "Mac OS + Homebrew"

    ```bash
    sudo brew upgrade cmake
    ```

=== "Website"

    Download CMake from [https://cmake.org/download/](https://cmake.org/download/) and install it

**Python**

Make sure you have Python 3.6.9+ installed:

```bash
python3 --version
```

If you need to update, then

=== "Ubuntu"
    
    Use `apt-get` or download it from https://www.python.org/downloads/.

=== "Mac OS"

    ```bash
    sudo brew upgrade python3
    ```

    or download the latest release version from https://www.python.org/downloads/

=== "Windows"

    Download Python from [https://www.python.org/downloads/](https://www.python.org/downloads/) and install it

If using a Python installer, make sure you add the application directory to your PATH environment variable.



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
