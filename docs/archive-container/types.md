---
layout: default
title: Types
nav_order: 2
has_children: false
parent: Archive Container
has_toc: false
---
# Types

This table summarizes the public types in a `pareto::archive<K,M,T,C>`:

| Concept/Type Name                                            | Type                                                         | Notes                                                        |
| ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| [**Container**](https://en.cppreference.com/w/cpp/named_req/Container) |                                                              |                                                              |
| `value_type`                                                 | `container_type::value_type`                                 | The pair key is `const`, like in other associative containers |
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
| `key_type`                                                   | `pareto::point<K,M>`                                         | Unlike in `value_type`, `key_type`  is not const, so you can use it to construct and manipulate new points |
| `mapped_type`                                                | `T`                                                          |                                                              |
| `key_compare`                                                | `std::function<bool(const value_type &, const value_type &)>` | `key_compare` defines a lexicographic ordering relation over keys using `dimension_compare` |
| `value_compare`                                              | `std::function<bool(const value_type &, const value_type &)>` | `value_compare` defines an ordering relation over `value_type` using `key_compare` |
| [**AllocatorAwareContainer**](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer) |                                                              |                                                              |
| `allocator_type`                                             | `container_type::allocator_type`                             | `allocator_type::value_type` is the same as `value_type`     |
| **SpatialContainer**                                         |                                                              |                                                              |
| `dimension_type`                                                | `K`                                                          |                                                              |
| `dimension_compare`                                             | `container_type::dimension_compare`, or `std::less<K>` by default | `dimension_compare` defines an ordering relation over each `key_value` dimension using `C` |
| `box_type`                                                   | `pareto::query_box<dimension_type, M>`                          |                                                              |
| `predicate_list_type`                                        | `pareto::predicate_list<dimension_type, M, T>`                  |                                                              |
| **SpatialAdapter**                                           |                                                              |                                                              |
| `container_type`                                             | `C`                                                          | `C` needs to follow the SpatialContainer concept             |
| **ArchiveContainer**                                           |                                                              |                                                              |
| `front_set_type`                                             | `std::set<front_type, std::less<>, front_set_allocator_type>;`  | Set of fronts             |

**Notes**

!!! note
    All requirements of fronts also apply here.



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
