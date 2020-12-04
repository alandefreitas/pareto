//
// Created by Alan Freitas on 2020-06-03.
//

#ifndef PARETO_FRONT_MEMORY_POOL_H
#define PARETO_FRONT_MEMORY_POOL_H

#include <climits>
#include <cstddef>
#include <set>
#include <cassert>
#include <pareto/memory/interleaved_memory_pool.h>
#include <pareto/memory/free_memory_pool.h>

namespace pareto {
    /// \class This is the memory pool you want to use most of the time
    /// This makes non-contiguous containers (like list, set, and map) perform
    /// like contiguous containers (like vector, and deque) by allocating
    /// a lot of memory for the tree nodes only once.
    /// This allocator cannot be used with contiguous containers because
    /// it can only allocate one slot of memory at a time.
    template <typename T = int>
    using fast_memory_pool = interleaved_memory_pool<T, false>;

    /// \class This is a memory pool we can use for contiguous containers
    /// Although this version of the memory pool works for contiguous containers
    /// the standard new/delete operators are probably going to work better than this
    template <typename T>
    using contiguous_memory_pool = interleaved_memory_pool<T, true>;
}

#endif //PARETO_FRONT_MEMORY_POOL_H