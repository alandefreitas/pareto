//
// Created by Alan Freitas on 12/1/20.
//

#ifndef PARETO_WITHIN_H
#define PARETO_WITHIN_H

#include <pareto/point.h>
#include <pareto/query/query_box.h>

namespace pareto {

    /// \class Within predicate
    /// This predicate is very similar to intersects
    /// The only difference is that it does not consider the borders
    /// when comparing elements. This is important for finding
    /// dominated regions of the search space
    template <typename NUMBER_T, std::size_t DimensionCount>
    class within {
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
        query_box_type data_;

    public /* constructors */:
        /// \brief Construct predicate from query box
        explicit within(const query_box_type &data) : data_(data) {}

        /// \brief Construct predicate from query box corners
        within(const point_type &min_corner, const point_type &max_corner) : data_(query_box_type(min_corner,max_corner)) {}

    public:
        /// \brief Get the underlying query box
        const query_box_type &data() const {
            return data_;
        }

        /// \brief Set the underlying query box
        void data(const query_box_type &data) { data_ = data; }

        /// \brief Does the box pass the predicate?
        /// We use the querybox function to check that
        bool pass_predicate(const query_box_type &rhs) const {
            return data_.within(rhs);
        }

        /// \brief Does the point pass the predicate?
        /// We use the querybox function to check that
        bool pass_predicate(const point_type &rhs) const {
            return data_.within(rhs);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        bool might_pass_predicate(const query_box_type &rhs) const {
            return data_.overlap(rhs);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        bool might_pass_predicate(const point_type &rhs) const {
            return data_.within(rhs);
        }

        /// \brief Does the value pass the predicate?
        /// We use the querybox function to check that
        template <class mapped_type>
        bool pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return data_.within(rhs.first);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        template <class mapped_type>
        bool might_pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return data_.overlap(rhs.first);
        }

        /// \brief Compare underlying hyperboxes
        bool operator==(const within &rhs) const {
            return data_ == rhs.data_;
        }

        /// \brief Compare underlying hyperboxes
        bool operator!=(const within &rhs) const {
            return data_ != rhs.data_;
        }
    };
}


#endif //PARETO_WITHIN_H
