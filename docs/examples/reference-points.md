---
layout: default
title: Reference points
nav_order: 3
has_children: false
parent: Examples
has_toc: false
---
# Reference points

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




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
