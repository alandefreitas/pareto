---
layout: default
title: Types
nav_order: 2
has_children: false
parent: Spatial Containers
has_toc: false
---
# Types

This table summarizes the public types in all SpatialContainers:

| Name                                                         | Type                                                         | Notes                                                        |
| ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| [**Container**](https://en.cppreference.com/w/cpp/named_req/Container) |                                                              |                                                              |
| `value_type`                                                 | `std::pair<const pareto::point<K,M>,T>`                      | The pair key is `const`, like in other associative containers |
| `reference`                                                  | `value_type&`                                                |                                                              |
| `const_reference`                                            | `value_type const &`                                         |                                                              |
| `iterator`                                                   | Iterator pointing to a `value_type`                          | A [LegacyBidirectionalIterator](https://en.cppreference.com/w/cpp/named_req/ForwardIterator) convertible to `const_iterator` |
| `const_iterator`                                             | Iterator pointing to a `const value_type`                    | Implements [LegacyBidirectionalIterator](https://en.cppreference.com/w/cpp/named_req/ForwardIterator) concept |
| `difference_type`                                            | A signed integer                                             |                                                              |
| `size_type`                                                  | An unsigned integer                                          |                                                              |
| [**ReversibleContainer**](https://en.cppreference.com/w/cpp/named_req/ReversibleContainer) |                                                              |                                                              |
| `reverse_iterator`                                           | `std::reverse_iterator<iterator>`                            |                                                              |
| `const_reverse_iterator`                                     | `std::reverse_iterator<const_iterator>`                      |                                                              |
| [**AssociativeContainer**](https://en.cppreference.com/w/cpp/named_req/AssociativeContainer) |                                                              |                                                              |
| `key_type`                                                   | `pareto::point<K,M>`                                         | `key_type` is not const, so you can use it to construct and manipulate new points |
| `mapped_type`                                                | `T`                                                          |                                                              |
| `key_compare`                                                | `std::function<bool(const value_type &, const value_type &)>` | `key_compare` defines a lexicographic ordering relation over keys using `dimension_compare` |
| `value_compare`                                              | `std::function<bool(const value_type &, const value_type &)>` | `value_compare` defines an ordering relation over `value_type` using `key_compare` |
| [**AllocatorAwareContainer**](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer) |                                                              |                                                              |
| `allocator_type`                                             | `A`, or `pareto::default_allocator<value_type>` by default   | `allocator_type::value_type` is the same as `value_type`     |
| **SpatialContainer**                                         |                                                              |                                                              |
| `dimension_type`                                                | `K`                                                          |                                                              |
| `dimension_compare`                                             | `C`, or `std::less<K>` by default                            | `dimension_compare` defines an ordering relation over each `key_value` dimension using `C` |
| `box_type`                                                   | `pareto::query_box<dimension_type, M>`                          |                                                              |
| `predicate_list_type`                                        | `pareto::predicate_list<dimension_type, M, T>`                  |                                                              |

**Notes**

`dimension_type` refers to a single dimension in `key_type`. Although this is usually a number, it might be an object of any other type.

!!! info "Key type"
    While the container is defined with the uni-dimensional key `K`, the container expands that into an `M`-dimensional point of type `pareto::point<K,M>`.  This does not break any named requirement for containers, as types can be different from their template parameters.

!!! info "Iterators to constant keys"
    The first type in `value_type` (`const pareto::point<K,M>`) is `const`. This is a requirement of associative containers. Otherwise, the user could externally change keys through references and the container nodes would no longer be properly ordered.

!!! info "Bidirectional Iterators"
    A `spatial_map<K,M,T,A>::iterator` is a [LegacyBidirectionalIterator](https://en.cppreference.com/w/cpp/named_req/ForwardIterator) convertible to a `const_iterator` (but to the other way around). This means iterators can move forward and backward. However, we can also use **queries** to explore specific regions of space, so it's still reasonably easy to look for random points and things like that.

<!-- Include example -->



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
