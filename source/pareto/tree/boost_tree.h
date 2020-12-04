#ifndef PARETO_FRONTS_PREDICATE_TREE_H
#define PARETO_FRONTS_PREDICATE_TREE_H

#include <boost/geometry/geometry.hpp>
#include <pareto/point.h>
#include <pareto/query/query_box.h>

namespace pareto {
    template<typename NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, typename ELEMENT_TYPE, typename TAG>
    class front;

    struct boost_tree_tag;

    /// This is a r-tree with all the operations we need using
    /// boost r-tree as underlying data structure.
    /// The boost r-tree is planned to work in lots of scenarios
    /// that we don't really need for pareto fronts. It also does
    /// not make good use of the custom allocators that are
    /// fundamental in small trees.
    template<typename NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, typename ELEMENT_TYPE, typename ALLOCATOR = std::allocator<std::pair<::pareto::point<NUMBER_TYPE, NUMBER_OF_DIMENSIONS>, ELEMENT_TYPE>>>
    class boost_tree {
    public:
        friend front<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, boost_tree_tag>;
        using self_type = boost_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>;
        using number_type = NUMBER_TYPE;
        static constexpr size_t number_of_compile_dimensions = NUMBER_OF_DIMENSIONS;
        using point_type = point<number_type, number_of_compile_dimensions>;
        using key_type = point_type;
        using mapped_type = ELEMENT_TYPE;
        using value_type = std::pair<key_type, mapped_type>;
        using box_type = query_box<number_type, number_of_compile_dimensions>;

        using tree_type = boost::geometry::index::rtree<value_type, boost::geometry::index::quadratic<16>>;
        using difference_type = typename tree_type::const_query_iterator::difference_type;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = value_type *;
        using const_pointer = value_type const *;

        constexpr static bool is_using_default_fast_allocator() {
            return false;
        }

        using node_allocator_type = ALLOCATOR;

    public /* iterators */:
        // This iterator keeps a reference to an rtree query iterators.
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
            friend boost_tree;
            using difference_type = typename self_type::difference_type;
            using value_type = std::pair<const key_type, mapped_type>;
            using reference = value_type &;
            using const_reference = value_type const &;
            using pointer = value_type *;
            using const_pointer = value_type const *;
            using iterator_category = std::forward_iterator_tag;
            using raw_query_iterator = typename tree_type::const_query_iterator;

            /// \brief Construct null iterator
            iterator() : query_it_(typename tree_type::const_query_iterator()) {}

            /// \brief Copy constructor
            iterator(const iterator &rhs) : query_it_(rhs.query_it_) {}

            /// \begin Create iterator from raw iterators
            /// raw_end is ignored here because all "ends" are the same object in
            /// the rtrees. This is important for compatibility with the vector
            /// iterators, where each vector has a different end iterator.
            iterator(const raw_query_iterator &raw_begin, const raw_query_iterator &raw_end)
                    : query_it_(raw_begin) {}

            /// \brief Destructor
            ~iterator() = default;

            /// \brief Copy attribution operator
            iterator &operator=(const iterator &rhs) {
                if (&rhs == this) {
                    return *this;
                }
                query_it_ = rhs.query_it_;
                return *this;
            }


            /// \brief Equality operator
            bool operator==(const iterator &rhs) const {
                return (query_it_ == rhs.query_it_);
            }

            /// \brief Inequality operator
            bool operator!=(const iterator &rhs) const {
                return !(this->operator==(rhs));
            }

            /// \brief Advance iterator
            iterator &operator++() {
                query_it_ = query_it_.operator++();
                return *this;
            }

            /// \brief Advance iterator with copy
            iterator operator++(int i) { // NOLINT(cert-dcl21-cpp): This is the expected return type for iterators
                auto tmp = *this;
                query_it_ = query_it_.operator++(i);
                return tmp;
            }

            /// \brief Go to previous iterator
            iterator &operator--() {
                throw std::logic_error("Boost r-trees don't support the operator-- for their iterators");
            }

            /// \brief Go to previous iterator
            iterator operator--(int i) { // NOLINT(cert-dcl21-cpp): This is the expected return type for iterators
                throw std::logic_error("Boost r-trees don't support the operator-- for their iterators");
            }

            /// We make a conversion where
            ///     const std::pair<key_type, mapped_type>
            /// becomes
            ///     std::pair<const key_type, mapped_type>&
            /// like it's the case with maps.
            /// The user cannot change the key because it would mess
            /// the data structure. But the user CAN change the key.
            reference operator*() const {
                const std::pair<key_type, mapped_type> &p = query_it_.operator*();
                auto *p2 = (std::pair<const key_type, mapped_type> *) &p;
                std::pair<const key_type, mapped_type> &p3 = *p2;
                return p3;
            }

            /// \brief Dereference iterator
            pointer operator->() const {
                const std::pair<key_type, mapped_type> &p = query_it_.operator*();
                auto *p2 = (std::pair<const key_type, mapped_type> *) &p;
                return p2;
            }

        private:
            /// \brief The original boost iterator we are wrapping
            raw_query_iterator query_it_{};

        };

        /// Iterator wraps a tree_type::const_query_iterator
        class const_iterator {
        public:
            friend boost_tree;
            using difference_type = typename self_type::difference_type;
            using value_type = const std::pair<key_type, mapped_type>;
            using reference = value_type &;
            using const_reference = value_type const &;
            using pointer = value_type *;
            using const_pointer = value_type const *;
            using iterator_category = std::forward_iterator_tag;
            using raw_query_iterator = typename tree_type::const_query_iterator;

            /// \brief Construct null iterator
            const_iterator() {
                query_it_ = typename tree_type::const_query_iterator();
            }

            /// \brief Copy constructor
            const_iterator(const const_iterator &rhs) : query_it_(rhs.query_it_) {}

            /// \brief Copy constructor from non-const iterator
            const_iterator(const iterator &rhs) : query_it_(
                    rhs.query_it_) {} // NOLINT(google-explicit-constructor): Iterators should be implicitly convertible

            /// \begin Create iterator from raw iterators
            /// raw_end is ignored here because all "ends" are the same object in
            /// the rtrees. This is important for compatibility with the vector
            /// iterators, where each vector has a different end iterator.
            const_iterator(const raw_query_iterator &raw_begin, const raw_query_iterator &raw_end)
                    : query_it_(raw_begin) {}

            /// \brief Destructor
            ~const_iterator() = default;

            /// \brief Copy attribution operator
            const_iterator &operator=(const const_iterator &rhs) {
                query_it_ = rhs.query_it_;
                return *this;
            }

            /// \brief Equality operator
            bool operator==(const const_iterator &rhs) const {
                return (query_it_ == rhs.query_it_);
            }

            /// \brief Inequality operator
            bool operator!=(const const_iterator &rhs) const {
                return !(this->operator==(rhs));
            }

            /// \brief Advance iterator
            const_iterator &operator++() {
                query_it_ = query_it_.operator++();
                return *this;
            }

            /// \brief Advance iterator with copy
            const_iterator operator++(int i) { // NOLINT(cert-dcl21-cpp): This is the expected return type for iterators
                auto tmp = *this;
                query_it_ = query_it_.operator++(i);
                return tmp;
            }

            /// \brief Go to previous element
            const_iterator &operator--() {
                throw std::logic_error("Boost r-trees don't implement operator-- for their iterators");
            }

            /// \brief Go to previous element
            const_iterator operator--(int i) { // NOLINT(cert-dcl21-cpp): This is the expected return type for iterators
                throw std::logic_error("Boost r-trees don't implement operator-- for their iterators");
            }

            /// \brief Dereference iterator
            reference operator*() const {
                return query_it_.operator*();
            }

            /// \brief Dereference iterator
            pointer operator->() const {
                return query_it_.operator->();
            }

            /// \brief Get reference to i-th element
            reference operator[](size_t i) const {
                query_it_ = query_it_.operator[](i);
                return *this;
            }

        private:
            /// \brief Original boost iterator we are wrapping
            raw_query_iterator query_it_;
        };

    public /* constructors */:
        /// \brief Default constructor / empty tree
        boost_tree() = default;

        /// \brief Constructor using an external allocator
        /// Only archives should use this
        /// Boost trees can't use our memory pools so we do nothing here
        explicit boost_tree(std::shared_ptr<node_allocator_type> &external_allocator)
                : boost_tree() {}

        /// \brief Construct from iterators
        template<class InputIterator>
        boost_tree(InputIterator first, InputIterator last)
                : data_(first, last) {}


    public /* iterators */:
        /// \brief Get iterator to first element
        const_iterator begin() const noexcept {
            return const_iterator(data_.qbegin(boost::geometry::index::satisfies([](auto const &x) { return true; })),
                                  data_.qend());
        }

        /// \brief Get iterator to last + 1 element
        const_iterator end() const noexcept {
            return const_iterator(data_.qend(), data_.qend());
        }

        /// \brief Get iterator to first element
        iterator begin() noexcept {
            return iterator(data_.qbegin(boost::geometry::index::satisfies([](auto const &x) { return true; })),
                            data_.qend());
        }

        /// \brief Get iterator to last + 1 element
        iterator end() noexcept {
            return iterator(data_.qend(), data_.qend());
        }

        /// \brief Equality operator
        bool operator==(const boost_tree &rhs) const {
            return std::equal(data_.begin(), data_.end(), rhs.data_.begin(), rhs.data_.end());
        }

        /// \brief Inequality operator
        bool operator!=(const boost_tree &rhs) const {
            return !(rhs.operator==(*this));
        }

        /// \brief Find point
        const_iterator find(const point_type &p) const {
            boost::geometry::model::box<point_type> query_box(p, p);
            return const_iterator(data_.qbegin(boost::geometry::index::intersects(query_box)), data_.qend());
        }

        /// \brief Find point
        iterator find(const point_type &p) {
            boost::geometry::model::box<point_type> query_box(p, p);
            return iterator(data_.qbegin(boost::geometry::index::intersects(query_box)), data_.qend());
        }

        /// \brief Find intersection between points and query box
        const_iterator begin_intersection(const point_type &min_corner, const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_, max_corner_);
            return const_iterator(data_.qbegin(boost::geometry::index::intersects(query_box)), data_.qend());
        }

        /// \brief Find intersection between points and query box that satisfy a predicate
        template<class PREDICATE_TYPE>
        const_iterator
        begin_intersection(const point_type &min_corner, const point_type &max_corner, PREDICATE_TYPE fn) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_, max_corner_);
            return const_iterator(data_.qbegin(boost::geometry::index::intersects(query_box)
                                               && boost::geometry::index::satisfies(fn)), data_.qend());
        }

        /// \brief Find points within a query box
        const_iterator begin_within(const point_type &min_corner, const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_, max_corner_);
            return const_iterator(data_.qbegin(boost::geometry::index::within(query_box)), data_.qend());
        }

        /// \brief Find points within a query box
        template<class PREDICATE_TYPE>
        const_iterator
        begin_within(const point_type &min_corner, const point_type &max_corner, PREDICATE_TYPE fn) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_, max_corner_);
            return const_iterator(data_.qbegin(boost::geometry::index::within(query_box)
                                               && boost::geometry::index::satisfies(fn)), data_.qend());
        }

        /// \brief Find points outside a query box
        const_iterator begin_disjoint(const point_type &min_corner, const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_, max_corner_);
            return const_iterator(data_.qbegin(boost::geometry::index::disjoint(query_box)), data_.qend());
        }

        /// \brief Find points outside a query box
        template<class PREDICATE_TYPE>
        const_iterator
        begin_disjoint(const point_type &min_corner, const point_type &max_corner, PREDICATE_TYPE fn) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_, max_corner_);
            return const_iterator(data_.qbegin(boost::geometry::index::disjoint(query_box)
                                               && boost::geometry::index::satisfies(fn)), data_.qend());
        }

        /// \brief Find points closest to a reference point
        const_iterator begin_nearest(const point_type &p) const {
            return const_iterator(data_.qbegin(boost::geometry::index::nearest(p, 1)), data_.qend());
        }

        /// \brief Find points closest to a reference point
        const_iterator begin_nearest(const point_type &p, size_t k) const {
            return const_iterator(data_.qbegin(boost::geometry::index::nearest(p, k)), data_.qend());
        }

        /// \brief Find points closest to a reference point
        template<class PREDICATE_TYPE>
        const_iterator begin_nearest(const point_type &p, size_t k, PREDICATE_TYPE fn) const {
            return const_iterator(data_.qbegin(boost::geometry::index::nearest(p, k)
                                               && boost::geometry::index::satisfies(fn)), data_.qend());
        }

        /// \brief Find points closest to a reference point
        const_iterator begin_nearest(const box_type &p, size_t k) const {
            boost::geometry::model::box<point_type> boost_box(p.first(), p.second());
            return const_iterator(data_.qbegin(boost::geometry::index::nearest(boost_box, k)), data_.qend());
        }

        template<class PREDICATE_TYPE>
        const_iterator begin_nearest(const box_type &p, size_t k, PREDICATE_TYPE fn) const {
            boost::geometry::model::box<point_type> boost_box(p.first(), p.second());
            return const_iterator(data_.qbegin(boost::geometry::index::nearest(boost_box, k)
                                               && boost::geometry::index::satisfies(fn)), data_.qend());
        }

    public /* non-modifying functions */:
        /// \brief True if container is empty
        [[nodiscard]] bool empty() const noexcept {
            return data_.empty();
        }

        /// \brief Get container size
        [[nodiscard]] size_t size() const noexcept {
            return data_.size();
        }

        /// \brief Get container dimensions
        [[nodiscard]] size_t dimensions() const noexcept {
            return NUMBER_OF_DIMENSIONS;
        }

        /// \brief Get maximum value in a given dimension
        number_type max_value(size_t dimension) const {
            auto it = std::max_element(begin(), end(), [&dimension](const value_type &a, const value_type &b) {
                return a.first[dimension] < b.first[dimension];
            });
            if (it != end()) {
                return it->first[dimension];
            }
            return std::numeric_limits<number_type>::min();
        }

        /// \brief Get iterator to element with maximum value in a given dimension
iterator max_element(size_t dimension) {
            return std::max_element(begin(), end(), [&dimension](const value_type &a, const value_type &b) {
                return a.first[dimension] < b.first[dimension];
            });
        }

        /// \brief Get iterator to element with maximum value in a given dimension
        const_iterator max_element(size_t dimension) const {
            return std::max_element(begin(), end(), [&dimension](const value_type &a, const value_type &b) {
                return a.first[dimension] < b.first[dimension];
            });
        }

/// \brief Get minimum value in a given dimension
        number_type min_value(size_t dimension) const {
            auto it = std::min_element(begin(), end(), [&dimension](const value_type &a, const value_type &b) {
                return a.first[dimension] < b.first[dimension];
            });
            if (it != end()) {
                return it->first[dimension];
            }
            return std::numeric_limits<number_type>::min();
        }

        /// \brief Get iterator to element with minimum value in a given dimension
iterator min_element(size_t dimension) {
            return std::min_element(begin(), end(), [&dimension](const value_type &a, const value_type &b) {
                return a.first[dimension] < b.first[dimension];
            });
        }

        /// \brief Get iterator to element with minimum value in a given dimension
        const_iterator min_element(size_t dimension) const {
            return std::min_element(begin(), end(), [&dimension](const value_type &a, const value_type &b) {
                return a.first[dimension] < b.first[dimension];
            });
        }

    public /* modifying functions */:
/// \brief Insert element pair
        /// Insertion removes any point dominated by the point
        /// before inserting the element in the rtree
        /// \param v Pair value <point, value>
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        std::pair<iterator, bool> insert(const value_type &v) {
            data_.insert(v);
            auto it = find(v.first);
            return {it, it != end()};
        }

        /// \brief Insert list of elements
        template<class InputIterator>
        void insert(InputIterator first, InputIterator last) {
            data_.insert(first, last);
        }

        /// Erase element
        size_t erase(const_iterator position) {
            if (position != end()) {
                return data_.remove(*find(position->first));
            } else {
                return 0;
            }
        }

        /// Remove range of iterators from the front
        size_t erase(const_iterator first, const_iterator last) {
            // get copy of all elements in the query
            std::vector<value_type> v(first.query_it_, last.query_it_);
            // remove using these copies
            return data_.remove(v.begin(), v.end());
        }

        /// Clear the front
        void clear() noexcept {
            data_.clear();
        }

        /// Swap the content of two fronts
        void swap(self_type &m) {
            m.data_.swap(data_);
        }

    private:
        boost::geometry::index::rtree<value_type, boost::geometry::index::quadratic<16>> data_;

    };
}

#endif // PARETO_FRONTS_PREDICATE_TREE_H

