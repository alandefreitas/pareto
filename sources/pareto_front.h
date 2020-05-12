//
// Created by Alan Freitas on 2020-05-12.
//

#ifndef PARETO_FRONTS_PARETO_FRONTS_H
#define PARETO_FRONTS_PARETO_FRONTS_H

#include <boost/geometry.hpp>
#include <random>

namespace pareto_front {
    using namespace boost;

    template<typename NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, typename ELEMENT_TYPE>
    class pareto_front {
    public:
        using number_type = NUMBER_TYPE;
        static constexpr size_t dimensions = NUMBER_OF_DIMENSIONS;
        using point_type = geometry::model::point<number_type, dimensions, geometry::cs::cartesian>;
        using box_type = geometry::model::box<point_type>;

        using key_type = point_type;
        using mapped_type = ELEMENT_TYPE;
        using value_type = std::pair<key_type, mapped_type>;
        using size_type = size_t;

        using tree_type = geometry::index::rtree<value_type, geometry::index::quadratic<16>>;
        // using iterator = typename tree_type::iterator;
        using const_iterator = typename tree_type::const_query_iterator;

    public /* constructors */:
        /// Create an empty pareto set
        pareto_front() = default;

        template<class InputIterator>
        pareto_front(InputIterator first, InputIterator last)
                : rtree_(first,last) {}

        pareto_front(const pareto_front &m) {
            rtree_ = m.rtree_;
        }

        pareto_front(pareto_front &&m) noexcept {
            rtree_ = std::move(m.rtree_);
        }

        pareto_front(std::initializer_list<value_type> il)
        : rtree_(il.begin(),il.end()) {}

    public /* iterators */:
        const_iterator begin() const noexcept {
            return rtree_.qbegin(geometry::index::satisfies([](auto const& x){return true;}));
        }

        const_iterator end() const noexcept {
            return rtree_.qend();
        }

    public /* capacity */:
        bool empty() const noexcept {
            return rtree_.empty();
        }

        size_type size() const noexcept {
            return rtree_.size();
        }

        size_type max_size() const noexcept {
            return rtree_.max_size();
        }

    public /* modifiers */:
        template<class... Args>
        std::pair<const_iterator, bool> emplace(Args &&... args) {
            auto v = value_type(args...);
            rtree_.insert(v);
            clear_dominated(v.first);
            return {find_nearest(v.first), true};
        }

        std::pair<const_iterator, bool> insert(const value_type &v) {
            clear_dominated(v.first);
            rtree_.insert(v);
            return {find_nearest(v.first), true};
        }

        std::pair<const_iterator, bool> insert(value_type &&v) {
            clear_dominated(v.first);
            rtree_.insert(std::move(v));
            return {find_nearest(v.first), true};
        }

        template<class P>
        std::pair<const_iterator, bool> insert(const P &p) {
            rtree_.insert(p);
            clear_dominated(p);
            return {find_nearest(p.first), true};
        }

        const_iterator insert(point_type position, const value_type &v) {
            rtree_.insert(std::make_pair(position, v));
            clear_dominated(position);
            return {find_nearest(v.first), true};
        }

        const_iterator insert(point_type position, value_type &&v) {
            rtree_.insert(std::make_pair(position, std::move(v)));
            clear_dominated(position);
            return {find_nearest(v.first), true};
        }

        template<class P>
        const_iterator insert(point_type position, P &&p) {
            insert(position, mapped_type(p));
            clear_dominated(position);
            return {find_nearest(position), true};
        }

        template<class InputIterator>
        void insert(InputIterator first, InputIterator last) {
            rtree_.insert(first, last);
            for (auto it = first; it != last; ++it) {
                clear_dominated(it->first);
            }
        }

        void insert(std::initializer_list<value_type> il) {
            for (auto &v : il) {
                rtree_.insert(v);
                clear_dominated(v);
            }
        }

        void erase(const value_type& v) {
            rtree_.remove(v);
        }

        size_type erase(const key_type& v) {
            value_type val = find_nearest(v);
            if (to_vector(val.first) == to_vector(v)) {
                rtree_.remove(val);
                return 1;
            } else {
                return 0;
            }
        }

        // const_iterator
        size_t erase(const_iterator position) {
            return rtree_.remove(*position);
        }

        //const_iterator
        size_t erase(const_iterator first, const_iterator last) {
            return rtree_.remove(first,last);
        }

        void clear() noexcept {
            rtree_.clear();
        }

        void merge(pareto_front<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE> &source) {
            insert(source.begin(), source.end());
        }

        void merge(pareto_front<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE> &&source) {
            insert(source.begin(), source.end());
            source.clear();
        }

        void swap(pareto_front<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE> &m) {
            m.rtree_.swap(rtree_);
        }

    public /* pareto operations */:
        const_iterator find_intersection(point_type min_corner, point_type max_corner) {
            box_type query_box(min_corner, max_corner);
            return rtree_.qbegin(geometry::index::intersects(query_box));
        }

        const_iterator find_nearest(point_type p) {
            return rtree_.qbegin(geometry::index::nearest(p, 1));
        }

        const_iterator find_nearest(point_type p, size_t k) {
            return rtree_.qbegin(geometry::index::nearest(p, k));
        }

        number_type hypervolume(point_type reference_point) {
            double hv_upper_limit = 1;
            auto m = to_vector(ideal());
            auto r = to_vector(reference_point);
            for (size_t i = 0; i < m.size(); ++i) {
                hv_upper_limit *= abs(r[i] - m[i]);
            }

            size_t max_monte_carlo_iter = 1000;
            size_t hit = 0;
            size_t miss = 0;
            for (size_t i = 0; i < max_monte_carlo_iter; ++i) {
                std::vector<number_type> rand(dimensions);
                for (size_t j = 0; j < dimensions; ++j) {
                    std::uniform_real_distribution<number_type> d(m[j],r[j]);
                    rand[j] = d(generator());
                }
                if (dominates(to_point(rand))) {
                    hit++;
                } else {
                    miss++;
                }
            }

            return hv_upper_limit * hit / (hit+miss);
        }

        number_type hypervolume() {
            return hypervolume(nadir());
        }

        bool dominates(point_type p) {
            return find_intersection(ideal(),p) != end();
        }

        point_type ideal() const {
            auto ideal = to_vector(begin()->first);
            for (auto it = begin(); it != end(); ++it) {
                auto cur = to_vector(it->first);
                for (size_t d = 0; d < dimensions; ++d) {
                    if (cur[d] < ideal[d]) {
                        ideal[d] = cur[d];
                    }
                }
            }
            return to_point(ideal);
        }

        point_type nadir() const {
            std::vector<std::vector<number_type>> ideals;
            for (size_t d = 0; d < dimensions; ++d) {
                auto dimension_ideal = to_vector(begin()->first);
                for (auto it = begin(); it != end(); ++it) {
                    auto cur = to_vector(it->first);
                    if (cur[d] < dimension_ideal[d]) {
                        dimension_ideal = cur;
                    }
                }
                ideals.emplace_back(dimension_ideal);
            }
            std::vector<number_type> nadir = ideals[0];
            for (auto it = begin(); it != end(); ++it) {
                for (size_t d = 0; d < dimensions; ++d) {
                    auto cur = to_vector(it->first);
                    if (cur[d] > nadir[d]) {
                        nadir[d] = cur[d];
                    }
                }
            }
            return to_point(nadir);
        }

        point_type worst() const {
            std::vector<number_type> worst_point = to_vector(begin()->first);
            for (auto it = begin(); it != end(); ++it) {
                for (size_t d = 0; d < dimensions; ++d) {
                    auto cur = to_vector(it->first);
                    if (cur[d] > worst_point[d]) {
                        worst_point[d] = cur[d];
                    }
                }
            }
            return to_point(worst_point);
        }

        const_iterator find(const key_type &k) const {
            auto it = find_nearest(k);
            return (k == it->first) ? it : end();
        }

        template<typename K>
        const_iterator find(const K &x) const {
            auto it = find_nearest(x);
            return (x == it->first) ? it : end();
        }

    private /* functions */:
        static std::vector<number_type> to_vector(const key_type& v) {
            std::vector<number_type> result;
            visit_point(v,[&](number_type p) {
                result.emplace_back(p);
            });
            return result;
        }

        static point_type to_point(const std::vector<number_type>& v) {
            point_type result;
            int i = 0;
            visit_point(result,[&](number_type& p) {
                p = v[i];
                i++;
            });
            return result;
        }


        template <size_t idx = 0, typename FUNCTION_T>
        static
        std::enable_if_t<idx < dimensions,void>
        visit_point(const key_type& k, FUNCTION_T fn) {
            fn(k.template get<idx>());
            visit_point<idx+1>(k,fn);
        }

        template <size_t idx, typename FUNCTION_T>
        static
        std::enable_if_t<idx == dimensions,void>
        visit_point(const key_type& k, FUNCTION_T fn) {}

        template <size_t idx = 0, typename FUNCTION_T>
        static
        std::enable_if_t<idx < dimensions,void>
        visit_point(key_type& k, FUNCTION_T fn) {
            number_type tmp = k.template get<idx>();
            fn(tmp);
            k.template set<idx>(tmp);
            visit_point<idx+1>(k,fn);
        }

        template <size_t idx, typename FUNCTION_T>
        static
        std::enable_if_t<idx == dimensions,void>
        visit_point(key_type& k, FUNCTION_T fn) {}

        static bool point_equal(const point_type& a, const point_type& b) {
            return (to_vector(a) == to_vector(b));
        }

        void clear_dominated(point_type v) {
            for (auto it = find_intersection(v,worst()); it != end(); ++it) {
                if (point_equal(it->first,v)) {
                    erase(it);
                }
            }
        }

        void clear_dominated() {
            for (const auto& p: rtree_) {
                clear_dominated(p);
            }
        }

        static std::mt19937& generator() {
            static std::mt19937 g((std::random_device()()) | std::chrono::high_resolution_clock::now().time_since_epoch().count());
            return g;
        }

    private:
        /// Pareto represented in a r-tree of points
        tree_type rtree_;

    };
}

#endif //PARETO_FRONTS_PARETO_FRONTS_H
