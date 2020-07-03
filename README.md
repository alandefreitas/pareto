# Pareto Fronts and Archives / C++ and Python

This is container representing a data structure to cache and query multi-dimensional pareto fronts and archives with its most expensive operations in O(n log n) time. Pareto fronts can be used to cache objects in any situation where there is a trade-off between the quality of multiple objectives. Whenever the front pushes an element better than the other in all dimensions, the dominated points are automatically removed from the set. In Pareto archive, whenever the front pushes an element better than the other in all dimensions, the dominated points are moved to higher fronts in the archive.

## Functions

Besides the usual container functions (`size()`, `empty()`, `clear()`, `begin()`, `end()`, etc), it includes the functions and extra objects for:

- Data structures
    - Pareto Front
    - Pareto Archive
    - Linear lists
    - Quadtrees
    - kd-trees
    - R-trees
    - R*-trees
- Queries
    - Hyperbox interesection
    - Hyperbox disjunction
    - Points within hyperboxes
    - Nearest points
- Reference points
    - Ideal point
    - Nadir point
    - Worst point
- Dominance
    - Dominance between points and points
    - Dominance between fronts and fronts
    - Dominance between fronts and points  
- Indicators
    - Convergence and Distribution
        - Exact Hypervolume
        - Monte-Carlo Hypervolume Approximation
    - Cardinality
        - Coverage of two sets (C-metric)
        - Coverage Ratio
    - Convergence
        - Generational Distance (GD)
        - Standard Deviation from the Generational Distance (STDGD)
        - Inverted Generational Distance (IGD)
        - Standard Deviation from the Inverted Generational Distance (STDIGD)
        - Averaged Hausdorff Distance
        - Inverted Modified Generational Distance (IGD<sup>+</sup>)
        - Standard Deviation from the Inverted Modified Generational Distance (STDIGD<sup>+</sup>)
    - Distribution and spread
        - Uniformity
        - Average Distance
        - Average Nearest Distance
        - Average Crowding Distance
    - Objective Correlation
        - Direct Conflict
        - Maxmin Conflict
        - Non-parametric Conflict
- Bindings for Python and C++

Assuming the well-known $O(m \log n)$ average time complexity for search, insertion, and delete in trees, the following table presents the time complexity for these operations.

| Operation | Front | Archive |
|-----|------|---------|
|  Space                     |      O(n)                                  |      O(n) |
|  Search                    |      O(m log n)         |      O(log m + m log n) |
|  Insert                    |      O(m log n)              |      O(n m log n) |
|  Delete                    |      O(m log n)              |      O(n m log n) |
|  Allocation                |      O(1)                                  |      O(1) |
|  Deallocation              |      O(1)                                  |      O(1) |
|  Extreme value             |      O(1)                                  |      O(1) |
|  Extreme element           |      O(m log n)                    |      O(m log n) |
|  Extreme point             |      O(m)                                  |      O(m) |
|  Next Nearest Element      |      O(m log n)                  |      O(n m log n) |
|  Next Query Element        |      O(m log n)                    |      O(m log n) |
|  Front Dominance           |      O(m log n)                    |      O(m log n) |
|  Exact hypervolume         |      O(n^{m-2} log n)        |      O(n^{m-2} log n) |
|  Monte-Carlo hypervolume   |      O(s m log n)                |      O(s m log n) |
|  C-metric                  |      O(n m log n)                |      O(n m log n) |
|  Generational Distance     |      O(n m log n)                |      O(n m log n) |
|  Objective conflict        |      O(mn)                                |      O(mn) |

## Design goals

- Intuitive syntax: Get anything with one line of code. Interface like any other native data structure.
- Easy integration: C++ and Python.
- Speed: spatial indexes. No pairwise comparisons on linear lists.
- Efficient memory allocation to make small fronts efficient.
- Lots of unit tests and benchmarks.

## Examples

For very quick complete examples, see the directory [examples](./examples/).

### Constructing fronts

Python:

```python
import pyfront
pf = pyfront.front(2)
```

C++, assuming we are in the `pareto_front` namespace:

```cpp
#include <pareto_front/pareto_front.h>
front<double, 2, unsigned> pf;
```

This is 2-dimensional front with including a few elements:

![2-dimensional front](documentation/img/front2d_b.svg)

Note that the dimension is defined at compile-time. The default constructor will choose an appropriate spatial index to represent the front. Also, each point in space is associated to an object. In your application, `unsigned` will probably be replaced by a pointer to an object that has the `double` attributes. 

If the dimensions are not supposed to be minimized, we can define one optimization direction for each dimension:

```python
pf = front(['minimization', 'maximization'])print(pf)
print(len(pf), 'elements in the front')
if pf:
print('Front is not empty')
print(pf.dimensions(), 'dimensions')
print('All' if pf.is_minimization() else 'Not all', 'dimensions are minimization')
print('Dimension 0 is', 'minimization' if pf.is_minimization(0) else 'not minimization')
print('Dimension 1 is', 'maximization' if pf.is_maximization(1) else 'not maximization')
```

In C++, the examples assume `std::cout` and `std::endl` are visible in the current namespace:

```cpp
pareto_front<double, 2, unsigned> pf({minimization, maximization});
cout << pf << endl;
cout << pf.size() << " elements in the front" << endl;
if (!pf.empty()) {
    cout << "Front is not empty" << endl;
}
cout << pf.dimensions() << " dimensions" << endl;
cout << (pf.is_minimization() ? "All" : "Not all") << " dimensions are minimization" << endl;
cout << "Dimension 0 is " << (pf.is_minimization(0) ? "minimization" : "not minimization") << endl;
cout << "Dimension 1 is " << (pf.is_maximization(1) ? "maximization" : "not maximization") << endl;
```

This is a 2-dimensional front if we set all directions to `maximization`: 

![2-dimensional front](documentation/img/front2d.svg)

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

### Insertion

The insertion and removal algorithms for all data structures can be accessed through overloaded operators or the usual `insert`/`erase` functions. These functions return an iterator to the new element.

```python
pf[-2.01773, -1.25209] = 27
pf[-2.31613, -0.219302] = 8
pf[1.51927, -0.923787] = 12
pf[-0.283076, -1.58758] = 2
del pf[-2.31613, -0.219302]
pf.clear()
```

```cpp
pf(-2.01773, -1.25209) = 27;
pf(-2.31613, -0.219302) = 8;
pf(1.51927, -0.923787) = 12;
pf(-0.283076, -1.58758) = 2;
pf.erase({-2.31613, -0.219302});
pf.clear();
```

We use `operator()` for C++ because the `operator[]` does not allow more than one element. We can use `operator[point_type]` though. The insertion operator will already remove any points that are worse than the new point in all dimensions.

### Reference points

We can find all extreme values in O(1) time, and iterators to the extreme elements in O(m \log n) time.

```python
print('Ideal point:', pf.ideal())
print('Ideal point in dimension 0:', pf.dimension_ideal(0))
print('Ideal point in dimension 1:', pf.dimension_ideal(1))
print('Nadir point:', pf.nadir())
print('Nadir point in dimension 0:', pf.dimension_nadir(0))
print('Nadir point in dimension 1:', pf.dimension_nadir(1))
```

```cpp
cout << "Ideal point: " << pf.ideal() << endl;
cout << "Nadir point: " << pf.nadir() << endl;
cout << "Ideal point in dimension 0: " << pf.dimension_ideal(0)->first << endl;
cout << "Ideal point in dimension 1: " << pf.dimension_ideal(1)->first << endl;
cout << "Nadir point in dimension 0: " << pf.dimension_nadir(0)->first << endl;
cout << "Nadir point in dimension 1: " << pf.dimension_nadir(1)->first << endl;
```

### Iterators

The elements can be accessed through iterators and reverse iterators:

```python
for [point, value] in pf:
    print(point, '->', value)
for [point, value] in reversed(pf):
    print(point, '->', value)
```

```cpp
for (const auto& [point, value]: pf) {
    cout << point << " -> " << value << endl;
}
for (auto it = pf.rbegin(); it != pf.rend(); ++it) {
    cout << it->first << " -> " << it->second << endl;
}
```

At each iteration, these iterators report the next tree element in a depth-first search algorithm. The reverse iterators perform a reversed depth-first search algorithm, where we get the next element at the rightmost element of the left sibling node or return the parent node when there are no more siblings.

### Queries

All iterators can contain a list of predicates that limit iterators to query results. There are five types of predicates:

-   `intersects`: return only elements that intersect a given query box.
-   `within`: return only elements within a given query box. This is the same as `intersects` but it excludes the borders.
-   `disjoint`: return only elements that do not intersect a given query box.
-   `nearest`: return only the $k$ nearest elements to a reference point or query box.
-   `satisfies`: return only elements that pass a predicate provided by the user.

```python
for [point, value] in pf.find_intersection(pf.ideal(), pf.point([-1.21188, -1.24192])):
    print(point, '->', value)
for [point, value] in pf.find_within(pf.ideal(), pf.point([-1.21188, -1.24192])):
    print(point, '->', value)
for [point, value] in pf.find_disjoint(pf.worst(), pf.point([+0.71, +1.19])):
    print(point, '->', value)
for [point, value] in pf.find_nearest(pf.point([-1.21188, -1.24192]), 2):
    print(point, '->', value)
p = pf.get_nearest(pf.point([2.5, 2.5]))
print(p[0], '->', p[1])
```

```cpp
for (auto it = pf.find_intersection(pf.ideal(), {-1.21188, -1.24192}); it != pf.end(); ++it) {
    cout << it->first << " -> " << it->second << endl;
}
for (auto it = pf.find_within(pf.ideal(), {-1.21188, -1.24192}); it != pf.end(); ++it) {
    cout << it->first << " -> " << it->second << endl;
}
for (auto it = pf.find_disjoint(pf.worst(), {+0.71, +1.19}); it != pf.end(); ++it) {
    cout << it->first << " -> " << it->second << endl;
}
for (auto it = pf.find_nearest({-1.21188, -1.24192}, 2); it != pf.end(); ++it) {
    cout << it->first << " -> " << it->second << endl;
}
auto it = pf.find_nearest({2.5, 2.5});
cout << it->first << " -> " << it->second << endl;
```

### Dominance

A solution `x1` dominates `x^2` if it `x1` is better than `x2` in at least one dimension:

![Point/point dominance](documentation/img/point_dominance.svg)

We can check the dominance relationships between points with:

```python
p1 = pf.point([0, 0])
p2 = pf.point([1, 1])
print('p1 dominates p2' if p1.dominates(p2) else 'p1 does not dominate p2')
print('p1 strongly dominates p2' if p1.strongly_dominates(p2) else 'p1 does not strongly dominate p2')
print('p1 non-dominates p2' if p1.non_dominates(p2) else 'p1 does not non-dominate p2')
```

```cpp
pareto_front<double, 2, unsigned>::point_type p1({0, 0});
pareto_front<double, 2, unsigned>::point_type p2({1, 1});
cout << (p1.dominates(p2) ? "p1 dominates p2" : "p1 does not dominate p2") << endl;
cout << (p1.strongly_dominates(p2) ? "p1 strongly dominates p2" : "p1 does not strongly dominate p2") << endl;
cout << (p1.non_dominates(p2) ? "p1 non-dominates p2" : "p1 does not non-dominate p2") << endl;
```

The `point` function and the `point_type` alias are shortcuts to obtain the appropriate point type for a front.

We can also check for dominance between fronts and points:

![Front/point dominance](documentation/img/pareto_dominance.svg)

```python
print('pf dominates p2' if pf.dominates(p2) else 'pf does not dominate p2')
print('pf strongly dominates p2' if pf.strongly_dominates(p2) else 'pf does not strongly dominate p2')
print('pf non-dominates p2' if pf.non_dominates(p2) else 'pf does not non-dominate p2')
print('pf is partially dominated by p2' if pf.is_partially_dominated_by(p2) else 'pf is not is partially dominated by p2')
print('pf is completely dominated by p2' if pf.is_completely_dominated_by(p2) else 'pf is not is completely dominated by p2')
```

```cpp
cout << (pf.dominates(p2) ? "pf dominates p2" : "pf does not dominate p2") << endl;
cout << (pf.strongly_dominates(p2) ? "pf strongly dominates p2" : "pf does not strongly dominate p2") << endl;
cout << (pf.non_dominates(p2) ? "pf non-dominates p2" : "pf does not non-dominate p2") << endl;
cout << (pf.is_partially_dominated_by(p2) ? "pf is partially dominated by p2" : "pf is not is partially dominated by p2") << endl;
cout << (pf.is_completely_dominated_by(p2) ? "pf is completely dominated by p2" : "pf is not is completely dominated by p2") << endl;
```

At last, we can check the dominance relationship between fronts. `P1` dominates `P2` if `P1` dominates all points in `P2`.

```python
pf2 = pyfront.front(2)
for [p,v] in pf:
    pf2[p-1] = v
print('pf dominates pf2' if pf.dominates(pf2) else 'pf does not dominate pf2')
print('pf strongly dominates pf2' if pf.strongly_dominates(pf2) else 'pf does not strongly dominate pf2')
print('pf non-dominates pf2' if pf.non_dominates(pf2) else 'pf does not non-dominate pf2')
print('pf is partially dominated by pf2' if pf.is_partially_dominated_by(pf2) else 'pf is not is partially dominated by pf2')
print('pf is completely dominated by pf2' if pf.is_completely_dominated_by(pf2) else 'pf is not is completely dominated by pf2')
```

```cpp
front<double, 2, unsigned> pf2;
for (const auto& [p,v]: pf) {
    pf2[p - 1] = v;
}
cout << (pf.dominates(pf2) ? "pf dominates pf2" : "pf does not dominate pf2") << endl;
cout << (pf.strongly_dominates(pf2) ? "pf strongly dominates pf2" : "pf does not strongly dominate pf2") << endl;
cout << (pf.non_dominates(pf2) ? "pf non-dominates pf2" : "pf does not non-dominate pf2") << endl;
cout << (pf.is_partially_dominated_by(pf2) ? "pf is partially dominated by pf2" : "pf is not is partially dominated by pf2") << endl;
cout << (pf.is_completely_dominated_by(pf2) ? "pf is completely dominated by pf2" : "pf is not is completely dominated by pf2") << endl;
```

### Indicators

Indicators can measure several front attributes, such as cardinality, convergence, distribution, and spread. Correlation indicators can estimate the relationship between objectives in a set.

The most important indicator of a front quality is probably its hypervolume, as it measures both convergence and distribution quality:

```python
print('Exact hypervolume:', pf.hypervolume(pf.nadir()))
print('Hypervolume approximation (10000 samples):', pf.hypervolume(pf.nadir(), 10000))
```

```cpp
cout << "Exact hypervolume: " << pf.hypervolume(pf.nadir()) << endl;
cout << "Hypervolume approximation (10000 samples): " << pf.hypervolume(pf.nadir(), 10000) << endl;
```

Cardinality indicators compare two fronts and indicate how many points in one front are non-dominated by point in the other.

```python
print('C-metric:', pf.coverage(pf2))
print('Coverage ratio:', pf.coverage_ratio(pf2))
```

```cpp
cout << "C-metric: " << pf.coverage(pf2) << endl;
cout << "Coverage ratio: " << pf.coverage_ratio(pf2) << endl;
```

Convergence indicators measure some distance metric from a front approximation to the exact Pareto front.

```python
print('GD:', pf.gd(pf_star))
print('STDGD:', pf.std_gd(pf_star))
print('IGD:', pf.igd(pf_star))
print('STDGD:', pf.std_igd(pf_star))
print('Hausdorff:', pf.hausdorff(pf_star))
print('IGD+:', pf.igd_plus(pf_star))
print('STDIGD+:', pf.std_igd_plus(pf_star))
```

```cpp
cout << "GD: " << pf.gd(pf_star) << endl;
cout << "STDGD: " << pf.std_gd(pf_star) << endl;
cout << "IGD: " << pf.igd(pf_star) << endl;
cout << "STDGD: " << pf.std_igd(pf_star) << endl;
cout << "Hausdorff: " << pf.hausdorff(pf_star) << endl;
cout << "IGD+: " << pf.igd_plus(pf_star) << endl;
cout << "STDIGD+: " << pf.std_igd_plus(pf_star) << endl;
```

Distribution indicators measure how uniformly the points are distributed on the front.

```python
print("Uniformity:", pf.uniformity())
print("Average distance:", pf.average_distance())
print("Average nearest distance:", pf.average_nearest_distance(5))
near_origin = pf.get_nearest(pf.point([0.0,0.0]))
print("Crowding distance:", pf.crowding_distance(near_origin[0]))
print("Average crowding distance:", pf.average_crowding_distance())
```

```cpp
cout << "Uniformity: " << pf.uniformity() << endl;
cout << "Average distance: " << pf.average_distance() << endl;
cout << "Average nearest distance: " << pf.average_nearest_distance(5) << endl;
auto near_origin = pf.find_nearest({0.0,0.0});
cout << "Crowding distance: " << pf.crowding_distance(near_origin) << endl;
cout << "Average crowding distance: " << pf.average_crowding_distance() << endl;
```

Correlation indicators can measure the relationship between objectives in a front.

```python
print('Direct conflict:', pf.direct_conflict(0,1))
print('Normalized direct conflict:', pf.normalized_direct_conflict(0,1))
print('Maxmin conflict:', pf.maxmin_conflict(0,1))
print('Normalized maxmin conflict:', pf.normalized_maxmin_conflict(0,1))
print('Non-parametric conflict:', pf.conflict(0,1))
print('Normalized conflict:', pf.normalized_conflict(0,1))
```

```cpp
cout << "Direct conflict: " << pf.direct_conflict(0,1) << endl;
cout << "Normalized direct conflict: " << pf.normalized_direct_conflict(0,1) << endl;
cout << "Maxmin conflict: " << pf.maxmin_conflict(0,1) << endl;
cout << "Normalized maxmin conflict: " << pf.normalized_maxmin_conflict(0,1) << endl;
cout << "Non-parametric conflict: " << pf.conflict(0,1) << endl;
cout << "Normalized conflict: " << pf.normalized_conflict(0,1) << endl;
```

### Archives

Many applications require Pareto archives rather than fronts. An archive with capacity for 100 elements can be constructed with the interface below:

```python
ar = pyfront.archive(2, 100)
```

```cpp
archive<double, 2, unsigned> ar(100);
```

The archive interface has the same functions as the front interface: insertion, removal, and searching operations identify the proper front for the elements; functions for indicators and dominance relationships use the first front as reference.

## Benchmarks

The object will choose an appropriate data structure according the the parameters. Here are some benchmarks comparing these data structures. We use the notation `L`, `Q`, `K`, `B`, `R`, and `*` for Lists, Quadtrees, $k$-d trees, Boost.Geomtry R-trees, Point R-trees, and R*-Trees.

### Constructor

![Construct (n=50)](documentation/img/construct_n_50.png)
![Construct (n=500)](documentation/img/construct_n_500.png)
![Construct (n=5000)](documentation/img/construct_n_5000.png)

### Insert

![Insertion (n=50)](documentation/img/insertion_n_50.png)
![Insertion (n=500)](documentation/img/insertion_n_500.png)
![Insertion (n=5000)](documentation/img/insertion_n_5000.png)

### Delete

![Removal (n=50)](documentation/img/removal_n_50.png)
![Removal (n=500)](documentation/img/removal_n_500.png)
![Removal (n=5000)](documentation/img/removal_n_5000.png)

### Test dominance

![Check dominance (n=50)](documentation/img/check_dominance_n_50.png)
![Check dominance (n=500)](documentation/img/check_dominance_n_500.png)
![Check dominance (n=5000)](documentation/img/check_dominance_n_5000.png)

### Query and iterate

![Query and iterate (n=50)](documentation/img/query_n_50.png)
![Query and iterate (n=500)](documentation/img/query_n_500.png)
![Query and iterate (n=5000)](documentation/img/query_n_5000.png)

### Nearest points

![Find nearest 5 and iterate (n=50)](documentation/img/nearest_n_50.png)
![Find nearest 5 and iterate (n=500)](documentation/img/nearest_n_500.png)
![Find nearest 5 and iterate (n=5000)](documentation/img/nearest_n_5000.png)

### IGD indicator

![IGD (n=50)](documentation/img/igd_n_50.png)
![IGD (n=500)](documentation/img/igd_n_500.png)
![IGD (n=5000)](documentation/img/igd_n_5000.png)

### Hypervolume

#### Time

![Hypervolume (m=1)](documentation/img/hypervolume_m_1.png)
![Hypervolume (m=2)](documentation/img/hypervolume_m_2.png)
![Hypervolume (m=3)](documentation/img/hypervolume_m_3.png)
![Hypervolume (m=5)](documentation/img/hypervolume_m_5.png)
![Hypervolume (m=9)](documentation/img/hypervolume_m_9.png)

#### Gap from the exact hypervolume

![Hypervolume Gap](documentation/img/hypervolume_gap.png)

### Operations

![Operation Averages (n=50)](documentation/img/op_averages_n_50.png)
![Operation Averages (n=500)](documentation/img/op_averages_n_500.png)
![Operation Averages (n=5000)](documentation/img/op_averages_n_5000.png)

### Data structures

![Data Structure Averages (n=50,m=1)](documentation/img/structure_averages_n_50_m_1.png)
![Data Structure Averages (n=50,m=2)](documentation/img/structure_averages_n_50_m_2.png)
![Data Structure Averages (n=50,m=3)](documentation/img/structure_averages_n_50_m_3.png)
![Data Structure Averages (n=50,m=5)](documentation/img/structure_averages_n_50_m_5.png)
![Data Structure Averages (n=50,m=9)](documentation/img/structure_averages_n_50_m_9.png)
![Data Structure Averages (n=50,m=13)](documentation/img/structure_averages_n_50_m_13.png)
![Data Structure Averages (n=500,m=1)](documentation/img/structure_averages_n_500_m_1.png)
![Data Structure Averages (n=500,m=2)](documentation/img/structure_averages_n_500_m_2.png)
![Data Structure Averages (n=500,m=3)](documentation/img/structure_averages_n_500_m_3.png)
![Data Structure Averages (n=500,m=5)](documentation/img/structure_averages_n_500_m_5.png)
![Data Structure Averages (n=500,m=9)](documentation/img/structure_averages_n_500_m_9.png)
![Data Structure Averages (n=500,m=13)](documentation/img/structure_averages_n_500_m_13.png)
![Data Structure Averages (n=5000,m=1)](documentation/img/structure_averages_n_5000_m_1.png)
![Data Structure Averages (n=5000,m=2)](documentation/img/structure_averages_n_5000_m_2.png)
![Data Structure Averages (n=5000,m=3)](documentation/img/structure_averages_n_5000_m_3.png)
![Data Structure Averages (n=5000,m=5)](documentation/img/structure_averages_n_5000_m_5.png)
![Data Structure Averages (n=5000,m=9)](documentation/img/structure_averages_n_5000_m_9.png)
![Data Structure Averages (n=5000,m=13)](documentation/img/structure_averages_n_5000_m_13.png)

## Limitations

One limitation of the library interface is that the most efficient data structures have their dimension defined at compile-time. This is not an inconvenience to C++ users but it makes it impossible to have Python bindings for all front dimensions in compile-time. 

Our solution was to include the fronts with m=1,...,10 dimensions set at compile-time in our Python bindings. For each data structure, we provide a subsidiary implementation that has its dimension determined at runtime. The Python bindings fallback to this special implementation whenever m>10. 

This value can be altered when compiling the library. Change the value of `max_num_dimensions` in [pyfront.cpp](bindings/pyfront.cpp) if you want to change that. The higher this value, the larger the final binary.

## Integration

### Python

Check if you have [Cmake](http://cmake.org) 3.14+ installed:

```bash
cmake -version
```

Install or update [Cmake](http://cmake.org) 3.14+ if needed.
 
Compile the library with:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

This should get you a library `pyfront.*` in the `build/bindings` directory. Put that library anywhere in your `PYTHONPATH` or your project directory and you're good to go:

```python
import pyfront
``` 

### C++

### CMake (manual download)

Check if you have [Cmake](http://cmake.org) 3.14+ installed:

```bash
cmake -version
```

Download the whole project and add the subdirectory to your Cmake project:

```cmake
add_subdirectory(pareto_front)
```

When creating your executable, link the library to the targets you want:

```
add_executable(my_target main.cpp)
target_link_libraries(my_target PUBLIC pareto_front)
```

Add this header to your source files:

```cpp
#include <pareto_front/pareto_front.h>
```   

### CMake (automatic download)

Check if you have [Cmake](http://cmake.org) 3.14+ installed:

```bash
cmake -version
```

Install [CPM.cmake](https://github.com/TheLartians/CPM.cmake) and then:

```cmake
CPMAddPackage(
    NAME pareto_front
    GITHUB_REPOSITORY alandefreitas/pareto_front
)
# ...
target_link_libraries(my_target PUBLIC pareto_front)
```

### Other build systems 

If you're not using Cmake, your project needs to include the headers and compile all source files in `sources` directory. You can then include the library with:

```cpp
#include <pareto_front/pareto_front.h>
```

## Thanks

We would like to thank the developers of these libraries:

- [Boost.Geometry package](https://github.com/boostorg/geometry)
- [Trase library](https://github.com/trase-cpp/trase)
- [hv-2.0rc2/hv.h for calculating hyper-volumes](https://ieeexplore.ieee.org/document/1688440)

## References

* J. Blank and K. Deb, pymoo:
Multi-objective Optimization in Python, IEEE Access,
2020, 1-1 

* ECJ then and now, Luke,
Sean, Proceedings of the Genetic and Evolutionary
Computation Conference Companion, 1223--1230, 2017 

* PlatEMO: A MATLAB platform for
evolutionary multi-objective optimization \[educational forum\],
Tian, Ye and Cheng, Ran and Zhang, Xingyi and Jin, Yaochu,
IEEE Computational Intelligence Magazine, 12,
4, 73--87, 2017, IEEE 

* PISA---a platform and programming
language independent interface for search algorithms, Bleuler,
Stefan and Laumanns, Marco and Thiele, Lothar and Zitzler, Eckart,
International Conference on Evolutionary Multi-Criterion
Optimization, 494--508, 2003, Springer 

* ParadisEO-MOEO: A
framework for evolutionary multi-objective optimization,
Liefooghe, Arnaud and Basseur, Matthieu and Jourdan, Laetitia
and Talbi, El-Ghazali, International Conference on
Evolutionary Multi-Criterion Optimization, 386--400,
2007, Springer 

* Opt4J: a modular framework
for meta-heuristic optimization, Lukasiewycz, Martin and
Glass=tex, Michael and Reimann, Felix and Teich, Jurgen,
Proceedings of the 13th annual conference on Genetic and
evolutionary computation, 1723--1730, 2011 

* MOEA framework: a free and open source java
framework for multiobjective optimization, Hadka, David,
2012, Version 

* Redesigning the jMetal
multi-objective optimization framework, Nebro, Antonio J and
Durillo, Juan J and Vergne, Matthieu, Proceedings of the
companion publication of the 2015 annual conference on genetic and
evolutionary computation, 1093--1100, 2015 

* jMetal: A Java framework for
multi-objective optimization, Durillo, Juan J and Nebro,
Antonio J, Advances in Engineering Software, 42,
10, 760--771, 2011, Elsevier 

* An extensible JCLEC-based
solution for the implementation of multi-objective evolutionary
algorithms, Ramirez, Aurora and Romero, Jose
Raul and Ventura, Sebastian, Proceedings of the
Companion Publication of the 2015 Annual Conference on Genetic and
Evolutionary Computation, 1085--1092, 2015 

* EvA: a tool for optimization with
evolutionary algorithms, Wakunda, Jurgen and Zell,
Andreas, EUROMICRO 97. Proceedings of the 23rd EUROMICRO
Conference: New Frontiers of Information Technology (Cat.
No. 97TB100167), 644--651, 1997, IEEE 

* Interpersonal Comparisons of
the Good: Epistemic not Impossible, Coakley, Mathew,
Utilitas, 28, 3, 288--313,
2016, Cambridge University Press 

* Multiobjective genetic
algorithms, Fonseca, Carlos M and Fleming, Peter J,
IEE colloquium on genetic algorithms for control systems
engineering, 6--1, 1993, IET 

* The efficiency theorems and
market failure, Hammond, P, Elements of General
Equilibrium Analysis, 211--260, 1998, Oxford:
Blackwell 

* MCDM---if not a roman numeral, then
what?, Zionts, Stanley, Interfaces, 9,
4, 94--101, 1979, INFORMS 

* Performance indicators in
multiobjective optimization, Audet, Charles and Bigeon, J and
Cartier, D and Le Digabel, Sebastien and Salomon, Ludovic,
Optimization Online, 2018 

* Output-sensitive peeling of convex
and maximal layers, Nielsen, Franck, Information
processing letters, 59, 5, 255--259,
1996, Elsevier 

* On finding the maxima of a set of
vectors, Kung, Hsiang-Tsung and Luccio, Fabrizio and Preparata,
Franco P, Journal of the ACM (JACM), 22, 4,
469--476, 1975, ACM New York, NY, USA 

* Algorithms and analyses for
maximal vector computation, Godfrey, Parke and Shipley, Ryan
and Gryz, Jarek, The VLDB Journal, 16, 1,
5--28, 2007, Springer 

* Adaptive weighted sum method for
multiobjective optimization: a new method for Pareto front generation,
Kim, Il Yong and De Weck, OL, Structural and
multidisciplinary optimization, 31, 2,
105--116, 2006, Springer 

* On a bicriterion formation of the
problems of integrated system identification and system optimization,
YV, YV HAIMES and Lasdon, Leon S and Da Wismer, DA,
IEEE Transactions on Systems, Man and Cybernetics, 3,
296--297, 1971, Institute of Electrical and
Electronics Engineers Inc. 

* Effective implementation of the
$\varepsilon$-constraint method in multi-objective mathematical
programming problems, Mavrotas, George, Applied
mathematics and computation, 213, 2,
455--465, 2009, Elsevier 

* Deductive sort and climbing
sort: New methods for non-dominated sorting, McClymont, Kent
and Keedwell, Ed, Evolutionary computation, 20,
1, 1--26, 2012, MIT Press 

* An efficient non-dominated sorting
method for evolutionary algorithms, Fang, Hongbing and Wang,
Qian and Tu, Yi-Cheng and Horstemeyer, Mark F, Evolutionary
computation, 16, 3, 355--384, 2008,
MIT Press 

* Reducing the run-time complexity of
multiobjective EAs: The NSGA-II and other algorithms, Jensen,
Mikkel T, IEEE Transactions on Evolutionary Computation,
7, 5, 503--515, 2003, IEEE

* An efficient approach to
nondominated sorting for evolutionary multiobjective optimization,
Zhang, Xingyi and Tian, Ye and Cheng, Ran and Jin, Yaochu,
IEEE Transactions on Evolutionary Computation, 19,
2, 201--213, 2014, IEEE 

* Comparison of data
structures for storing Pareto-sets in MOEAs, Mostaghim, Sanaz
and Teich, Jurgen and Tyagi, Ambrish, Proceedings of the
2002 Congress on Evolutionary Computation. CEC'02 (Cat. No. 02TH8600),
1, 843--848, 2002, IEEE 

* A fast incremental BSP tree
archive for non-dominated points, Glasmachers, Tobias,
International Conference on Evolutionary Multi-Criterion
Optimization, 252--266, 2017, Springer 

* Data structures in multi-objective
evolutionary algorithms, Altwaijry, Najwa and Menai, Mohamed El
Bachir, Journal of Computer Science and Technology,
27, 6, 1197--1210, 2012,
Springer 

* An efficient approach to
unbounded bi-objective archives- introducing the mak_tree algorithm,
Berry, Adam and Vamplew, Peter, Proceedings of the
8th annual conference on Genetic and evolutionary computation,
619--626, 2006 

* ND-tree-based update: a fast
algorithm for the dynamic nondominance problem, Jaszkiewicz,
Andrzej and Lust, Thibaut, IEEE Transactions on Evolutionary
Computation, 22, 5, 778--791, 2018,
IEEE  @articlealberto2004representation,
Representation and management of MOEA populations based on
graphs, Alberto, Isolina and Mateo, Pedro M, European
Journal of Operational Research, 159, 1,
52--65, 2004, Elsevier 
A dominance tree and its application
in evolutionary multi-objective optimization, Shi, Chuan and
Yan, Zhenyu and Lu, Kevin and Shi, Zhongzhi and Wang, Bai,
Information Sciences, 179, 20,
3540--3560, 2009, Elsevier 

* Using unconstrained elite archives
for multiobjective optimization, Fieldsend, Jonathan E and
Everson, Richard M and Singh, Sameer, IEEE Transactions on
Evolutionary Computation, 7, 3, 305--323,
2003, IEEE 

* A fast multi-objective evolutionary
algorithm based on a tree structure, Shi, Chuan and Yan, Zhenyu
and Shi, Zhongzhi and Zhang, Lei, Applied Soft Computing,
10, 2, 468--480, 2010,
Elsevier 

* InterQuad: An interactive quad tree
based procedure for solving the discrete alternative multiple criteria
problem, Sun, Minghe and Steuer, Ralph E, European
Journal of Operational Research, 89, 3,
462--472, 1996, Elsevier 

* Quad trees, a datastructure for
discrete vector optimization problems, Habenicht, Walter,
Essays and Surveys on Multiple Criteria Decision Making,
136--145, 1983, Springer 

* Full elite sets for
multi-objective optimisation, Everson, Richard M and Fieldsend,
Jonathan E and Singh, Sameer, Adaptive Computing in Design
and Manufacture V, 343--354, 2002, Springer

* Priority search trees,
McCreight, Edward M, SIAM Journal on Computing,
14, 2, 257--276, 1985, SIAM

* Data structures for range searching,
Bentley, Jon Louis and Friedman, Jerome H, ACM
Computing Surveys (CSUR), 11, 4, 397--409,
1979, ACM New York, NY, USA 

* Multidimensional binary
search trees used for associative searching, Bentley, Jon
Louis, Communications of the ACM, 18, 9,
509--517, 1975, ACM New York, NY, USA 

* The art of Unix programming,
Raymond, Eric S, 2003, Addison-Wesley
Professional 

* Quad-trees and linear lists for identifying
nondominated criterion vectors, Sun, Minghe and Steuer, Ralph
E, INFORMS Journal on Computing, 8, 4,
367--375, 1996, INFORMS 

* A new data structure for the
nondominance problem in multi-objective optimization,
Schutze, Oliver, International Conference on
Evolutionary Multi-Criterion Optimization, 509--518,
2003, Springer 

* The legacy of modern portfolio
theory, Fabozzi, Frank J and Gupta, Francis and Markowitz,
Harry M, The Journal of Investing, 11, 3,
7--22, 2002, Institutional Investor Journals
Umbrella 

* Tensorflow: A system for
large-scale machine learning, Abadi, Marti=texn and
Barham, Paul and Chen, Jianmin and Chen, Zhifeng and Davis, Andy and
Dean, Jeffrey and Devin, Matthieu and Ghemawat, Sanjay and Irving,
Geoffrey and Isard, Michael and others, 12th $\$USENIX$\$
Symposium on Operating Systems Design and Implementation ($\$OSDI$\$
16), 265--283, 2016 

* Quad trees a data structure for
retrieval on composite keys, Finkel, Raphael A. and Bentley,
Jon Louis, Acta informatica, 4, 1,
1--9, 1974, Springer 

* Multidimensional binary
search trees used for associative searching, Bentley, Jon
Louis, Communications of the ACM, 18, 9,
509--517, 1975, ACM New York, NY, USA 

* Geometric modeling using octree
encoding, Meagher, Donald, Computer graphics and
image processing, 19, 2, 129--147,
1982, Elsevier 

* The R*-tree: an efficient and
robust access method for points and rectangles, Beckmann,
Norbert and Kriegel, Hans-Peter and Schneider, Ralf and Seeger,
Bernhard, Proceedings of the 1990 ACM SIGMOD international
conference on Management of data, 322--331, 1990 

* R-trees: A dynamic index structure
for spatial searching, Guttman, Antonin,
Proceedings of the 1984 ACM SIGMOD international conference
on Management of data, 47--57, 1984 

* The X-tree: An index structure for
high-dimensional data, Berchtold, Stefan and Keim, Daniel A and
Kriegel, Hans-Peter, Very Large Data-Bases, 28--39,
1996 

* Quadboost: A scalable concurrent
quadtree, Zhou, Keren and Tan, Guangming and Zhou, Wei,
IEEE Transactions on Parallel and Distributed Systems,
29, 3, 673--686, 2017, IEEE

* Distance browsing in spatial
databases, Hjaltason, Gi=texsli R and Samet, Hanan,
ACM Transactions on Database Systems (TODS), 24,
2, 265--318, 1999, ACM New York, NY,
USA 

* An improved dimension-sweep
algorithm for the hypervolume indicator, Fonseca, Carlos M and
Paquete, Lui=texs and Lopez-Ibanez, Manuel,
2006 IEEE international conference on evolutionary
computation, 1157--1163, 2006, IEEE 

* Speeding up many-objective
optimization by Monte Carlo approximations, Bringmann, Karl and
Friedrich, Tobias and Igel, Christian and Voss=tex, Thomas,
Artificial Intelligence, 204, 22--29,
2013, Elsevier 

* On the complexity of computing the
hypervolume indicator, Beume, Nicola and Fonseca, Carlos M and
Lopez-Ibanez, Manuel and Paquete, Lui=texs and
Vahrenhold, Jan, IEEE Transactions on Evolutionary
Computation, 13, 5, 1075--1082, 2009,
IEEE 

* Aggregation trees for visualization
and dimension reduction in many-objective optimization, de
Freitas, Alan RR and Fleming, Peter J and Guimaraes, Frederico G,
Information Sciences, 298, 288--314,
2015, Elsevier 

* Difficulties in
specifying reference points to calculate the inverted generational
distance for many-objective optimization problems, Ishibuchi,
Hisao and Masuda, Hiroyuki and Tanigaki, Yuki and Nojima, Yusuke,
2014 IEEE Symposium on Computational Intelligence in
Multi-Criteria Decision-Making (MCDM), 170--177, 2014,
IEEE 


