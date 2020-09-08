//
// Created by Alan Freitas on 08/09/20.
//

#include "test_instantiations.h"
#include <pareto_front/point.h>

namespace pareto {
    template class point<double, 0>;
    template class point<double, 1>;
    template class point<double, 3>;
    template class point<double, 5>;
    template class point<double, 9>;
    template class point<double, 13>;
}