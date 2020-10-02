---
layout: default
title: Archives
nav_order: 8
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

The archive interface has the same functions as the front interface: insertion, removal, and searching operations identify the proper front for the elements; functions for indicators and dominance relationships use the first fronts as reference.





<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
