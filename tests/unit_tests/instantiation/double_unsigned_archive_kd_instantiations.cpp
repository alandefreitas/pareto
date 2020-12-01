//
// Created by Alan Freitas on 08/09/20.
//

#include "test_instantiations.h"
#include <pareto/front.h>

namespace pareto {
    template class archive<double, 0, unsigned, kd_tree_tag>;
    template class archive<double, 1, unsigned, kd_tree_tag>;
    template class archive<double, 3, unsigned, kd_tree_tag>;
    template class archive<double, 5, unsigned, kd_tree_tag>;
    template class archive<double, 9, unsigned, kd_tree_tag>;
    template class archive<double, 13, unsigned, kd_tree_tag>;
}