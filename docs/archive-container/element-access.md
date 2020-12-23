---
layout: default
title: Element Access
nav_order: 5
has_children: false
parent: Archive Container
has_toc: false
---
# Element Access

| Method                                                       |
| ------------------------------------------------------------ |
| **MapContainer**                                             |
| Access and throw exception if it doesn't exist               |
| `mapped_type &at(const key_type &k);`                        |
| `const mapped_type &at(const key_type &k) const;`            |
| Access and create new element if it doesn't exist            |
| `mapped_type &operator[](https://github.com/alandefreitas/pareto/blob/master/);`                |
| `mapped_type &operator[](https://github.com/alandefreitas/pareto/blob/master/);`                     |
| `template <typename... Targs> mapped_type &operator()(const dimension_type &x1, const Targs &...xs);` |

**Parameters**

* `k` - the key of the element to find
* `x1` - the value of the element to find in the first dimension
* `xs` - the value of the element to find in other dimensions

**Return value**

A reference to the element associated with that key.

**Exceptions**

[`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) if the container does not have an element with the specified `key`

**Complexity**

$$
O(m \log n)
$$

**Notes**

Unlike in a `pareto::spatial_map`, the `insert` operation for archives is allowed to fail when the new element is already dominated by all fronts *and* exceeds the maximum capacity of the archive. In this case, the `operator[]` will return a reference to a placeholder that is not ultimately inserted in the front.

!!! info
    See the section on spatial containers / element access for more information.

**Example**

=== "C++"

    ```cpp
    archive<double, 3, unsigned> ar({min, max, min});
    // Set some values
    ar(-2.57664, -1.52034, 0.600798) = 17;
    ar(-2.14255, -0.518684, -2.92346) = 32;
    ar(-1.63295, 0.912108, -2.12953) = 36;
    ar(-0.653036, 0.927688, -0.813932) = 13;
    ar(-0.508188, 0.871096, -2.25287) = 32;
    ar(-2.55905, -0.271349, 0.898137) = 6;
    ar(-2.31613, -0.219302, 0) = 8;
    ar(-0.639149, 1.89515, 0.858653) = 10;
    ar(-0.401531, 2.30172, 0.58125) = 39;
    ar(0.0728106, 1.91877, 0.399664) = 25;
    ar(-1.09756, 1.33135, 0.569513) = 20;
    ar(-0.894115, 1.01387, 0.462008) = 11;
    ar(-1.45049, 1.35763, 0.606019) = 17;
    ar(0.152711, 1.99514, -0.112665) = 13;
    ar(-2.3912, 0.395611, 2.78224) = 11;
    ar(-0.00292544, 1.29632, -0.578346) = 20;
    ar(0.157424, 2.30954, -1.23614) = 6;
    ar(0.453686, 1.02632, -2.24833) = 30;
    ar(0.693712, 1.12267, -1.37375) = 12;
    ar(1.49101, 3.24052, 0.724771) = 24;

    // Access value
    if (ar.contains({1.49101, 3.24052, 0.724771})) {
        std::cout << "Element access: " << ar(1.49101, 3.24052, 0.724771) << std::endl;
    } else {
        std::cout << "{1.49101, 3.24052, 0.724771} was dominated" << std::endl;
    }
    ```

=== "Python"

    ```python
    ar = pareto.archive()
    # Set some values
    ar[-2.57664, -1.52034, 0.600798] = 17
    ar[-2.14255, -0.518684, -2.92346] = 32
    ar[-1.63295, 0.912108, -2.12953] = 36
    ar[-0.653036, 0.927688, -0.813932] = 13
    ar[-0.508188, 0.871096, -2.25287] = 32
    ar[-2.55905, -0.271349, 0.898137] = 6
    ar[-2.31613, -0.219302, 0] = 8
    ar[-0.639149, 1.89515, 0.858653] = 10
    ar[-0.401531, 2.30172, 0.58125] = 39
    ar[0.0728106, 1.91877, 0.399664] = 25
    ar[-1.09756, 1.33135, 0.569513] = 20
    ar[-0.894115, 1.01387, 0.462008] = 11
    ar[-1.45049, 1.35763, 0.606019] = 17
    ar[0.152711, 1.99514, -0.112665] = 13
    ar[-2.3912, 0.395611, 2.78224] = 11
    ar[-0.00292544, 1.29632, -0.578346] = 20
    ar[0.157424, 2.30954, -1.23614] = 6
    ar[0.453686, 1.02632, -2.24833] = 30
    ar[0.693712, 1.12267, -1.37375] = 12
    ar[1.49101, 3.24052, 0.724771] = 24
    
    # Access value
    if [1.49101, 3.24052, 0.724771] in ar:
        print('Element access:', ar[1.49101, 3.24052, 0.724771])
    else:
        print("[1.49101, 3.24052, 0.724771] was dominated")

    ```

=== "Output"

    ```console
    Element access: 24
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
