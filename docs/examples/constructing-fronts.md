---
layout: default
title: Constructing fronts
nav_order: 1
has_children: false
parent: Examples
has_toc: false
---
# Constructing fronts

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

![2-dimensional front](../img/front2d_b.svg)

If you need to plot these fronts, [`examples/matplotpp_example.cpp`](https://github.com/alandefreitas/pareto/blob/master/examples/matplotpp_example.cpp) includes an example that uses [Matplot++](https://github.com/alandefreitas/matplotplusplus). In Python, you can use [Matplotlib](https://matplotlib.org).

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

![2-dimensional front](../img/front2d.svg)

And these are the combinations for two-dimensional fronts:

![2-dimensional front](../img/front2d_directions.svg)

In more than two dimensions, we usually represent the fronts with parallel coordinates:

![2-dimensional front](../img/front3d.svg)

It's often useful to cache elements even if they are not in the Pareto front. For instance, we might need a replacement for an element in the front that's not longer available, or few elements might make it to the more and we might need more options. In these cases, you can use [Pareto archives](archives.md) to keep track of the elements which are second-best, third-best, ....

![2-dimensional front](../img/archive2d.svg)




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
