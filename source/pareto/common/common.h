//
// Created by Alan Freitas on 2020-06-04.
//

#ifndef PARETO_FRONT_COMMON_H
#define PARETO_FRONT_COMMON_H

#include <pareto/common/metaprogramming.h>
#include <pareto/common/operators.h>

namespace pareto {
    /// \brief Convert an initializer list to a vector
    /// This is a minor convenience for some containers
    [[maybe_unused,deprecated]] inline std::vector<uint8_t> init_list_to_vector(std::initializer_list<bool> is_minimization) {
        std::vector<uint8_t> v(is_minimization.begin(), is_minimization.end());
        return v;
    }
}

#endif //PARETO_FRONT_COMMON_H
