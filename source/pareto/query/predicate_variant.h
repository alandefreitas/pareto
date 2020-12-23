//
// Created by Alan Freitas on 12/1/20.
//

#ifndef PARETO_PREDICATE_VARIANT_H
#define PARETO_PREDICATE_VARIANT_H

#include <pareto/point.h>
#include <pareto/query/query_box.h>
#include <pareto/query/disjoint.h>
#include <pareto/query/intersects.h>
#include <pareto/query/nearest.h>
#include <pareto/query/satisfies.h>
#include <pareto/query/within.h>

namespace pareto {

    /// \class Predicate variant
    /// This is a variant object that can keep any type of predicate
    /// In practice, that is the kind of predicate iterators and
    /// predicate lists hold so that it can achieve the exact same
    /// features with a common interface.
    template<typename NUMBER_T, std::size_t DimensionCount, class ELEMENT_TYPE>
    class predicate_variant {
        using dimension_type = NUMBER_T;
        static constexpr size_t number_of_compile_dimensions = DimensionCount;
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
        using key_type = point_type;
        using mapped_type = ELEMENT_TYPE;
        using value_type = std::pair<key_type, mapped_type>;
        using variant_type = std::variant<
            intersects<dimension_type, number_of_compile_dimensions>,
            disjoint<dimension_type, number_of_compile_dimensions>,
            within<dimension_type, number_of_compile_dimensions>,
            nearest<dimension_type, number_of_compile_dimensions>,
            satisfies<dimension_type, number_of_compile_dimensions,
                      mapped_type>>;

        variant_type predicate_;

      public /* constructors */:
        // NOLINTNEXTLINE(google-explicit-constructor): We need implicit
        // constructors to allow the syntax of initializer lists with predicate
        // variants
        predicate_variant(const variant_type &predicate)
            : predicate_(predicate) {}

        // NOLINTNEXTLINE(google-explicit-constructor): We need implicit
        // constructors to allow the syntax of initializer lists with predicate
        // variants
        predicate_variant(
            const intersects<dimension_type, number_of_compile_dimensions>
                &predicate)
            : predicate_(predicate) {}

        // NOLINTNEXTLINE(google-explicit-constructor): We need implicit constructors to allow the syntax of initializer lists with predicate variants
        predicate_variant(
            const disjoint<dimension_type, number_of_compile_dimensions>
                &predicate)
            : predicate_(predicate) {}

        // NOLINTNEXTLINE(google-explicit-constructor): We need implicit constructors to allow the syntax of initializer lists with predicate variants
        predicate_variant(const within<dimension_type,
                                       number_of_compile_dimensions> &predicate)
            : predicate_(predicate) {}

        // NOLINTNEXTLINE(google-explicit-constructor): We need implicit constructors to allow the syntax of initializer lists with predicate variants
        predicate_variant(
            const nearest<dimension_type, number_of_compile_dimensions>
                &predicate)
            : predicate_(predicate) {}

        // NOLINTNEXTLINE(google-explicit-constructor): We need implicit constructors to allow the syntax of initializer lists with predicate variants
        predicate_variant(
            const satisfies<dimension_type, number_of_compile_dimensions,
                            mapped_type> &predicate)
            : predicate_(predicate) {}

    public:
        /// \brief Check if predicate is of type intersects
        [[nodiscard]] bool is_intersects() const {
            return std::holds_alternative<
                intersects<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Check if predicate is of type disjoint
        [[nodiscard]] bool is_disjoint() const {
            return std::holds_alternative<
                disjoint<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Check if predicate is of type within
        [[nodiscard]] bool is_within() const {
            return std::holds_alternative<
                within<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Check if predicate is of type nearest
        [[nodiscard]] bool is_nearest() const {
            return std::holds_alternative<
                nearest<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Check if predicate is of type satisfies
        [[nodiscard]] bool is_satisfies() const {
            return std::holds_alternative<satisfies<
                dimension_type, number_of_compile_dimensions, mapped_type>>(
                predicate_);
        }

        /// \brief Get predicate as predicate of type intersects
        /// This will throw an error if predicate is not of type intersects
        /// If not sure, use the function is_intersects() before
        const intersects<dimension_type, number_of_compile_dimensions> &
        as_intersects() const {
            return std::get<
                intersects<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Get predicate as predicate of type disjoint
        /// This will throw an error if predicate is not of type disjoint
        /// If not sure, use the function is_disjoint() before
        const disjoint<dimension_type, number_of_compile_dimensions> &
        as_disjoint() const {
            return std::get<
                disjoint<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Get predicate as predicate of type within
        /// This will throw an error if predicate is not of type within
        /// If not sure, use the function is_within() before
        const within<dimension_type, number_of_compile_dimensions> &
        as_within() const {
            return std::get<
                within<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Get predicate as predicate of type nearest
        /// This will throw an error if predicate is not of type nearest
        /// If not sure, use the function is_nearest() before
        const nearest<dimension_type, number_of_compile_dimensions> &
        as_nearest() const {
            return std::get<
                nearest<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Get predicate as predicate of type satisfies
        /// This will throw an error if predicate is not of type satisfies
        /// If not sure, use the function is_satisfies() before
        const satisfies<dimension_type, number_of_compile_dimensions,
                        mapped_type> &
        as_satisfies() const {
            return std::get<satisfies<
                dimension_type, number_of_compile_dimensions, mapped_type>>(
                predicate_);
        }

        /// \brief Get predicate as predicate of type intersects
        /// This will throw an error if predicate is not of type intersects
        /// If not sure, use the function is_intersects() before
        intersects<dimension_type, number_of_compile_dimensions> &
        as_intersects() {
            return std::get<
                intersects<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Get predicate as predicate of type disjoint
        /// This will throw an error if predicate is not of type disjoint
        /// If not sure, use the function is_disjoint() before
        disjoint<dimension_type, number_of_compile_dimensions> &as_disjoint() {
            return std::get<
                disjoint<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Get predicate as predicate of type within
        /// This will throw an error if predicate is not of type within
        /// If not sure, use the function is_within() before
        within<dimension_type, number_of_compile_dimensions> &as_within() {
            return std::get<
                within<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Get predicate as predicate of type nearest
        /// This will throw an error if predicate is not of type nearest
        /// If not sure, use the function is_nearest() before
        nearest<dimension_type, number_of_compile_dimensions> &as_nearest() {
            return std::get<
                nearest<dimension_type, number_of_compile_dimensions>>(
                predicate_);
        }

        /// \brief Get predicate as predicate of type satisfies
        /// This will throw an error if predicate is not of type satisfies
        /// If not sure, use the function is_satisfies() before
        satisfies<dimension_type, number_of_compile_dimensions, mapped_type> &
        as_satisfies() {
            return std::get<satisfies<
                dimension_type, number_of_compile_dimensions, mapped_type>>(
                predicate_);
        }

        /// \brief Index of the current variant type
        /// This is useful to check if two predicates are of the same type
        [[nodiscard]] size_t index() const {
            return predicate_.index();
        }

        /// \brief Does the box pass the predicate?
        /// We use the underlying predicate function to check that
        /// The underlying predicate uses the querybox functions to check that
        bool pass_predicate(const query_box_type &rhs) const {
            return std::visit([&rhs](const auto &predicate) { return predicate.pass_predicate(rhs); }, predicate_);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the underlying predicate function to check that
        /// The underlying predicate uses the querybox functions to check that
        bool might_pass_predicate(const query_box_type &rhs) const {
            return std::visit([&rhs](const auto &predicate) { return predicate.might_pass_predicate(rhs); },
                              predicate_);
        }

        /// \brief Does the point pass the predicate?
        /// We use the underlying predicate function to check that
        /// The underlying predicate uses the querybox functions to check that
        bool pass_predicate(const point_type &rhs) const {
            return std::visit([&rhs](const auto &predicate) { return predicate.pass_predicate(rhs); }, predicate_);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the underlying predicate function to check that
        /// The underlying predicate uses the querybox functions to check that
        bool might_pass_predicate(const point_type &rhs) const {
            return std::visit([&rhs](const auto &predicate) { return predicate.might_pass_predicate(rhs); },
                              predicate_);
        }

        /// \brief Does the value pass the predicate?
        /// We use the underlying predicate function to check that
        /// The underlying predicate uses the querybox functions to check that
        bool pass_predicate(const value_type &rhs) const {
            return std::visit([&rhs](const auto &predicate) { return predicate.pass_predicate(rhs); }, predicate_);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the underlying predicate function to check that
        /// The underlying predicate uses the querybox functions to check that
        bool might_pass_predicate(const value_type &rhs) const {
            return std::visit([&rhs](const auto &predicate) { return predicate.might_pass_predicate(rhs); },
                              predicate_);
        }

        /// \brief Return which predicate is more restrictive
        /// This is useful to sort predicates in predicate lists
        /// Predicate lists should always have the most restrictive
        /// predicates first.
        /// Because we can only infer how restrictive the disjoint predicate is
        /// if we know the total volume, we need the total front volume to make
        /// this comparison.
        /// \param other Other predicate
        /// \param total_volume Total volume of the containers we are
        /// manipulating \return If this predicate is more restrictive than the
        /// other
        bool is_more_restrictive(const predicate_variant &other,
                                 dimension_type total_volume) const {
            // "satisfies" and "nearest" are the least restrictive
            // because any point can potentially pass the predicate
            // "satisfies" because the function is black-box to us
            // and "nearest" because it transforms how the iterator
            // works instead of using the predicate object
            if (is_satisfies() || is_nearest()) {
                return false;
            }
            if (other.is_satisfies() || other.is_nearest()) {
                return true;
            }

            // predicates now can only be intersect, within or disjoint
            dimension_type volume_a = dimension_type{0};
            dimension_type volume_b = dimension_type{0};
            if (is_intersects()) {
                volume_a = as_intersects().data().volume();
            } else if (is_within()) {
                volume_a = as_within().data().volume();
            } else {
                volume_a = total_volume - as_disjoint().data().volume();
            }

            if (other.is_intersects()) {
                volume_a = other.as_intersects().data().volume();
            } else if (other.is_within()) {
                volume_a = other.as_within().data().volume();
            } else {
                volume_a = total_volume - other.as_disjoint().data().volume();
            }
            return volume_a < volume_b;
        }

        /// \brief Check if two predicates are the same type and have the same parameters
        bool operator==(const predicate_variant &rhs) const {
            if (index() != rhs.index()) {
                return false;
            }
            if (is_intersects()) {
                if (as_intersects() != rhs.as_intersects()) {
                    return false;
                }
            }
            if (is_disjoint()) {
                if (as_disjoint() != rhs.as_disjoint()) {
                    return false;
                }
            }
            if (is_within()) {
                if (as_within() != rhs.as_within()) {
                    return false;
                }
            }
            if (is_nearest()) {
                if (as_nearest() != rhs.as_nearest()) {
                    return false;
                }
            }
            if (is_satisfies()) {
                if (as_satisfies() != rhs.as_satisfies()) {
                    return false;
                }
            }
            return true;
        }

        /// \brief Check if two predicates are of different types or have different parameters
        bool operator!=(const predicate_variant &rhs) const {
            return !(this->operator==(rhs));
        }
    };
}

#endif //PARETO_PREDICATE_VARIANT_H
