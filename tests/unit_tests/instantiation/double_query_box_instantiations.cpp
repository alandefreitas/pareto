//
// Created by Alan Freitas on 08/09/20.
//

#include "test_instantiations.h"
#include <pareto/query/query_box.h>

namespace pareto {
    template class query_box<double, 0>;
    template class query_box<double, 1>;
    template class query_box<double, 3>;
    template class query_box<double, 5>;
    template class query_box<double, 9>;
    template class query_box<double, 13>;
}