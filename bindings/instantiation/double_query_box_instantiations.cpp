//
// Created by Alan Freitas on 08/09/20.
//

#include "python_instantiations.h"
#include <pareto_front/query_box.h>

namespace pareto {
    template class query_box<double, 0>;
    template class query_box<double, 1>;
    template class query_box<double, 2>;
    template class query_box<double, 3>;
    template class query_box<double, 4>;
    template class query_box<double, 5>;
    template class query_box<double, 6>;
    template class query_box<double, 7>;
    template class query_box<double, 8>;
    template class query_box<double, 9>;
    template class query_box<double, 10>;
}