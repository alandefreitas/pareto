---
layout: default
title: Element Access
nav_order: 5
has_children: false
parent: Spatial Containers
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
| `mapped_type &operator[] (const key_type &k);`                |
| `mapped_type &operator[] (key_type &&k);`                     |
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

While the `at` function throws an error when the element is not found, `operator[]` creates a new element with that key if the element is not found. Like other libraries that handle multidimensional data, we use the `operator()` for element access as a convenience because the `operator[]` does not allow multiple parameters. We can still use `operator[]` with a `front::key_type` though. 

!!! note
    Like `std::map`, and unlike `std::multimap`, spatial containers implement the element access operators even though duplicate keys are permitted. The reason `std::multimap` does not implement these operators is because the operator might be ambiguous when there is more than one element that matches the given key. 

    By convention we formally remove this ambiguity by always using the first element that matches that key. It's up to the library user to decide if this behaviour is appropriate for their application. If not, the modifier functions should be used instead.

**Example**

=== "C++"

    ```cpp
    spatial_map<double, 3, unsigned> m;
    // Set some values
    m(-2.57664, -1.52034, 0.600798) = 17;
    m(-2.14255, -0.518684, -2.92346) = 32;
    m(-1.63295, 0.912108, -2.12953) = 36;
    m(-0.653036, 0.927688, -0.813932) = 13;
    m(-0.508188, 0.871096, -2.25287) = 32;
    m(-2.55905, -0.271349, 0.898137) = 6;
    m(-2.31613, -0.219302, 0) = 8;
    m(-0.639149, 1.89515, 0.858653) = 10;
    m(-0.401531, 2.30172, 0.58125) = 39;
    m(0.0728106, 1.91877, 0.399664) = 25;
    m(-1.09756, 1.33135, 0.569513) = 20;
    m(-0.894115, 1.01387, 0.462008) = 11;
    m(-1.45049, 1.35763, 0.606019) = 17;
    m(0.152711, 1.99514, -0.112665) = 13;
    m(-2.3912, 0.395611, 2.78224) = 11;
    m(-0.00292544, 1.29632, -0.578346) = 20;
    m(0.157424, 2.30954, -1.23614) = 6;
    m(0.453686, 1.02632, -2.24833) = 30;
    m(0.693712, 1.12267, -1.37375) = 12;
    m(1.49101, 3.24052, 0.724771) = 24;
    // Access value
    std::cout << "Element access: " << m(1.49101, 3.24052, 0.724771) << std::endl;
    ```

=== "Python"

    ```python
    m = pareto.spatial_map()
    # Set some values
    m[-2.57664, -1.52034, 0.600798] = 17
    m[-2.14255, -0.518684, -2.92346] = 32
    m[-1.63295, 0.912108, -2.12953] = 36
    m[-0.653036, 0.927688, -0.813932] = 13
    m[-0.508188, 0.871096, -2.25287] = 32
    m[-2.55905, -0.271349, 0.898137] = 6
    m[-2.31613, -0.219302, 0] = 8
    m[-0.639149, 1.89515, 0.858653] = 10
    m[-0.401531, 2.30172, 0.58125] = 39
    m[0.0728106, 1.91877, 0.399664] = 25
    m[-1.09756, 1.33135, 0.569513] = 20
    m[-0.894115, 1.01387, 0.462008] = 11
    m[-1.45049, 1.35763, 0.606019] = 17
    m[0.152711, 1.99514, -0.112665] = 13
    m[-2.3912, 0.395611, 2.78224] = 11
    m[-0.00292544, 1.29632, -0.578346] = 20
    m[0.157424, 2.30954, -1.23614] = 6
    m[0.453686, 1.02632, -2.24833] = 30
    m[0.693712, 1.12267, -1.37375] = 12
    m[1.49101, 3.24052, 0.724771] = 24
    # Access value
    print('Element access:', m[1.49101, 3.24052, 0.724771])
    ```

=== "Output"

    ```console
    Element access: 24
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
