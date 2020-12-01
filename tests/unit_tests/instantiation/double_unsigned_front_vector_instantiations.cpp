//
// Created by Alan Freitas on 08/09/20.
//

#include "test_instantiations.h"
#include <pareto/front.h>

namespace pareto {
    template class front<double, 0, unsigned, vector_tree_tag>;
    template class front<double, 1, unsigned, vector_tree_tag>;
    template class front<double, 3, unsigned, vector_tree_tag>;
    template class front<double, 5, unsigned, vector_tree_tag>;
    template class front<double, 9, unsigned, vector_tree_tag>;
    template class front<double, 13, unsigned, vector_tree_tag>;
}