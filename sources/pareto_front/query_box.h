//
// Created by Alan Freitas on 2020-06-04.
//

#ifndef PARETO_FRONT_QUERY_BOX_H
#define PARETO_FRONT_QUERY_BOX_H

#include <pareto_front/point.h>

namespace pareto_front {
    template <typename NUMBER_T, std::size_t DimensionCount>
    class query_box {
        static constexpr size_t number_of_compile_dimensions_ = DimensionCount;
        using point_type = point<NUMBER_T, number_of_compile_dimensions_>;
        using number_type = typename point_type::number_type;
        using distance_type = typename point_type::distance_type;
        point_type first_;
        point_type second_;
        using box_type = query_box<NUMBER_T, number_of_compile_dimensions_>;

    public /* constructors */:
        query_box() {
            reset();
        };

        /// Construct with n dimensions (only if dimensions are not set at compile time)
        explicit query_box(size_t n)
            : first_(n), second_(n) {
            reset();
        }

        explicit query_box(const point_type& single_point_box) : query_box(single_point_box, single_point_box) {}

        query_box(const point_type& center, number_type half_width)
            : first_(center - half_width), second_(center + half_width) {}

        query_box(const point_type& center, number_type* half_widths)
            : first_(center), second_(center) {
            for (size_t i = 0; i < dimensions(); ++i) {
                first_[i] -= half_widths[i];
                second_[i] += half_widths[i];
            }
        }

        query_box(const point_type& first, const point_type& second)
            : first_(first), second_(second){}

    public /* static functions */:
        /// Returns a new bounding box that has the maximum boundaries
        static query_box maximum_bound_box(size_t n) {
            query_box bound(n);
            bound.reset();
            return bound;
        }

    public /* non-modifying functions */:

        point_type& first() {
            return first_;
        }

        point_type& second() {
            return second_;
        }

        point_type& min() {
            return first_;
        }

        point_type& max() {
            return second_;
        }

        const point_type& first() const {
            return first_;
        }

        const point_type& second() const {
            return second_;
        }

        const point_type& min() const {
            return first_;
        }

        const point_type& max() const {
            return second_;
        }

        point_type center() const {
            return (first_ + second_)/2.0;
        }

        size_t dimensions() const {
            return first_.dimensions();
        }

        point_type half_width(size_t index) const {
            return (second_[index] - first_[index])/2.0;
        }

        bool empty(size_t index) const {
            for (size_t index=0; index < dimensions(); ++index) {
                if (second_[index] - first_[index] == 0) {
                    return true;
                }
            }
            return false;
        }

        number_type volume() const {
            number_type v = 1.;
            for (size_t i = 0; i < dimensions(); ++i) {
                v *= (second_[i] - first_[i]);
            }
            return v;
        }

        number_type area() const {
            return volume();
        }

        /// Combine two rectangles into larger one containing both
        query_box combine(const query_box& other) const {
            box_type new_query_box(dimensions());
            for (int index = 0; index < dimensions(); ++index) {
                new_query_box.min()[index] = std::min(min()[index], other.min()[index]);
                new_query_box.max()[index] = std::max(max()[index], other.max()[index]);
            }
            return new_query_box;
        }

        query_box operator|(const query_box& other) const {
            return combine(other);
        }

        /// Return the intersection of two rectangles into a smaller rectangle
        query_box intersection(const query_box& other) const {
            box_type new_query_box(dimensions());
            for (int index = 0; index < dimensions(); ++index) {
                new_query_box.min()[index] = std::max(min()[index], other.min()[index]);
                new_query_box.max()[index] = std::min(max()[index], other.max()[index]);
            }
            return new_query_box;
        }

        query_box operator&(const query_box& other) const {
            return intersection(other);
        }

        /// Combine the rectangle into larger one containing the point
        query_box combine(const point_type& p) const {
            box_type new_query_box;
            for (int index = 0; index < dimensions(); ++index) {
                new_query_box.min()[index] = std::min(min()[index], p[index]);
                new_query_box.max()[index] = std::max(max()[index], p[index]);
            }
            return new_query_box;
        }

        /// Check if they have some area in common (ar any point)
        bool overlap(const point_type& p) const {
            // for each dimension
            for (size_t index=0; index < first_.dimensions(); ++index) {
                // check min and max intersection
                if (first_[index] > p[index] || p[index] > second_[index]) {
                    return false;
                }
            }
            return true;
        }

        /// Check if they have some area in common (on any point including borders)
        bool overlap(const query_box& rhs) const {
            // for each dimension
            for (size_t index=0; index < first_.dimensions(); ++index) {
                // check min and max intersection
                if (first_[index] > rhs.second_[index] ||
                    rhs.first_[index] > second_[index]) {
                    return false;
                }
            }
            return true;
        }

        /// Calculate how much area they have in common
        /// If they have only borders in common, this will return zero
        /// while overlap will return true.
        number_type overlap_area(const query_box& rhs) const {
            number_type area = 1.;
            // for each dimension
            for (size_t index=0; area && index < first_.dimensions(); ++index) {
                // left edge outside left edge
                if (first_[index] < rhs.first_[index]) {
                    // and right edge inside left edge
                    if (rhs.first_[index] < second_[index]) {
                        // right edge outside right edge
                        if (rhs.second_[index] < second_[index]) {
                            area *= (double) (rhs.second_[index] - rhs.first_[index]);
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

        /// Check if point is inside the box (including border)
        bool intersects(const point_type& p) const {
            // for each dimension
            for (size_t index=0; index < p.dimensions(); ++index) {
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

        /// Check if box is inside the box (including border)
        bool intersects(const box_type& b) const {
            // for each dimension
            for (size_t index=0; index < b.dimensions(); ++index) {
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
        bool encloses(const box_type& b) const {
            return intersects(b);
        }

        /// \\brief Returns true if a point is inside the half-closed rectangle
        /// For our purposes, we're considering half-closed rectangles.
        /// If a point is on the boundary of a rectangle, then it is considered
        /// to be inside the rectangle if it's on the upper boundary but not
        /// inside the rectangle if it's on the lower boundary. This is to avoid
        /// annoying edge cases because computational geometry is hard. */
        inline bool in_half_closed_rectangle( point_type& p ) {
            bool ret = true;
            for (size_t i=0; i < dimensions(); i++) {
                if (p[i] > second_[i]) {
                    return false;
                }
                if (p[i] <= first_[i]) {
                    return false;
                }
            }
            return true;
        }

        /// Check if point is inside the box (excluding border)
        bool within(const point_type& p) const {
            // for each dimension
            for (size_t index=0; index < p.dimensions(); ++index) {
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

        /// Check if box is inside the box (excluding border)
        bool within(const box_type& b) const {
            // for each dimension
            for (size_t index=0; index < b.dimensions(); ++index) {
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

        template <std::size_t DimensionCount2, typename CoordinateSystem>
        distance_type distance(const point<NUMBER_T, DimensionCount2, CoordinateSystem>& p) const {
            distance_type dist = 0.0;
            for (size_t i = 0; i < dimensions(); ++i) {
                distance_type di = std::max(std::max(first()[i] - p[i], p[i] - second()[i]), 0.0);
                dist += di * di;
            }
            return sqrt(dist);
        }

        distance_type distance(const point_type& p) const {
            distance_type dist = 0.0;
            for (size_t i = 0; i < dimensions(); ++i) {
                distance_type di = std::max(std::max(first()[i] - p[i], p[i] - second()[i]), 0.0);
                dist += di * di;
            }
            return sqrt(dist);
        }

        distance_type distance(const query_box& b) const {
            point_type closest_point(dimensions());
            for (size_t i = 0; i < b.dimensions(); ++i) {
                closest_point[i] = b.min()[i] > min()[i] ? max()[i] : min()[i];
            }
            return distance(closest_point);
        }

        /// Distance from center point to the center of another bounding box
        distance_type distance_from_center(const query_box& b) const {
            return center().distance(b.center());
        }

        /// Distance from center point to another point
        distance_type distance_from_center(const point_type& p) const {
            return p.distance(center());
        }

        /// Check if point is outside the box
        bool disjoint(const point_type& p) const {
            return !intersects(p);
        }

        /// Check if box is outside the query box
        bool disjoint(const box_type& b) const {
            return !overlap(b);
        }

        /// The sum of all deltas between edges
        inline number_type edge_deltas() const {
            number_type distance = 0;
            for (std::size_t axis = 0; axis < dimensions(); axis++) {
                distance += second_[axis] - first_[axis];
            }
            return distance;
        }

        bool operator==(const query_box &rhs) const {
            return first_ == rhs.first_ && second_ == rhs.second_;
        }

        bool operator!=(const query_box &rhs) const {
            return !(rhs == *this);
        }

    public /* modifying functions */:
        /// Forces all edges to their extremes so we can stretch() it
        void reset() {
            for (std::size_t axis = 0; axis < dimensions(); axis++) {
                first_[axis] = std::numeric_limits<number_type>::max();
                second_[axis] = -std::numeric_limits<number_type>::max();
            }
        }

        /// Fits another box inside of this box
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

        /// Fits another point inside of this box
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
    };
}

#endif //PARETO_FRONT_QUERY_BOX_H
