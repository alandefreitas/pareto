//
// Created by Alan Freitas on 10/09/20.
//

#include "custom_equality.h"

// create a custom equality operator for py::object
namespace pareto {
    template <>
    bool mapped_type_custom_equality_operator<pybind11::object>(const pybind11::object& m1, const pybind11::object& m2) {
        return m1.is(m2);
    }
}
