#ifndef PARETO_FRONTS_PREDICATE_TREE_H
#define PARETO_FRONTS_PREDICATE_TREE_H

#include <boost/geometry/geometry.hpp>
#include <pareto/point.h>
#include <pareto/query/query_box.h>

namespace pareto {
    /// \class R-Tree based on Boost.Geometry
    /// This is a r-containers with all the operations we need using
    /// boost r-containers as an underlying data structure.
    /// R-Tree also does not make good use of the custom allocators that are
    /// fundamental in small trees, if our benchmarks are correct.
    /// Although we used this container for our initial experiments,
    /// we are deprecating this class in later versions of this
    /// library because:
    /// * Very poor performance for fronts compared with our own R-Tree
    /// * R-Trees are not even design for fast performance when the the
    ///   containers contains only points, and we already have two R-Tree
    ///   implementations besides this one
    /// * It seems like boost.geometry is not being maintained and C++
    ///   has changed a lot since then
    /// * Boost.Geometry has a lot of boost dependencies. Besides
    ///   (i) the pain of having the library limited by a boost dependency just
    ///   because of header-only libraries that could easily be on github
    ///   nowadays, (ii) boost libraries tend to depend on other boost libraries
    ///   implementing features that have been on the C++ standard library for
    ///   more than a decade, (iii) boost is famous for having cyclic
    ///   dependencies, (iv) boost.geometry depends on other boost libraries
    ///   that are now deprecated, (v) it emits lots of warnings on modern C++
    ///   besides the deprecated dependencies, (vi) and it requires lots of
    ///   workarounds to make it work with the same interface
    ///        as our other trees. For instance, we cannot create predicate
    ///        lists in runtime with boost and we have to recur to some
    ///        inefficient workarounds that make boost.geometry iterate the
    ///        whole containers
    ///   (vii) the front dimension cannot be set in runtime with boost.geometry
    template <typename K, size_t M, typename T, typename C = std::less<K>,
              typename A =
                  std::allocator<std::pair<const ::pareto::point<K, M>, T>>>
    class [[deprecated]] boost_tree {
      private /* Internal types */:
        static_assert(M != 0);
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
        using unprotected_boost_iterator =
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

      public /* Boost Options */:
        using tree_parameters = boost::geometry::index::quadratic<16>;
        using tree_indexable =
            boost::geometry::index::indexable<unprotected_value_type>;
        using tree_value_equal =
            boost::geometry::index::equal_to<unprotected_value_type>;
        using tree_type =
            boost::geometry::index::rtree<unprotected_value_type,
                                          tree_parameters, tree_indexable,
                                          tree_value_equal, allocator_type>;

      public /* iterators */:
        // This iterator keeps a reference to an rtree query iterators.
        // What these iterators do is skip the elements that don't match our
        // queries / predicates. This is very useful to give us a nice interface
        // to query the pareto front because we can create good iterators of
        // this kind for rtrees. There are not as efficient for vectors, but
        // they keep a common interface with the case that matters the most.
        // https://stackoverflow.com/questions/7758580/writing-your-own-stl-container/7759622#7759622
        template <bool is_const> class iterator_impl {
          public:
          private /* Internal Types */:
            template <class TYPE, class CONST_TYPE>
            using const_toggle =
                std::conditional_t<!is_const, TYPE, CONST_TYPE>;

            template <class U>
            using maybe_add_const = const_toggle<U, std::add_const_t<U>>;

            using boost_iterator_unconst =
                typename tree_type::const_query_iterator;
            using boost_iterator_const =
                typename tree_type::const_query_iterator;

            using boost_iterator =
                const_toggle<boost_iterator_unconst, boost_iterator_const>;

            using query_function = std::function<bool(const value_type &)>;

          public /* LegacyIterator Types */:
            using value_type = maybe_add_const<boost_tree::value_type>;
            using reference = const_toggle<boost_tree::reference,
                                           boost_tree::const_reference>;
            using difference_type = typename boost_tree::difference_type;
            using pointer =
                const_toggle<boost_tree::pointer, boost_tree::const_pointer>;
            using iterator_category = std::forward_iterator_tag;

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
                : query_it_(rhs.query_it_) {}

            /// \brief Copy assignment
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            iterator_impl &operator=(const iterator_impl<rhs_is_const> &rhs) {
                if constexpr (is_const == rhs_is_const) {
                    if (&rhs == this) {
                        return *this;
                    }
                }
                query_it_ = rhs.query_it_;
                return *this;
            }

            /// \brief Destructor
            ~iterator_impl() = default;

          public /* LegacyForwardIterator Constructors */:
            /// \brief Default constructor
            iterator_impl() : query_it_(boost_iterator()) {}

          public /* ContainerConcept Constructors */:
            /// \brief Convert to const iterator
            // NOLINTNEXTLINE(google-explicit-constructor)
            operator iterator_impl<true>() {
                const_iterator r(query_it_);
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
                : query_it_(std::move(rhs.query_it_)) {}

            /// \brief Move assignment
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            iterator_impl &operator=(iterator_impl<rhs_is_const> &&rhs) {
                query_it_ = std::move(rhs.query_it_);
                return *this;
            }

          public /* Internal Constructors / Used by Container */:
            /// \brief Use vector range as source of values
            explicit iterator_impl(boost_iterator begin) : query_it_(begin) {}

            /// \brief Use vector and a function predicate as source of values
            explicit iterator_impl(unprotected_vector_type &source,
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
                return tmp;
            }

          public /* LegacyBidirectionalIterator */:
            /*
             *  Boost r-trees don't support the operator--
             *  for their query iterators. For compatibility in
             *  our experiments, the reverse iterator will just
             *  move forward anyway. We don't recommend seriously
             *  using this
             */

            /// \brief Decrement iterator
            iterator_impl &operator--() {
                query_it_ = query_it_.operator++();
                return *this;
            }

            /// \brief Decrement iterator
            /// This is the expected return type for iterators
            iterator_impl operator--(int) { // NOLINT(cert-dcl21-cpp)
                auto tmp = *this;
                query_it_ = query_it_.operator++();
                return tmp;
            }

          private:
            /// \brief The original boost iterator we are wrapping
            boost_iterator query_it_{};

          public:
            /// Let implicit tree access the spatial private constructors
            friend boost_tree;
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
        explicit boost_tree(const allocator_type &alloc = allocator_type())
            : data_(tree_parameters(), tree_indexable(), tree_value_equal(),
                    alloc),
              comp_(C()) {}

        /// \brief Copy constructor
        /// Copy constructors of AllocatorAwareContainers
        /// obtain their instances of the allocator by calling
        /// std::allocator_traits<allocator_type>::
        ///       select_on_container_copy_construction
        /// on the allocator of the container being copied.
        /// \param rhs
        boost_tree(const boost_tree &rhs)
            : data_(rhs.data_, std::allocator_traits<allocator_type>::
                                   select_on_container_copy_construction(
                                       rhs.get_allocator())),
              comp_(rhs.comp_){};

        /// \brief Copy constructor data but use another allocator
        boost_tree(const boost_tree &rhs, const allocator_type &alloc)
            : data_(rhs.data_, alloc), comp_(rhs.comp_){};

        /// \brief Move constructor
        /// Move constructors obtain their instances of allocators
        /// by move-constructing from the allocator belonging to
        /// the old container
        boost_tree(boost_tree &&rhs) noexcept
            : data_(std::move(rhs.data_)), comp_(std::move(rhs.comp_)) {}

        /// \brief Move constructor data but use new allocator
        boost_tree(boost_tree &&rhs, const allocator_type &alloc) noexcept
            : data_(std::move(rhs.data_), alloc), comp_(std::move(rhs.comp_)) {}

        /// \brief Destructor
        ~boost_tree() = default;

      public /* Constructors: AssociativeContainer + AllocatorAwareContainer */:
        /// \brief Create container with custom comparison function
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        explicit boost_tree(const C &comp,
                            const allocator_type &alloc = allocator_type())
            : data_(tree_parameters(), tree_indexable(), tree_value_equal(),
                    alloc),
              comp_(comp) {}

        /// \brief Construct with iterators + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        boost_tree(InputIt first, InputIt last, const C &comp = C(),
                   const allocator_type &alloc = allocator_type())
            : data_(first, last, alloc), comp_(comp) {}

        /// \brief Construct with list + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        boost_tree(std::initializer_list<value_type> il, const C &comp = C(),
                   const allocator_type &alloc = allocator_type())
            : data_(il.begin(), il.end(), alloc), comp_(comp) {}

        /// \brief Construct with iterators
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        boost_tree(InputIt first, InputIt last, const allocator_type &alloc)
            : data_(first, last, alloc) {}

        /// \brief Construct with iterators
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        boost_tree(std::initializer_list<value_type> il,
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
        boost_tree &operator=(const boost_tree &rhs) {
            if (&rhs == this) {
                return *this;
            }
            data_.clear();
            for (const auto &item : rhs.data_) {
                data_.insert(item);
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
        boost_tree &operator=(boost_tree &&rhs) noexcept {
            if (&rhs == this) {
                return *this;
            }
            data_ = std::move(rhs.data_);
            return *this;
        }

      public /* Assignment: AssociativeContainer */:
        /// \brief Initializer list assignment
        boost_tree &operator=(std::initializer_list<value_type> il) noexcept {
            data_.clear();
            data_.insert(il.begin(), il.end());
            return *this;
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
        void swap(boost_tree &other) noexcept {
            other.data_.swap(data_);
            std::swap(comp_, other.comp_);
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

      public /* Non-Modifying Functions: AssociativeContainer */:
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

      public /* Non-Modifying Functions: Container Concept */:
        /// \brief Get iterator to first element
        const_iterator begin() const noexcept { return cbegin(); }

        /// \brief Get iterator to past-the-end element
        const_iterator end() const noexcept { return cend(); }

        /// \brief Get iterator to first element
        const_iterator cbegin() const noexcept {
            return const_iterator(
                data_.qbegin(boost::geometry::index::satisfies(
                    [](auto const &x) { return true; })));
        }

        /// \brief Get iterator to past-the-end element
        const_iterator cend() const noexcept {
            return const_iterator(data_.qend());
        }

        /// \brief Get iterator to first element
        iterator begin() noexcept {
            return iterator(data_.qbegin(boost::geometry::index::satisfies(
                [](auto const &x) { return true; })));
        }

        /// \brief Get iterator to past-the-end element
        iterator end() noexcept { return iterator(data_.qend()); }

        /// \brief Equality operator
        bool operator==(const boost_tree &rhs) const {
            return std::equal(data_.begin(), data_.end(), rhs.data_.begin(),
                              rhs.data_.end(),
                              [](const auto &a, const auto &b) {
                                  return a.first == b.first &&
                                         mapped_type_custom_equality_operator(
                                             a.second, b.second);
                              });
        }

        /// \brief Inequality operator
        bool operator!=(const boost_tree &rhs) const {
            return !(rhs.operator==(*this));
        }

        /// \brief Get container size
        [[nodiscard]] size_t size() const noexcept { return data_.size(); }

        /// \brief Get container max size
        [[nodiscard]] size_t max_size() const noexcept {
            return std::numeric_limits<size_t>::max();
        }

        /// \brief True if container is empty
        [[nodiscard]] bool empty() const noexcept { return data_.empty(); }

      public /* Non-Modifying Functions: ReversibleContainer Concept */:
        /*
         * The iterators are inverted here.
         * See comments on LegacyBidirectional iterator.
         */
        /// \brief Get iterator to first element in reverse
        std::reverse_iterator<const_iterator> rbegin() const noexcept {
            return std::reverse_iterator(begin());
        }

        /// \brief Get iterator to past-the-end element in reverse
        std::reverse_iterator<const_iterator> rend() const noexcept {
            return std::reverse_iterator(end());
        }

        /// \brief Get iterator to first element in reverse
        std::reverse_iterator<iterator> rbegin() noexcept {
            return std::reverse_iterator(begin());
        }

        /// \brief Get iterator to past-the-end element in reverse
        std::reverse_iterator<iterator> rend() noexcept {
            return std::reverse_iterator(end());
        }

        /// \brief Get iterator to first element in reverse
        std::reverse_iterator<const_iterator> crbegin() const noexcept {
            return std::reverse_iterator(cbegin());
        }

        /// \brief Get iterator to past-the-end element in reverse
        std::reverse_iterator<const_iterator> crend() const noexcept {
            return std::reverse_iterator(cend());
        }

      public /* iterators */:
        /// \brief Get iterator to first element with the predicates
        const_iterator find(const predicate_list_type &ps) const noexcept {
            return const_iterator(
                data_.qbegin(boost::geometry::index::satisfies(
                    [&ps](auto const &x) { return ps.pass_predicate(x); })));
        }

        /// \brief Get iterator to first element with the predicates
        iterator find(const predicate_list_type &ps) noexcept {
            return iterator(data_.qbegin(boost::geometry::index::satisfies(
                [&ps](auto const &x) { return ps.pass_predicate(x); })));
        }

        /// \brief Find point
        const_iterator find(const point_type &p) const {
            boost::geometry::model::box<point_type> query_box(p, p);
            return const_iterator(
                data_.qbegin(boost::geometry::index::intersects(query_box)));
        }

        /// \brief Find point
        iterator find(const point_type &p) {
            boost::geometry::model::box<point_type> query_box(p, p);
            return iterator(
                data_.qbegin(boost::geometry::index::intersects(query_box)));
        }

        /// \brief Find intersection between points and query box
        const_iterator find_intersection(const point_type &min_corner,
                                         const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return const_iterator(
                data_.qbegin(boost::geometry::index::intersects(query_box)));
        }

        /// \brief Find intersection between points and query box
        iterator find_intersection(const point_type &min_corner,
                                   const point_type &max_corner) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return iterator(
                data_.qbegin(boost::geometry::index::intersects(query_box)));
        }

        /// \brief Find intersection between points and query box that satisfy a
        /// predicate
        template <class PREDICATE_TYPE>
        const_iterator find_intersection(const point_type &min_corner,
                                         const point_type &max_corner,
                                         PREDICATE_TYPE fn) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return const_iterator(
                data_.qbegin(boost::geometry::index::intersects(query_box) &&
                             boost::geometry::index::satisfies(fn)));
        }

        /// \brief Find intersection between points and query box that satisfy a
        /// predicate
        template <class PREDICATE_TYPE>
        iterator find_intersection(const point_type &min_corner,
                                   const point_type &max_corner,
                                   PREDICATE_TYPE fn) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return iterator(
                data_.qbegin(boost::geometry::index::intersects(query_box) &&
                             boost::geometry::index::satisfies(fn)));
        }

        /// \brief Find points within a query box
        const_iterator find_within(const point_type &min_corner,
                                   const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return const_iterator(
                data_.qbegin(boost::geometry::index::within(query_box)));
        }

        /// \brief Find points within a query box
        iterator find_within(const point_type &min_corner,
                             const point_type &max_corner) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return iterator(
                data_.qbegin(boost::geometry::index::within(query_box)));
        }

        /// \brief Find points within a query box
        template <class PREDICATE_TYPE>
        const_iterator find_within(const point_type &min_corner,
                                   const point_type &max_corner,
                                   PREDICATE_TYPE fn) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return const_iterator(
                data_.qbegin(boost::geometry::index::within(query_box) &&
                             boost::geometry::index::satisfies(fn)));
        }

        /// \brief Find points within a query box
        template <class PREDICATE_TYPE>
        iterator find_within(const point_type &min_corner,
                             const point_type &max_corner, PREDICATE_TYPE fn) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return iterator(
                data_.qbegin(boost::geometry::index::within(query_box) &&
                             boost::geometry::index::satisfies(fn)));
        }

        /// \brief Find points outside a query box
        const_iterator find_disjoint(const point_type &min_corner,
                                     const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return const_iterator(
                data_.qbegin(boost::geometry::index::disjoint(query_box)));
        }

        /// \brief Find points outside a query box
        iterator find_disjoint(const point_type &min_corner,
                               const point_type &max_corner) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return iterator(
                data_.qbegin(boost::geometry::index::disjoint(query_box)));
        }

        /// \brief Find points outside a query box
        template <class PREDICATE_TYPE>
        const_iterator find_disjoint(const point_type &min_corner,
                                     const point_type &max_corner,
                                     PREDICATE_TYPE fn) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return const_iterator(
                data_.qbegin(boost::geometry::index::disjoint(query_box) &&
                             boost::geometry::index::satisfies(fn)));
        }

        /// \brief Find points outside a query box
        template <class PREDICATE_TYPE>
        iterator find_disjoint(const point_type &min_corner,
                               const point_type &max_corner,
                               PREDICATE_TYPE fn) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            boost::geometry::model::box<point_type> query_box(min_corner_,
                                                              max_corner_);
            return iterator(
                data_.qbegin(boost::geometry::index::disjoint(query_box) &&
                             boost::geometry::index::satisfies(fn)));
        }

        /// \brief Find points closest to a reference point
        const_iterator find_nearest(const point_type &p) const {
            return const_iterator(
                data_.qbegin(boost::geometry::index::nearest(p, 1)));
        }

        /// \brief Find points closest to a reference point
        iterator find_nearest(const point_type &p) {
            return iterator(
                data_.qbegin(boost::geometry::index::nearest(p, 1)));
        }

        /// \brief Find points closest to a reference point
        const_iterator find_nearest(const point_type &p, size_t k) const {
            return const_iterator(
                data_.qbegin(boost::geometry::index::nearest(p, k)));
        }

        /// \brief Find points closest to a reference point
        iterator find_nearest(const point_type &p, size_t k) {
            return iterator(
                data_.qbegin(boost::geometry::index::nearest(p, k)));
        }

        /// \brief Find points closest to a reference point
        template <class PREDICATE_TYPE>
        const_iterator find_nearest(const point_type &p, size_t k,
                                    PREDICATE_TYPE fn) const {
            return const_iterator(
                data_.qbegin(boost::geometry::index::nearest(p, k) &&
                             boost::geometry::index::satisfies(fn)));
        }

        /// \brief Find points closest to a reference point
        template <class PREDICATE_TYPE>
        iterator find_nearest(const point_type &p, size_t k,
                              PREDICATE_TYPE fn) {
            return iterator(
                data_.qbegin(boost::geometry::index::nearest(p, k) &&
                             boost::geometry::index::satisfies(fn)));
        }

        /// \brief Find points closest to a reference point
        const_iterator find_nearest(const box_type &p, size_t k) const {
            boost::geometry::model::box<point_type> boost_box(p.first(),
                                                              p.second());
            return const_iterator(
                data_.qbegin(boost::geometry::index::nearest(boost_box, k)));
        }

        /// \brief Find points closest to a reference point
        iterator find_nearest(const box_type &p, size_t k) {
            boost::geometry::model::box<point_type> boost_box(p.first(),
                                                              p.second());
            return iterator(
                data_.qbegin(boost::geometry::index::nearest(boost_box, k)));
        }

        template <class PREDICATE_TYPE>
        const_iterator find_nearest(const box_type &p, size_t k,
                                    PREDICATE_TYPE fn) const {
            boost::geometry::model::box<point_type> boost_box(p.first(),
                                                              p.second());
            return const_iterator(
                data_.qbegin(boost::geometry::index::nearest(boost_box, k) &&
                             boost::geometry::index::satisfies(fn)));
        }

        template <class PREDICATE_TYPE>
        iterator find_nearest(const box_type &p, size_t k, PREDICATE_TYPE fn) {
            boost::geometry::model::box<point_type> boost_box(p.first(),
                                                              p.second());
            return iterator(
                data_.qbegin(boost::geometry::index::nearest(boost_box, k) &&
                             boost::geometry::index::satisfies(fn)));
        }

      public /* non-modifying functions */:
        /// \brief Get container dimensions
        [[nodiscard]] size_t dimensions() const noexcept {
            return number_of_compile_dimensions;
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

      public /* modifying functions */:
        /// \brief Insert element pair
        /// Insertion removes any point dominated by the point
        /// before inserting the element in the rtree
        /// \param v Pair value <point, value>
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        iterator insert(const value_type &v) {
            data_.insert(v);
            auto it = find(v.first);
            return it;
        }

        /// \brief Insert list of elements
        template <class InputIterator>
        void insert(InputIterator first, InputIterator last) {
            data_.insert(first, last);
        }

        /// Erase element
        iterator erase(const_iterator position) {
            const_iterator next_position = std::next(position);
            if (next_position != end()) {
                key_type next_key = next_position->first;
                data_.remove(*find(position->first));
                return find(next_key);
            } else {
                data_.remove(*find(position->first));
                return end();
            }
        }

        /// Erase element
        size_type erase(const key_type &k) { return data_.remove(*find(k)); }

        /// Remove range of iterators from the front
        iterator erase(const_iterator first, const_iterator last) {
            // Get copy of all keys in the query.
            // Unfortunately, we cannot guarantee erasing elements
            // does not invalidate iterators yet.
            std::vector<key_type> v;
            v.reserve(static_cast<size_t>(std::distance(first, last)));
            while (first != last) {
                v.emplace_back(first->first);
                ++first;
            }

            // Remove elements
            iterator next = end();
            for (auto &k : v) {
                auto it = find(k);
                next = erase(it);
            }
            return next;
        }

        /// Clear the front
        void clear() noexcept { data_.clear(); }

      private:
        tree_type data_;

        dimension_compare comp_{std::less<dimension_type>()};
    };

} // namespace pareto

#endif // PARETO_FRONTS_PREDICATE_TREE_H
