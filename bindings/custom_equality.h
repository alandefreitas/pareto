//
// Created by Alan Freitas on 10/09/20.
//

#ifndef PARETO_FRONT_CUSTOM_EQUALITY_H
#define PARETO_FRONT_CUSTOM_EQUALITY_H

#include <pybind11/pybind11.h>
#include <pareto_front/common.h>

// create a custom equality operator for py::object
namespace pareto {
    template <>
    bool mapped_type_custom_equality_operator<pybind11::object>(const pybind11::object& m1, const pybind11::object& m2);
}


#endif // PARETO_FRONT_CUSTOM_EQUALITY_H
