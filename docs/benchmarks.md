---
layout: default
title: Benchmarks
nav_order: 6
has_children: false
has_toc: false
---
# Benchmarks

The default tag for fronts and archives is converted to an appropriate data structure according to the front or archive parameters. This section presents some benchmarks comparing these data structures. We use the notation `L`, `Q`, `K`, `B`, `R`, and `*` for Lists, Quadtrees, <img src="https://render.githubusercontent.com/render/math?math=k">-d trees, Boost.Geomtry R-trees, Point R-trees, and R*-Trees. The tree data structures in the benchmark used a [memory pool allocator](https://github.com/alandefreitas/pareto/blob/master/) for faster allocation. This is intented to make trees more competitive with linear lists for small fronts. If using this code in production, it is more prudent to use [`std::pmr::unsynchronized_pool_resource`](https://en.cppreference.com/w/cpp/memory/unsynchronized_pool_resource/unsynchronized_pool_resource) (if your compiler supports it), [`std::allocator`](https://en.cppreference.com/w/cpp/memory/allocator) (if you want to be conservative), or execute *many* tests to make sure [`pareto::fast_memory_pool`](https://github.com/alandefreitas/pareto/blob/master/) works properly on your system.


Constructor

![Construct (n=50)](img/construct_n_50.png)
![Construct (n=500)](img/construct_n_500.png)
![Construct (n=5000)](img/construct_n_5000.png)




Insert

![Insertion (n=50)](img/insertion_n_50.png)
![Insertion (n=500)](img/insertion_n_500.png)
![Insertion (n=5000)](img/insertion_n_5000.png)




Delete

![Removal (n=50)](img/removal_n_50.png)
![Removal (n=500)](img/removal_n_500.png)
![Removal (n=5000)](img/removal_n_5000.png)




Test dominance

![Check dominance (n=50)](img/check_dominance_n_50.png)
![Check dominance (n=500)](img/check_dominance_n_500.png)
![Check dominance (n=5000)](img/check_dominance_n_5000.png)




Query and iterate

![Query and iterate (n=50)](img/query_n_50.png)
![Query and iterate (n=500)](img/query_n_500.png)
![Query and iterate (n=5000)](img/query_n_5000.png)




Nearest points

![Find nearest 5 and iterate (n=50)](img/nearest_n_50.png)
![Find nearest 5 and iterate (n=500)](img/nearest_n_500.png)
![Find nearest 5 and iterate (n=5000)](img/nearest_n_5000.png)




IGD indicator

![IGD (n=50)](img/igd_n_50.png)
![IGD (n=500)](img/igd_n_500.png)
![IGD (n=5000)](img/igd_n_5000.png)




Hypervolume: Time

![Hypervolume (m=1)](img/hypervolume_m_1.png)
![Hypervolume (m=2)](img/hypervolume_m_2.png)
![Hypervolume (m=3)](img/hypervolume_m_3.png)
![Hypervolume (m=5)](img/hypervolume_m_5.png)
![Hypervolume (m=9)](img/hypervolume_m_9.png)




Hypervolume: Gap from the exact hypervolume

![Hypervolume Gap](img/hypervolume_gap.png)




Operations

![Operation Averages (n=50)](img/op_averages_n_50.png)
![Operation Averages (n=500)](img/op_averages_n_500.png)
![Operation Averages (n=5000)](img/op_averages_n_5000.png)




Data structures

![Data Structure Averages (n=50,m=1)](img/structure_averages_n_50_m_1.png)
![Data Structure Averages (n=50,m=2)](img/structure_averages_n_50_m_2.png)
![Data Structure Averages (n=50,m=3)](img/structure_averages_n_50_m_3.png)
![Data Structure Averages (n=50,m=5)](img/structure_averages_n_50_m_5.png)
![Data Structure Averages (n=50,m=9)](img/structure_averages_n_50_m_9.png)
![Data Structure Averages (n=50,m=13)](img/structure_averages_n_50_m_13.png)
![Data Structure Averages (n=500,m=1)](img/structure_averages_n_500_m_1.png)
![Data Structure Averages (n=500,m=2)](img/structure_averages_n_500_m_2.png)
![Data Structure Averages (n=500,m=3)](img/structure_averages_n_500_m_3.png)
![Data Structure Averages (n=500,m=5)](img/structure_averages_n_500_m_5.png)
![Data Structure Averages (n=500,m=9)](img/structure_averages_n_500_m_9.png)
![Data Structure Averages (n=500,m=13)](img/structure_averages_n_500_m_13.png)
![Data Structure Averages (n=5000,m=1)](img/structure_averages_n_5000_m_1.png)
![Data Structure Averages (n=5000,m=2)](img/structure_averages_n_5000_m_2.png)
![Data Structure Averages (n=5000,m=3)](img/structure_averages_n_5000_m_3.png)
![Data Structure Averages (n=5000,m=5)](img/structure_averages_n_5000_m_5.png)
![Data Structure Averages (n=5000,m=9)](img/structure_averages_n_5000_m_9.png)
![Data Structure Averages (n=5000,m=13)](img/structure_averages_n_5000_m_13.png)






<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
