//
// Created by Alan Freitas on 08/09/20.
//

#ifndef PARETO_FRONT_PYTHON_INSTANTIATIONS_H
#define PARETO_FRONT_PYTHON_INSTANTIATIONS_H

#include <pareto_front/front.h>
#include <pareto_front/archive.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace pareto {
    // That tells the test .cpp files not to implicitly instantiate these
    extern template class point<double, 0>;
    extern template class point<double, 1>;
    extern template class point<double, 2>;
    extern template class point<double, 3>;
    extern template class point<double, 4>;
    extern template class point<double, 5>;
    extern template class point<double, 6>;
    extern template class point<double, 7>;
    extern template class point<double, 8>;
    extern template class point<double, 9>;
    extern template class point<double, 10>;

    extern template class query_box<double, 0>;
    extern template class query_box<double, 1>;
    extern template class query_box<double, 2>;
    extern template class query_box<double, 3>;
    extern template class query_box<double, 4>;
    extern template class query_box<double, 5>;
    extern template class query_box<double, 6>;
    extern template class query_box<double, 7>;
    extern template class query_box<double, 8>;
    extern template class query_box<double, 9>;
    extern template class query_box<double, 10>;

    extern template class vector_tree<double, 0, py::object>;
    extern template class vector_tree<double, 1, py::object>;
    extern template class vector_tree<double, 2, py::object>;
    extern template class vector_tree<double, 3, py::object>;
    extern template class vector_tree<double, 4, py::object>;
    extern template class vector_tree<double, 5, py::object>;
    extern template class vector_tree<double, 6, py::object>;
    extern template class vector_tree<double, 7, py::object>;
    extern template class vector_tree<double, 8, py::object>;
    extern template class vector_tree<double, 9, py::object>;
    extern template class vector_tree<double, 10, py::object>;

    extern template class quad_tree<double, 0, py::object>;
    extern template class quad_tree<double, 1, py::object>;
    extern template class quad_tree<double, 2, py::object>;
    extern template class quad_tree<double, 3, py::object>;
    extern template class quad_tree<double, 4, py::object>;
    extern template class quad_tree<double, 5, py::object>;
    extern template class quad_tree<double, 6, py::object>;
    extern template class quad_tree<double, 7, py::object>;
    extern template class quad_tree<double, 8, py::object>;
    extern template class quad_tree<double, 9, py::object>;
    extern template class quad_tree<double, 10, py::object>;

    extern template class kd_tree<double, 0, py::object>;
    extern template class kd_tree<double, 1, py::object>;
    extern template class kd_tree<double, 2, py::object>;
    extern template class kd_tree<double, 3, py::object>;
    extern template class kd_tree<double, 4, py::object>;
    extern template class kd_tree<double, 5, py::object>;
    extern template class kd_tree<double, 6, py::object>;
    extern template class kd_tree<double, 7, py::object>;
    extern template class kd_tree<double, 8, py::object>;
    extern template class kd_tree<double, 9, py::object>;
    extern template class kd_tree<double, 10, py::object>;

    extern template class r_tree<double, 0, py::object>;
    extern template class r_tree<double, 1, py::object>;
    extern template class r_tree<double, 2, py::object>;
    extern template class r_tree<double, 3, py::object>;
    extern template class r_tree<double, 4, py::object>;
    extern template class r_tree<double, 5, py::object>;
    extern template class r_tree<double, 6, py::object>;
    extern template class r_tree<double, 7, py::object>;
    extern template class r_tree<double, 8, py::object>;
    extern template class r_tree<double, 9, py::object>;
    extern template class r_tree<double, 10, py::object>;

    extern template class r_star_tree<double, 0, py::object>;
    extern template class r_star_tree<double, 1, py::object>;
    extern template class r_star_tree<double, 2, py::object>;
    extern template class r_star_tree<double, 3, py::object>;
    extern template class r_star_tree<double, 4, py::object>;
    extern template class r_star_tree<double, 5, py::object>;
    extern template class r_star_tree<double, 6, py::object>;
    extern template class r_star_tree<double, 7, py::object>;
    extern template class r_star_tree<double, 8, py::object>;
    extern template class r_star_tree<double, 9, py::object>;
    extern template class r_star_tree<double, 10, py::object>;

    extern template class front<double, 0, py::object>;
    extern template class front<double, 1, py::object>;
    extern template class front<double, 2, py::object>;
    extern template class front<double, 3, py::object>;
    extern template class front<double, 4, py::object>;
    extern template class front<double, 5, py::object>;
    extern template class front<double, 6, py::object>;
    extern template class front<double, 7, py::object>;
    extern template class front<double, 8, py::object>;
    extern template class front<double, 9, py::object>;
    extern template class front<double, 10, py::object>;

    extern template class front<double, 0, py::object, vector_tree_tag>;
    extern template class front<double, 1, py::object, vector_tree_tag>;
    extern template class front<double, 2, py::object, vector_tree_tag>;
    extern template class front<double, 3, py::object, vector_tree_tag>;
    extern template class front<double, 4, py::object, vector_tree_tag>;
    extern template class front<double, 5, py::object, vector_tree_tag>;
    extern template class front<double, 6, py::object, vector_tree_tag>;
    extern template class front<double, 7, py::object, vector_tree_tag>;
    extern template class front<double, 8, py::object, vector_tree_tag>;
    extern template class front<double, 9, py::object, vector_tree_tag>;
    extern template class front<double, 10, py::object, vector_tree_tag>;

    extern template class front<double, 0, py::object, quad_tree_tag>;
    extern template class front<double, 1, py::object, quad_tree_tag>;
    extern template class front<double, 2, py::object, quad_tree_tag>;
    extern template class front<double, 3, py::object, quad_tree_tag>;
    extern template class front<double, 4, py::object, quad_tree_tag>;
    extern template class front<double, 5, py::object, quad_tree_tag>;
    extern template class front<double, 6, py::object, quad_tree_tag>;
    extern template class front<double, 7, py::object, quad_tree_tag>;
    extern template class front<double, 8, py::object, quad_tree_tag>;
    extern template class front<double, 9, py::object, quad_tree_tag>;
    extern template class front<double, 10, py::object, quad_tree_tag>;

    extern template class front<double, 0, py::object, kd_tree_tag>;
    extern template class front<double, 1, py::object, kd_tree_tag>;
    extern template class front<double, 2, py::object, kd_tree_tag>;
    extern template class front<double, 3, py::object, kd_tree_tag>;
    extern template class front<double, 4, py::object, kd_tree_tag>;
    extern template class front<double, 5, py::object, kd_tree_tag>;
    extern template class front<double, 6, py::object, kd_tree_tag>;
    extern template class front<double, 7, py::object, kd_tree_tag>;
    extern template class front<double, 8, py::object, kd_tree_tag>;
    extern template class front<double, 9, py::object, kd_tree_tag>;
    extern template class front<double, 10, py::object, kd_tree_tag>;

    extern template class front<double, 0, py::object, r_tree_tag>;
    extern template class front<double, 1, py::object, r_tree_tag>;
    extern template class front<double, 2, py::object, r_tree_tag>;
    extern template class front<double, 3, py::object, r_tree_tag>;
    extern template class front<double, 4, py::object, r_tree_tag>;
    extern template class front<double, 5, py::object, r_tree_tag>;
    extern template class front<double, 6, py::object, r_tree_tag>;
    extern template class front<double, 7, py::object, r_tree_tag>;
    extern template class front<double, 8, py::object, r_tree_tag>;
    extern template class front<double, 9, py::object, r_tree_tag>;
    extern template class front<double, 10, py::object, r_tree_tag>;

    extern template class front<double, 0, py::object, r_star_tree_tag>;
    extern template class front<double, 1, py::object, r_star_tree_tag>;
    extern template class front<double, 2, py::object, r_star_tree_tag>;
    extern template class front<double, 3, py::object, r_star_tree_tag>;
    extern template class front<double, 4, py::object, r_star_tree_tag>;
    extern template class front<double, 5, py::object, r_star_tree_tag>;
    extern template class front<double, 6, py::object, r_star_tree_tag>;
    extern template class front<double, 7, py::object, r_star_tree_tag>;
    extern template class front<double, 8, py::object, r_star_tree_tag>;
    extern template class front<double, 9, py::object, r_star_tree_tag>;
    extern template class front<double, 10, py::object, r_star_tree_tag>;

    extern template class archive<double, 0, py::object>;
    extern template class archive<double, 1, py::object>;
    extern template class archive<double, 2, py::object>;
    extern template class archive<double, 3, py::object>;
    extern template class archive<double, 4, py::object>;
    extern template class archive<double, 5, py::object>;
    extern template class archive<double, 6, py::object>;
    extern template class archive<double, 7, py::object>;
    extern template class archive<double, 8, py::object>;
    extern template class archive<double, 9, py::object>;
    extern template class archive<double, 10, py::object>;

    extern template class archive<double, 0, py::object, vector_tree_tag>;
    extern template class archive<double, 1, py::object, vector_tree_tag>;
    extern template class archive<double, 2, py::object, vector_tree_tag>;
    extern template class archive<double, 3, py::object, vector_tree_tag>;
    extern template class archive<double, 4, py::object, vector_tree_tag>;
    extern template class archive<double, 5, py::object, vector_tree_tag>;
    extern template class archive<double, 6, py::object, vector_tree_tag>;
    extern template class archive<double, 7, py::object, vector_tree_tag>;
    extern template class archive<double, 8, py::object, vector_tree_tag>;
    extern template class archive<double, 9, py::object, vector_tree_tag>;
    extern template class archive<double, 10, py::object, vector_tree_tag>;

    extern template class archive<double, 0, py::object, quad_tree_tag>;
    extern template class archive<double, 1, py::object, quad_tree_tag>;
    extern template class archive<double, 2, py::object, quad_tree_tag>;
    extern template class archive<double, 3, py::object, quad_tree_tag>;
    extern template class archive<double, 4, py::object, quad_tree_tag>;
    extern template class archive<double, 5, py::object, quad_tree_tag>;
    extern template class archive<double, 6, py::object, quad_tree_tag>;
    extern template class archive<double, 7, py::object, quad_tree_tag>;
    extern template class archive<double, 8, py::object, quad_tree_tag>;
    extern template class archive<double, 9, py::object, quad_tree_tag>;
    extern template class archive<double, 10, py::object, quad_tree_tag>;

    extern template class archive<double, 0, py::object, kd_tree_tag>;
    extern template class archive<double, 1, py::object, kd_tree_tag>;
    extern template class archive<double, 2, py::object, kd_tree_tag>;
    extern template class archive<double, 3, py::object, kd_tree_tag>;
    extern template class archive<double, 4, py::object, kd_tree_tag>;
    extern template class archive<double, 5, py::object, kd_tree_tag>;
    extern template class archive<double, 6, py::object, kd_tree_tag>;
    extern template class archive<double, 7, py::object, kd_tree_tag>;
    extern template class archive<double, 8, py::object, kd_tree_tag>;
    extern template class archive<double, 9, py::object, kd_tree_tag>;
    extern template class archive<double, 10, py::object, kd_tree_tag>;

    extern template class archive<double, 0, py::object, r_tree_tag>;
    extern template class archive<double, 1, py::object, r_tree_tag>;
    extern template class archive<double, 2, py::object, r_tree_tag>;
    extern template class archive<double, 3, py::object, r_tree_tag>;
    extern template class archive<double, 4, py::object, r_tree_tag>;
    extern template class archive<double, 5, py::object, r_tree_tag>;
    extern template class archive<double, 6, py::object, r_tree_tag>;
    extern template class archive<double, 7, py::object, r_tree_tag>;
    extern template class archive<double, 8, py::object, r_tree_tag>;
    extern template class archive<double, 9, py::object, r_tree_tag>;
    extern template class archive<double, 10, py::object, r_tree_tag>;

    extern template class archive<double, 0, py::object, r_star_tree_tag>;
    extern template class archive<double, 1, py::object, r_star_tree_tag>;
    extern template class archive<double, 2, py::object, r_star_tree_tag>;
    extern template class archive<double, 3, py::object, r_star_tree_tag>;
    extern template class archive<double, 4, py::object, r_star_tree_tag>;
    extern template class archive<double, 5, py::object, r_star_tree_tag>;
    extern template class archive<double, 6, py::object, r_star_tree_tag>;
    extern template class archive<double, 7, py::object, r_star_tree_tag>;
    extern template class archive<double, 8, py::object, r_star_tree_tag>;
    extern template class archive<double, 9, py::object, r_star_tree_tag>;
    extern template class archive<double, 10, py::object, r_star_tree_tag>;
}

#endif // PARETO_FRONT_PYTHON_INSTANTIATIONS_H
