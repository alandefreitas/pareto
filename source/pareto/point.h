//
// Created by Alan Freitas on 2020-05-20.
//

#ifndef PARETO_FRONT_POINT_H
#define PARETO_FRONT_POINT_H

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <numeric>
#include <ostream>
#include <utility>
#include <vector>

#ifdef BUILD_BOOST_TREE
#include <boost/geometry/geometry.hpp>
#endif

#include <pareto/common/common.h>

namespace pareto {

#ifdef BUILD_BOOST_TREE
    using default_coordinate_system_for_points = boost::geometry::cs::cartesian;
#else
    using default_coordinate_system_for_points = void;
#endif

    /// \class Data point
    /// We need an special structure for point types because
    ///    * Other point types are very limited for our purposes.
    ///    * The boost geometry point type is meant to be used internally
    ///    * We need conveniences for calculating dominance relationships
    ///    * Dimension can be set in compile-time or runtime
    /// Only one type of coordinate is allowed because we need the same
    /// number type on all coordinates to later calculate indicators
    /// If your objective is an integer, you can promote it to a double
    /// in the front
    /// \tparam T Number type for points
    /// \tparam M Number of dimensions (zero for runtime)
    /// \tparam CoordinateSystem This is important for boost only (to be
    /// deprecated)
    template <typename T, std::size_t M = 0,
              typename CoordinateSystem = default_coordinate_system_for_points>
    class point {
      public:
        using dimension_type = T;
        using distance_type =
            std::conditional_t<std::is_floating_point_v<dimension_type>,
                               dimension_type, double>;
        static_assert((M >= 0));
        using coordinate_system_t = CoordinateSystem;

        /// You can set the number of dimensions in compile time
        /// but you can also set the number of dimension in runtime.
        /// We need both options to support all kinds of pareto sets.
        /// In the first case, we use an array as data structure.
        /// In the second case, we use a vector as data structure.
        static constexpr size_t compile_dimensions = M;

        using array_type =
            std::conditional_t<compile_dimensions == 0,
                               std::vector<dimension_type>,
                               std::array<dimension_type, compile_dimensions>>;

      public:
        /// \brief Default constructor
        /// Fill values with number type
        /// Not useful if you want runtime dimensions
        point() {
            std::fill(values_.begin(), values_.end(), dimension_type());
        };

        /// \brief Construct with size n
        /// Has no effect if dimension is set at compile-time
        explicit point(size_t n) {
            maybe_resize(values_, n);
            std::fill(values_.begin(), values_.end(), dimension_type{0});
            assert(values_.size() == n);
        }

        /// \brief Construct with size n
        /// Has no effect if dimension is set at compile-time
        explicit point(size_t n, const dimension_type value) {
            maybe_resize(values_, n);
            std::fill(values_.begin(), values_.end(), value);
            assert(values_.size() == n);
        }

        /// \brief Copy constructor
        point(const point &rhs)
                : values_(rhs.values_) {}

        /// \brief Move constructor
        point(point &&rhs) noexcept
                : values_(std::move(rhs.values_)) {}

        /// \brief Constructor to set values from any other container
        point(std::initializer_list<dimension_type> il) {
            maybe_resize(values_, static_cast<size_t>(il.size()));
            if (il.size() == values_.size()) {
                std::copy(il.begin(), il.end(), values_.begin());
            } else if (il.size() == 1) {
                std::fill(values_.begin(), values_.end(), *il.begin());
            } else {
                throw std::logic_error("point:: Initializer list and expected "
                                       "point dimensions don't match");
            }
        }

        /// \brief Constructor to set values from any other container
        template <size_t M2>
        explicit point(const point<T, M2, CoordinateSystem> &p2)
            : point(p2.begin(), p2.end()) {}

        /// \brief Constructor to set values from any other container
        template<class Iterator, std::enable_if_t<!std::is_fundamental_v<Iterator>, int> = 0>
        point(const Iterator &begin, const Iterator &end) {
            maybe_resize(values_, std::distance(begin, end));
            std::copy(begin, end, values_.begin());
        }

        /// \brief Attribution operator
        point &operator=(const point &x) {
            values_ = x.values_;
            return *this;
        }

        /// \brief Attribution move operator
        point &operator=(point &&x) noexcept {
            values_ = std::move(x.values_);
            return *this;
        }

        /// \brief Get a coordinate
        /// boost rtrees need this templated version
        /// Other containers should probably use operator[]
        /// \tparam K coordinate to get
        /// \return the coordinate
        template <std::size_t K> inline dimension_type const &get() const {
            static_assert(K < compile_dimensions || compile_dimensions == 0);
            return values_[K];
        }

        /// \brief Set a coordinate
        /// Boost rtrees need this templated version
        /// Other containers should probably use operator[]
        /// \tparam K coordinate to set
        /// \param value value to set
        template <std::size_t K> inline void set(dimension_type const &value) {
            static_assert(K < compile_dimensions);
            values_[K] = value;
        }

        /// \brief Return number of dimensions / array size
        [[nodiscard]] size_t dimensions() const {
            return values_.size();
        }

        /// \brief Return number of dimensions / array size
        [[nodiscard]] size_t size() const {
            return values_.size();
        }

        /// \brief Check for weak Pareto dominance
        /// This is often simply referred to as Pareto dominance.
        /// A solution x weakly dominates a solution x∗ (x < x∗)
        /// if x is better than x∗ in at least one objective and
        /// is as good as x∗ in all other objectives.
        /// \note Some other works distinguish between weak dominance
        /// and simple dominance, where weak dominance accepts ties.
        /// We don't do that here. Use operator== to check for ties.
        /// \tparam Rng Range telling whether each dimension is minimization
        /// \param p Point being compared
        /// \param is_minimization Range with the optimization direction of each component
        /// \return True if this point dominates p
        template<class Rng>
        bool dominates(const point &p, const Rng &is_minimization) const {
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

        /// \brief Check for weak dominance
        bool dominates(const point &p, bool is_minimization) const {
            return dominates(p, std::vector<uint8_t>(dimensions(), is_minimization));
        }

        /// \brief Check for weak dominance
        bool dominates(const point &p) const {
            return dominates(p, true);
        }

        /// \brief Check for strong dominance
        /// A solution x strongly dominates a solution x∗ (x << x∗)
        /// if x is strictly better than x∗ in all objectives.
        template<class Rng>
        bool strongly_dominates(const point &p, const Rng &is_minimization) const {
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

        /// \brief Check for strong dominance
        bool strongly_dominates(const point &p, bool is_minimization) const {
            return strongly_dominates(p, std::vector<uint8_t>(dimensions(), is_minimization));
        }

        /// \brief Check for strong dominance
        bool strongly_dominates(const point &p) const {
            return strongly_dominates(p, true);
        }

        /// \brief Check for non-dominance
        /// If neither x dominates x∗ nor x∗ dominates x (weakly or strongly),
        /// then both solutions are said to be incomparable or mutually
        /// non-dominated. In this case, no solution is clearly preferred
        /// over the other. Note that this includes solutions that are equal.
        template<class Rng>
        bool non_dominates(const point &p, const Rng &is_minimization) const {
            return !dominates(p, is_minimization) && !p.dominates(p, is_minimization);
        }

        /// \brief Check for non-dominance
        bool non_dominates(const point &p, bool is_minimization) const {
            return !dominates(p, is_minimization) && !p.dominates(p, is_minimization);
        }

        /// \brief Check for non-dominance
        bool non_dominates(const point &p) const {
            return !dominates(p) && !p.dominates(*this);
        }

        /// \brief Calculates the distance between two points (another template
        /// type)
        template <std::size_t M2>
        distance_type distance(const point<T, M2, CoordinateSystem> &p2) const {
            distance_type dist = 0.;
            for (size_t i = 0; i < dimensions(); ++i) {
                dist += pow(operator[](i) - p2[i], 2);
            }
            return sqrt(dist);
        }

        /// \brief Calculates the distance between two points (same template type)
        distance_type distance(const point &p2) const {
            if constexpr (M == 1) {
                return operator[](0) > p2[0] ? operator[](0) - p2[0]
                                             : p2[0] - operator[](0);
            } else {
                distance_type dist = 0.;
                for (size_t i = 0; i < dimensions(); ++i) {
                    dist += pow(operator[](i) - p2[i], 2);
                }
                return sqrt(dist);
            }
        }

        /// \brief Calculates the distance between a point and a box from point p2
        /// Distance from p2 to the hyperbox with the region dominated by this point
        /// \param p2
        /// \return
        template<class Rng>
        distance_type distance_to_dominated_box(const point &p2, const Rng &is_minimization) const {
            double sum = 0.0;
            auto is_mini_begin = is_minimization.begin();
            for (size_t i = 0; i < dimensions(); ++i) {
                auto term = *is_mini_begin ? operator[](i) - p2[i]
                                           : p2[i] - operator[](i);
                auto modified_term = std::max(dimension_type(0), term);
                auto pow_term = pow(modified_term, 2.0);
                sum += pow_term;
                ++is_mini_begin;
            }
            return sqrt(sum);
        }

        array_type &values() { return values_; }

        const array_type &values() const { return values_; }

        /// \brief Iterator to first point component
        typename array_type::const_iterator begin() const {
            return values_.begin();
        }

        /// \brief Iterator to past-the-end point component
        typename array_type::const_iterator end() const {
            return values_.end();
        }

        /// \brief Iterator to first point component
        typename array_type::iterator begin() {
            return values_.begin();
        }

        /// \brief Iterator to past-the-end point component
        typename array_type::iterator end() { return values_.end(); }

        /// \brief operator< : this dominates rhs
        /// This allows some conveniences but you're recommended to
        /// use the "dominates", "strongly_dominates", and "non_dominates"
        /// functions instead
        /// bool operator<(const point &rhs) const {
        ///    return this->dominates(rhs);
        /// }

        /// \brief operator> : rhs dominates this
        /// This allows some conveniences but you're recommended to
        /// use the "dominates", "strongly_dominates", and "non_dominates"
        /// functions instead
        /// bool operator>(const point &rhs) const {
        ///    return rhs.dominates(*this);
        /// }

        /// \brief operator< : this dominates rhs or is at least non-dominated
        /// This allows some conveniences but you're recommended to
        /// use the "dominates", "strongly_dominates", and "non_dominates"
        /// functions instead
        /// bool operator<=(const point &rhs) const {
        ///    return this->dominates(rhs) || !rhs.dominates(*this);
        /// }

        /// \brief operator< : this dominates rhs or is at least non-dominated
        /// This allows some conveniences but you're recommended to
        /// use the "dominates", "strongly_dominates", and "non_dominates"
        /// functions instead
        /// bool operator>=(const point &rhs) const {
        ///    return rhs.dominates(*this) || !this->non_dominates(rhs);
        /// }

        /// \brief operator< : this dominates rhs or is at least non-dominated
        /// This allows some conveniences but you're recommended to
        /// use the "dominates", "strongly_dominates", and "non_dominates"
        /// functions instead
        bool operator==(const point &rhs) const {
            return values_ == rhs.values_;
        }

        /// \brief operator< : this dominates rhs or is at least non-dominated
        /// This allows some conveniences but you're recommended to
        /// use the "dominates", "strongly_dominates", and "non_dominates"
        /// functions instead
        bool operator!=(const point &rhs) const {
            return values_ != rhs.values_;
        }

        /// \brief Point addition
        point &operator+=(point const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] += y.values_[i];
            }
            return *this;
        }

        /// \brief Point addition
        point operator+(point const &y) const {
            point c = *this;
            c += y;
            return c;
        }

        /// \brief Point subtraction
        point &operator-=(point const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] -= y.values_[i];
            }
            return *this;
        }

        /// \brief Point subtraction
        point operator-(point const &y) const {
            point c = *this;
            c -= y;
            return c;
        }

        /// \brief Point multiplication
        point &operator*=(point const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] *= y.values_[i];
            }
            return *this;
        }

        /// \brief Point multiplication
        point operator*(point const &y) const {
            point c = *this;
            c *= y;
            return c;
        }

        /// \brief Point division
        point &operator/=(point const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] /= y.values_[i];
            }
            return *this;
        }

        /// \brief Point division
        point operator/(point const &y) const {
            point c = *this;
            c /= y;
            return c;
        }

        /// \brief Point addition
        point &operator+=(dimension_type const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] += y;
            }
            return *this;
        }

        /// \brief Point addition
        point operator+(dimension_type const &y) const {
            point c = *this;
            c += y;
            return c;
        }

        /// \brief Point subtraction
        point &operator-=(dimension_type const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] -= y;
            }
            return *this;
        }

        /// \brief Point subtraction
        point operator-(dimension_type const &y) const {
            point c = *this;
            c -= y;
            return c;
        }

        /// \brief Point multiplication
        point &operator*=(dimension_type const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] *= y;
            }
            return *this;
        }

        /// \brief Point multiplication
        point operator*(dimension_type const &y) const {
            point c = *this;
            c *= y;
            return c;
        }

        /// \brief Point division
        point &operator/=(dimension_type const &y) {
            for (size_t i = 0; i < dimensions(); ++i) {
                values_[i] /= y;
            }
            return *this;
        }

        /// \brief Point division
        point operator/(dimension_type const &y) const {
            point c = *this;
            c /= y;
            return c;
        }

        /// \brief Clear the values (operate on container)
        void clear() {
            maybe_clear(values_);
        }

        /// \brief Push value (operate on container)
        void push_back(const dimension_type &v) { maybe_push_back(values_, v); }

        /// \brief Push value (operate on container)
        void push_back(dimension_type &&v) {
            maybe_push_back(values_, std::move(v));
        }

        /// \brief Access point component
        dimension_type &operator[](size_t n) { return values_[n]; }

        /// \brief Access point component
        const dimension_type &operator[](size_t n) const { return values_[n]; }

        /// \brief Relative to the this point, return on which quadrant the
        /// point p is Given an input point p, returns an integer specifying in
        /// which quadrant p is, relative to the this point.
        ///
        /// Bit #k of the returned int is 1 if p is below this point along
        /// dimension k. This attributes an index to each of the 2^m quadrants
        /// around this point. This is useful for quadtrees, that depend on this
        /// index to work.
        ///
        /// \param p Point for which we want to know the quadrant
        /// \return Quadrant
        template <class F> size_t quadrant(const point &p, F comp) const {
            size_t quad = 0;
            for (size_t i = 0; i < dimensions(); i++) {
                quad += comp(p[i], values_[i]) ? 1u << i : 0;
            }
            return quad;
        }

        /// \brief Stream point
        friend std::ostream &operator<<(std::ostream &os, const point &point) {
            if (point.dimensions() == 0) {
                return os << "[ ]";
            }
            os << "[" << point.values_[0];
            for (size_t i = 1; i < point.values_.size(); ++i) {
                os << ", " << point.values_[i];
            }
            return os << "]";
        }

    private:

        /// \brief Underlying data structure holding the point components
        /// This might be an array or a vector, depending on whether the point
        /// dimension was set at compile time
        array_type values_;

    };

}

#ifdef BUILD_BOOST_TREE
/// Define traits for boost geometry to understand out point type
/// This is for our preliminary experiments comparing our r-containers with
/// boost r-containers
namespace boost {
    namespace geometry {
        namespace traits {
            template <typename CoordinateType, std::size_t M,
                      typename CoordinateSystem>
            struct tag<pareto::point<CoordinateType, M, CoordinateSystem>> {
                typedef point_tag type;
            };

            template <typename CoordinateType, std::size_t M,
                      typename CoordinateSystem>
            struct coordinate_type<
                pareto::point<CoordinateType, M, CoordinateSystem>> {
                typedef CoordinateType type;
            };

            template <typename CoordinateType, std::size_t M,
                      typename CoordinateSystem>
            struct coordinate_system<
                pareto::point<CoordinateType, M, CoordinateSystem>> {
                typedef CoordinateSystem type;
            };

            template <typename CoordinateType, std::size_t M,
                      typename CoordinateSystem>
            struct dimension<pareto::point<CoordinateType, M, CoordinateSystem>>
                : boost::mpl::int_<M> {};

            template <typename CoordinateType, std::size_t M,
                      typename CoordinateSystem, std::size_t Dimension>
            struct access<pareto::point<CoordinateType, M, CoordinateSystem>,
                          Dimension> {
                static inline CoordinateType
                get(pareto::point<CoordinateType, M, CoordinateSystem> const
                        &p) {
                    return p.template get<Dimension>();
                }

                static inline void
                set(pareto::point<CoordinateType, M, CoordinateSystem> &p,
                    CoordinateType const &value) {
                    p.template set<Dimension>(value);
                }
            };

        } // namespace traits
    }
}
#endif

#endif //PARETO_FRONT_POINT_H
