---
layout: default
title: Spatial Containers
nav_order: 1
has_children: false
parent: Quick start
has_toc: false
---
# Spatial Containers

This library defines and implements **spatial containers**, which are an extension of the *AssociativeContainer* named requirement for multi-dimensional containers:

=== "C++"

    ```cpp hl_lines="4"
    // Unidimensional associative container 
    std::map<double, unsigned> m;
    // Multidimensional associative container
    pareto::spatial_map<double, 3, unsigned> n;
    ```

=== "Python"

    ```python hl_lines="4"
    # Unidimensional associative container
    m = sortedcontainers.SortedDict()
    # Multidimensional associative container
    n = pareto.spatial_map(3)
    ```

Spatial containers allow you to later find its elements with query iterators:

=== "C++"

    ```cpp
    spatial_map<double, 2, unsigned> m;
    m(-2.5, -1.5) = 17;
    m(-2.1, -0.5) = 32;
    m(-1.6, 0.9) = 36;
    m(-0.6, 0.9) = 13;
    m(-0.5, 0.8) = 32;
    std::cout << "Closest elements to [0, 0]:" << std::endl;
    for (auto it = m.find_nearest({0.,0.}, 2); it != m.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "Elements between [-1, -1] and [+1, +1]:" << std::endl;
    for (auto it = m.find_intersection({-1.,-1.}, {+1, +1}); it != m.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    ```

=== "Python"

    ```python
    m = pareto.spatial_map()
    m[-2.5, -1.5] = 17
    m[-2.1, -0.5] = 32
    m[-1.6, 0.9] = 36
    m[-0.6, 0.9] = 13
    m[-0.5, 0.8] = 32
    print("Closest elements to [0, 0]:")
    for [k, v] in m.find_nearest(pareto.point([0.,0.]), 2):
        print(k, ":", v)
    
    print("Elements between [-1, -1] and [+1, +1]:")
    for [k, v] in m.find_intersection(pareto.point([-1.,-1.]), pareto.point([+1, +1])):
        print(k, ":", v)

    ```

=== "Output"

    ```console
    Closest elements to [0, 0]:
    [-0.5, 0.8]: 32
    [-0.6, 0.9]: 13
    Elements between [-1, -1] and [+1, +1]:
    [-0.6, 0.9]: 13
    [-0.5, 0.8]: 32
    ```

Multi-dimensional associative containers are useful in applications where you need to simultaneously order objects according to a number for criteria, such as in:

* games
* maps
* nearest neighbor search
* range search
* compression algorithms
* statistics
* mechanics
* graphics libraries
* database queries. 
  
Many applications already need to implement such kinds of containers, although in a less generic way.

!!! info "Complexity"
    Inserting, removing, and finding solutions cost $O(m \log n)$, where $m$ is the number of dimensions and $n$ is the number of elements. 

!!! tip "Unidimensional Spatial Containers"
    When $m=1$, a `pareto::spatial_map` internally decays into a `std::multimap`, which is useful for applications where we don't know $m$ beforehand or need to handle many possible values of $m$ without maintaining two different implementations.

!!! info "Runtime dimensions"
    Some problems are so dynamic that even the number of dimensions changes at runtime. In these applications, you can set the number of compile-time dimensions to `0`, and the containers will accept keys with any number of dimensions. This, of course, comes at a cost of an extra dynamic memory allocation per element.

The usual `find(k)`, `lower_bound(k)`, and `upper_bound(k)` functions of unidimensional maps are not enough for spatial
containers. We fix this with **query iterators**, that explore the spatial data according to a list of predicates.
Queries can limit or expand their search region with a conjunction of predicates such as intersections, disjunctions,
and nearest points.

!!! tip "Predicate Lists"
    To make queries more efficient, the `pareto::predicate_list` object compresses redundant predicates and sorts these predicates by how restrictive they are. All tree nodes store their minimum bounding rectangles, and these underlying data structures are then explored to avoid nodes that might not pass the predicate list. This allows us to find each query element in $O(m \log n)$ time, regardless of how complex the query is.



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
