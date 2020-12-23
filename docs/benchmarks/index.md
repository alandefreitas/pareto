---
layout: default
title: Introduction
nav_order: 7
has_children: true
has_toc: false
---
# Benchmarks

The directory `tests/benchmarks` include a number of benchmarks we run regularly to infer the performance of our implementations. 

This section presents the results for each spatial container used with fronts. After building the library, you can replicate these benchmarks locally with:

```bash
containers_benchmark --benchmark_repetitions=30 --benchmark_display_aggregates_only=true --benchmark_out=containers_benchmark.json --benchmark_out_format=json
```

You can later create these plots with the target `benchmark_analysis` (requires [Matplot++](https://github.com/alandefreitas/matplotplusplus)).

This section presents some benchmarks comparing the following data structures in Pareto fronts: 

* Implicit Tree
* Quadtree
* $k$d-tree
* R-tree
* R*-Tree
* Boost.Geometry R-Tree

!!! info "Boost.Geometry"
    We started this library by looking at some alternatives for spatial containers. The alternative that gave us the best results at the time was Boost.Geometry. The header `source/pareto/boost_tree.h` implements a wrapper on this library that implements a subset of our interface so that we could run these benchmarks. This header is intended for benchmarks only.  

    Although the basic operations in the Boost.Geometry wrapper have optimal asymptotic complexity, this wrapper is an optional dependency mostly used as a reference for our benchmarks. It takes some workarounds to make this wrapper work as the other containers in this library and it's soon to be deprecated so that this library can move forward. These are a few reasons why we are deprecating Geometry.Boost as a spatial container:

    * You can't define the dimensions and predicates in runtime, which are important use cases for us
    * Boost.Geometry doesn't completely follow the same C++ named requirements for containers
    * It is not specialized for point trees, so we had better performance with our containers
    * The query iterators are not bidirectional
    * It depends on Boost libraries and functions that are now deprecated



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
