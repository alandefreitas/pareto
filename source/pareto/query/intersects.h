//
// Created by Alan Freitas on 12/1/20.
//

#ifndef PARETO_INTERSECTS_H
#define PARETO_INTERSECTS_H

#include <pareto/point.h>
#include <pareto/query/query_box.h>

namespace pareto {

    /// \class Predicate to check if a point intersects a query box
    template <typename NUMBER_T, std::size_t DimensionCount>
    class intersects {
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;

    public /* constructors */:
        /// \brief Construct predicate from query box
        explicit intersects(const query_box_type &data) : predicate_querybox_(data) {}

        /// \brief Construct predicate from query box corners
        intersects(const point_type &min_corner, const point_type &max_corner) : predicate_querybox_(query_box_type(min_corner,max_corner)) {}

    public:
        /// \brief Get the underlying query box
        const query_box_type &data() const {
            return predicate_querybox_;
        }

        /// \brief Set the underlying query box
        void data(const query_box_type &data) {
            predicate_querybox_ = data;
        }

        /// \brief Does the box pass the predicate?
        /// We use the querybox function to check that
        bool pass_predicate(const query_box_type& rhs) const {
            return predicate_querybox_.contains(rhs);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        bool might_pass_predicate(const query_box_type& rhs) const {
            return predicate_querybox_.overlap(rhs);
        }

        /// \brief Does the point pass the predicate?
        /// We use the querybox function to check that
        bool pass_predicate(const point_type& rhs) const {
            return predicate_querybox_.contains(rhs);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        bool might_pass_predicate(const point_type& rhs) const {
            return predicate_querybox_.overlap(rhs);
        }

        /// \brief Does the value pass the predicate?
        /// We use the querybox function to check that
        template <class mapped_type>
        bool pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return predicate_querybox_.contains(rhs.first);
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        template <class mapped_type>
        bool might_pass_predicate(const std::pair<point_type, mapped_type>& rhs) const {
            return predicate_querybox_.overlap(rhs.first);
        }

        /// Check if two intersect predicates have the same hyperbox
        bool operator==(const intersects &rhs) const {
            return predicate_querybox_ == rhs.predicate_querybox_;
        }

        /// Check if two intersect predicates have the different hyperboxes
        bool operator!=(const intersects &rhs) const {
            return predicate_querybox_ != rhs.predicate_querybox_;
        }

    private:
        query_box_type predicate_querybox_;
    };
}

#endif //PARETO_INTERSECTS_H
