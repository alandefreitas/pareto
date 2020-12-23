---
layout: default
title: Iterators
nav_order: 6
has_children: false
parent: Front Container
has_toc: false
---
# Iterators

| Method                                                       |
| ------------------------------------------------------------ |
| **MultimapContainer**                                        |
| Get constant iterators                                       |
| `const_iterator begin() const noexcept;`                     |
| `const_iterator end() const noexcept;`                       |
| `const_iterator cbegin() const noexcept;`                    |
| `const_iterator cend() const noexcept;`                      |
| Get iterators                                                |
| `iterator begin() noexcept;`                                 |
| `iterator end() noexcept;`                                   |
| Get reverse iterators                                        |
| `std::reverse_iterator<const_iterator> rbegin() const noexcept;` |
| `std::reverse_iterator<const_iterator> rend() const noexcept;` |
| `std::reverse_iterator<iterator> rbegin() noexcept`;         |
| `std::reverse_iterator<iterator> rend() noexcept;`           |
| Get constant reverse iterators                               |
| `std::reverse_iterator<const_iterator> crbegin() const noexcept;` |
| `std::reverse_iterator<const_iterator> crend() const noexcept;` |

**Return value**

* `begin()` - Iterator to the first element in the container
* `end()` - Iterator to the past-the-end element in the container (see notes)

**Complexity**

$$
O(1)
$$

**Notes**

All requirements of a **SpatialContainer** also apply here.

!!! info
    See the section on spatial containers / iterators for more information.

**Example**

Continuing from the previous example:

=== "C++"

    ```cpp
    std::cout << "Iterators:" << std::endl;
    for (const auto& [point, value]: pf) {
        std::cout << point << " -> " << value << std::endl;
    }

    std::cout << "Reversed Iterators:" << std::endl;
    for (auto it = pf.rbegin(); it != pf.rend(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
    ```

=== "Python"

    ```python
    print('Iterators')
    for [point, value] in m:
        print(point, '->', value)
    
    print('Reversed Iterators')
    for [point, value] in reversed(m):
        print(point, '->', value)
    ```

=== "Output"

    ```console
    Iterators:
    [-2.14255, -0.518684, -2.92346] -> 32
    [-1.63295, 0.912108, -2.12953] -> 36
    [-0.653036, 0.927688, -0.813932] -> 13
    [-0.508188, 0.871096, -2.25287] -> 32
    [0.453686, 1.02632, -2.24833] -> 30
    [0.693712, 1.12267, -1.37375] -> 12
    [-2.57664, -1.52034, 0.600798] -> 17
    [-2.55905, -0.271349, 0.898137] -> 6
    [-2.31613, -0.219302, 0] -> 8
    [-0.894115, 1.01387, 0.462008] -> 11
    [-2.3912, 0.395611, 2.78224] -> 11
    [-0.639149, 1.89515, 0.858653] -> 10
    [-0.401531, 2.30172, 0.58125] -> 39
    [-1.09756, 1.33135, 0.569513] -> 20
    [-1.45049, 1.35763, 0.606019] -> 17
    [-0.00292544, 1.29632, -0.578346] -> 20
    [0.0728106, 1.91877, 0.399664] -> 25
    [0.152711, 1.99514, -0.112665] -> 13
    [0.157424, 2.30954, -1.23614] -> 6
    [1.49101, 3.24052, 0.724771] -> 24
    Reversed Iterators:
    [1.49101, 3.24052, 0.724771] -> 24
    [0.157424, 2.30954, -1.23614] -> 6
    [0.152711, 1.99514, -0.112665] -> 13
    [0.0728106, 1.91877, 0.399664] -> 25
    [-0.00292544, 1.29632, -0.578346] -> 20
    [-1.45049, 1.35763, 0.606019] -> 17
    [-1.09756, 1.33135, 0.569513] -> 20
    [-0.401531, 2.30172, 0.58125] -> 39
    [-0.639149, 1.89515, 0.858653] -> 10
    [-2.3912, 0.395611, 2.78224] -> 11
    [-0.894115, 1.01387, 0.462008] -> 11
    [-2.31613, -0.219302, 0] -> 8
    [-2.55905, -0.271349, 0.898137] -> 6
    [-2.57664, -1.52034, 0.600798] -> 17
    [0.693712, 1.12267, -1.37375] -> 12
    [0.453686, 1.02632, -2.24833] -> 30
    [-0.508188, 0.871096, -2.25287] -> 32
    [-0.653036, 0.927688, -0.813932] -> 13
    [-1.63295, 0.912108, -2.12953] -> 36
    [-2.14255, -0.518684, -2.92346] -> 32
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
