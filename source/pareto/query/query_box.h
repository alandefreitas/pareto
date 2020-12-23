//
// Created by Alan Freitas on 2020-06-04.
//

#ifndef PARETO_FRONT_QUERY_BOX_H
#define PARETO_FRONT_QUERY_BOX_H

#include <pareto/point.h>

namespace pareto {

    /// \brief Normalize query box corners so that lower_bound < upper_bound always
    template<typename NUMBER_T, std::size_t DimensionCount, typename CoordinateSystem>
    void normalize_corners(point <NUMBER_T, DimensionCount, CoordinateSystem> &min_corner,
                           point <NUMBER_T, DimensionCount, CoordinateSystem> &max_corner) {
        for (size_t i = 0; i < min_corner.dimensions(); ++i) {
            if (min_corner[i] > max_corner[i]) {
                std::swap(min_corner[i], max_corner[i]);
            }
        }
    }

    /// \class Query box
    /// A query box is a hyperbox used for queries
    /// Almost all query predicates need a hyperbox to limit the query
    /// Hyperboxes allow the queries to go faster because they only inspect
    /// the nodes that might have children inside a give hyperbox
    template<typename NUMBER_T, std::size_t DimensionCount>
    class query_box {
        /// Number of dimensions (zero for runtime dimension)
        static constexpr size_t number_of_compile_dimensions_ = DimensionCount;

        /// Vertices type
        using point_type = point<NUMBER_T, number_of_compile_dimensions_>;

        /// Number type of the vertices
        using dimension_type = typename point_type::dimension_type;

        /// \brief Distance between two vertices
        /// If the number type is integer, this might need to be promoted
        using distance_type = typename point_type::distance_type;

        using box_type = query_box<NUMBER_T, number_of_compile_dimensions_>;

    public /* Constructors */:
        /// \brief Construct box from two points
        /// This is the constructor you are most likely to use
        query_box(const point_type &first, const point_type &second)
                : first_(first), second_(second) {
            normalize_corners(first_,second_);
        }

        /// \brief Construct hyperbox
        /// Use this version only if dimension is set at compile-time
        query_box() {
            stretch_to_infinity();
        };

        /// \brief Construct with n dimensions
        /// Only if dimensions are not set at compile time)
        explicit query_box(size_t n)
                : first_(n), second_(n) {
            stretch_to_infinity();
        }

        /// \brief Construct box for a single point
        explicit query_box(const point_type &single_point_box) : query_box(single_point_box, single_point_box) {}

        /// \brief Construct box from a center and a half width
        query_box(const point_type &center, dimension_type half_width)
            : first_(center - half_width), second_(center + half_width) {
            normalize_corners(first_,second_);
        }

        /// \brief Construct box from a center and a list of half widths for dimension
        query_box(const point_type &center, dimension_type *half_widths)
            : first_(center), second_(center) {
            for (size_t i = 0; i < dimensions(); ++i) {
                first_[i] -= half_widths[i];
                second_[i] += half_widths[i];
            }
            normalize_corners(first_,second_);
        }

    public /* static constructor-like functions */:
        /// \brief Returns a new bounding box that has the maximum boundaries
        static query_box maximum_bound_box(size_t n) {
            query_box bound(n);
            bound.stretch_to_infinity();
            return bound;
        }

    public /* non-modifying functions */:

        /// \brief Return the first (min) point
        point_type &first() {
            return first_;
        }

        /// \brief Return the min (first) point
        point_type &min() {
            return first_;
        }

        /// \brief Return the second (max) point
        point_type &second() {
            return second_;
        }

        /// \brief Return the max (max) point
        point_type &max() {
            return second_;
        }

        /// \brief Return the first (min) point
        const point_type &first() const {
            return first_;
        }

        /// \brief Return the min (first) point
        const point_type &min() const {
            return first_;
        }

        /// \brief Return the second (max) point
        const point_type &second() const {
            return second_;
        }

        /// \brief Return the max (max) point
        const point_type &max() const {
            return second_;
        }

        /// \brief Return center of the hyperbox
        point_type center() const {
            return (first_ + second_) / 2.0;
        }

        /// \brief Get number of dimensions of a hyperbox
        [[nodiscard]] size_t dimensions() const {
            return first_.dimensions();
        }

        /// \brief Return the half width of a hyperbox in a given dimension
        dimension_type half_width(size_t index) const {
            return (second_[index] - first_[index]) / 2.0;
        }

        /// \brief Return true is a hyperbox has hypervolume zero
        [[nodiscard]] bool empty() const {
            for (size_t index = 0; index < dimensions(); ++index) {
                if (second_[index] - first_[index] == 0) {
                    return true;
                }
            }
            return false;
        }

        /// \brief Return hypervolume of a hyperbox
        dimension_type volume() const {
            dimension_type v = 1.;
            for (size_t i = 0; i < dimensions(); ++i) {
                v *= (second_[i] - first_[i]);
            }
            return v;
        }

        /// \brief Return hypervolume of a hyperbox
        /// Just is just a convenience function as some
        /// boxes are 2d and "volume" would be hard to guess
        /// For this reason, we inline calls to this function
        inline dimension_type area() const { return volume(); }

        /// \brief Combine two hyperboxes into larger one containing both
        query_box combine(const query_box &other) const {
            box_type new_query_box(dimensions());
            for (size_t index = 0; index < dimensions(); ++index) {
                new_query_box.min()[index] = std::min(min()[index], other.min()[index]);
                new_query_box.max()[index] = std::max(max()[index], other.max()[index]);
            }
            return new_query_box;
        }

        /// \brief Combine two hyperboxes into larger one containing both
        query_box operator|(const query_box &other) const {
            return combine(other);
        }

        /// \brief Combine the rectangle into larger one containing the point
        query_box combine(const point_type &p) const {
            box_type new_query_box;
            for (size_t index = 0; index < dimensions(); ++index) {
                new_query_box.min()[index] = std::min(min()[index], p[index]);
                new_query_box.max()[index] = std::max(max()[index], p[index]);
            }
            return new_query_box;
        }

        /// \brief Combine two hyperboxes into larger one containing the point p
        query_box operator|(const point_type &p) const {
            return combine(p);
        }

        /// \brief Return the intersection of two rectangles into a smaller rectangle
        query_box intersection(const query_box &other) const {
            box_type new_query_box(dimensions());
            for (size_t index = 0; index < dimensions(); ++index) {
                new_query_box.min()[index] = std::max(min()[index], other.min()[index]);
                new_query_box.max()[index] = std::min(max()[index], other.max()[index]);
            }
            return new_query_box;
        }

        /// \brief Return the intersection of two rectangles into a smaller rectangle
        query_box operator&(const query_box &other) const {
            return intersection(other);
        }

        /// \brief Check if the hyperbox and the point have some area in common (or any point)
        bool overlap(const point_type &p) const {
            // for each dimension
            for (size_t index = 0; index < first_.dimensions(); ++index) {
                // check min and max intersection
                if (first_[index] > p[index] || p[index] > second_[index]) {
                    return false;
                }
            }
            return true;
        }

        /// \brief Check if two hyperboxes have some area in common (or any point)
        bool overlap(const query_box &rhs) const {
            // for each dimension
            for (size_t index = 0; index < first_.dimensions(); ++index) {
                // check min and max intersection
                if (first_[index] > rhs.second_[index] ||
                    rhs.first_[index] > second_[index]) {
                    return false;
                }
            }
            return true;
        }

        /// \brief Calculate how much area two query boxes have in common
        /// If they have only borders in common, this will return zero
        /// while overlap will still return true.
        dimension_type overlap_area(const query_box &rhs) const {
            dimension_type area = 1.;
            // for each dimension
            for (size_t index = 0; area && index < first_.dimensions();
                 ++index) {
                // left edge outside left edge
                if (first_[index] < rhs.first_[index]) {
                    // and right edge inside left edge
                    if (rhs.first_[index] < second_[index]) {
                        // right edge outside right edge
                        if (rhs.second_[index] < second_[index]) {
                            area *= (double)(rhs.second_[index] -
                                             rhs.first_[index]);
                        } else {
                            area *= (double) (second_[index] - rhs.first_[index]);
                        }
                        continue;
                    }
                }
                // right edge inside left edge
                else if (first_[index] < rhs.second_[index]) {
                    // right edge outside right edge
                    if (second_[index] < rhs.second_[index]) {
                        area *= (double) (second_[index] - first_[index]);
                    } else {
                        area *= (double) (rhs.second_[index] - first_[index]);
                    }
                    continue;
                }

                // if we get here, there is no overlap
                return 0.0;
            }
            return true;
        }

        /// \brief Check if a point is inside the box (including borders)
        bool contains(const point_type &p) const {
            // for each dimension
            for (size_t index = 0; index < p.dimensions(); ++index) {
                // check min and max intersection
                if (p[index] < first_[index]) {
                    return false;
                }
                if (p[index] > second_[index]) {
                    return false;
                }
            }
            return true;
        }

        /// \brief Check if hyperbox b is inside this hyperbox (including border)
        bool contains(const box_type &b) const {
            // for each dimension
            for (size_t index = 0; index < b.dimensions(); ++index) {
                // check min and max intersection
                if (b.min()[index] < first_[index]) {
                    return false;
                }
                if (b.max()[index] > second_[index]) {
                    return false;
                }
            }
            return true;
        }

        /// This determines if a bounding box is fully contained within this bounding box
        bool encloses(const box_type &b) const {
            return contains(b);
        }

        /// \\brief Returns true if a point is inside the half-closed rectangle
        /// This seems like a weird operation but it's useful to make sure
        /// trees can have a order relationship between points and boxes.
        /// For our purposes, we're considering half-closed rectangles.
        /// If a point is on the boundary of a rectangle, then it is considered
        /// to be inside the rectangle if it's on the upper boundary but not
        /// inside the rectangle if it's on the lower boundary. This is to avoid
        /// annoying edge cases because computational geometry is hard. */
        inline bool in_half_closed_rectangle(point_type &p) {
            for (size_t i = 0; i < dimensions(); i++) {
                if (p[i] > second_[i]) {
                    return false;
                }
                if (p[i] <= first_[i]) {
                    return false;
                }
            }
            return true;
        }

        /// \brief Check if point is inside the box (excluding border)
        bool within(const point_type &p) const {
            // for each dimension
            for (size_t index = 0; index < p.dimensions(); ++index) {
                // check min and max intersection
                if (p[index] <= first_[index]) {
                    return false;
                }
                if (p[index] >= second_[index]) {
                    return false;
                }
            }
            return true;
        }

        /// Check if another hyperbox is inside this hyperbox (excluding border)
        bool within(const box_type &b) const {
            // for each dimension
            for (size_t index = 0; index < b.dimensions(); ++index) {
                // check min and max intersection
                if (b.min()[index] <= first_[index]) {
                    return false;
                }
                if (b.max()[index] >= second_[index]) {
                    return false;
                }
            }
            return true;
        }

        /// \brief Calculates the distance between this hyperbox and a point
        template<std::size_t DimensionCount2, typename CoordinateSystem>
        distance_type distance(const point <NUMBER_T, DimensionCount2, CoordinateSystem> &p) const {
            distance_type dist = 0.0;
            for (size_t i = 0; i < dimensions(); ++i) {
                distance_type di = std::max(std::max(first()[i] - p[i], p[i] - second()[i]), 0.0);
                dist += di * di;
            }
            return sqrt(dist);
        }

        /// \brief Calculates the distance between this hyperbox and a point
        distance_type distance(const point_type &p) const {
            distance_type dist = 0.0;
            for (size_t i = 0; i < dimensions(); ++i) {
                distance_type di = static_cast<distance_type>(
                    std::max(std::max(first()[i] - p[i], p[i] - second()[i]),
                             dimension_type{0}));
                dist += di * di;
            }
            return sqrt(dist);
        }

        /// \brief Calculates the distance between this hyperbox and another hyperbox
        distance_type distance(const query_box &b) const {
            point_type closest_point(dimensions());
            for (size_t i = 0; i < b.dimensions(); ++i) {
                closest_point[i] = b.min()[i] > min()[i] ? max()[i] : min()[i];
            }
            return distance(closest_point);
        }

        /// \brief Distance from center point to the center of another bounding box
        distance_type distance_from_center(const query_box &b) const {
            return center().distance(b.center());
        }

        /// \brief Distance from center point of this hyperbox to another point
        distance_type distance_from_center(const point_type &p) const {
            return p.distance(center());
        }

        /// \brief Check if point is outside the box
        bool disjoint(const point_type &p) const {
            return !contains(p);
        }

        /// \brief Check if box is outside the query box
        bool disjoint(const box_type &b) const {
            return !overlap(b);
        }

        /// \brief The sum of all deltas between edges
        /// This is equivalent to the perimeter for a normalized hyperbox
        inline dimension_type edge_deltas() const {
            dimension_type distance = 0;
            for (std::size_t axis = 0; axis < dimensions(); axis++) {
                distance += second_[axis] - first_[axis];
            }
            return distance;
        }

        /// \brief Compare hyperboxes
        bool operator==(const query_box &rhs) const {
            return first_ == rhs.first_ && second_ == rhs.second_;
        }

        /// \brief Compare hyperboxes
        bool operator!=(const query_box &rhs) const {
            return first_ != rhs.first_ || second_ != rhs.second_;
        }

    public /* modifying functions */:
        /// \brief Forces all edges to their extremes in +infinity/-infinity so we can stretch() it later
        /// New hyperboxes start with invalid values larger and smaller than any other values
        /// This makes it possible to stretch the hyperbox later because the new value will always
        /// be smaller than min and larger than max
        void stretch_to_infinity() {
            for (std::size_t axis = 0; axis < dimensions(); axis++) {
                first_[axis] = std::numeric_limits<dimension_type>::max();
                second_[axis] = -std::numeric_limits<dimension_type>::max();
            }
        }

        /// \brief Fit another hyperbox inside of this box
        /// \param bb Another query box
        /// \return true if a stretch occurred
        bool stretch(const query_box &bb) {
            bool ret = false;
            for (std::size_t axis = 0; axis < dimensions(); axis++) {
                if (first_[axis] > bb.first_[axis]) {
                    first_[axis] = bb.first_[axis];
                    ret = true;
                }
                if (second_[axis] < bb.second_[axis]) {
                    second_[axis] = bb.second_[axis];
                    ret = true;
                }
            }
            return ret;
        }

        /// \brief Fits another point inside this box
        /// \param bb Another query box
        /// \return true if a stretch occurred
        bool stretch(const point_type &p) {
            bool ret = false;
            for (std::size_t axis = 0; axis < dimensions(); axis++) {
                if (first_[axis] > p[axis]) {
                    first_[axis] = p[axis];
                    ret = true;
                }
                if (second_[axis] < p[axis]) {
                    second_[axis] = p[axis];
                    ret = true;
                }
            }
            return ret;
        }

    private /* members */:
        /// First query vertex (min point)
        point_type first_;

        /// First query vertex (max point)
        point_type second_;
    };
}

#endif //PARETO_FRONT_QUERY_BOX_H
