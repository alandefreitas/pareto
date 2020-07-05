///////////////////////////////////////////////////////////////////////////////
/// \file algorithm.hpp
///   Includes the range-based versions of the algorithms in the
///   C++ standard header file <algorithm>
//
/////////////////////////////////////////////////////////////////////////////

// Copyright 2009 Neil Groves.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Acknowledgements:
// This code uses combinations of ideas, techniques and code snippets
// from: Thorsten Ottosen, Eric Niebler, Jeremy Siek,
// and Vladimir Prus'
//
// The original mutating algorithms that served as the first version
// were originally written by Vladimir Prus'
// <ghost@cs.msu.su> code from Boost Wiki

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BOOST_RANGE_ALGORITHM_HPP_INCLUDED_01012009
#define BOOST_RANGE_ALGORITHM_HPP_INCLUDED_01012009

#include <pareto_front/boost_subset/range/concepts.hpp>
#include <pareto_front/boost_subset/range/iterator_range.hpp>
#include <pareto_front/boost_subset/range/difference_type.hpp>
#include <pareto_front/boost_subset/range/detail/range_return.hpp>
#include <pareto_front/boost_subset/iterator/iterator_traits.hpp>
#include <pareto_front/boost_subset/next_prior.hpp>
#include <algorithm>

// Non-mutating algorithms
#include <pareto_front/boost_subset/range/algorithm/adjacent_find.hpp>
#include <pareto_front/boost_subset/range/algorithm/count.hpp>
#include <pareto_front/boost_subset/range/algorithm/count_if.hpp>
#include <pareto_front/boost_subset/range/algorithm/equal.hpp>
#include <pareto_front/boost_subset/range/algorithm/for_each.hpp>
#include <pareto_front/boost_subset/range/algorithm/find.hpp>
#include <pareto_front/boost_subset/range/algorithm/find_end.hpp>
#include <pareto_front/boost_subset/range/algorithm/find_first_of.hpp>
#include <pareto_front/boost_subset/range/algorithm/find_if.hpp>
#include <pareto_front/boost_subset/range/algorithm/lexicographical_compare.hpp>
#include <pareto_front/boost_subset/range/algorithm/mismatch.hpp>
#include <pareto_front/boost_subset/range/algorithm/search.hpp>
#include <pareto_front/boost_subset/range/algorithm/search_n.hpp>

// Mutating algorithms
#include <pareto_front/boost_subset/range/algorithm/copy.hpp>
#include <pareto_front/boost_subset/range/algorithm/copy_backward.hpp>
#include <pareto_front/boost_subset/range/algorithm/fill.hpp>
#include <pareto_front/boost_subset/range/algorithm/fill_n.hpp>
#include <pareto_front/boost_subset/range/algorithm/generate.hpp>
#include <pareto_front/boost_subset/range/algorithm/inplace_merge.hpp>
#include <pareto_front/boost_subset/range/algorithm/merge.hpp>
#include <pareto_front/boost_subset/range/algorithm/nth_element.hpp>
#include <pareto_front/boost_subset/range/algorithm/partial_sort.hpp>
#include <pareto_front/boost_subset/range/algorithm/partial_sort_copy.hpp>
#include <pareto_front/boost_subset/range/algorithm/partition.hpp>
#include <pareto_front/boost_subset/range/algorithm/random_shuffle.hpp>
#include <pareto_front/boost_subset/range/algorithm/remove.hpp>
#include <pareto_front/boost_subset/range/algorithm/remove_copy.hpp>
#include <pareto_front/boost_subset/range/algorithm/remove_copy_if.hpp>
#include <pareto_front/boost_subset/range/algorithm/remove_if.hpp>
#include <pareto_front/boost_subset/range/algorithm/replace.hpp>
#include <pareto_front/boost_subset/range/algorithm/replace_copy.hpp>
#include <pareto_front/boost_subset/range/algorithm/replace_copy_if.hpp>
#include <pareto_front/boost_subset/range/algorithm/replace_if.hpp>
#include <pareto_front/boost_subset/range/algorithm/reverse.hpp>
#include <pareto_front/boost_subset/range/algorithm/reverse_copy.hpp>
#include <pareto_front/boost_subset/range/algorithm/rotate.hpp>
#include <pareto_front/boost_subset/range/algorithm/rotate_copy.hpp>
#include <pareto_front/boost_subset/range/algorithm/sort.hpp>
#include <pareto_front/boost_subset/range/algorithm/stable_partition.hpp>
#include <pareto_front/boost_subset/range/algorithm/stable_sort.hpp>
#include <pareto_front/boost_subset/range/algorithm/transform.hpp>
#include <pareto_front/boost_subset/range/algorithm/unique.hpp>
#include <pareto_front/boost_subset/range/algorithm/unique_copy.hpp>

// Binary search
#include <pareto_front/boost_subset/range/algorithm/binary_search.hpp>
#include <pareto_front/boost_subset/range/algorithm/equal_range.hpp>
#include <pareto_front/boost_subset/range/algorithm/lower_bound.hpp>
#include <pareto_front/boost_subset/range/algorithm/upper_bound.hpp>

// Set operations of sorted ranges
#include <pareto_front/boost_subset/range/algorithm/set_algorithm.hpp>

// Heap operations
#include <pareto_front/boost_subset/range/algorithm/heap_algorithm.hpp>

// Minimum and Maximum
#include <pareto_front/boost_subset/range/algorithm/max_element.hpp>
#include <pareto_front/boost_subset/range/algorithm/min_element.hpp>

// Permutations
#include <pareto_front/boost_subset/range/algorithm/permutation.hpp>

#endif // include guard

