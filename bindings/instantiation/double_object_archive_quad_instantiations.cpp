//
// Created by Alan Freitas on 08/09/20.
//

#include <pybind11/pybind11.h>
#include "../custom_equality.h"
#include <pareto_front/archive.h>

namespace pareto {

    template class archive<double, 0, pybind11::object, quad_tree_tag>;
    template class archive<double, 1, pybind11::object, quad_tree_tag>;
    template class archive<double, 2, pybind11::object, quad_tree_tag>;
    template class archive<double, 3, pybind11::object, quad_tree_tag>;
    template class archive<double, 4, pybind11::object, quad_tree_tag>;
    template class archive<double, 5, pybind11::object, quad_tree_tag>;
    template class archive<double, 6, pybind11::object, quad_tree_tag>;
    template class archive<double, 7, pybind11::object, quad_tree_tag>;
    template class archive<double, 8, pybind11::object, quad_tree_tag>;
    template class archive<double, 9, pybind11::object, quad_tree_tag>;
    template class archive<double, 10, pybind11::object, quad_tree_tag>;
}