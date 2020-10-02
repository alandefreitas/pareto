---
layout: default
title: Indicators
nav_order: 7
has_children: false
parent: Examples
has_toc: false
---
# Indicators

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





<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
