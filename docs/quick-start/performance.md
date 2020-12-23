---
layout: default
title: Performance
nav_order: 5
has_children: false
parent: Quick start
has_toc: false
---
# Performance

The problem of storing multidimensional data is simple to explain but not so easy to solve. It might seem like linear lists, even with their $O(n^2)$ pair-wise comparisons, wouldn't fair much worse than these alternative containers. Even large scale multidimensional problems have at least some subproblems with less than a hundred solutions. 

One common problem in scientific applications is that most of these containers can only outperform linear lists when storing thousands of objects. This happens mainly because data structures based on trees require one memory allocation per node.

!!! info "Setting the Number of Dimensions"
    The first strategy we use to mitigate this problem is to allow the number of dimensions to be set at compile-time or runtime. This reduces the number of memory allocations because setting the dimension at runtime require one extra memory allocation per node.

!!! info "Memory Allocation"
    However, to make these associative containers fully competitive with linear lists in all scenarios, we need memory allocators. To avoid one dynamic allocation per node, pool allocators, like linear lists, pre-allocate fixed-size chucks of memory for tree nodes.

    All containers implement the [AllocatorAwareContainer](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer) concept, that includes constructors that can receive custom allocators. All memory allocations happen through these custom allocators. If no allocator is provided, the build script will try to infer a proper allocator for each data structure.



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
