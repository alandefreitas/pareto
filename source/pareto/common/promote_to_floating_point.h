//
// Created by Alan Freitas on 12/26/20.
//

#ifndef PARETO_PROMOTE_TO_FLOATING_POINT_H
#define PARETO_PROMOTE_TO_FLOATING_POINT_H

namespace pareto {
    template <class INTEGER>
    using promote_signed_integral = std::conditional_t<
        sizeof(INTEGER) <= 4, float,
        std::conditional_t<sizeof(INTEGER) <= 8, double, long double>>;

    template <class INTEGER>
    using promote_unsigned_interal =
        std::conditional_t<sizeof(INTEGER) <= 4, double, long double>;

    template <class INTEGER>
    using promote_integral =
        std::conditional_t<std::is_signed_v<INTEGER>,
                           promote_signed_integral<INTEGER>,
                           promote_unsigned_interal<INTEGER>>;

    template <class NUMBER_TYPE>
    using promote_to_floating_point =
        std::conditional_t<std::is_integral_v<NUMBER_TYPE>,
                           promote_integral<NUMBER_TYPE>, NUMBER_TYPE>;

} // namespace pareto

#endif // PARETO_PROMOTE_TO_FLOATING_POINT_H
