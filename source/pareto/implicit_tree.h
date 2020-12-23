#ifndef PARETO_FRONTS_PREDICATE_VECTOR_H
#define PARETO_FRONTS_PREDICATE_VECTOR_H

#include <functional>
#include <memory>
#include <vector>

#include <pareto/common/default_allocator.h>
#include <pareto/point.h>
#include <pareto/query/predicate_list.h>
#include <pareto/query/query_box.h>

namespace pareto {

    /// \brief This is a vector emulating an associative spatial container
    /// For all operations, we iterate through the vector looking for elements
    /// that match the predicates.
    /// This container type implements the following concepts
    /// Container: https://en.cppreference.com/w/cpp/named_req/Container
    /// ReversibleContainer:
    /// https://en.cppreference.com/w/cpp/named_req/Container
    /// AssociativeContainer:
    /// https://en.cppreference.com/w/cpp/named_req/AssociativeContainer
    ///
    /// \tparam K Number/key type
    /// \tparam M Number of dimensions
    /// \tparam E Element/mapped type
    /// \tparam C Comparison function type in one dimension
    /// \tparam A Allocator type
    template <class K, size_t M, class T, typename C = std::less<K>,
              class A = std::allocator<std::pair<const point<K, M>, T>>>
    class implicit_tree {
      private /* Internal types */:
        using unprotected_point_type = point<K, M>;
        using protected_point_type = const point<K, M>;
        using unprotected_mapped_type = T;
        using unprotected_key_type = unprotected_point_type;
        using protected_key_type = protected_point_type;
        using unprotected_value_type =
            std::pair<unprotected_key_type, unprotected_mapped_type>;
        using protected_value_type =
            std::pair<protected_key_type, unprotected_mapped_type>;
        using unprotected_allocator_type = typename std::allocator_traits<
            A>::template rebind_alloc<unprotected_value_type>;
        using unprotected_vector_type =
            std::vector<unprotected_value_type, unprotected_allocator_type>;
        using unprotected_vector_iterator =
            typename unprotected_vector_type::iterator;
        using unprotected_vector_const_iterator =
            typename unprotected_vector_type::const_iterator;
        using point_type = unprotected_point_type;

      public /* Forward declarations */:
        template <bool is_const> class iterator_impl;

      public /* Container Concept */:
        using value_type = protected_value_type;
        using reference = value_type &;
        using const_reference = value_type const &;
        using iterator = iterator_impl<false>;
        using const_iterator = iterator_impl<true>;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using difference_type =
            typename unprotected_vector_type::difference_type;
        using size_type = typename unprotected_vector_type::size_type;

      public /* ReversibleContainer Concept */:
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      public /* AssociativeContainer Concept */:
        using key_type = unprotected_point_type;
        using mapped_type = T;
        using key_compare =
            std::function<bool(const key_type &, const key_type &)>;
        using value_compare =
            std::function<bool(const value_type &, const value_type &)>;

      public /* AllocatorAwareContainer Concept */:
        using allocator_type = A;

      public /* SpatialContainer Concept */:
        static constexpr size_t number_of_compile_dimensions = M;
        using dimension_type = K;
        using dimension_compare = C;
        using box_type =
            query_box<dimension_type, number_of_compile_dimensions>;
        using predicate_list_type =
            predicate_list<dimension_type, number_of_compile_dimensions, T>;

      public /* Iterators */:
        /// \brief Vector containers iterator
        /// This iterator keeps a reference to vector iterators and a
        /// predicate. What these iterators do is skip the elements that don't
        /// match our queries / predicates. This is very useful to give us a
        /// nice interface to query the pareto front because we can create good
        /// iterators of this kind for rtrees. There are not as efficient for
        /// vectors, but they keep a common interface with the case that matters
        /// the most.
        /// https://stackoverflow.com/questions/7758580/writing-your-own-stl-container/7759622#7759622
        template <bool is_const> class iterator_impl {
          private /* Internal Types */:
            template <class TYPE, class CONST_TYPE>
            using const_toggle =
                std::conditional_t<!is_const, TYPE, CONST_TYPE>;

            template <class U>
            using maybe_add_const = const_toggle<U, std::add_const_t<U>>;

            using vector_iterator =
                const_toggle<unprotected_vector_iterator,
                             unprotected_vector_const_iterator>;

            using vector_iterator_unconst = unprotected_vector_iterator;
            using vector_iterator_const = unprotected_vector_const_iterator;

            using query_function = std::function<bool(const value_type &)>;

          public /* LegacyIterator Types */:
            using value_type = maybe_add_const<implicit_tree::value_type>;
            using reference = const_toggle<implicit_tree::reference,
                                           implicit_tree::const_reference>;
            using difference_type = implicit_tree::difference_type;
            using pointer = const_toggle<implicit_tree::pointer,
                                         implicit_tree::const_pointer>;
            using iterator_category = std::bidirectional_iterator_tag;

          public /* LegacyIterator Constructors */:
            /// \brief Copy constructor
            /// This is the trickiest part, because const_iterator can be
            /// constructed from const_iterator and iterator. But iterator
            /// cannot be constructed from const_iterator.
            /// The template below handles that
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            // NOLINTNEXTLINE(google-explicit-constructor)
            iterator_impl(const iterator_impl<rhs_is_const> &rhs)
                : query_it_(rhs.query_it_), query_it_end_(rhs.query_it_end_),
                  query_function_(rhs.query_function_) {}

            /// \brief Copy assignment
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            iterator_impl &operator=(const iterator_impl<rhs_is_const> &rhs) {
                query_it_ = rhs.query_it_;
                query_it_end_ = rhs.query_it_end_;
                query_function_ = rhs.query_function_;
                return *this;
            }

            /// \brief Destructor
            ~iterator_impl() = default;

          public /* LegacyForwardIterator Constructors */:
            /// \brief Default constructor
            iterator_impl() : query_it_(vector_iterator()) {}

          public /* ContainerConcept Constructors */:
            /// \brief Convert to const iterator
            // NOLINTNEXTLINE(google-explicit-constructor)
            operator iterator_impl<true>() {
                const_iterator r(query_it_, query_it_end_);
                r.query_function_ = query_function_;
                return r;
            }

          public /* SpatialContainer Concept Constructors */:
            /// \brief Move constructor
            /// Move constructors are important for spatial containers
            /// because iterators *might* contain large lists of predicates
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            // NOLINTNEXTLINE(google-explicit-constructor)
            iterator_impl(iterator_impl<rhs_is_const> &&rhs)
                : query_it_(rhs.query_it_), query_it_end_(rhs.query_it_end_),
                  query_function_(rhs.query_function_) {}

            /// \brief Move assignment
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            iterator_impl &operator=(iterator_impl<rhs_is_const> &&rhs) {
                query_it_ = rhs.query_it_;
                query_it_end_ = rhs.query_it_end_;
                query_function_ = rhs.query_function_;
                return *this;
            }

          public /* Internal Constructors / Used by Container */:
            /// \brief Use vector range as source of values
            iterator_impl(vector_iterator begin, vector_iterator end,
                          query_function fn = nullptr)
                : query_it_(begin), query_it_end_(end), query_function_(fn) {
                if (query_function_ != nullptr) {
                    maybe_advance_predicate();
                }
            }

            /// \brief Use vector and a function predicate as source of values
            explicit iterator_impl(
                maybe_add_const<unprotected_vector_type> &source,
                query_function fn = nullptr)
                : iterator_impl(source.begin(), source.end(), fn) {}

          public /* LegacyIterator */:
            /// \brief Dereference iterator
            /// We make a conversion where
            ///     const std::pair<key_type, mapped_type>
            /// becomes
            ///     std::pair<const key_type, mapped_type>&
            /// like it's the case with maps.
            /// The user cannot change the key because it would mess
            /// the data structure. But the user CAN change the key.
            reference operator*() const {
                const std::pair<key_type, mapped_type> &p =
                    query_it_.operator*();
                auto *p2 = (std::pair<const key_type, mapped_type> *)&p;
                std::pair<const key_type, mapped_type> &p3 = *p2;
                return p3;
            }

            iterator_impl &operator++() {
                query_it_ = query_it_.operator++();
                maybe_advance_predicate();
                return *this;
            }

          public /* LegacyInputIterator */:
            pointer operator->() const {
                const std::pair<key_type, mapped_type> &p =
                    query_it_.operator*();
                auto *p2 = (std::pair<const key_type, mapped_type> *)&p;
                return p2;
            }

          public /* LegacyForwardIterator */:
            /// \brief Equality operator
            template <bool rhs_is_const>
            bool operator==(const iterator_impl<rhs_is_const> &rhs) const {
                return (query_it_ == rhs.query_it_);
            }

            /// \brief Inequality operator
            template <bool rhs_is_const>
            bool operator!=(const iterator_impl<rhs_is_const> &rhs) const {
                return !(this->operator==(rhs));
            }

            /// \brief Advance iterator
            /// This is the expected return type for iterators
            iterator_impl operator++(int) { // NOLINT(cert-dcl21-cpp):
                auto tmp = *this;
                query_it_ = query_it_.operator++();
                maybe_advance_predicate();
                return tmp;
            }

          public /* LegacyBidirectionalIterator */:
            /// \brief Decrement iterator
            iterator_impl &operator--() {
                query_it_ = query_it_.operator--();
                maybe_rewind_predicate();
                return *this;
            }

            /// \brief Decrement iterator
            /// This is the expected return type for iterators
            iterator_impl operator--(int) { // NOLINT(cert-dcl21-cpp)
                auto tmp = *this;
                query_it_ = query_it_.operator--();
                maybe_rewind_predicate();
                return tmp;
            }

          private /* Internal Functions */:
            void maybe_advance_predicate() {
                if (query_function_) {
                    while (query_it_ != query_it_end_ &&
                           !query_function_(*query_it_)) {
                        query_it_ = query_it_.operator++();
                    }
                }
            }

            void maybe_rewind_predicate() {
                if (query_function_) {
                    while (query_it_ != query_it_end_ &&
                           !query_function_(*query_it_)) {
                        query_it_ = query_it_.operator--();
                    }
                }
            }

          private:
            /// \brief Current vector element
            vector_iterator query_it_;

            /// \brief Last vector element
            vector_iterator query_it_end_;

            /// \brief Query function, in case the iterator has a predicate
            query_function query_function_;

          public:
            /// Let implicit tree access the spatial private constructors
            friend implicit_tree;
        };

      public /* Constructors: Container + AllocatorAwareContainer */:
        /// \brief Create an empty container
        /// Allocator aware containers overload all constructors with
        /// an extra allocator with a default parameter
        ///
        /// Allocator-aware containers always call
        /// std::allocator_traits<A>::construct(m, p, args)
        /// to construct an object of type A at p using args, with
        /// m == get_allocator(). The default construct in std::allocator
        /// calls ::new((void*)p) T(args), but specialized allocators may
        /// choose a different definition
        explicit implicit_tree(const allocator_type &alloc = allocator_type())
            : data_(alloc), comp_(dimension_compare()) {}

        /// \brief Copy constructor
        /// Copy constructors of AllocatorAwareContainers
        /// obtain their instances of the allocator by calling
        /// std::allocator_traits<allocator_type>::
        ///       select_on_container_copy_construction
        /// on the allocator of the container being copied.
        /// \param rhs
        implicit_tree(const implicit_tree &rhs)
            : data_(rhs.data_), comp_(rhs.comp_){};

        /// \brief Copy constructor data but use another allocator
        implicit_tree(const implicit_tree &rhs, const allocator_type &alloc)
            : data_(rhs.data_, alloc), comp_(rhs.comp_){};

        /// \brief Move constructor
        /// Move constructors obtain their instances of allocators
        /// by move-constructing from the allocator belonging to
        /// the old container
        implicit_tree(implicit_tree &&rhs) noexcept
            : data_(std::move(rhs.data_)), comp_(std::move(rhs.comp_)) {}

        /// \brief Move constructor data but use new allocator
        implicit_tree(implicit_tree &&rhs, const allocator_type &alloc) noexcept
            : data_(std::move(rhs.data_), alloc), comp_(std::move(rhs.comp_)) {}

        /// \brief Destructor
        ~implicit_tree() = default;

      public /* Constructors: AssociativeContainer + AllocatorAwareContainer */:
        /// \brief Create container with custom comparison function
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        explicit implicit_tree(const C &comp,
                               const allocator_type &alloc = allocator_type())
            : data_(alloc), comp_(comp) {}

        /// \brief Construct with iterators + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        implicit_tree(InputIt first, InputIt last, const C &comp = C(),
                      const allocator_type &alloc = allocator_type())
            : data_(first, last, alloc), comp_(comp) {}

        /// \brief Construct with list + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        implicit_tree(std::initializer_list<value_type> il, const C &comp = C(),
                      const allocator_type &alloc = allocator_type())
            : data_(il.begin(), il.end(), alloc), comp_(comp) {}

        /// \brief Construct with iterators
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        implicit_tree(InputIt first, InputIt last, const allocator_type &alloc)
            : data_(first, last, alloc) {}

        /// \brief Construct with iterators
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        implicit_tree(std::initializer_list<value_type> il,
                      const allocator_type &alloc)
            : data_(il.begin(), il.end(), alloc) {}

      public /* Assignment: Container + AllocatorAwareContainer */:
        /// \brief Copy assignment
        /// Copy-assignment will replace the allocator only if
        /// std::allocator_traits<allocator_type>::
        ///     propagate_on_container_copy_assignment::value is true
        /// \note if the allocators of the source and the target containers
        /// do not compare equal, copy assignment has to deallocate the
        /// target's memory using the old allocator and then allocate it
        /// using the new allocator before copying the elements
        implicit_tree &operator=(const implicit_tree &rhs) {
            if (&rhs == this) {
                return *this;
            }
            data_.clear();
            for (const auto &item : rhs.data_) {
                data_.emplace_back(item);
            }
            return *this;
        };

        /// \brief Move assignment
        /// Move-assignment will replace the allocator only if
        /// std::allocator_traits<allocator_type>::
        ///     propagate_on_container_move_assignment::value is true
        ///
        /// \note If this propagate...move is not provided or derived from
        /// std::false_type and the allocators of the source and
        /// the target containers do not compare equal, move assignment
        /// cannot take ownership of the source memory and must
        /// move-assign or move-construct the elements individually,
        /// resizing its own memory as needed.
        implicit_tree &operator=(implicit_tree &&rhs) noexcept {
            if (&rhs == this) {
                return *this;
            }
            data_ = std::move(rhs.data_);
            return *this;
        }

      public /* Assignment: AssociativeContainer */:
        /// \brief Initializer list assignment
        implicit_tree &
        operator=(std::initializer_list<value_type> il) noexcept {
            data_.clear();
            data_.insert(data_.end(), il.begin(), il.end());
            return *this;
        }

      public /* Non-Modifying Functions: AllocatorAwareContainer */:
        /// \brief Obtains a copy of the allocator
        /// The accessor get_allocator() obtains a copy of
        /// the allocator that was used to construct the
        /// container or installed by the most recent allocator
        /// replacement operation
        allocator_type get_allocator() const noexcept {
            return data_.get_allocator();
        }

      public /* Element Access / Map Concept */:
        /// \brief Get reference to element at a given position, and throw error
        /// if it does not exist
        mapped_type &at(const key_type &k) {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("kd_tree::at: key not found");
            }
        }

        /// \brief Get reference to element at a given position, and throw error
        /// if it does not exist
        const mapped_type &at(const key_type &k) const {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("kd_tree::at: key not found");
            }
        }

        /// \brief Get reference to element at a given position, and create one
        /// if it does not exits
        mapped_type &operator[](const key_type &k) {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                auto it_new = insert(std::make_pair(k, mapped_type()));
                return it_new->second;
            }
        }

        /// \brief Get reference to element at a given position, and create one
        /// if it does not exits
        mapped_type &operator[](key_type &&k) {
            auto it = find(std::move(k));
            if (it != end()) {
                return it->second;
            } else {
                auto it_new = insert(std::make_pair(k, mapped_type()));
                return it_new->second;
            }
        }

        /// \brief Get reference to element at a given position, and create one
        /// if it does not exits
        template <typename... Targs>
        mapped_type &operator()(const dimension_type &x1, const Targs &...xs) {
            constexpr size_t m = sizeof...(Targs) + 1;
            assert(number_of_compile_dimensions == 0 ||
                   number_of_compile_dimensions == m);
            point_type p(m);
            copy_pack(p.begin(), x1, xs...);
            return operator[](p);
        }

      public /* Non-Modifying Functions: Container Concept */:
        /// \brief Get iterator to first element
        const_iterator begin() const noexcept {
            return const_iterator(data_.cbegin(), data_.cend());
        }

        /// \brief Get iterator to past-the-end element
        const_iterator end() const noexcept {
            return const_iterator(data_.cend(), data_.cend());
        }

        /// \brief Get iterator to first element
        const_iterator cbegin() const noexcept {
            return const_iterator(data_.cbegin(), data_.cend());
        }

        /// \brief Get iterator to past-the-end element
        const_iterator cend() const noexcept {
            return const_iterator(data_.cend(), data_.cend());
        }

        /// \brief Get iterator to first element
        iterator begin() noexcept {
            return iterator(data_.begin(), data_.end());
        }

        /// \brief Get iterator to past-the-end element
        iterator end() noexcept { return iterator(data_.end(), data_.end()); }

      public /* Non-Modifying Functions: ReversibleContainer Concept */:
        /// \brief Get iterator to first element in reverse
        std::reverse_iterator<const_iterator> rbegin() const noexcept {
            return std::reverse_iterator<const_iterator>(end());
        }

        /// \brief Get iterator to last element in reverse
        std::reverse_iterator<const_iterator> rend() const noexcept {
            return std::reverse_iterator<const_iterator>(begin());
        }

        /// \brief Get iterator to first element in reverse
        std::reverse_iterator<iterator> rbegin() noexcept {
            return std::reverse_iterator<iterator>(end());
        }

        /// \brief Get iterator to last element in reverse
        std::reverse_iterator<iterator> rend() noexcept {
            return std::reverse_iterator<iterator>(begin());
        }

        /// \brief Get iterator to first element in reverse
        std::reverse_iterator<const_iterator> crbegin() const noexcept {
            return std::reverse_iterator(cend());
        }

        /// \brief Get iterator to past-the-end element in reverse
        std::reverse_iterator<const_iterator> crend() const noexcept {
            return std::reverse_iterator(cbegin());
        }

      public /* Non-Modifying Functions / Capacity / Container Concept */:
        /// \brief True if container is empty
        [[nodiscard]] bool empty() const noexcept { return data_.empty(); }

        /// \brief Get container size
        [[nodiscard]] size_t size() const noexcept { return data_.size(); }

        /// \brief Get container max size
        [[nodiscard]] size_t max_size() const noexcept {
            return data_.max_size();
        }

      public /* Non-Modifying Functions / Capacity / Spatial Concept */:
        /// \brief Get container dimensions
        [[nodiscard]] size_t dimensions() const noexcept {
            if constexpr (M != 0) {
                return M;
            } else {
                return data_.front().first.dimensions();
            }
        }

        /// \brief Get maximum value in a given dimension
        dimension_type max_value(size_t dimension) const {
            auto it = std::max_element(
                begin(), end(),
                [&dimension](const value_type &a, const value_type &b) {
                    return a.first[dimension] < b.first[dimension];
                });
            if (it != end()) {
                return it->first[dimension];
            }
            return std::numeric_limits<dimension_type>::min();
        }

        /// \brief Get minimum value in a given dimension
        dimension_type min_value(size_t dimension) const {
            auto it = std::min_element(
                begin(), end(),
                [&dimension](const value_type &a, const value_type &b) {
                    return a.first[dimension] < b.first[dimension];
                });
            if (it != end()) {
                return it->first[dimension];
            }
            return std::numeric_limits<dimension_type>::min();
        }

      public /* Modifying Functions: Container + AllocatorAwareContainer */:
        /// \brief Swap the content of two objects
        /// Swap will replace the allocator only if
        /// std::allocator_traits<allocator_type>::
        ///     propagate_on_container_swap::value is true
        /// Specifically, it will exchange the allocator
        ///     instances through an unqualified call to the
        ///     non-member function swap, see Swappable
        /// Swapping two containers with unequal allocators
        ///     if propagate_on_container_swap is false
        ///     is undefined behavior
        /// \note If this member is not provided or derived from std::false_type
        /// and the allocators of the two containers do not compare equal,
        /// the behavior of container swap is undefined.
        void swap(implicit_tree &other) noexcept {
            other.data_.swap(data_);
            std::swap(comp_, other.comp_);
        }

      public /* Modifiers: Multimap Concept */:
        /// Clear the front
        void clear() noexcept { data_.clear(); }

        /// \brief Insert element pair
        iterator insert(const value_type &v) {
            data_.emplace_back(v);
            auto it = data_.end();
            --it;
            return iterator(it, data_.end());
        }

        iterator insert(value_type &&v) {
            data_.emplace_back(std::move(v));
            auto it = data_.end();
            --it;
            return iterator(it, data_.end());
        }

        template <class P> iterator insert(P &&v) {
            static_assert(std::is_constructible_v<value_type, P &&>);
            return emplace(std::forward<P>(v));
        }

        /// \brief Insert element with a hint
        /// It's still possible to implement hints for spatial
        /// maps. However, we need to come up with one strategy
        /// for each map container.
        iterator insert(iterator, const value_type &v) { return insert(v); }

        iterator insert(const_iterator, const value_type &v) {
            return insert(v);
        }

        iterator insert(const_iterator, value_type &&v) {
            return insert(std::move(v));
        }

        template <class P> iterator insert(const_iterator hint, P &&v) {
            static_assert(std::is_constructible_v<value_type, P &&>);
            return emplace_hint(hint, std::forward<P>(v));
        }

        /// \brief Insert list of elements
        template <class InputIterator>
        void insert(InputIterator first, InputIterator last) {
            data_.insert(data_.end(), first, last);
        }

        /// \brief Insert list of elements
        void insert(std::initializer_list<value_type> init) {
            insert(init.begin(), init.end());
        }

        template <class... Args> iterator emplace(Args &&...args) {
            return insert(value_type(std::forward<Args>(args)...));
        }

        template <class... Args>
        iterator emplace_hint(const_iterator, Args &&...args) {
            return insert(value_type(std::forward<Args>(args)...));
        }

        /// Erase element
        iterator erase(const_iterator position) {
            if (position != end()) {
                auto r = position.query_it_ - data_.begin();
                data_.erase(position.query_it_);
                return iterator(data_.begin() + r, data_.end());
            } else {
                return end();
            }
        }

        iterator erase(iterator position) {
            return erase(const_iterator(position));
        }

        /// Remove range of iterators from the front
        iterator erase(const_iterator first, const_iterator last) {
            // get copy of all elements in the query
            std::vector<value_type> v(first, last);
            auto next = data_.end();
            // remove using these copies as reference
            for (const value_type &x : v) {
                auto it = std::find_if(
                    data_.cbegin(), data_.cend(), [&x](const auto &a) {
                        return a.first == x.first &&
                               mapped_type_custom_equality_operator(a.second,
                                                                    x.second);
                    });
                if (it != data_.end()) {
                    next = data_.erase(it);
                }
            }
            return iterator(next, data_.end());
        }

        /// \brief Erase element by value
        size_t erase(const key_type &k) {
            // k might be a reference to the element
            // we are about to delete
            key_type k_copy(k);
            iterator it = find(k_copy);
            size_type s = 0;
            while (it != end()) {
                erase(it);
                ++s;
                it = find(k_copy);
            }
            return s;
        }

        /// \brief Splices nodes from another container
        void merge(implicit_tree &source) noexcept {
            insert(source.begin(), source.end());
        }

      public /* Lookup / Multimap Concept */:
        /// \brief Returns the number of elements with key that compares
        /// equivalent to the specified argument.
        size_type count(const key_type &k) const {
            iterator it =
                (const_cast<implicit_tree *>(this))->find_intersection(k);
            iterator end = (const_cast<implicit_tree *>(this))->end();
            return static_cast<size_type>(std::distance(it, end));
        }

        /// \brief Returns the number of elements with key that compares
        /// equivalent to the specified argument.
        template <class L> size_type count(const L &k) const {
            iterator it = find_intersection(k);
            return static_cast<size_type>(std::distance(it, end()));
        }

        /// \brief Find point
        iterator find(const key_type &p) {
            auto vec_begin =
                std::find_if(data_.begin(), data_.end(),
                             [p](const value_type &v) { return v.first == p; });
            return iterator(vec_begin, data_.end());
        }

        /// \brief Find point
        const_iterator find(const key_type &p) const {
            auto vec_begin =
                std::find_if(data_.begin(), data_.end(),
                             [p](const value_type &v) { return v.first == p; });
            return const_iterator(vec_begin, data_.end());
        }

        /// \brief Finds an element with key equivalent to key
        template <class L> iterator find(const L &x) {
            return find(key_type(x));
        }

        /// \brief Finds an element with key equivalent to key
        template <class L> const_iterator find(const L &x) const {
            return find(key_type(x));
        }

        /// \brief Finds an element with key equivalent to key
        bool contains(const key_type &k) const { return find(k) != end(); }

        /// \brief Finds an element with key equivalent to key
        template <class L> bool contains(const L &x) const {
            return find(x) != end();
        }

      public /* Modifiers: Lookup / Spatial Concept */:
        /// \brief Get iterator to first element that passes the list of
        /// predicates
        const_iterator find(const predicate_list_type &ps) const noexcept {
            return const_iterator(data_, [ps](const value_type &v) {
                return ps.pass_predicate(v);
            });
        }

        /// \brief Get iterator to first element that passes the list of
        /// predicates
        iterator find(const predicate_list_type &ps) noexcept {
            return iterator(data_, [ps](const value_type &v) {
                return ps.pass_predicate(v);
            });
        }

        /// \brief Find intersection between points and query box
        iterator find_intersection(const point_type &k) {
            return find_intersection(k, k);
        }

        /// \brief Find intersection between points and query box
        const_iterator find_intersection(const point_type &k) const {
            return find_intersection(k, k);
        }

        /// \brief Find intersection between points and query box
        const_iterator find_intersection(const point_type &lb,
                                         const point_type &ub) const {
            intersects p(lb, ub);
            return const_iterator(data_, [p](const value_type &v) {
                return p.pass_predicate(v.first);
            });
        }

        /// \brief Find intersection between points and query box
        iterator find_intersection(const point_type &lb, const point_type &ub) {
            intersects<dimension_type, number_of_compile_dimensions> p(lb, ub);
            return iterator(data_, [p](const value_type &v) {
                return p.pass_predicate(v.first);
            });
        }

        template <class PREDICATE_TYPE>
        const_iterator find_intersection(const point_type &lb,
                                         const point_type &ub,
                                         PREDICATE_TYPE fn) const {
            intersects<dimension_type, number_of_compile_dimensions> p(lb, ub);
            return const_iterator(data_, [p, fn](const value_type &v) {
                return p.pass_predicate(v.first) & fn(v);
            });
        }

        template <class PREDICATE_TYPE>
        iterator find_intersection(const point_type &lb, const point_type &ub,
                                   PREDICATE_TYPE fn) {
            intersects<dimension_type, number_of_compile_dimensions> p(lb, ub);
            return iterator(data_, [p, fn](const value_type &v) {
                return p.pass_predicate(v.first) && fn(v);
            });
        }

        /// \brief Find points within a query box
        const_iterator find_within(const point_type &lb,
                                   const point_type &ub) const {
            within<dimension_type, number_of_compile_dimensions> p(lb, ub);
            return const_iterator(data_, [p](const value_type &v) {
                return p.pass_predicate(v.first);
            });
        }

        /// \brief Find points within a query box
        iterator find_within(const point_type &lb, const point_type &ub) {
            within<dimension_type, number_of_compile_dimensions> p(lb, ub);
            return iterator(data_, [p](const value_type &v) {
                return p.pass_predicate(v.first);
            });
        }

        /*
        /// \brief Find points within a query box
        template <class PREDICATE_TYPE>
        const_iterator find_within(const point_type &lb,
                                    const point_type &ub,
                                    PREDICATE_TYPE fn) const {
            within<dimension_type, number_of_compile_dimensions> p(lb,
                                                                ub);
            return const_iterator(data_, [p, &fn](const value_type &v) {
                return p.pass_predicate(v.first) && fn(v);
            });
        }

        /// \brief Find points within a query box
        template <class PREDICATE_TYPE>
        iterator find_within(const point_type &lb,
                              const point_type &ub, PREDICATE_TYPE fn) {
            within<dimension_type, number_of_compile_dimensions> p(lb,
                                                                ub);
            return iterator(data_, [p, &fn](const value_type &v) {
                return p.pass_predicate(v.first) && fn(v);
            });
        }
        */

        /// \brief Find points outside a query box
        const_iterator find_disjoint(const point_type &lb,
                                     const point_type &ub) const {
            disjoint<dimension_type, number_of_compile_dimensions> p(lb, ub);
            return const_iterator(data_, [p](const value_type &v) {
                return p.pass_predicate(v.first);
            });
        }

        /// \brief Find points outside a query box
        iterator find_disjoint(const point_type &lb, const point_type &ub) {
            disjoint<dimension_type, number_of_compile_dimensions> p(lb, ub);
            return iterator(data_, [p](const value_type &v) {
                return p.pass_predicate(v.first);
            });
        }

        /// \brief Find points outside a query box
        template <class PREDICATE_TYPE>
        const_iterator find_disjoint(const point_type &lb, const point_type &ub,
                                     PREDICATE_TYPE fn) const {
            disjoint<dimension_type, number_of_compile_dimensions> p(lb, ub);
            return const_iterator(data_, [p, &fn](const value_type &v) {
                return p.pass_predicate(v.first) && fn(v);
            });
        }

        /// \brief Find points outside a query box
        template <class PREDICATE_TYPE>
        iterator find_disjoint(const point_type &lb, const point_type &ub,
                               PREDICATE_TYPE fn) {
            disjoint<dimension_type, number_of_compile_dimensions> p(lb, ub);
            return iterator(data_, [p, &fn](const value_type &v) {
                return p.pass_predicate(v.first) && fn(v);
            });
        }

        /*
        /// \brief Find points closest to a reference point
        const_iterator find_nearest(const point_type &p) const {
            auto it = std::min_element(
                data_.begin(), data_.end(),
                [p](const value_type &v1, const value_type &v2) {
                    return p.distance(v1.first) < p.distance(v2.first);
                });
            if (it != data_.end()) {
                auto nearest = it->first;
                return const_iterator(data_, [nearest](const value_type &v) {
                    return v.first == nearest;
                });
            }
            return end();
        }

        /// \brief Find points closest to a reference point
        iterator find_nearest(const point_type &p) {
            auto it = std::min_element(
                data_.begin(), data_.end(),
                [p](const value_type &v1, const value_type &v2) {
                    return p.distance(v1.first) < p.distance(v2.first);
                });
            if (it != data_.end()) {
                auto nearest = it->first;
                return iterator(data_, [nearest](const value_type &p) {
                    return p.first == nearest;
                });
            }
            return end();
        }
         */

        /// \brief Find points closest to a reference point
        const_iterator find_nearest(const point_type &p) const {
            auto it = std::min_element(
                data_.begin(), data_.end(),
                [p](const value_type &v1, const value_type &v2) {
                    return p.distance(v1.first) < p.distance(v2.first);
                });
            if (it != data_.end()) {
                auto nearest = it->first;
                return const_iterator(data_, [nearest](const value_type &p) {
                    return p.first == nearest;
                });
            }
            return end();
        }

        /// \brief Find points closest to a reference point
        iterator find_nearest(const point_type &p) {
            auto it = std::min_element(
                data_.begin(), data_.end(),
                [p](const value_type &v1, const value_type &v2) {
                    return p.distance(v1.first) < p.distance(v2.first);
                });
            if (it != data_.end()) {
                auto nearest = it->first;
                return iterator(data_, [nearest](const value_type &p) {
                    return p.first == nearest;
                });
            }
            return end();
        }

        /// Find k nearest points
        /// This is VERY inefficient with vectors.
        /// This could be improved BUT...
        ///     users shouldn't be using vectors for this.
        const_iterator find_nearest(const point_type &p, size_t k) const {
            if (k == 1) {
                return find_nearest(p);
            }
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(
                data_copy.begin(), data_copy.begin() + k, data_copy.end(),
                [p](value_type v1, value_type v2) {
                    return p.distance(v1.first) < p.distance(v2.first);
                });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin();
                 it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return const_iterator(data_, [nearest_set](const value_type &p) {
                return std::find(nearest_set.begin(), nearest_set.end(),
                                 p.first) != nearest_set.end();
            });
        }

        /// Find k nearest points
        /// This is VERY ineffient with vectors.
        /// This could be improved BUT...
        ///     users shouldn't be using vectors for this.
        iterator find_nearest(const point_type &p, size_t k) {
            if (k == 1) {
                return find_nearest(p);
            }
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(
                data_copy.begin(), data_copy.begin() + k, data_copy.end(),
                [p](value_type v1, value_type v2) {
                    return p.distance(v1.first) < p.distance(v2.first);
                });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin();
                 it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return iterator(data_, [nearest_set](const value_type &p) {
                return std::find(nearest_set.begin(), nearest_set.end(),
                                 p.first) != nearest_set.end();
            });
        }

        template <class PREDICATE_TYPE>
        const_iterator find_nearest(const point_type &p, size_t k,
                                    PREDICATE_TYPE fn) const {
            if (k == 1) {
                return find_nearest(p);
            }
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(
                data_copy.begin(), data_copy.begin() + k, data_copy.end(),
                [p](value_type v1, value_type v2) {
                    return p.distance(v1.first) < p.distance(v2.first);
                });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin();
                 it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return const_iterator(
                data_, [nearest_set, fn](const value_type &p) {
                    return std::find(nearest_set.begin(), nearest_set.end(),
                                     p.first) != nearest_set.end() &&
                           fn(p);
                });
        }

        template <class PREDICATE_TYPE>
        iterator find_nearest(const point_type &p, size_t k,
                              PREDICATE_TYPE fn) {
            if (k == 1) {
                return find_nearest(p);
            }
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(
                data_copy.begin(), data_copy.begin() + k, data_copy.end(),
                [p](value_type v1, value_type v2) {
                    return p.distance(v1.first) < p.distance(v2.first);
                });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin();
                 it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return iterator(data_, [nearest_set, fn](const value_type &p) {
                return std::find(nearest_set.begin(), nearest_set.end(),
                                 p.first) != nearest_set.end() &&
                       fn(p);
            });
        }

        /// \brief Find points closest to a reference point
        const_iterator find_nearest(const box_type &p, size_t k = 1) const {
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(
                data_copy.begin(), data_copy.begin() + k, data_copy.end(),
                [p](value_type v1, value_type v2) {
                    return p.distance(v1.first) < p.distance(v2.first);
                });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin();
                 it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return const_iterator(data_, [nearest_set](const value_type &p) {
                return std::find(nearest_set.begin(), nearest_set.end(),
                                 p.first) != nearest_set.end();
            });
        }

        /// \brief Find points closest to a reference point
        iterator find_nearest(const box_type &p, size_t k = 1) {
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(
                data_copy.begin(), data_copy.begin() + k, data_copy.end(),
                [p](value_type v1, value_type v2) {
                    return p.distance(v1.first) < p.distance(v2.first);
                });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin();
                 it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return iterator(data_, [nearest_set](const value_type &p) {
                return std::find(nearest_set.begin(), nearest_set.end(),
                                 p.first) != nearest_set.end();
            });
        }

        template <class PREDICATE_TYPE>
        const_iterator find_nearest(const box_type &p, size_t k,
                                    PREDICATE_TYPE fn) const {
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(
                data_copy.begin(), data_copy.begin() + k, data_copy.end(),
                [p](value_type v1, value_type v2) {
                    return distance(v1.first, p) < distance(v2.first, p);
                });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin();
                 it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return const_iterator(
                data_, [nearest_set, fn](const value_type &p) {
                    return std::find(nearest_set.begin(), nearest_set.end(),
                                     p.first) != nearest_set.end() &&
                           fn(p);
                });
        }

        template <class PREDICATE_TYPE>
        iterator find_nearest(const box_type &p, size_t k, PREDICATE_TYPE fn) {
            auto data_copy = data_;
            k = std::min(k, size());
            std::partial_sort(
                data_copy.begin(), data_copy.begin() + k, data_copy.end(),
                [p](value_type v1, value_type v2) {
                    return distance(v1.first, p) < distance(v2.first, p);
                });
            std::vector<point_type> nearest_set;
            nearest_set.reserve(k);
            for (auto it = data_copy.begin();
                 it != data_copy.end() && it != data_copy.begin() + k; ++it) {
                nearest_set.emplace_back(it->first);
            }
            return iterator(data_, [nearest_set, fn](const value_type &p) {
                return std::find(nearest_set.begin(), nearest_set.end(),
                                 p.first) != nearest_set.end() &&
                       fn(p);
            });
        }

        /// \brief Get iterator to element with maximum value in a given
        /// dimension
        iterator max_element(size_t dimension) {
            return std::max_element(
                begin(), end(),
                [&dimension](const value_type &a, const value_type &b) {
                    return a.first[dimension] < b.first[dimension];
                });
        }

        /// \brief Get iterator to element with maximum value in a given
        /// dimension
        const_iterator max_element(size_t dimension) const {
            return std::max_element(
                begin(), end(),
                [&dimension](const value_type &a, const value_type &b) {
                    return a.first[dimension] < b.first[dimension];
                });
        }

        /// \brief Get iterator to element with minimum value in a given
        /// dimension
        iterator min_element(size_t dimension) {
            return std::min_element(
                begin(), end(),
                [&dimension](const value_type &a, const value_type &b) {
                    return a.first[dimension] < b.first[dimension];
                });
        }

        /// \brief Get iterator to element with minimum value in a given
        /// dimension
        const_iterator min_element(size_t dimension) const {
            return std::min_element(
                begin(), end(),
                [&dimension](const value_type &a, const value_type &b) {
                    return a.first[dimension] < b.first[dimension];
                });
        }

      public /* Observers: AssociativeContainer */:
        /// \brief Returns the function object that compares keys
        /// This function is here mostly to conform with the
        /// AssociativeContainer concepts. It's possible but not
        /// very useful.
        key_compare key_comp() const noexcept {
            return [this](const key_type &a, const key_type &b) {
                return std::lexicographical_compare(a.begin(), a.end(),
                                                    b.begin(), b.end(), comp_);
            };
        }

        /// \brief Returns the function object that compares values
        value_compare value_comp() const noexcept {
            return [this](const value_type &a, const value_type &b) {
                return std::lexicographical_compare(
                    a.first.begin(), a.first.end(), b.first.begin(),
                    b.first.end(), comp_);
            };
        }

        /// \brief Returns the function object that compares numbers
        /// This is the comparison operator for a single dimension
        dimension_compare dimension_comp() const noexcept { return comp_; }

      private:
        /// \brief Vector storing the data
        /// Note that the key_type is not constant, unlike the value_type
        /// exposed to the user. We need an unprotected value type to be
        /// able to manipulate the vector. The iterators are responsible
        /// for converting these references to protected types where the
        /// key is const, in order to follow the standard for associative
        /// containers.
        unprotected_vector_type data_{};

        dimension_compare comp_{dimension_compare()};
    };

    /* Non-Modifying Functions / Comparison / Container Concept */
    /// \brief Equality operator
    /// \warning This operator tells us if the trees are equal
    /// and not if they contain the same elements.
    /// This is because the standard defines that this operation should
    /// take linear time.
    /// Two trees might contain the same elements in different
    /// subtrees if the insertion order was different.
    /// If you need to compare if the elements are the same, regardless
    /// of their trees, you have to iterate one container and call
    /// find on the second container. This operation takes loglinear time.
    template <class K, size_t M, class T, class C, class A>
    bool operator==(const implicit_tree<K, M, T, C, A> &lhs,
                    const implicit_tree<K, M, T, C, A> &rhs) {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        return std::equal(
            lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
            [](const typename implicit_tree<K, M, T, C, A>::value_type &a,
               const typename implicit_tree<K, M, T, C, A>::value_type &b) {
                return a.first == b.first &&
                       mapped_type_custom_equality_operator(a.second, b.second);
            });
    }

    /// \brief Inequality operator
    template <class K, size_t M, class T, class C, class A>
    bool operator!=(const implicit_tree<K, M, T, C, A> &lhs,
                    const implicit_tree<K, M, T, C, A> &rhs) {
        return !(lhs == rhs);
    }

} // namespace pareto

#endif // PARETO_FRONTS_PREDICATE_VECTOR_H
