---
layout: default
title: Building
nav_order: 2
has_children: false
parent: Building
grand_parent: Integration
has_toc: false
---
# Building

After installing or updating the dependencies, clone the project with

```bash
git clone https://github.com/alandefreitas/pareto.git
cd pareto
```

and then build it with

=== "Ubuntu"

    ```bash
    mkdir build
    cd build
    cmake -version
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2"
    cmake --build . -j 2 --config Release
    # The next command for installing
    sudo cmake --install .
    # The next command for building the packages / installers
    sudo cpack .
    ```

=== "Mac OS"

    ```bash
    mkdir build
    cd build
    cmake -version
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2"
    cmake --build . -j 2 --config Release
    # The next command for installing
    cmake --install .
    # The next command for building the packages / installers
    cpack .
    ```

=== "Windows"

    ```bash
    mkdir build
    cd build
    cmake -version
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/O2"
    cmake --build . -j 2 --config Release
    # The next command for installing
    cmake --install .
    # The next command for building the packages / installers
    cpack .
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
