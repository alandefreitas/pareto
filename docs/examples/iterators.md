---
layout: default
title: Iterators
nav_order: 4
has_children: false
parent: Examples
has_toc: false
---
# Iterators

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





<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
