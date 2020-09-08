//
// Created by Alan Freitas on 08/09/20.
//

#include "python_instantiations.h"
#include <pareto_front/tree/vector_tree.h>

namespace pareto {
    template class vector_tree<double, 0, py::object>;
    template class vector_tree<double, 1, py::object>;
    template class vector_tree<double, 2, py::object>;
    template class vector_tree<double, 3, py::object>;
    template class vector_tree<double, 4, py::object>;
    template class vector_tree<double, 5, py::object>;
    template class vector_tree<double, 6, py::object>;
    template class vector_tree<double, 7, py::object>;
    template class vector_tree<double, 8, py::object>;
    template class vector_tree<double, 9, py::object>;
    template class vector_tree<double, 10, py::object>;
}