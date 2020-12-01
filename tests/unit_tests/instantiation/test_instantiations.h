//
// Created by Alan Freitas on 08/09/20.
//

#ifndef PARETO_FRONT_PYTHON_INSTANTIATIONS_H
#define PARETO_FRONT_PYTHON_INSTANTIATIONS_H

#include <pareto/front.h>
#include <pareto/archive.h>

namespace pareto {
    // That tells the test .cpp files not to implicitly instantiate these
    extern template class point<double, 0>;
    extern template class point<double, 1>;
    extern template class point<double, 3>;
    extern template class point<double, 5>;
    extern template class point<double, 9>;
    extern template class point<double, 13>;

    extern template class query_box<double, 0>;
    extern template class query_box<double, 1>;
    extern template class query_box<double, 3>;
    extern template class query_box<double, 5>;
    extern template class query_box<double, 9>;
    extern template class query_box<double, 13>;

    extern template class vector_tree<double, 0, unsigned>;
    extern template class vector_tree<double, 1, unsigned>;
    extern template class vector_tree<double, 3, unsigned>;
    extern template class vector_tree<double, 5, unsigned>;
    extern template class vector_tree<double, 9, unsigned>;
    extern template class vector_tree<double, 13, unsigned>;

    extern template class quad_tree<double, 0, unsigned>;
    extern template class quad_tree<double, 1, unsigned>;
    extern template class quad_tree<double, 3, unsigned>;
    extern template class quad_tree<double, 5, unsigned>;
    extern template class quad_tree<double, 9, unsigned>;
    extern template class quad_tree<double, 13, unsigned>;

    extern template class kd_tree<double, 0, unsigned>;
    extern template class kd_tree<double, 1, unsigned>;
    extern template class kd_tree<double, 3, unsigned>;
    extern template class kd_tree<double, 5, unsigned>;
    extern template class kd_tree<double, 9, unsigned>;
    extern template class kd_tree<double, 13, unsigned>;

    extern template class r_tree<double, 0, unsigned>;
    extern template class r_tree<double, 1, unsigned>;
    extern template class r_tree<double, 3, unsigned>;
    extern template class r_tree<double, 5, unsigned>;
    extern template class r_tree<double, 9, unsigned>;
    extern template class r_tree<double, 13, unsigned>;

    extern template class r_star_tree<double, 0, unsigned>;
    extern template class r_star_tree<double, 1, unsigned>;
    extern template class r_star_tree<double, 3, unsigned>;
    extern template class r_star_tree<double, 5, unsigned>;
    extern template class r_star_tree<double, 9, unsigned>;
    extern template class r_star_tree<double, 13, unsigned>;

    extern template class front<double, 0, unsigned>;
    extern template class front<double, 1, unsigned>;
    extern template class front<double, 3, unsigned>;
    extern template class front<double, 5, unsigned>;
    extern template class front<double, 9, unsigned>;
    extern template class front<double, 13, unsigned>;

    extern template class front<double, 0, unsigned, vector_tree_tag>;
    extern template class front<double, 1, unsigned, vector_tree_tag>;
    extern template class front<double, 3, unsigned, vector_tree_tag>;
    extern template class front<double, 5, unsigned, vector_tree_tag>;
    extern template class front<double, 9, unsigned, vector_tree_tag>;
    extern template class front<double, 13, unsigned, vector_tree_tag>;

    extern template class front<double, 0, unsigned, quad_tree_tag>;
    extern template class front<double, 1, unsigned, quad_tree_tag>;
    extern template class front<double, 3, unsigned, quad_tree_tag>;
    extern template class front<double, 5, unsigned, quad_tree_tag>;
    extern template class front<double, 9, unsigned, quad_tree_tag>;
    extern template class front<double, 13, unsigned, quad_tree_tag>;

    extern template class front<double, 0, unsigned, kd_tree_tag>;
    extern template class front<double, 1, unsigned, kd_tree_tag>;
    extern template class front<double, 3, unsigned, kd_tree_tag>;
    extern template class front<double, 5, unsigned, kd_tree_tag>;
    extern template class front<double, 9, unsigned, kd_tree_tag>;
    extern template class front<double, 13, unsigned, kd_tree_tag>;

    extern template class front<double, 0, unsigned, r_tree_tag>;
    extern template class front<double, 1, unsigned, r_tree_tag>;
    extern template class front<double, 3, unsigned, r_tree_tag>;
    extern template class front<double, 5, unsigned, r_tree_tag>;
    extern template class front<double, 9, unsigned, r_tree_tag>;
    extern template class front<double, 13, unsigned, r_tree_tag>;

    extern template class front<double, 0, unsigned, r_star_tree_tag>;
    extern template class front<double, 1, unsigned, r_star_tree_tag>;
    extern template class front<double, 3, unsigned, r_star_tree_tag>;
    extern template class front<double, 5, unsigned, r_star_tree_tag>;
    extern template class front<double, 9, unsigned, r_star_tree_tag>;
    extern template class front<double, 13, unsigned, r_star_tree_tag>;

    extern template class archive<double, 0, unsigned>;
    extern template class archive<double, 1, unsigned>;
    extern template class archive<double, 3, unsigned>;
    extern template class archive<double, 5, unsigned>;
    extern template class archive<double, 9, unsigned>;
    extern template class archive<double, 13, unsigned>;

    extern template class archive<double, 0, unsigned, vector_tree_tag>;
    extern template class archive<double, 1, unsigned, vector_tree_tag>;
    extern template class archive<double, 3, unsigned, vector_tree_tag>;
    extern template class archive<double, 5, unsigned, vector_tree_tag>;
    extern template class archive<double, 9, unsigned, vector_tree_tag>;
    extern template class archive<double, 13, unsigned, vector_tree_tag>;

    extern template class archive<double, 0, unsigned, quad_tree_tag>;
    extern template class archive<double, 1, unsigned, quad_tree_tag>;
    extern template class archive<double, 3, unsigned, quad_tree_tag>;
    extern template class archive<double, 5, unsigned, quad_tree_tag>;
    extern template class archive<double, 9, unsigned, quad_tree_tag>;
    extern template class archive<double, 13, unsigned, quad_tree_tag>;

    extern template class archive<double, 0, unsigned, kd_tree_tag>;
    extern template class archive<double, 1, unsigned, kd_tree_tag>;
    extern template class archive<double, 3, unsigned, kd_tree_tag>;
    extern template class archive<double, 5, unsigned, kd_tree_tag>;
    extern template class archive<double, 9, unsigned, kd_tree_tag>;
    extern template class archive<double, 13, unsigned, kd_tree_tag>;

    extern template class archive<double, 0, unsigned, r_tree_tag>;
    extern template class archive<double, 1, unsigned, r_tree_tag>;
    extern template class archive<double, 3, unsigned, r_tree_tag>;
    extern template class archive<double, 5, unsigned, r_tree_tag>;
    extern template class archive<double, 9, unsigned, r_tree_tag>;
    extern template class archive<double, 13, unsigned, r_tree_tag>;

    extern template class archive<double, 0, unsigned, r_star_tree_tag>;
    extern template class archive<double, 1, unsigned, r_star_tree_tag>;
    extern template class archive<double, 3, unsigned, r_star_tree_tag>;
    extern template class archive<double, 5, unsigned, r_star_tree_tag>;
    extern template class archive<double, 9, unsigned, r_star_tree_tag>;
    extern template class archive<double, 13, unsigned, r_star_tree_tag>;
}

#endif // PARETO_FRONT_PYTHON_INSTANTIATIONS_H
