# 🦎 Pareto Fronts and Archives / C++ and Python 📉🤔

[![Build Status](https://img.shields.io/github/workflow/status/alandefreitas/pareto-front/ParetoFront?event=push&label=Build&logo=Github-Actions)](https://github.com/alandefreitas/pareto-front/actions?query=workflow%3AParetoFront+event%3Apush)
[![Latest Release](https://img.shields.io/github/release/alandefreitas/pareto-front.svg?label=Download)](https://GitHub.com/alandefreitas/pareto-front/releases/)
[![Website lbesson.bitbucket.org](https://img.shields.io/website-up-down-green-red/http/alandefreitas.github.io/pareto-front.svg?label=Documentation)](https://alandefreitas.github.io/pareto-front/)

![Two-dimensional front](docs/img/front2d_b.svg)

Containers based on Pareto efficiency simultaneously consider many criteria where we cannot improve one criterion without making at least one other criterion worse off. New elements can either replace elements that are worse at all criteria or send them to a higher-level archive front. While there are many libraries for multi-objective optimization, there are no libraries focused on efficient container types for storing these fronts in general applications.
This library provides a STL-like container representing a data structure to cache and query multi-dimensional Pareto fronts and archives with its most expensive operations in <img src="https://render.githubusercontent.com/render/math?math=O(n+\log+n)"> time.

<!-- https://github.com/bradvin/social-share-urls -->
[![Facebook](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Facebook&logo=facebook)](https://www.facebook.com/sharer/sharer.php?t=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python&u=https://github.com/alandefreitas/pareto-front/)
[![QZone](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+QZone&logo=qzone)](http://sns.qzone.qq.com/cgi-bin/qzshare/cgi_qzshare_onekey?url=https://github.com/alandefreitas/pareto-front/&title=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python&summary=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![Weibo](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Weibo&logo=sina-weibo)](http://sns.qzone.qq.com/cgi-bin/qzshare/cgi_qzshare_onekey?url=https://github.com/alandefreitas/pareto-front/&title=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python&summary=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![Reddit](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Reddit&logo=reddit)](http://www.reddit.com/submit?url=https://github.com/alandefreitas/pareto-front/&title=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![Twitter](https://img.shields.io/twitter/url/http/shields.io.svg?label=Share+on+Twitter&style=social)](https://twitter.com/intent/tweet?text=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python&url=https://github.com/alandefreitas/pareto-front/&hashtags=MOO,MultiObjectiveOptimization,Cpp,ScientificComputing,Optimization,Developers)
[![LinkedIn](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+LinkedIn&logo=linkedin)](https://www.linkedin.com/shareArticle?mini=false&url=https://github.com/alandefreitas/pareto-front/&title=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![WhatsApp](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+WhatsApp&logo=whatsapp)](https://api.whatsapp.com/send?text=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python:+https://github.com/alandefreitas/pareto-front/)
[![Line.me](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Line.me&logo=line)](https://lineit.line.me/share/ui?url=https://github.com/alandefreitas/pareto-front/&text=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![Telegram.me](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Telegram.me&logo=telegram)](https://telegram.me/share/url?url=https://github.com/alandefreitas/pareto-front/&text=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![HackerNews](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+HackerNews&logo=y-combinator)](https://news.ycombinator.com/submitlink?u=https://github.com/alandefreitas/pareto-front/&t=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
<details>
<summary>Table of Contents</summary>

- [Use cases](#use-cases)
- [Design goals](#design-goals)
- [Examples](#examples)
  - [Constructing fronts](#constructing-fronts)
  - [Insertion](#insertion)
  - [Reference points](#reference-points)
  - [Iterators](#iterators)
  - [Queries](#queries)
  - [Dominance](#dominance)
  - [Indicators](#indicators)
  - [Archives](#archives)
- [Functions](#functions)
- [Benchmarks](#benchmarks)
- [Integration](#integration)
  - [Packages](#packages)
  - [Build from source](#build-from-source)
  - [CMake targets](#cmake-targets)
  - [Other build systems](#other-build-systems)
- [Limitations](#limitations)
- [Contributing](#contributing)
- [Thanks](#thanks)
- [References](#references)

</details>
<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Use cases

Some use cases are to store objects with their best values according to the following trade-offs:

* Machine Learning: Model accuracy vs. model complexity
* Approximation algorithms: error vs. time
* Product design: investment vs. profit vs. safety 
* P2P networks: latency vs. trust 
* Robust optimization: quality vs. sensitivity / robustness  
* Design: Mean quality vs. standard deviation
* Systems control: Performance vs. price vs. quality
* Portfolio optimization: expected return vs. risk
* [More...](https://en.wikipedia.org/wiki/Multi-objective_optimization#Examples_of_applications)

## Design goals

- Intuitive syntax: interface like any other native data structure.
- Lots of indicators: calculate statistics with one line of code. 
- Easy integration: C++ and Python interfaces.
- Performance: spatial indexes. No pairwise comparisons on linear lists (unless you want to).
- Efficient memory allocation to make small fronts competitive with linear lists.
- Stability: lots of unit tests, benchmarks, and continuous integration.

## Examples

For complete examples, see the directory [examples](./examples/).

### Constructing fronts

Python:

```python
import pyfront
pf = pyfront.front(2)
```

C++, assuming we are in the `pareto_front` namespace:

```cpp
#include <pareto_front/front.h>
front<double, 2, unsigned> pf;
```

This is a 2-dimensional front with a few elements:

![2-dimensional front](docs/img/front2d_b.svg)

If you need to actually plot these fronts, have a look at [Matplot++](https://github.com/alandefreitas/matplotplusplus) or [Matplotlib](https://matplotlib.org). In particular, have a look at Scatter Plots for two-dimensional fronts, Plot matrices for three-dimensional fronts, or Parallel Coordinate Plots for many-objective fronts.
                                                                   
Note that the front dimension is defined at compile-time. Using the results from our benchmarks (See Section [Benchmarks](#benchmarks)), the default constructor will choose an appropriate spatial index to represent the front. Also, like a C++ `map` or a Python `dict`, each point in space is associated with an object. In your application, `unsigned` would probably be replaced by a pointer to an object that has the `double` attributes. 

If the dimensions are not supposed to be minimized, we can define one optimization direction for each dimension:

```python
pf = front(['minimization', 'maximization'])
print(pf)
print(len(pf), 'elements in the front')
if pf:
    print('Front is not empty')
print(pf.dimensions(), 'dimensions')
print('All' if pf.is_minimization() else 'Not all', 'dimensions are minimization')
print('Dimension 0 is', 'minimization' if pf.is_minimization(0) else 'not minimization')
print('Dimension 1 is', 'maximization' if pf.is_maximization(1) else 'not maximization')
```

In C++, the following examples assume `std::cout` and `std::endl` are visible in the current namespace:

```cpp
front<double, 2, unsigned> pf({minimization, maximization});
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

If we set all directions to `maximization`, this is what a 2-dimensional front looks like: 

![2-dimensional front](docs/img/front2d.svg)

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

We use `operator()` for C++ because the `operator[]` does not allow more than one element. We can use `operator[front::point_type]` though. The insertion operator will already remove any points that are worse than the new point in all dimensions.

### Reference points

We can find any extreme value in <img src="https://render.githubusercontent.com/render/math?math=O(1)"> time, and iterators to the extreme elements in <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)"> time.

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

We can access the elements with iterators and reverse iterators:

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
-   `nearest`: return only the <img src="https://render.githubusercontent.com/render/math?math=k"> nearest elements to a reference point or query box.
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

A solution `x1` (weakly) dominates `x2` if it `x1` is 1) better than `x2` in at least one dimension and 2) not worse than `x2` in any dimension:

![Point/point dominance](docs/img/point_dominance.svg)

We can check the dominance relationships between points with:

```python
p1 = pf.point([0, 0])
p2 = pf.point([1, 1])
print('p1 dominates p2' if p1.dominates(p2) else 'p1 does not dominate p2')
print('p1 strongly dominates p2' if p1.strongly_dominates(p2) else 'p1 does not strongly dominate p2')
print('p1 non-dominates p2' if p1.non_dominates(p2) else 'p1 does not non-dominate p2')
```

```cpp
front<double, 2, unsigned>::point_type p1({0, 0});
front<double, 2, unsigned>::point_type p2({1, 1});
cout << (p1.dominates(p2) ? "p1 dominates p2" : "p1 does not dominate p2") << endl;
cout << (p1.strongly_dominates(p2) ? "p1 strongly dominates p2" : "p1 does not strongly dominate p2") << endl;
cout << (p1.non_dominates(p2) ? "p1 non-dominates p2" : "p1 does not non-dominate p2") << endl;
```

The `point` function (Python) and the `point_type` alias (C++) are shortcuts to obtain the appropriate point type for a front (remember the front dimensions are defined at compile-time).

We can also check for dominance between fronts and points:

![Front/point dominance](docs/img/pareto_dominance.svg)

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

The most popular indicator of a front quality is its hypervolume, as it measures both convergence and distribution quality:

```python
print('Exact hypervolume:', pf.hypervolume(pf.nadir()))
print('Hypervolume approximation (10000 samples):', pf.hypervolume(pf.nadir(), 10000))
```

```cpp
cout << "Exact hypervolume: " << pf.hypervolume(pf.nadir()) << endl;
cout << "Hypervolume approximation (10000 samples): " << pf.hypervolume(pf.nadir(), 10000) << endl;
```

Cardinality indicators compare two fronts and indicate how many points in one front are non-dominated by points in the other front.

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

The archive interface has the same functions as the front interface: insertion, removal, and searching operations identify the proper front for the elements; functions for indicators and dominance relationships use the first fronts as reference.

## Functions

Besides the usual container functions (`size()`, `empty()`, `clear()`, `begin()`, `end()`, etc), these containers include functions and extra objects for:

| Data Structures | Queries       | Reference Points | Dominance   | Indicators   | Bindings |
|-----------------|---------------|------------------|-------------|--------------|----------|
| Pareto Front    | Intersection  | Ideal            | Point/point | Convergence  | C++      |
| Pareto Archive  | Disjunction   | Nadir            | Front/point | Distribution | Python   |
| Linear Lists    | k-nearest     | Worst            | Front/front | Cardinality  |          |
| Quadtrees       | Points within |                  |             | Spread       |          |
| kd-trees        |               |                  |             | Correlation  |          |
| R-trees         |               |                  |             |              |          |
| R*-trees        |               |                  |             |              |          |

- List of Indicators
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

Assuming the well-known <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)"> average time complexity for search, insertion, and delete in trees, the following table presents the time complexity for these operations.

| Operation | Front | Archive |
|-----|------|---------|
|  Space                     |      <img src="https://render.githubusercontent.com/render/math?math=O(n)">                                  |      <img src="https://render.githubusercontent.com/render/math?math=O(n)"> |
|  Search                    |      <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)">         |      <img src="https://render.githubusercontent.com/render/math?math=O(\log%20m%2Bm\log%20n)"> |
|  Insert                    |      <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)">              |      <img src="https://render.githubusercontent.com/render/math?math=O(nm\log%20n)"> |
|  Delete                    |      <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)">              |      <img src="https://render.githubusercontent.com/render/math?math=O(nm\log%20n)"> |
|  Allocation                |      <img src="https://render.githubusercontent.com/render/math?math=O(1)">                                  |      <img src="https://render.githubusercontent.com/render/math?math=O(1)"> |
|  Deallocation              |      <img src="https://render.githubusercontent.com/render/math?math=O(1)">                                  |      <img src="https://render.githubusercontent.com/render/math?math=O(1)"> |
|  Extreme value             |      <img src="https://render.githubusercontent.com/render/math?math=O(1)">                                  |      <img src="https://render.githubusercontent.com/render/math?math=O(1)"> |
|  Extreme element           |      <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)">                    |      <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)"> |
|  Extreme point             |      <img src="https://render.githubusercontent.com/render/math?math=O(m)">                                  |      <img src="https://render.githubusercontent.com/render/math?math=O(m)"> |
|  Next Nearest Element      |      <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)">                  |      <img src="https://render.githubusercontent.com/render/math?math=O(nm\log%20n)"> |
|  Next Query Element        |      <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)">                    |      <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)"> |
|  Front Dominance           |      <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)">                    |      <img src="https://render.githubusercontent.com/render/math?math=O(m\log+n)"> |
|  Exact hypervolume         |      <img src="https://render.githubusercontent.com/render/math?math=O(n^{m-2}\log%20n)">        |      <img src="https://render.githubusercontent.com/render/math?math=O(n^{m-2}\log%20n)"> |
|  Monte-Carlo hypervolume   |      <img src="https://render.githubusercontent.com/render/math?math=O(sm\log%20n)">                |      <img src="https://render.githubusercontent.com/render/math?math=O(sm\log%20n)"> |
|  C-metric                  |      <img src="https://render.githubusercontent.com/render/math?math=O(nm\log%20n)">                |      <img src="https://render.githubusercontent.com/render/math?math=O(nm\log%20n)"> |
|  Generational Distance     |      <img src="https://render.githubusercontent.com/render/math?math=O(nm\log%20n)">                |      <img src="https://render.githubusercontent.com/render/math?math=O(nm\log%20n)"> |
|  Objective conflict        |      <img src="https://render.githubusercontent.com/render/math?math=O(mn)">                                |      <img src="https://render.githubusercontent.com/render/math?math=O(mn)"> |


## Benchmarks

The default tag for fronts and archives is converted to an appropriate data structure according to the front or archive parameters. This section presents some benchmarks comparing these data structures. We use the notation `L`, `Q`, `K`, `B`, `R`, and `*` for Lists, Quadtrees, <img src="https://render.githubusercontent.com/render/math?math=k">-d trees, Boost.Geomtry R-trees, Point R-trees, and R*-Trees. The tree data structures in the benchmark used a [memory pool allocator](sources/pareto_front/memory_pool.h) for faster allocation. This is intented to make trees more competitive with linear lists for small fronts. If using this code in production, it is more prudent to use [`std::pmr::unsynchronized_pool_resource`](https://en.cppreference.com/w/cpp/memory/unsynchronized_pool_resource/unsynchronized_pool_resource) (if your compiler supports it), [`std::allocator`](https://en.cppreference.com/w/cpp/memory/allocator) (if you want to be conservative), or execute *many* tests to make sure [`pareto::fast_memory_pool`](sources/pareto_front/memory_pool.h) works properly on your system.

<details>
    <summary>Constructor</summary>

![Construct (n=50)](docs/img/construct_n_50.png)
![Construct (n=500)](docs/img/construct_n_500.png)
![Construct (n=5000)](docs/img/construct_n_5000.png)

</details>

<details>
    <summary>Insert</summary>

![Insertion (n=50)](docs/img/insertion_n_50.png)
![Insertion (n=500)](docs/img/insertion_n_500.png)
![Insertion (n=5000)](docs/img/insertion_n_5000.png)

</details>

<details>
    <summary>Delete</summary>

![Removal (n=50)](docs/img/removal_n_50.png)
![Removal (n=500)](docs/img/removal_n_500.png)
![Removal (n=5000)](docs/img/removal_n_5000.png)

</details>

<details>
    <summary>Test dominance</summary>

![Check dominance (n=50)](docs/img/check_dominance_n_50.png)
![Check dominance (n=500)](docs/img/check_dominance_n_500.png)
![Check dominance (n=5000)](docs/img/check_dominance_n_5000.png)

</details>

<details>
    <summary>Query and iterate</summary>

![Query and iterate (n=50)](docs/img/query_n_50.png)
![Query and iterate (n=500)](docs/img/query_n_500.png)
![Query and iterate (n=5000)](docs/img/query_n_5000.png)

</details>

<details>
    <summary>Nearest points</summary>

![Find nearest 5 and iterate (n=50)](docs/img/nearest_n_50.png)
![Find nearest 5 and iterate (n=500)](docs/img/nearest_n_500.png)
![Find nearest 5 and iterate (n=5000)](docs/img/nearest_n_5000.png)

</details>

<details>
    <summary>IGD indicator</summary>

![IGD (n=50)](docs/img/igd_n_50.png)
![IGD (n=500)](docs/img/igd_n_500.png)
![IGD (n=5000)](docs/img/igd_n_5000.png)

</details>

<details>
    <summary>Hypervolume: Time</summary>

![Hypervolume (m=1)](docs/img/hypervolume_m_1.png)
![Hypervolume (m=2)](docs/img/hypervolume_m_2.png)
![Hypervolume (m=3)](docs/img/hypervolume_m_3.png)
![Hypervolume (m=5)](docs/img/hypervolume_m_5.png)
![Hypervolume (m=9)](docs/img/hypervolume_m_9.png)

</details>

<details>
    <summary>Hypervolume: Gap from the exact hypervolume</summary>

![Hypervolume Gap](docs/img/hypervolume_gap.png)

</details>

<details>
    <summary>Operations</summary>

![Operation Averages (n=50)](docs/img/op_averages_n_50.png)
![Operation Averages (n=500)](docs/img/op_averages_n_500.png)
![Operation Averages (n=5000)](docs/img/op_averages_n_5000.png)

</details>

<details>
    <summary>Data structures</summary>

![Data Structure Averages (n=50,m=1)](docs/img/structure_averages_n_50_m_1.png)
![Data Structure Averages (n=50,m=2)](docs/img/structure_averages_n_50_m_2.png)
![Data Structure Averages (n=50,m=3)](docs/img/structure_averages_n_50_m_3.png)
![Data Structure Averages (n=50,m=5)](docs/img/structure_averages_n_50_m_5.png)
![Data Structure Averages (n=50,m=9)](docs/img/structure_averages_n_50_m_9.png)
![Data Structure Averages (n=50,m=13)](docs/img/structure_averages_n_50_m_13.png)
![Data Structure Averages (n=500,m=1)](docs/img/structure_averages_n_500_m_1.png)
![Data Structure Averages (n=500,m=2)](docs/img/structure_averages_n_500_m_2.png)
![Data Structure Averages (n=500,m=3)](docs/img/structure_averages_n_500_m_3.png)
![Data Structure Averages (n=500,m=5)](docs/img/structure_averages_n_500_m_5.png)
![Data Structure Averages (n=500,m=9)](docs/img/structure_averages_n_500_m_9.png)
![Data Structure Averages (n=500,m=13)](docs/img/structure_averages_n_500_m_13.png)
![Data Structure Averages (n=5000,m=1)](docs/img/structure_averages_n_5000_m_1.png)
![Data Structure Averages (n=5000,m=2)](docs/img/structure_averages_n_5000_m_2.png)
![Data Structure Averages (n=5000,m=3)](docs/img/structure_averages_n_5000_m_3.png)
![Data Structure Averages (n=5000,m=5)](docs/img/structure_averages_n_5000_m_5.png)
![Data Structure Averages (n=5000,m=9)](docs/img/structure_averages_n_5000_m_9.png)
![Data Structure Averages (n=5000,m=13)](docs/img/structure_averages_n_5000_m_13.png)

</details>

## Integration

### Packages

Get one of binary packages from the [release section](https://github.com/alandefreitas/pareto-front/releases). 

* Python Binary <OS>: this is only the binary for Python. Copy this file to your site-packages directory.
* pareto-front-<version>-<OS>.<package extension>: these packages contain the Python bindings and the C++ library.
* Binary Packages <OS>: these packages contain all packages for a given OS.

If using one the installers, make sure you install the Python bindings to your site-packages directory (this is the default directory for most packages). You can find your site-packages directory with:

```bash
python -c "from distutils.sysconfig import get_python_lib; print(get_python_lib());"
``` 

These binaries refer to the last release version. If you need a more recent version of pareto-front, you can download the [binary packages from the CI artifacts](https://github.com/alandefreitas/pareto-front/actions?query=workflow%3AParetoFront+event%3Apush) or build the library [from the source files](#build-from-source). 

Once the package is installed, you can use the Python library with

```
import pyfront
```
 
or link your C++ program to the library and include the directories where you installed pareto-front. 

Unless you changed the default options, the C++ library is likely to be in `/usr/local/` (Linux / Mac OS) or `C:/Program Files/` (Windows). The installer will try to find the directory where you usually keep your  libraries but that's not always perfect.

If you are using CMake, you can then find pareto-front with the usual `find_package` command:

```cmake
find_package(ParetoFront REQUIRED)
# ...
target_link_libraries(my_target PUBLIC pareto_front)
```

CMake should be able to locate the `ParetoFrontConfig.cmake` script automatically if you installed the library under `/usr/local/` (Linux / Mac OS) or `C:/Program Files/` (Windows). Otherwise, you need to include your installation directory in `CMAKE_MODULE_PATH` first: 

```cmake
list(APPEND CMAKE_MODULE_PATH put/your/installation/directory/here)
find_package(ParetoFront REQUIRED)
# ...
target_link_libraries(my_target PUBLIC pareto_front)
```

### Build from source

#### Dependencies

This section lists the dependencies you need before installing pareto-front from source. The build script will try to find all these dependencies for you:

* C++17
* CMake 3.14 or higher
* Python 3.6.9 or higher (for the Python bindings)

<details>
    <summary>Instructions: Linux/Ubuntu/GCC</summary>
    
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

[Later](#build-the-examples) when running CMake, make sure you are using GCC-8 or higher by appending the following options:

```bash
-DCMAKE_C_COMPILER=/usr/bin/gcc-8 -DCMAKE_CXX_COMPILER=/usr/bin/g++-8
```

If building the Python bindings, check your Python version:

```bash
python3 --version
```

If it's older than Python 3.6.9, update it with `apt-get` or download the lastest release version from https://www.python.org/downloads/ . If using an installer, make sure you add the application directory to your PATH environment variable.

</details>

<details>
    <summary>Instructions: Mac Os/Clang</summary>

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

</details>

<details>
    <summary>Instructions: Windows/MSVC</summary>
    
* Make sure you have a recent version of [Visual Studio](https://visualstudio.microsoft.com)
* Download Git from [https://git-scm.com/download/win](https://git-scm.com/download/win) and install it
* Download CMake from [https://cmake.org/download/](https://cmake.org/download/) and install it
* Download Python from [https://www.python.org/downloads/](https://www.python.org/downloads/) and install it

If using the Python installer, make sure you add the application directory to your PATH environment variable.

</details>

#### Build the Examples

This will build the examples in the `build/examples` directory:

```bash
mkdir build
cmake -version
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2"
cmake --build . -j 2 --config Release
```

On windows, replace `-O2` with `/O2`.

#### Installing pareto-front from Source

This will install pareto-front on your system:

```bash
mkdir build
cmake -version
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2" -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF 
cmake --build . -j 2 --config Release
cmake --install .
```

On windows, replace `-O2` with `/O2`. You might need `sudo` for this last command.

#### Building the packages

This will create the binary packages you can use to install pareto-front on your system:

```bash
mkdir build
cmake -version
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2" -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF
cmake --build . -j 2 --config Release
cmake --install .
cpack .
```

On windows, replace `-O2` with `/O2`. You might need `sudo` for this last command.

### CMake targets

#### Find it as a CMake Package

If you have the library installed, you can call

```cmake
find_package(ParetoFront)
```

from your CMake build script. 

When creating your executable, link the library to the targets you want:

```
add_executable(my_target main.cpp)
target_link_libraries(my_target PUBLIC pareto_front)
```

Add this header to your source files:

```cpp
#include <pareto_front/front.h>
```

Or to use Pareto archives:

```cpp
#include <pareto_front/archives.h>
```

#### Use it as a CMake subdirectory

You can use pareto-front directly in CMake projects without installing it. Check if you have [Cmake](http://cmake.org) 3.14+ installed:

```bash
cmake -version
```

Clone the whole project
 
```bash
git clone https://github.com/alandefreitas/pareto-front/
```

and add the subdirectory to your CMake project:

```cmake
add_subdirectory(pareto-front)
```

When creating your executable, link the library to the targets you want:

```
add_executable(my_target main.cpp)
target_link_libraries(my_target PUBLIC pareto-front)
```

Add this header to your source files:

```cpp
#include <pareto_front/front.h>
```

Or to use Pareto archives:

```cpp
#include <pareto_front/archives.h>
```

However, it's always recommended to look for ParetoFront with `find_package` before including it as a subdirectory. Otherwise, we can get [ODR errors](https://en.wikipedia.org/wiki/One_Definition_Rule) in larger projects. 

#### CMake with Automatic Download

Check if you have [Cmake](http://cmake.org) 3.14+ installed:

```bash
cmake -version
```

Install [CPM.cmake](https://github.com/TheLartians/CPM.cmake) and then:

```cmake
CPMAddPackage(
    NAME ParetoFront
    GITHUB_REPOSITORY alandefreitas/pareto-front
    GIT_TAG origin/master # or whatever tag you want
)
# ...
target_link_libraries(my_target PUBLIC pareto-front)
```

Then add this header to your source files:

```cpp
#include <pareto_front/front.h>
```

Or to use Pareto archives:

```cpp
#include <pareto_front/archives.h>
```

However, it's always recommended to look for ParetoFront with `find_package` before including it as a subdirectory. You can use:

```
option(CPM_USE_LOCAL_PACKAGES "Try `find_package` before downloading dependencies" ON)
```

to let CPM.cmake do that for you. Otherwise, we can get [ODR errors](https://en.wikipedia.org/wiki/One_Definition_Rule) in larger projects.

### Other build systems

If you want to use it in another build system you can either install the library (Section [*Installing*](#installing)) or you have to somehow rewrite the build script.

If you want to rewrite the build script, your project needs to 1) include the headers and compile all source files in the [`sources`](sources) directory, and 2) link the dependencies described in [`sources/CMakeLists.txt`](sources/CMakeLists.txt).

Then add this header to your source files:

```cpp
#include <pareto_front/front.h>
```

Or to use Pareto archives:

```cpp
#include <pareto_front/archives.h>
```

## Limitations

One limitation of the library interface is that the most efficient data structures have their dimension defined at compile-time. This is not an inconvenience to C++ users but it makes it impossible to have Python bindings for all front dimensions defined at compile-time. 

Our solution was to include the fronts with <img src="https://render.githubusercontent.com/render/math?math=m=1,...,10"> dimensions set at compile-time in our Python bindings. For each data structure, we provide a extra implementation that has its dimension determined at runtime. The Python bindings fallback to this special implementation whenever <img src="https://render.githubusercontent.com/render/math?math=m>10">. 

This value can be altered when compiling the library:
 
```bash
mkdir build
cmake -version
cmake .. -DMAX_NUM_DIMENSIONS_PYTHON=20 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2"
cmake --build . -j 2 --config Release
```

 Change the value of `MAX_NUM_DIMENSIONS_PYTHON` to whatever number of dimensions you need in compile-time. The higher this value, the larger the final binary. However, you shouldn't usually need to change this value. The rationale is that, due to the [curse of dimensionality](https://en.wikipedia.org/wiki/Curse_of_dimensionality), if you have a problem with more than 10 dimensions, if you have bigger problems than whether the dimension is set at compile or run-time. 

## Contributing

There are many ways in which you can contribute to this library:

* Testing the library in new environments <sup>see [1](https://github.com/alandefreitas/pareto-front/issues?q=is%3Aopen+is%3Aissue+label%3A%22cross-platform+issue+-+windows%22), [2](https://github.com/alandefreitas/pareto-front/issues?q=is%3Aopen+is%3Aissue+label%3A%22cross-platform+issue+-+linux%22), [3](https://github.com/alandefreitas/pareto-front/issues?q=is%3Aopen+is%3Aissue+label%3A%22cross-platform+issue+-+macos%22) </sup>
* Contributing with interesting examples <sup>see [1](examples)</sup>
* Including new indicators <sup>see [1](#indicators)</sup>
* Improving this documentation <sup>see [1](https://github.com/alandefreitas/pareto-front/issues?q=is%3Aopen+is%3Aissue+label%3A%22enhancement+-+documentation%22) </sup>
* New spacial data structures <sup>see [1](https://github.com/alandefreitas/pareto-front/issues?q=is%3Aopen+is%3Aissue+label%3A%22enhancement+-+data+structures%22) </sup>
* Finding bugs in general <sup>see [1](https://github.com/alandefreitas/pareto-front/issues?q=is%3Aopen+is%3Aissue+label%3A%22bug+-+compilation+error%22), [2](https://github.com/alandefreitas/pareto-front/issues?q=is%3Aopen+is%3Aissue+label%3A%22bug+-+compilation+warning%22), [3](https://github.com/alandefreitas/pareto-front/issues?q=is%3Aopen+is%3Aissue+label%3A%22bug+-+runtime+error%22), [4](https://github.com/alandefreitas/pareto-front/issues?q=is%3Aopen+is%3Aissue+label%3A%22bug+-+runtime+warning%22) </sup>
* Whatever idea seems interesting to you

If contributing with code, please leave the pedantic mode ON (`-DBUILD_WITH_PEDANTIC_WARNINGS=ON`).

<details>
    <summary>Example: CLion</summary>
    
![CLion Settings with Pedantic Mode](docs/img/pedantic_clion.png)
    
</details>


## Thanks

We would like to thank the developers of these libraries:

- [Boost.Geometry package](https://github.com/boostorg/geometry): we use Boost.Geometry for our benchmarks
- [Trase library](https://github.com/trase-cpp/trase): we used this library to generate the plots for the first version of this library
- [hv-2.0rc2/hv.h for calculating hyper-volumes](https://ieeexplore.ieee.org/document/1688440): we use this implementation to calculate exact hypervolumes in high dimensions.

## References

These are some references we used for this work:

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


