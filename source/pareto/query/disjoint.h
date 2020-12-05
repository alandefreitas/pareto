//
// Created by Alan Freitas on 12/1/20.
//

#ifndef PARETO_DISJOINT_H
#define PARETO_DISJOINT_H

#include <pareto/point.h>
#include <pareto/query/query_box.h>

namespace pareto {

    /// \class Predicate to check if a point is outside a query box
    /// This is useful to eliminate areas from the search space
    template <typename NUMBER_T, std::size_t DimensionCount>
    class disjoint {
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
    public /* constructors */:
        /// \brief Construct predicate from query box
        explicit disjoint(const query_box_type &data) : data_(data) {}

        /// \brief Construct predicate from query box corners
        explicit disjoint(const point_type &single_point) : data_(query_box_type(single_point,single_point)) {}

        /// \brief Construct predicate from query box corners
        disjoint(const point_type &min_corner, const point_type &max_corner) : data_(query_box_type(min_corner,max_corner)) {}

    public:
        /// \brief Get the underlying query box
        const query_box_type &data() const {
            return data_;
        }

        /// \brief Set the underlying query box
        void data(const query_box_type &data) {
            data_ = data;
        }

        /// \brief Does the box pass the predicate?
        /// We use the querybox function to check that
        bool pass_predicate(const query_box_type& rhs) const {
            return data_.disjoint(rhs);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        bool might_pass_predicate(const query_box_type& rhs) const {
            return !data_.contains(rhs);
        }

        /// \brief Does the point pass the predicate?
        /// We use the querybox function to check that
        bool pass_predicate(const point_type& rhs) const {
            return data_.disjoint(rhs);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        bool might_pass_predicate(const point_type& rhs) const {
            return !data_.contains(rhs);
        }

        /// \brief Does the value pass the predicate?
        /// We use the querybox function to check that
        template <class mapped_type>
        bool pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return data_.disjoint(rhs.first);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        template <class mapped_type>
        bool might_pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return !data_.contains(rhs.first);
        }

        /// \brief Compare underlying hyperboxes
        bool operator==(const disjoint &rhs) const {
            return data_ == rhs.data_;
        }

        /// \brief Compare underlying hyperboxes
        bool operator!=(const disjoint &rhs) const {
            return data_ != rhs.data_;
        }

    private:
        /// \brief Hyperbox to which we compare
        query_box_type data_;
    };
}

#endif //PARETO_DISJOINT_H
