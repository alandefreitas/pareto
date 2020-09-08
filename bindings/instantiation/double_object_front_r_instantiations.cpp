//
// Created by Alan Freitas on 08/09/20.
//

#include "python_instantiations.h"
#include <pareto_front/front.h>

namespace pareto {
    template class front<double, 0, py::object, r_tree_tag>;
    template class front<double, 1, py::object, r_tree_tag>;
    template class front<double, 2, py::object, r_tree_tag>;
    template class front<double, 3, py::object, r_tree_tag>;
    template class front<double, 4, py::object, r_tree_tag>;
    template class front<double, 5, py::object, r_tree_tag>;
    template class front<double, 6, py::object, r_tree_tag>;
    template class front<double, 7, py::object, r_tree_tag>;
    template class front<double, 8, py::object, r_tree_tag>;
    template class front<double, 9, py::object, r_tree_tag>;
    template class front<double, 10, py::object, r_tree_tag>;
}