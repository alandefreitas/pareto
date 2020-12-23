//
// Created by Alan Freitas on 12/1/20.
//

#ifndef PARETO_NEAREST_H
#define PARETO_NEAREST_H

#include <pareto/point.h>
#include <pareto/query/query_box.h>

namespace pareto {

    /// \class Predicate to query the k-nearest points to a point or hyperbox
    /// If we use a hyperbox as reference, points inside the box will be
    /// considered to have distance zero
    /// This is a very special predicate because it changes how the query
    /// algorithm happens in a containers. If an iterator has a nearest
    /// predicate, the iterator moves from its default depth-first search to a
    /// custom/// algorithm that keeps track of the potential distance to a hyperbox
    /// as the iterator moves forward
    template <typename NUMBER_T, std::size_t DimensionCount>
    class nearest {
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
        using object_variant = std::variant<point_type, query_box_type>;

    public /* constructors */:
      /// \brief Construct predicate from point and k nearest
      nearest(const point_type &data, size_t k = 1) : k_(k), data_(data) {}

      /// \brief Construct predicate from query box and k nearest
      nearest(const query_box_type &data, size_t k = 1)
          : k_(k), data_(object_variant(data)) {}

    public:
        /// \brief Get number of points k considered in the query
        [[nodiscard]] size_t k() const {
            return k_;
        }

        /// \brief Set number of points k considered in the query
        void k(size_t k) {
            k_ = k;
        }

        /// \brief Get the underlying point of reference as a variant
        const point_type &data() const {
            return data_;
        }

        /// \brief Get the underlying point of reference as a variant
        void data(const point_type &data) {
            data_ = data;
        }

        /// \brief Set the underlying reference as a point
        void reference_point(const point_type &data) {
            data_ = data;
        }

        /// \brief Set the underlying reference as a query box
        void reference_box(const query_box_type &data) {
            data_ = data;
        }

        /// \brief Check if the predicate uses a point as reference
        [[nodiscard]] bool has_reference_point() const {
            return std::holds_alternative<point_type>(data_);
        }

        /// \brief Check if the predicate uses a hyperbox as reference
        [[nodiscard]] bool has_reference_box() const {
            return std::holds_alternative<query_box_type>(data_);
        }

        /// \brief Get the underlying reference as a hyperbox
        /// This will throw an error if the predicate is using a
        /// point as reference
        const query_box_type &reference_box() const {
            return std::get<query_box_type>(data_);
        }

        /// \brief Get the underlying reference as a point
        /// This will throw an error if the predicate is using a
        /// hyperbox as reference
        const point_type &reference_point() const {
            return std::get<point_type>(data_);
        }

        /// \brief Calculate the distance to a point
        double distance(const point_type& p) {
            if (has_reference_point()) {
                return reference_point().distance(p);
            } else {
                return reference_box().distance(p);
            }
        }

        /// \brief Calculate the distance to a query box
        double distance(const query_box_type& b) {
            if (has_reference_point()) {
                return b.distance(reference_point());
            } else {
                return b.distance(reference_box());
            }
        }

        /// \brief Does the box pass the predicate?
        /// We assume any box can, in principle, pass the predicate
        bool pass_predicate(const query_box_type& rhs [[maybe_unused]]) const {
            // This is just a placeholder
            // The function of nearest predicates is to
            // change how iterators work. Not to calculate
            // the distances here as this would be VERY
            // inefficient.
            return true;
        }

        /// \brief Can a child in this box pass the predicate?
        /// In principle, any box might pass the predicate
        bool might_pass_predicate(const query_box_type& rhs [[maybe_unused]]) const {
            // This is just a placeholder
            return true;
        }

        /// \brief Does the point pass the predicate?
        /// In principle, any box might pass the predicate
        bool pass_predicate(const point_type& rhs [[maybe_unused]]) const {
            // This is just a placeholder
            return true;
        }

        /// \brief Can a child in this box pass the predicate?
        /// In principle, any box might pass the predicate
        bool might_pass_predicate(const point_type& rhs [[maybe_unused]]) const {
            // This is just a placeholder
            return true;
        }

        /// \brief Does the value pass the predicate?
        /// In principle, any box might pass the predicate
        template <class mapped_type>
        bool pass_predicate(const std::pair<point_type, mapped_type>& rhs [[maybe_unused]]) const {
            // This is just a placeholder
            return true;
        }

        /// \brief Can a child in this box pass the predicate?
        /// In principle, any box might pass the predicate
        template <class mapped_type>
        bool might_pass_predicate(const std::pair<point_type, mapped_type>& rhs [[maybe_unused]]) const {
            // This is just a placeholder
            return true;
        }

        /// \brief Compare underlying data structures
        bool operator==(const nearest &rhs) const {
            return k_ == rhs.k_ &&
                   data_ == rhs.data_;
        }

        /// \brief Compare underlying data structures
        bool operator!=(const nearest &rhs) const {
            return k_ != rhs.k_ ||
                   data_ != rhs.data_;
        }

    private:
      /// \brief Number of nearest points considered in the query
      size_t k_{1};

      /// \brief Reference point or hyperbox
        object_variant data_;
    };

}


#endif //PARETO_NEAREST_H
