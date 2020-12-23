---
layout: default
title: Interfaces
nav_order: 4
has_children: false
parent: Quick start
has_toc: false
---
# Interfaces

These containers formally follow and extend on the named requirements of the C++ standard library. If you know how to use `std::map`, you already know how to use 90% any of these containers. You can use `m.erase(it)`, `m.insert(v)`, `m.empty()`, `m.size()`, `m.begin()` , and `m.end()` like you would with any other associative container.

!!! important "Python Bindings"
    Although this library is completely implemented in C++17, because data scientists love Python, we also include Python bindings for all these data structures. We further replicate the syntax of the native Python data structures, so that `m.erase(k)` becomes `del m[k]`, `if m.empty()` becomes `if m:`, and  `m.insert(k,v)` becomes `m[k] = v`. If you're a C++ programmer using Python, the C++ container syntax is still available in Python.

!!! summary "C++ Concepts / Named Requirements"
    Formally, these containers implement the [Container](https://en.cppreference.com/w/cpp/named_req/Container), [ReversibleContainer](https://en.cppreference.com/w/cpp/named_req/ReversibleContainer), [AllocatorAwareContainer](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer), and [AssociativeContainer](https://en.cppreference.com/w/cpp/named_req/AssociativeContainer) Concepts / Named Requirements. Their iterators also implement the [LegacyBidirectionalIterator](https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator) concepts and they can use memory allocators that follow the [Allocator](https://en.cppreference.com/w/cpp/named_req/Allocator) concept. The extensions are formally defined as the concepts [SpatialContainer](https://github.com/alandefreitas/pareto/blob/master/tests/unit_tests/concepts.cpp), [FrontContainer](https://github.com/alandefreitas/pareto/blob/master/tests/unit_tests/concepts.cpp), and [ArchiveContainer](https://github.com/alandefreitas/pareto/blob/master/tests/unit_tests/concepts.cpp), whose pre- and post- conditions are checked with our unit tests.

All that means they work transparently with other native data structures. We include lots of unit tests, benchmarks, and continuous integration to make sure this compatibility is maintained. This also means they're easy to integrate with other libraries. For instance, the source file [`examples/matplotpp_example.cpp`](https://github.com/alandefreitas/pareto/blob/master/examples/matplotpp_example.cpp) and the headers in [`source/pareto/matplot`](https://github.com/alandefreitas/pareto/blob/master/source/pareto/matplot) exemplify how to create the plots you are seeing in this documentation with [Matplot++](https://github.com/alandefreitas/matplotplusplus). 



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
