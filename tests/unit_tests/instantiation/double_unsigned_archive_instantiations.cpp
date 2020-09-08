//
// Created by Alan Freitas on 08/09/20.
//

#include "test_instantiations.h"
#include <pareto_front/front.h>

namespace pareto {
    template class archive<double, 0, unsigned>;
    template class archive<double, 1, unsigned>;
    template class archive<double, 3, unsigned>;
    template class archive<double, 5, unsigned>;
    template class archive<double, 9, unsigned>;
    template class archive<double, 13, unsigned>;
}