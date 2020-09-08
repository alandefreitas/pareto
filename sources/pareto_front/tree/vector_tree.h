#ifndef PARETO_FRONTS_PREDICATE_VECTOR_H
#define PARETO_FRONTS_PREDICATE_VECTOR_H

#include <pareto_front/point.h>
#include <pareto_front/memory_pool.h>
#include <vector>

namespace pareto {

    template <typename NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, typename ELEMENT_TYPE, typename TAG>
    class front;

    struct vector_tree_tag;

    /// \brief This is a vector emulating a tree
    /// For all operations, we iterate through the vector looking for elements
    /// that match the predicates.
    template <typename NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, typename ELEMENT_TYPE, typename ALLOCATOR = std::allocator<std::pair<::pareto::point<NUMBER_TYPE, NUMBER_OF_DIMENSIONS>, ELEMENT_TYPE>>>
    class vector_tree {
    public:
        friend front<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, vector_tree_tag>;
        using self_type = vector_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>;
        using number_type = NUMBER_TYPE;
        static constexpr size_t number_of_compile_dimensions = NUMBER_OF_DIMENSIONS;
        using point_type = point<number_type, number_of_compile_dimensions>;
        using key_type = point_type;
        using mapped_type = ELEMENT_TYPE;
        using value_type = std::pair<key_type, mapped_type>;
        using box_type = query_box<number_type, number_of_compile_dimensions>;

        using vector_type = std::vector<value_type, ALLOCATOR>;

        constexpr static bool is_using_default_fast_allocator() {
            return false;
        }

        using node_allocator_type = ALLOCATOR;

        using difference_type = typename vector_type::difference_type;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = value_type *;
        using const_pointer = value_type const *;

    public /* iterators */:
        // // This iterator keeps a reference to vector iterators and a predicate.
        // What these iterators do is skip the elements that don't match our
        // queries / predicates. This is very useful to give us a nice interface
        // to query the pareto front because we can create good iterators of this kind
        // for rtrees. There are not as efficient for vectors, but they keep a common
        // interface with the case that matters the most.
        // https://stackoverflow.com/questions/7758580/writing-your-own-stl-container/7759622#7759622
        class const_iterator;
        class iterator {
        public:
            friend const_iterator;
            friend vector_tree;
            using difference_type = typename self_type::difference_type;
            using value_type = std::pair<const key_type, mapped_type>;
            using reference = value_type &;
            using const_reference = value_type const &;
            using pointer = value_type *;
            using const_pointer = value_type const *;
            using iterator_category = std::bidirectional_iterator_tag;
            using raw_vector_iterator = typename vector_type::iterator;

            iterator() : query_it_(raw_vector_iterator()) {}

            iterator(vector_type &source, std::function<bool(const value_type&)> fn)
                    : query_it_(source.begin()), query_it_end_(source.end()), query_function_(fn) {
                maybe_advance_predicate();
            }

            iterator(vector_type &source) : query_it_(source.begin()), query_it_end_(source.end()) {}

            iterator(const raw_vector_iterator &raw_begin_it, const raw_vector_iterator &raw_end_it) : query_it_(raw_begin_it), query_it_end_(raw_end_it) {}

            iterator(const iterator& rhs)
                    : query_it_(rhs.query_it_), query_it_end_(rhs.query_it_end_), query_function_(rhs.query_function_) {
                maybe_advance_predicate();
            }

            ~iterator() = default;

            iterator &operator=(const iterator &rhs) {
                query_it_ = rhs.query_it_;
                query_it_end_ = rhs.query_it_end_;
                query_function_ = rhs.query_function_;
                return *this;
            }

            bool operator==(const iterator &rhs) const {
                return (query_it_ == rhs.query_it_);
            }

            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            iterator &operator++() {
                query_it_ = query_it_.operator++();
                maybe_advance_predicate();
                return *this;
            }

            iterator operator++(int i) {
                auto tmp = *this;
                query_it_ = query_it_.operator++(i);
                maybe_advance_predicate();
                return tmp;
            }

            iterator &operator--() {
                query_it_ = query_it_.operator--();
                maybe_rewind_predicate();
                return *this;
            }

            iterator operator--(int i) {
                auto tmp = *this;
                query_it_ = query_it_.operator--(i);
                maybe_rewind_predicate();
                return tmp;
            }

            /// We make a conversion where
            ///     const std::pair<key_type, mapped_type>
            /// becomes
            ///     std::pair<const key_type, mapped_type>&
            /// like it's the case with maps.
            /// The user cannot change the key because it would mess
            /// the data structure. But the user CAN change the key.
            reference operator*() const {
                const std::pair<key_type, mapped_type>& p = query_it_.operator*();
                std::pair<const key_type, mapped_type>* p2 = (std::pair<const key_type, mapped_type>*) &p;
                std::pair<const key_type, mapped_type>& p3 = *p2;
                return p3;
            }

            pointer operator->() const {
                const std::pair<key_type, mapped_type>& p = query_it_.operator*();
                std::pair<const key_type, mapped_type>* p2 = (std::pair<const key_type, mapped_type>*) &p;
                return p2;
            }

            reference operator[](size_t i) const {
                const std::pair<key_type, mapped_type>& p = query_it_.operator*();
                std::pair<const key_type, mapped_type>* p2 = (std::pair<const key_type, mapped_type>*) &p;
                return *p2;
            }

        private /* functions */:
            void maybe_advance_predicate() {
                if (query_function_) {
                    while (query_it_ != query_it_end_ && !query_function_(*query_it_)) {
                        query_it_ = query_it_.operator++();
                    }
                }
            }

            void maybe_rewind_predicate() {
                if (query_function_) {
                    while (query_it_ != query_it_end_ && !query_function_(*query_it_)) {
                        query_it_ = query_it_.operator--();
                    }
                }
            }

        private:
            raw_vector_iterator query_it_;
            raw_vector_iterator query_it_end_;
            std::function<bool(const value_type&)> query_function_;

        };

        /// Iterator wraps a vector_type::const_iterator
        class const_iterator {
        public:
            friend vector_tree;
            using difference_type = typename self_type::difference_type;
            using value_type = const std::pair<key_type, mapped_type>;
            using reference = value_type &;
            using const_reference = value_type const &;
            using pointer = value_type *;
            using const_pointer = value_type const *;
            using iterator_category = std::bidirectional_iterator_tag;
            using raw_vector_iterator = typename vector_type::const_iterator;

            const_iterator() : query_it_(raw_vector_iterator()) {}

            const_iterator(const vector_type &source, std::function<bool(const value_type&)> fn)
                    : query_it_(source.begin()), query_it_end_(source.end()), query_function_(fn) {
                maybe_advance_predicate();
            }

            const_iterator(const const_iterator &rhs)
                : query_it_(rhs.query_it_), query_it_end_(rhs.query_it_end_), query_function_(rhs.query_function_) {
                maybe_advance_predicate();
            }

            const_iterator(const iterator &rhs)
                : query_it_(rhs.query_it_), query_it_end_(rhs.query_it_end_), query_function_(rhs.query_function_) {
                maybe_advance_predicate();
            }

            const_iterator(const raw_vector_iterator &raw_begin_it, const raw_vector_iterator &raw_end_it) : query_it_(raw_begin_it), query_it_end_(raw_end_it) {}

            ~const_iterator() = default;

            const_iterator &operator=(const const_iterator &rhs) {
                query_it_ = rhs.query_it_;
                query_it_end_ = rhs.query_it_end_;
                query_function_ = rhs.query_function_;
                return *this;
            }

            bool operator==(const const_iterator &rhs) const {
                return (query_it_ == rhs.query_it_);
            }

            bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }

            const_iterator &operator++() {
                query_it_ = query_it_.operator++();
                maybe_advance_predicate();
                return *this;
            }

            const_iterator operator++(int i) {
                auto tmp = *this;
                query_it_ = query_it_.operator++(i);
                maybe_advance_predicate();
                return tmp;
            }

            const_iterator &operator--() {
                query_it_ = query_it_.operator--();
                maybe_rewind_predicate();
                return *this;
            }

            const_iterator operator--(int i) {
                auto tmp = *this;
                query_it_ = query_it_.operator--(i);
                maybe_rewind_predicate();
                return tmp;
            }

            reference operator*() const {
                return query_it_.operator*();
            }

            pointer operator->() const {
                return query_it_.operator->();
            }

            const_reference operator[](size_t i) const {
                return query_it_.operator[](i);
            }

        private:
            void maybe_advance_predicate() {
                if (query_function_) {
                    while (query_it_ != query_it_end_ && !query_function_(*query_it_)) {
                        query_it_ = query_it_.operator++();
                    }
                }
            }

            void maybe_rewind_predicate() {
                if (query_function_) {
                    while (query_it_ != query_it_end_ && !query_function_(*query_it_)) {
                        query_it_ = query_it_.operator--();
                    }
                }
            }

        private:
            raw_vector_iterator query_it_;
            raw_vector_iterator query_it_end_;
            std::function<bool(const value_type&)> query_function_;
        };

    public /* constructors */:
        vector_tree() = default;

        /// \brief Constructor using an external allocator
        /// Only archives should use this
        /// Vector trees can't have memory pools so we do nothing here
        vector_tree(std::shared_ptr<node_allocator_type>& external_allocator)
                : vector_tree() {}


        template<class InputIterator>
        vector_tree(InputIterator first, InputIterator last)
         : data_(first,last) {}


    public /* iterators */:
        const_iterator begin() const noexcept {
            return const_iterator(data_.begin(), data_.end());
        }

        const_iterator end() const noexcept {
            return const_iterator(data_.end(), data_.end());
        }

        iterator begin() noexcept {
            return iterator(data_.begin(), data_.end());
        }

        iterator end() noexcept {
            return iterator(data_.end(), data_.end());
        }

        bool operator==(const vector_tree &rhs) const {
            return data_ == rhs.data_;
        }

        bool operator!=(const vector_tree &rhs) const {
            return !(rhs == *this);
        }

        const_iterator find(const point_type& p) const {
            auto vec_begin = std::find_if(data_.begin(), data_.end(), [&p](const value_type& v) { return v.first == p; });
            return const_iterator(vec_begin, data_.end());
        }

        iterator find(const point_type& p) {
            auto vec_begin = std::find_if(data_.begin(), data_.end(), [&p](const value_type& v) { return v.first == p; });
            return iterator(vec_begin, data_.end());
        }

        const_iterator begin_intersection(const point_type& min_corner, const point_type& max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            box_type query_box(min_corner_, max_corner_);
            return const_iterator(data_, [query_box, this](const value_type& p) {
                return intersects(p.first, query_box);
            });
        }

        template <class PREDICATE_TYPE>
        const_iterator begin_intersection(const point_type& min_corner, const point_type& max_corner, PREDICATE_TYPE fn) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            box_type query_box(min_corner_, max_corner_);
            return const_iterator(data_, [query_box, fn, this](const value_type& p) {
                return intersects(p.first, query_box) && fn(p);
            });
        }

        const_iterator begin_within(const point_type& min_corner, const point_type& max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            box_type query_box(min_corner_, max_corner_);
            return const_iterator(data_, [query_box, this](const value_type& p) {
                return is_within(p.first, query_box);
            });
        }

        template <class PREDICATE_TYPE>
        const_iterator begin_within(const point_type& min_corner, const point_type& max_corner, PREDICATE_TYPE fn) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            box_type query_box(min_corner_, max_corner_);
            return const_iterator(data_, [query_box, this, fn](const value_type& p) {
                return is_within(p.first, query_box) && fn(p);
            });
        }

        const_iterator begin_disjoint(const point_type& min_corner, const point_type& max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            box_type query_box(min_corner_, max_corner_);
            return const_iterator(data_, [query_box, this](const value_type& p) {
                return is_disjoint(p.first, query_box);
            });
        }

        template <class PREDICATE_TYPE>
        const_iterator begin_disjoint(const point_type& min_corner, const point_type& max_corner, PREDICATE_TYPE fn) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            box_type query_box(min_corner_, max_corner_);
            return const_iterator(data_, [query_box, this, fn](const value_type& p) {
                return is_disjoint(p.first, query_box) && fn(p);
            });
        }

        const_iterator begin_nearest(const point_type& p) const {
            auto it = std::min_element(data_.begin(), data_.end(), [this, p](const value_type& v1, const value_type& v2) {
                return p.distance(v1.first) < p.distance(v2.first);
            });
            if (it != data_.end()) {
                auto nearest = it->first;
                return const_iterator(data_, [nearest](const value_type& p) {
                    return p.first == nearest;
                });
            }
            return end();
        }

        /// Find k nearest points
        /// This is VERY ineffient with vectors.
        /// This could be improved BUT...
        ///     users shouldn't be using vectors for this.
        const_iterator begin_nearest(const point_type& p, size_t k) const {
            if (k == 1) {
                return begin_nearest(p);
            }
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(data_copy.begin(), data_copy.begin() + k, data_copy.end(), [this, p](value_type v1, value_type v2) {
                return p.distance(v1.first) < p.distance(v2.first);
            });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin(); it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return const_iterator(data_, [nearest_set](const value_type& p) {
                return std::find(nearest_set.begin(), nearest_set.end(), p.first) != nearest_set.end();
            });
        }

        template <class PREDICATE_TYPE>
        const_iterator begin_nearest(const point_type& p, size_t k, PREDICATE_TYPE fn) const {
            if (k == 1) {
                return begin_nearest(p);
            }
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(data_copy.begin(), data_copy.begin() + k, data_copy.end(), [this, p](value_type v1, value_type v2) {
                return p.distance(v1.first) < p.distance(v2.first);
            });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin(); it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return const_iterator(data_, [nearest_set, fn](const value_type& p) {
                return std::find(nearest_set.begin(), nearest_set.end(), p.first) != nearest_set.end() && fn(p);
            });
        }

        const_iterator begin_nearest(const box_type& p, size_t k) const {
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(data_copy.begin(), data_copy.begin() + k, data_copy.end(), [this, p](value_type v1, value_type v2) {
                return p.distance(v1.first) < p.distance(v2.first);
            });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin(); it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return const_iterator(data_, [nearest_set](const value_type& p) {
                return std::find(nearest_set.begin(), nearest_set.end(), p.first) != nearest_set.end();
            });
        }

        template <class PREDICATE_TYPE>
        const_iterator begin_nearest(const box_type& p, size_t k, PREDICATE_TYPE fn) const {
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(data_copy.begin(), data_copy.begin() + k, data_copy.end(), [this, p](value_type v1, value_type v2) {
                return distance(v1.first, p) < distance(v2.first, p);
            });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin(); it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return const_iterator(data_, [nearest_set, fn](const value_type& p) {
                return std::find(nearest_set.begin(), nearest_set.end(), p.first) != nearest_set.end() && fn(p);
            });
        }

    public /* non-modifying functions */:
        bool empty() const noexcept {
            return data_.empty();
        }

        size_t size() const noexcept {
            return data_.size();
        }

        size_t dimensions() const noexcept {
            if constexpr (NUMBER_OF_DIMENSIONS != 0) {
                return NUMBER_OF_DIMENSIONS;
            } else {
                return data_.front().first.dimensions();
            }
        }

        number_type max_value(size_t dimension) const {
            auto it = std::max_element(begin(), end(), [&dimension](const value_type& a, const value_type& b) {
                return a.first[dimension] < b.first[dimension];
            });
            if (it != end()) {
                return it->first[dimension];
            }
            return std::numeric_limits<number_type>::min();
        }

        iterator max_element(size_t dimension) {
            return std::max_element(begin(), end(), [&dimension](const value_type& a, const value_type& b) {
                return a.first[dimension] < b.first[dimension];
            });
        }

        const_iterator max_element(size_t dimension) const {
            return std::max_element(begin(), end(), [&dimension](const value_type& a, const value_type& b) {
                return a.first[dimension] < b.first[dimension];
            });
        }

        number_type min_value(size_t dimension) const {
            auto it = std::min_element(begin(), end(), [&dimension](const value_type& a, const value_type& b) {
                return a.first[dimension] < b.first[dimension];
            });
            if (it != end()) {
                return it->first[dimension];
            }
            return std::numeric_limits<number_type>::min();
        }

        iterator min_element(size_t dimension) {
            return std::min_element(begin(), end(), [&dimension](const value_type& a, const value_type& b) {
                return a.first[dimension] < b.first[dimension];
            });
        }

        const_iterator min_element(size_t dimension) const {
            return std::min_element(begin(), end(), [&dimension](const value_type& a, const value_type& b) {
                return a.first[dimension] < b.first[dimension];
            });
        }

    public /* modifying functions */:
        /// \brief Insert element pair
        std::pair<iterator, bool> insert(const value_type &v) {
            data_.emplace_back(v);
            auto it = data_.end();
            --it;
            return {iterator(it, data_.end()), true};
        }

        /// \brief Insert list of elements
        template<class InputIterator>
        void insert(InputIterator first, InputIterator last) {
            data_.insert(data_.end(), first, last);
        }

        /// Erase element
        size_t erase(const_iterator position) {
            if (position != end()) {
                data_.erase(position.query_it_);
                return 1;
            } else {
                return 0;
            }
        }

        /// Remove range of iterators from the front
        size_t erase(const_iterator first, const_iterator last) {
            // get copy of all elements in the query
            std::vector<value_type> v(first, last);
            // remove using these copies as reference
            for (const value_type& x: v) {
                data_.erase(std::find(data_.begin(), data_.end(), x));
            }
            return v.size();
        }

        /// Clear the front
        void clear() noexcept {
            data_.clear();
        }

        /// Swap the content of two fronts
        void swap(self_type &m) {
            m.data_.swap(data_);
        }

    private /* functions */:
        bool intersects(const point_type& p, const box_type& b) const {
            for (size_t i = 0; i < p.dimensions(); ++i) {
                if (p[i] > b.second()[i] || p[i] < b.first()[i]) {
                    return false;
                }
            }
            return true;
        }

        bool is_within(const point_type& p, const box_type& b) const {
            for (size_t i = 0; i < p.dimensions(); ++i) {
                if (p[i] >= b.second()[i] || p[i] <= b.first()[i]) {
                    return false;
                }
            }
            return true;
        }

        bool is_disjoint(const point_type& p, const box_type& b) const {
            for (size_t i = 0; i < p.dimensions(); ++i) {
                if (p[i] <= b.second()[i] || p[i] >= b.first()[i]) {
                    return false;
                }
            }
            return true;
        }


    private:
        vector_type data_;

    };

}

#endif // PARETO_FRONTS_PREDICATE_VECTOR_H

