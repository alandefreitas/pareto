//
// Created by Alan Freitas on 12/1/20.
//

#ifndef PARETO_SATISFIES_H
#define PARETO_SATISFIES_H

#include <pareto/point.h>
#include <pareto/query/query_box.h>

namespace pareto {

    /// \class Satisfies predicate
    /// This predicate checks if a point passes a lambda function
    /// This is the most expensive predicate as any point can,
    /// in principle, pass a function predicate until we test it.
    /// We should replace this whenever we can use a predicate based
    /// on hyperboxes or points.
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
    public /* constructors */:
        /// \brief Construct predicate from function that depends on value_type pair <point, mapped_type>
        explicit satisfies(const std::function<bool(const value_type &)> &predicate) : predicate_(predicate) {}

        /// \brief Construct predicate from function that depends on point_type only
        explicit satisfies(const std::function<bool(const point_type &)> &predicate) : predicate_(predicate) {}

    public:

        /// \brief Get the predicate function
        const std::function<bool(const point_type &)> &predicate() const {
            return predicate_;
        }

        /// \brief Set the predicate function
        void predicate(const std::function<bool(const point_type &)> &predicate) {
            predicate_ = predicate;
        }

        /// \brief Does the box pass the predicate?
        /// We use the querybox function to check that
        bool pass_predicate(const query_box_type& rhs [[maybe_unused]]) const {
            return true;
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        bool might_pass_predicate(const query_box_type& rhs [[maybe_unused]]) const {
            return true;
        }

        /// \brief Does the point pass the predicate?
        /// We use the querybox function to check that
        bool pass_predicate(const point_type& rhs) const {
            if (std::holds_alternative<point_predicate>(predicate_)) {
                return std::get<point_predicate>(predicate_)(rhs);
            } else {
                throw std::logic_error("You should never pass a value predicate and then try to evaluate that on a point only");
            }
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        bool might_pass_predicate(const point_type& rhs [[maybe_unused]]) const {
            return true;
        }

        /// \brief Does the value pass the predicate?
        /// We use the querybox function to check that
        bool pass_predicate(const value_type& rhs) const {
            if (std::holds_alternative<point_predicate>(predicate_)) {
                return std::get<point_predicate>(predicate_)(rhs.first);
            } else {
                return std::get<value_predicate>(predicate_)(rhs);
            }
        }

        /// \brief Can a child in this box pass the predicate?
        /// We use the querybox function to check that
        bool might_pass_predicate(const value_type& rhs [[maybe_unused]]) const {
            return true;
        }

        /// \brief Compare the underlying data structure
        bool operator==(const satisfies &rhs) const {
            return &predicate_ == &rhs.predicate_;
        }

        /// \brief Compare the underlying data structure
        bool operator!=(const satisfies &rhs) const {
            return &predicate_ != &rhs.predicate_;
        }

    private:

        /// \brief Function representing the predicate
        predicate_function predicate_;

    };
}

#endif //PARETO_SATISFIES_H
