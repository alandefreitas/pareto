---
layout: default
title: Archives
nav_order: 9
has_children: false
parent: Examples
has_toc: false
---
# Archives

Many applications require Pareto archives rather than fronts. An archive with capacity for 100 elements can be constructed with the interface below:

```python
ar = pyfront.archive(2, 100)
```

```cpp
archive<double, 2, unsigned> ar(100);
```

![2-dimensional front](../img/archive2d.svg)

The archives have the functions `begin()`/`end()` to iterate their elements, and `begin_front()`/`end_front()` to iterate their fronts.

The container makes sure the archive never has more elements than allowed by the `max_size` parameter. If the capacity is about the exceed, the container will remove the element in the most crowded region of the worst front in the archive.

If we insert a new element to the archive, and this element dominates other solutions in the archive, the container makes sure the dominated elements move to higher fronts efficiently.

The archive interface has all functions a usual front has: insertion, removal, and querying. Searching operations identify the proper front for the elements. Functions for indicators and dominance relationships use the first fronts as reference.




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
