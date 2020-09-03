//
// Created by Alan Freitas on 2020-06-03.
//

#ifndef PARETO_FRONT_PREDICATES_H
#define PARETO_FRONT_PREDICATES_H

#include <variant>
#include <pareto_front/point.h>
#include <pareto_front/query_box.h>

namespace pareto_front {

    template <typename NUMBER_T, std::size_t DimensionCount>
    class intersects {
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
        query_box_type data_;
    public:
        intersects(const query_box_type &data) : data_(data) {}

        intersects(const point_type &min_corner, const point_type &max_corner) : data_(query_box_type(min_corner,max_corner)) {}

        const query_box_type &data() const {
            return data_;
        }

        void data(const query_box_type &data) {
            data_ = data;
        }

        /// Does the box pass the predicate?
        bool pass_predicate(const query_box_type& rhs) const {
            return data_.intersects(rhs);
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const query_box_type& rhs) const {
            return data_.overlap(rhs);
        }

        /// Does the point pass the predicate?
        bool pass_predicate(const point_type& rhs) const {
            return data_.intersects(rhs);
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const point_type& rhs) const {
            return data_.overlap(rhs);
        }

        /// Does the value pass the predicate?
        template <class mapped_type>
        bool pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return data_.intersects(rhs.first);
        }

        /// Can a child in this box pass the predicate?
        template <class mapped_type>
        bool might_pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return data_.overlap(rhs.first);
        }

        bool operator==(const intersects &rhs) const {
            return data_ == rhs.data_;
        }

        bool operator!=(const intersects &rhs) const {
            return !(rhs == *this);
        }
    };

    template <typename NUMBER_T, std::size_t DimensionCount>
    class disjoint {
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
        query_box_type data_;
    public:
        disjoint(const query_box_type &data) : data_(data) {}

        disjoint(const point_type &min_corner, const point_type &max_corner) : data_(query_box_type(min_corner,max_corner)) {}

        const query_box_type &data() const {
            return data_;
        }

        void data(const query_box_type &data) {
            data_ = data;
        }

        /// Does the box pass the predicate?
        bool pass_predicate(const query_box_type& rhs) const {
            return data_.disjoint(rhs);
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const query_box_type& rhs) const {
            return !data_.intersects(rhs);
        }

        /// Does the point pass the predicate?
        bool pass_predicate(const point_type& rhs) const {
            return data_.disjoint(rhs);
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const point_type& rhs) const {
            return !data_.intersects(rhs);
        }

        /// Does the value pass the predicate?
        template <class mapped_type>
        bool pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return data_.disjoint(rhs.first);
        }

        /// Can a child in this box pass the predicate?
        template <class mapped_type>
        bool might_pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return !data_.intersects(rhs.first);
        }

        bool operator==(const disjoint &rhs) const {
            return data_ == rhs.data_;
        }

        bool operator!=(const disjoint &rhs) const {
            return !(rhs == *this);
        }
    };

    template <typename NUMBER_T, std::size_t DimensionCount>
    class within {
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
        query_box_type data_;
    public:
        within(const query_box_type &data) : data_(data) {}

        within(const point_type &min_corner, const point_type &max_corner) : data_(query_box_type(min_corner,max_corner)) {}

        const query_box_type &data() const {
            return data_;
        }

        void data(const query_box_type &data) {
            data_ = data;
        }

        /// Does the box pass the predicate?
        bool pass_predicate(const query_box_type& rhs) const {
            return data_.within(rhs);
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const query_box_type& rhs) const {
            return data_.overlap(rhs);
        }

        /// Does the point pass the predicate?
        bool pass_predicate(const point_type& rhs) const {
            return data_.within(rhs);
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const point_type& rhs) const {
            return data_.overlap(rhs);
        }

        /// Does the value pass the predicate?
        template <class mapped_type>
        bool pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return data_.within(rhs.first);
        }

        /// Can a child in this box pass the predicate?
        template <class mapped_type>
        bool might_pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return data_.overlap(rhs.first);
        }

        bool operator==(const within &rhs) const {
            return data_ == rhs.data_;
        }

        bool operator!=(const within &rhs) const {
            return !(rhs == *this);
        }
    };

    template <typename NUMBER_T, std::size_t DimensionCount>
    class nearest {
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
        size_t k_;
        using object_variant = std::variant<point_type, query_box_type>;
        object_variant data_;
    public:
        nearest(const point_type &data, size_t k) : data_(data), k_(k) {}
        nearest(const point_type &data) : data_(object_variant(data)), k_(1) {}

        nearest(const query_box_type &data, size_t k) : data_(object_variant(data)), k_(k) {}
        nearest(const query_box_type &data) : data_(object_variant(data)), k_(1) {}

        size_t k() const {
            return k_;
        }

        void k(size_t k) {
            k_ = k;
        }

        const point_type &data() const {
            return data_;
        }

        void data(const point_type &data) {
            data_ = data;
        }

        const point_type &reference_point() const {
            return std::get<point_type>(data_);
        }

        void reference_point(const point_type &data) {
            data_ = data;
        }

        bool has_reference_point() const {
            return std::holds_alternative<point_type>(data_);
        }

        const query_box_type &reference_box() const {
            return std::get<query_box_type>(data_);
        }

        void reference_box(const query_box_type &data) {
            data_ = data;
        }

        bool has_reference_box() const {
            return std::holds_alternative<query_box_type>(data_);
        }

        double distance(const point_type& p) {
            if (has_reference_point()) {
                return reference_point().distance(p);
            } else {
                return reference_box().distance(p);
            }
        }

        double distance(const query_box_type& b) {
            if (has_reference_point()) {
                return b.distance(reference_point());
            } else {
                return b.distance(reference_box());
            }
        }

        /// Does the box pass the predicate?
        bool pass_predicate(const query_box_type& rhs) const {
            // This is just a placeholder
            // The function of nearest predicates is to
            // change how iterators work. Not to calculate
            // the distances here as this would be VERY
            // inefficient.
            return true;
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const query_box_type& rhs) const {
            // This is just a placeholder
            return true;
        }

        /// Does the point pass the predicate?
        bool pass_predicate(const point_type& rhs) const {
            // This is just a placeholder
            return true;
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const point_type& rhs) const {
            // This is just a placeholder
            return true;
        }

        /// Does the value pass the predicate?
        template <class mapped_type>
        bool pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            // This is just a placeholder
            return true;
        }

        /// Can a child in this box pass the predicate?
        template <class mapped_type>
        bool might_pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            // This is just a placeholder
            return true;
        }

        bool operator==(const nearest &rhs) const {
            return k_ == rhs.k_ &&
                   data_ == rhs.data_;
        }

        bool operator!=(const nearest &rhs) const {
            return !(rhs == *this);
        }
    };

    template <typename NUMBER_T, std::size_t DimensionCount, class ELEMENT_TYPE = unsigned>
    class satisfies {
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
        using key_type = point_type;
        using mapped_type = ELEMENT_TYPE;
        using value_type = std::pair<key_type, mapped_type>;
        using point_predicate = std::function<bool(const point_type&)>;
        using value_predicate = std::function<bool(const value_type&)>;
        using predicate_function = std::variant<point_predicate, value_predicate>;
        predicate_function predicate_;
    public:
        satisfies(const std::function<bool(const value_type &)> &predicate) : predicate_(predicate) {}

        satisfies(const std::function<bool(const point_type &)> &predicate) : predicate_(predicate) {}

        const std::function<bool(const point_type &)> &predicate() const {
            return predicate_;
        }

        void predicate(const std::function<bool(const point_type &)> &predicate) {
            predicate_ = predicate;
        }

        /// Does the box pass the predicate?
        bool pass_predicate(const query_box_type& rhs) const {
            return true;
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const query_box_type& rhs) const {
            return true;
        }

        /// Does the point pass the predicate?
        bool pass_predicate(const point_type& rhs) const {
            if (std::holds_alternative<point_predicate>(predicate_)) {
                return std::get<point_predicate>(predicate_)(rhs);
            } else {
                throw std::logic_error("You should never pass a value predicate and then try to evaluate that on a point only");
            }
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const point_type& rhs) const {
            return true;
        }

        /// Does the value pass the predicate?
        bool pass_predicate(const value_type& rhs) const {
            if (std::holds_alternative<point_predicate>(predicate_)) {
                return std::get<point_predicate>(predicate_)(rhs.first);
            } else {
                return std::get<value_predicate>(predicate_)(rhs);
            }
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const value_type& rhs) const {
            return true;
        }

        bool operator==(const satisfies &rhs) const {
            return &predicate_ == &rhs.predicate_;
        }

        bool operator!=(const satisfies &rhs) const {
            return !(rhs == *this);
        }
    };

    template <typename NUMBER_T, std::size_t DimensionCount, class ELEMENT_TYPE>
    class query_predicate {
        using number_type = NUMBER_T;
        static constexpr size_t number_of_compile_dimensions = DimensionCount;
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
        using key_type = point_type;
        using mapped_type = ELEMENT_TYPE;
        using value_type = std::pair<key_type, mapped_type>;
        using variant_type = std::variant<intersects<number_type, number_of_compile_dimensions>,
                     disjoint<number_type, number_of_compile_dimensions>,
                     within<number_type, number_of_compile_dimensions>,
                     nearest<number_type, number_of_compile_dimensions>,
                     satisfies<number_type, number_of_compile_dimensions, mapped_type>>;
        variant_type predicate_;
    public:
        query_predicate(const variant_type &predicate) : predicate_(predicate) {}
        query_predicate(const intersects<number_type, number_of_compile_dimensions>& predicate) : predicate_(predicate) {}
        query_predicate(const disjoint<number_type, number_of_compile_dimensions>& predicate) : predicate_(predicate) {}
        query_predicate(const within<number_type, number_of_compile_dimensions>& predicate) : predicate_(predicate) {}
        query_predicate(const nearest<number_type, number_of_compile_dimensions>& predicate) : predicate_(predicate) {}
        query_predicate(const satisfies<number_type, number_of_compile_dimensions, mapped_type>& predicate) : predicate_(predicate) {}

        bool is_intersects() const {
            return std::holds_alternative<intersects<number_type, number_of_compile_dimensions>>(predicate_);
        }

        bool is_disjoint() const {
            return std::holds_alternative<disjoint<number_type, number_of_compile_dimensions>>(predicate_);
        }

        bool is_within() const {
            return std::holds_alternative<within<number_type, number_of_compile_dimensions>>(predicate_);
        }

        bool is_nearest() const {
            return std::holds_alternative<nearest<number_type, number_of_compile_dimensions>>(predicate_);
        }

        bool is_satisfies() const {
            return std::holds_alternative<satisfies<number_type, number_of_compile_dimensions,mapped_type>>(predicate_);
        }

        const intersects<number_type, number_of_compile_dimensions>& as_intersects() const {
            return std::get<intersects<number_type, number_of_compile_dimensions>>(predicate_);
        }

        const disjoint<number_type, number_of_compile_dimensions>& as_disjoint() const {
            return std::get<disjoint<number_type, number_of_compile_dimensions>>(predicate_);
        }

        const within<number_type, number_of_compile_dimensions>& as_within() const {
            return std::get<within<number_type, number_of_compile_dimensions>>(predicate_);
        }

        const nearest<number_type, number_of_compile_dimensions>& as_nearest() const {
            return std::get<nearest<number_type, number_of_compile_dimensions>>(predicate_);
        }

        const satisfies<number_type, number_of_compile_dimensions,mapped_type>& as_satisfies() const {
            return std::get<satisfies<number_type, number_of_compile_dimensions,mapped_type>>(predicate_);
        }

        intersects<number_type, number_of_compile_dimensions>& as_intersects() {
            return std::get<intersects<number_type, number_of_compile_dimensions>>(predicate_);
        }

        disjoint<number_type, number_of_compile_dimensions>& as_disjoint() {
            return std::get<disjoint<number_type, number_of_compile_dimensions>>(predicate_);
        }

        within<number_type, number_of_compile_dimensions>& as_within() {
            return std::get<within<number_type, number_of_compile_dimensions>>(predicate_);
        }

        nearest<number_type, number_of_compile_dimensions>& as_nearest() {
            return std::get<nearest<number_type, number_of_compile_dimensions>>(predicate_);
        }

        satisfies<number_type, number_of_compile_dimensions,mapped_type>& as_satisfies() {
            return std::get<satisfies<number_type, number_of_compile_dimensions,mapped_type>>(predicate_);
        }

        size_t index() const {
            return predicate_.index();
        }

        /// Does the box pass the predicate?
        bool pass_predicate(const query_box_type& rhs) const {
            return std::visit([&rhs](const auto& predicate_){ return predicate_.pass_predicate(rhs); }, predicate_);
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const query_box_type& rhs) const {
            return std::visit([&rhs](const auto& predicate_){ return predicate_.might_pass_predicate(rhs); }, predicate_);
        }

        /// Does the point pass the predicate?
        bool pass_predicate(const point_type& rhs) const {
            return std::visit([&rhs](const auto& predicate_){ return predicate_.pass_predicate(rhs); }, predicate_);
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const point_type& rhs) const {
            return std::visit([&rhs](const auto& predicate_){ return predicate_.might_pass_predicate(rhs); }, predicate_);
        }

        /// Does the value pass the predicate?
        bool pass_predicate(const value_type& rhs) const {
            return std::visit([&rhs](const auto& predicate_){ return predicate_.pass_predicate(rhs); }, predicate_);
        }

        /// Can a child in this box pass the predicate?
        bool might_pass_predicate(const value_type& rhs) const {
            return std::visit([&rhs](const auto& predicate_){ return predicate_.might_pass_predicate(rhs); }, predicate_);
        }

        /// Return which predicate is more restrictive
        /// \param other Other predicate
        /// \param total_volume Total volume of the tree we are manipulating
        /// \return If this predicate is more restrictive than the other
        bool is_more_restrictive(const query_predicate& other, number_type total_volume) const {
            // "satisfies" and "nearest" are the least restrictive
            // because anyone can potentially pass the predicate
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
            number_type volume_a;
            number_type volume_b;
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

        bool operator==(const query_predicate &rhs) const {
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

        bool operator!=(const query_predicate &rhs) const {
            return !(rhs == *this);
        }
    };

}


#endif //PARETO_FRONT_PREDICATES_H
