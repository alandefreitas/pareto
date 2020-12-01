//
// Created by Alan Freitas on 2020-05-20.
//

#ifndef PARETO_FRONT_POINT_H
#define PARETO_FRONT_POINT_H

#include <vector>
#include <array>
#include <algorithm>
#include <numeric>
#include <ostream>
#include <cassert>
#include <cmath>

#ifdef BUILD_BOOST_TREE
#include <boost/geometry/geometry.hpp>
#endif

#include <pareto/common.h>

namespace pareto {

#ifdef BUILD_BOOST_TREE
using default_coordinate_system_for_points = boost::geometry::cs::cartesian;
#else
using default_coordinate_system_for_points = void;
#endif

    /// We need an special structure for point types because
    ///    the other point type was very limited for our purposes.
    /// The geometry point type is meant to be used internally
    ///    while this point type includes more conveniences for
    ///    the user.
    /// \tparam NUMBER_T
    /// \tparam DimensionCount
    /// \tparam CoordinateSystem

    template <typename NUMBER_T, std::size_t DimensionCount = 0, typename CoordinateSystem = default_coordinate_system_for_points>
    class point {
    public:
        using number_type = NUMBER_T;
        using distance_type = std::conditional_t<std::is_floating_point_v<number_type>, number_type, double>;
        static_assert((DimensionCount >= 0));
        using coordinate_system_t = CoordinateSystem;

        /// You can set the number of dimensions in compile time
        /// but you can also set the number of dimension in runtime.
        /// We need both options to support all kinds of pareto sets.
        /// In the first case, we use an array as data structure.
        /// In the second case, we use a vector as data structure.
        static constexpr size_t compile_dimensions = DimensionCount;

        using array_type = std::conditional_t<
                compile_dimensions == 0, std::vector<number_type>, std::array<number_type, compile_dimensions>>;

    public:
        /// \brief Default constructor
        point() {
            std::fill(values_.begin(), values_.end(), number_type());
        };

        explicit point(size_t n) {
            maybe_resize(values_,n);
            assert(values_.size() == n);
        }

        explicit point(size_t n, const number_type value) {
            maybe_resize(values_,n);
            std::fill(values_.begin(), values_.end(), value);
            assert(values_.size() == n);
        }

        point(std::initializer_list<number_type> il) {
            maybe_resize(values_,il.size());
            std::copy(il.begin(), il.end(), values_.begin());
        }

        template <typename... Targs>
        point(const number_type& k, const Targs&... ks) {
            constexpr size_t pack_dimension = sizeof...(Targs) + 1;
            maybe_resize(values_, pack_dimension);
            copy_pack(0, k, ks...);
        }

        point(const point& x) {
            values_ = x.values_;
        }

        point(point&& x) noexcept {
            values_ = std::move(x.values_);
        }

        /// \brief Constructor to set values from any other container
        template<class Rng>
        explicit point(const Rng &il) {
            maybe_resize(values_, il.size());
            std::copy(il.begin(), il.end(), values_.begin());
        }

        /// \brief Constructor to set values from any other container
        template<size_t DimensionCount2>
        explicit point(const point<NUMBER_T, DimensionCount2, CoordinateSystem>& p2)
            : point(p2.begin(), p2.end())
        {}

        /// \brief Constructor to set values from any other container
        template<class Iterator, std::enable_if_t<!std::is_fundamental_v<Iterator>,int> = 0>
        point(const Iterator& begin, const Iterator& end) {
            maybe_resize(values_, std::distance(begin,end));
            std::copy(begin, end, values_.begin());
        }

        point& operator=(const point& x) {
            values_ = x.values_;
            return *this;
        }

        point& operator=(point&& x) {
            values_ = std::move(x.values_);
            return *this;
        }

        /// \brief Get a coordinate (rtrees need this templated version)
        /// \tparam K coordinate to get
        /// \return the coordinate
        template<std::size_t K>
        inline number_type const &get() const {
            static_assert(K < compile_dimensions || compile_dimensions == 0);
            return values_[K];
        }

        /// \brief Set a coordinate (rtrees need this templated version)
        /// \tparam K coordinate to set
        /// \param value value to set
        template<std::size_t K>
        inline void set(number_type const &value) {
            static_assert(K < compile_dimensions);
            values_[K] = value;
        }

        size_t dimensions() const {
            return values_.size();
        }

        size_t size() const {
            return values_.size();
        }

        /// \brief This is often simply referred to as Pareto dominance.
        /// A solution x weakly dominates a solution x∗ (x ≽ x∗)
        /// if x is better than x∗ in at least one objective and
        /// is as good as x∗ in all other objectives.
        /// Some other works distinguish between weak dominance
        /// and simple dominance, where weak dominance accepts ties
        template <class Rng>
        bool dominates(const point& p, const Rng& is_minimization) const {
            auto il = is_minimization.begin();
            auto pi = p.values_.begin();
            bool better_at_any = false;
            for (auto it = values_.begin(); it != values_.end(); it++) {
                if (*il ? *it > *pi : *it < *pi) {
                    return false;
                }
                if (!better_at_any) {
                    if (*il ? *it < *pi : *it > *pi) {
                        better_at_any = true;
                    }
                }
                ++pi;
                ++il;
            }
            return better_at_any;
        }

        bool dominates(const point& p, bool is_minimization) const {
            return dominates(p, std::vector<uint8_t>(dimensions(), is_minimization));
        }

        bool dominates(const point& p) const {
            return dominates(p, true);
        }

        /// A solution x strongly dominates a solution x∗ (x ≻ x∗)
        /// if x is strictly better than x∗ in all objectives.
        template <class Rng>
        bool strongly_dominates(const point& p, const Rng& is_minimization) const {
            auto il = is_minimization.begin();
            auto pi = p.values_.begin();
            for (auto it = values_.begin(); it != values_.end(); it++) {
                if (*il ? *it >= *pi : *it <= *pi) {
                    return false;
                }
                ++pi;
                ++il;
            }
            return true;
        }

        bool strongly_dominates(const point& p, bool is_minimization) const {
            return strongly_dominates(p, std::vector<uint8_t>(dimensions(),is_minimization));
        }

        bool strongly_dominates(const point& p) const {
            return strongly_dominates(p, true);
        }

        /// If neither x dominates x∗ nor x∗ dominates x (weakly or strongly),
        /// then both solutions are said to be incomparable or mutually
        /// non-dominated. In this case, no solution is clearly preferred
        /// over the other. Note that this includes solutions that are equal.
        template <class Rng>
        bool non_dominates(const point& p, const Rng& is_minimization) const {
            return !dominates(p,is_minimization) && !p.dominates(p,is_minimization);
        }

        bool non_dominates(const point& p, bool is_minimization) const {
            return !dominates(p,is_minimization) && !p.dominates(p,is_minimization);
        }

        bool non_dominates(const point& p) const {
            return !dominates(p) && !p.dominates(*this);
        }

        template <std::size_t DimensionCount2>
        distance_type distance(const point<NUMBER_T, DimensionCount2, CoordinateSystem>& p2) const {
            distance_type dist = 0.;
            for (size_t i = 0; i < dimensions(); ++i) {
                dist += pow(operator[](i) - p2[i], 2);
            }
            return sqrt(dist);
        }

        distance_type distance(const point& p2) const {
            if constexpr (DimensionCount == 1) {
                return operator[](0) > p2[0] ? operator[](0) - p2[0] : p2[0] - operator[](0);
            } else {
                distance_type dist = 0.;
                for (size_t i = 0; i < dimensions(); ++i) {
                    dist += pow(operator[](i) - p2[i], 2);
                }
                return sqrt(dist);
            }
        }

        /// Distance from p2 to the hyperbox with the region dominated by this point
        /// \param p2
        /// \return
        template <class Rng>
        distance_type distance_to_dominated_box(const point& p2, const Rng& is_minimization) const {
            double sum = 0.0;
            auto is_mini_begin = is_minimization.begin();
            for (size_t i = 0; i < dimensions(); ++i) {
                auto term = *is_mini_begin ? operator[](i) - p2[i] : p2[i] - operator[](i);
                auto modified_term = std::max(number_type(0), term);
                auto pow_term = pow(modified_term, 2.0);
                sum += pow_term;
//                sum += pow(static_cast<double>(std::max(number_type(0), *is_mini_begin ? operator[](i) - p2[i] : p2[i] - operator[](i))), 2.);
                ++is_mini_begin;
            }
            return sqrt(sum);
        }

        typename array_type::const_iterator begin() const {
            return values_.begin();
        }

        typename array_type::const_iterator end() const {
            return values_.end();
        }

        typename array_type::iterator begin() {
            return values_.begin();
        }

        typename array_type::iterator end() {
            return values_.end();
        }

        bool operator<(const point &rhs) const {
            return values_ < rhs.values_;
        }

        bool operator>(const point &rhs) const {
            return rhs < *this;
        }

        bool operator<=(const point &rhs) const {
            return !(rhs < *this);
        }

        bool operator>=(const point &rhs) const {
            return !(*this < rhs);
        }

        bool operator==(const point &rhs) const {
            return values_ == rhs.values_;
        }

        bool operator!=(const point &rhs) const {
            return !(rhs == *this);
        }

        point& operator+=(point const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] += y.values_[i];
            }
            return *this;
        }

        point operator+ ( point const& y ) const {
            point c = *this;
            c += y;
            return c;
        }


        point& operator-=(point const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] -= y.values_[i];
            }
            return *this;
        }

        point operator-( point const& y ) const {
            point c = *this;
            c -= y;
            return c;
        }

        point& operator*=(point const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] *= y.values_[i];
            }
            return *this;
        }

        point operator* ( point const& y ) const {
            point c = *this;
            c *= y;
            return c;
        }


        point& operator/=(point const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] /= y.values_[i];
            }
            return *this;
        }

        point operator/ ( point const& y ) const {
            point c = *this;
            c /= y;
            return c;
        }

        point& operator+=(number_type const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] += y;
            }
            return *this;
        }

        point operator+ ( number_type const& y ) const {
            point c = *this;
            c += y;
            return c;
        }


        point& operator-=(number_type const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] -= y;
            }
            return *this;
        }

        point operator- ( number_type const& y ) const {
            point c = *this;
            c -= y;
            return c;
        }


        point& operator*=(number_type const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] *= y;
            }
            return *this;
        }

        point operator* ( number_type const& y ) const {
            point c = *this;
            c *= y;
            return c;
        }


        point& operator/=(number_type const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] /= y;
            }
            return *this;
        }

        point operator/ ( number_type const& y ) const {
            point c = *this;
            c /= y;
            return c;
        }

        void clear() {
            maybe_clear(values_);
        }

        void push_back(const number_type& v) {
            maybe_push_back(values_, v);
        }

        void push_back(number_type&& v) {
            maybe_push_back(values_, std::move(v));
        }

        number_type& operator[](size_t n){
            return values_[n];
        }

        const number_type& operator[](size_t n) const {
            return values_[n];
        }

        /// \brief Quadrant p is in relative to the this point
        /// Given an input point p, returns an int specifying which quadrant
        /// p is in relative to the this point. Bit #k of the returned int is 1
        /// if p is below this point along dimension k.
        /// \param p Point for which we want to know the quadrant
        /// \return Quadrant
        size_t quadrant( const point& p ) const {
            size_t quad = 0;
            for (size_t i = 0; i < dimensions(); i++) {
                quad += p[i] <= values_[i] ? 1u << i : 0;
            }
            return quad;
        }

        friend std::ostream &operator<<(std::ostream &os, const point &point) {
            if (point.dimensions() == 0) {
                return os << "( )";
            }
            os << "(" << point.values_[0];
            for (size_t i = 1; i < point.values_.size(); ++i) {
                os << ", " << point.values_[i];
            }
            return os << ")";
        }

    private /* helpers */:
        template <typename... Targs>
        inline void copy_pack(size_t i, const number_type& k, const Targs&... ks) {
            values_[i] = k;
            copy_pack(i + 1, ks...);
        }

        template <typename... Targs>
        inline void copy_pack(size_t i, const number_type& k) {
            values_[i] = k;
        }

    private:
        array_type values_;

    };


    template <typename NUMBER_T, std::size_t DimensionCount, typename CoordinateSystem>
    void normalize_corners(point<NUMBER_T, DimensionCount, CoordinateSystem>& min_corner, point<NUMBER_T, DimensionCount, CoordinateSystem>& max_corner) {
        for (size_t i = 0; i < min_corner.dimensions(); ++i) {
            if (min_corner[i] > max_corner[i]) {
                std::swap(min_corner[i], max_corner[i]);
            }
        }
    }

}

#ifdef BUILD_BOOST_TREE
/// Define traits for boost geometry to understand out point type
/// This is for our preliminary experiments comparing our r-tree with boost r-tree
namespace boost {
    namespace geometry {
        namespace traits {
            template <typename CoordinateType, std::size_t DimensionCount, typename CoordinateSystem>
            struct tag<pareto::point<CoordinateType, DimensionCount, CoordinateSystem> > {
                typedef point_tag type;
            };

            template < typename CoordinateType, std::size_t DimensionCount, typename CoordinateSystem >
            struct coordinate_type<pareto::point<CoordinateType, DimensionCount, CoordinateSystem> > {
                typedef CoordinateType type;
            };

            template < typename CoordinateType, std::size_t DimensionCount, typename CoordinateSystem >
            struct coordinate_system<pareto::point<CoordinateType, DimensionCount, CoordinateSystem> > {
                typedef CoordinateSystem type;
            };

            template < typename CoordinateType, std::size_t DimensionCount, typename CoordinateSystem >
            struct dimension<pareto::point<CoordinateType, DimensionCount, CoordinateSystem> >
                    : boost::mpl::int_<DimensionCount> {
            };

            template < typename CoordinateType, std::size_t DimensionCount, typename CoordinateSystem, std::size_t Dimension >
            struct access<pareto::point<CoordinateType, DimensionCount, CoordinateSystem>, Dimension> {
                static inline CoordinateType get( pareto::point<CoordinateType, DimensionCount, CoordinateSystem> const &p) {
                    return p.template get<Dimension>();
                }

                static inline void set( pareto::point<CoordinateType, DimensionCount, CoordinateSystem> &p, CoordinateType const &value) {
                    p.template set<Dimension>(value);
                }
            };

        }
    }
}
#endif

#endif //PARETO_FRONT_POINT_H
