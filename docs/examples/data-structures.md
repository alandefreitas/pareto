---
layout: default
title: Data Structures
nav_order: 2
has_children: false
parent: Examples
has_toc: false
---
# Data Structures

Note that the front dimension is defined at compile-time. Using the results from our benchmarks (See Section [Benchmarks](../benchmarks.md)), the default constructor will choose an appropriate spatial index to represent the front. Also, like a C++ `map` or a Python `dict`, each point in space is associated with an object. In your application, `unsigned` would probably be replaced by a pointer to an object that has the `double` attributes. 


The interface permits choosing a specific data structure for the front through tags:

```python
pf = pyfront.front('list', 2)
pf = pyfront.front('quadtree', 2)
pf = pyfront.front('kdtree', 2)
pf = pyfront.front('boostrtree', 2)
pf = pyfront.front('rtree', 2)
pf = pyfront.front('rstartree', 2)
```

```cpp
front<double, 2, unsigned, vector_tree_tag> pf;
front<double, 2, unsigned, quad_tree_tag> pf;
front<double, 2, unsigned, kd_tree_tag> pf;
front<double, 2, unsigned, boost_tree_tag> pf;
front<double, 2, unsigned, r_tree_tag> pf;
front<double, 2, unsigned, r_star_tree_tag> pf;
```




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
