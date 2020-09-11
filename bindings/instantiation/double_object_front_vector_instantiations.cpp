//
// Created by Alan Freitas on 08/09/20.
//

#include <pybind11/pybind11.h>
#include "../custom_equality.h"
#include <pareto_front/front.h>

namespace pareto {
    template class front<double, 0, pybind11::object, vector_tree_tag>;
    template class front<double, 1, pybind11::object, vector_tree_tag>;
    template class front<double, 2, pybind11::object, vector_tree_tag>;
    template class front<double, 3, pybind11::object, vector_tree_tag>;
    template class front<double, 4, pybind11::object, vector_tree_tag>;
    template class front<double, 5, pybind11::object, vector_tree_tag>;
    template class front<double, 6, pybind11::object, vector_tree_tag>;
    template class front<double, 7, pybind11::object, vector_tree_tag>;
    template class front<double, 8, pybind11::object, vector_tree_tag>;
    template class front<double, 9, pybind11::object, vector_tree_tag>;
    template class front<double, 10, pybind11::object, vector_tree_tag>;
}