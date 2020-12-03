//
// Created by Alan Freitas on 12/3/20.
//

#ifndef PARETO_OPERATORS_H
#define PARETO_OPERATORS_H

#include <vector>
#include <array>

namespace pareto {
    /// \brief This function is what our containers use for equality comparison
    /// Using this instead of only operator== allows us to customize how to compare our
    /// points. This is mostly important for the python bindings on objects, for which
    /// pybind's operator== is deprecated
    template <class MAPPED_TYPE>
    bool mapped_type_custom_equality_operator(const MAPPED_TYPE& m1, const MAPPED_TYPE& m2) {
        return m1 == m2;
    }

}

#endif //PARETO_OPERATORS_H
