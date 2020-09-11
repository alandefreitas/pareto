//
// Created by Alan Freitas on 08/09/20.
//

#include <pybind11/pybind11.h>
#include "../custom_equality.h"
#include <pareto_front/tree/r_tree.h>

namespace pareto {

    template class r_tree<double, 0, pybind11::object>;
    template class r_tree<double, 1, pybind11::object>;
    template class r_tree<double, 2, pybind11::object>;
    template class r_tree<double, 3, pybind11::object>;
    template class r_tree<double, 4, pybind11::object>;
    template class r_tree<double, 5, pybind11::object>;
    template class r_tree<double, 6, pybind11::object>;
    template class r_tree<double, 7, pybind11::object>;
    template class r_tree<double, 8, pybind11::object>;
    template class r_tree<double, 9, pybind11::object>;
    template class r_tree<double, 10, pybind11::object>;
}