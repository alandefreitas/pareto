---
layout: default
title: Functions
nav_order: 5
has_children: false
has_toc: false
---
# Functions

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






<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
