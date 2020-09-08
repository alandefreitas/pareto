//
// Created by Alan Freitas on 08/09/20.
//

#include "test_instantiations.h"
#include <pareto_front/front.h>

namespace pareto {
    template class front<double, 0, unsigned, kd_tree_tag>;
    template class front<double, 1, unsigned, kd_tree_tag>;
    template class front<double, 3, unsigned, kd_tree_tag>;
    template class front<double, 5, unsigned, kd_tree_tag>;
    template class front<double, 9, unsigned, kd_tree_tag>;
    template class front<double, 13, unsigned, kd_tree_tag>;
}