//
// Created by Alan Freitas on 08/09/20.
//

#include "test_instantiations.h"
#include <pareto_front/front.h>

namespace pareto {
    template class front<double, 0, unsigned, r_star_tree_tag>;
    template class front<double, 1, unsigned, r_star_tree_tag>;
    template class front<double, 3, unsigned, r_star_tree_tag>;
    template class front<double, 5, unsigned, r_star_tree_tag>;
    template class front<double, 9, unsigned, r_star_tree_tag>;
    template class front<double, 13, unsigned, r_star_tree_tag>;
}