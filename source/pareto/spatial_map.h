//
// Created by Alan Freitas on 12/13/20.
//

#ifndef PARETO_SPATIAL_MAP_H
#define PARETO_SPATIAL_MAP_H

#include <pareto/r_tree.h>

namespace pareto {
    template <class K, size_t M, class T, typename C = std::less<K>,
              class A = default_allocator_type<std::pair<const point<K, M>, T>>>
    using spatial_map = r_tree<K, M, T, C, A>;
}

#endif // PARETO_SPATIAL_MAP_H
