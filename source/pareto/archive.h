//
// Created by Alan Freitas on 2020-05-20.
//

#ifndef PARETO_FRONT_ARCHIVE_H
#define PARETO_FRONT_ARCHIVE_H

#include <iostream>
#include <pareto/front.h>
#include <set>
#include <vector>

namespace pareto {

    template <typename K, size_t M, typename T,
              class Container = spatial_map<K, M, T>>
    class archive : container_with_pool {
      public /* SpatialAdapter Concept */:
        using container_type = Container;

      private /* Internal types */:
        using unprotected_point_type = point<K, M>;
        using protected_point_type = const point<K, M>;
        using unprotected_mapped_type = T;
        using unprotected_key_type = unprotected_point_type;
        using protected_key_type = protected_point_type;
        using unprotected_value_type =
            std::pair<unprotected_key_type, unprotected_mapped_type>;
        using point_type = typename container_type::key_type;

      public /* Forward declarations */:
        template <bool is_const> class iterator_impl;

      public /* Container Concept */:
        using value_type = typename container_type::value_type;
        using reference = typename container_type::reference;
        using const_reference = typename container_type::const_reference;
        using iterator = iterator_impl<false>;
        using const_iterator = iterator_impl<true>;
        using pointer = typename container_type::pointer;
        using const_pointer = typename container_type::const_pointer;
        using difference_type = typename container_type::difference_type;
        using size_type = typename container_type::size_type;

      public /* ReversibleContainer Concept */:
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      public /* AssociativeContainer Concept */:
        using key_type = typename container_type::key_type;
        using mapped_type = typename container_type::mapped_type;
        using key_compare = typename container_type::key_compare;
        using value_compare = typename container_type::value_compare;

      public /* AllocatorAwareContainer Concept */:
        using allocator_type = typename container_type::allocator_type;

      public /* SpatialContainer Concept */:
        static constexpr size_t number_of_compile_dimensions = M;
        static_assert(number_of_compile_dimensions ==
                      container_type::number_of_compile_dimensions);
        using dimension_type = typename container_type::dimension_type;
        using dimension_compare = typename container_type::dimension_compare;
        using box_type = typename container_type::box_type;
        using predicate_list_type =
            typename container_type::predicate_list_type;

      public /* ParetoConcept */:
        using directions_type = std::conditional_t<M == 0, std::vector<uint8_t>,
                                                   std::array<uint8_t, M>>;

      public /* ArchiveConcept */:
        using front_type = front<K, M, T, container_type>;
        using front_set_allocator_type = typename std::allocator_traits<
            typename container_type::allocator_type>::
            template rebind_alloc<front_type>;
        using front_set_type =
            std::set<front_type, std::less<>, front_set_allocator_type>;
        static constexpr size_t default_capacity =
            number_of_compile_dimensions == 0 ? 1000
            : number_of_compile_dimensions <= 10
                ? static_cast<unsigned long long>(50) << (number_of_compile_dimensions - 1)
                : 100000;

      public /* iterators */:
        /// \class Archive iterator
        /// The archive iterator includes pairs of iterators for
        /// fronts and front elements. Each front that might pass
        /// the query is included with the first element that
        /// passes the query. Iterating the archives elements
        /// will iterate each front that might pass the query.
        /// To avoid an unnecessary cost of storing pointers
        /// to all fronts, the methods use lots of algorithms
        /// to make sure 1) only fronts with results are included,
        /// and 2) other fronts are only included later.
        template <bool is_const = false> class iterator_impl {
          private /* Internal Types */:
            template <class TYPE, class CONST_TYPE>
            using const_toggle =
                std::conditional_t<!is_const, TYPE, CONST_TYPE>;

            template <class U>
            using maybe_add_const = const_toggle<U, std::add_const_t<U>>;

            // We don't use const_toggle here because we always
            // want the non-const version of pointers inside the iterator
            // We apply the constness only when dereferencing the elements
            using archive_pointer = archive *;
            using front_set_iterator = typename front_set_type::iterator;
            using container_iterator = typename container_type::iterator;
            using fronts_and_elements_type = typename std::vector<
                std::pair<front_set_iterator, container_iterator>>;
            using query_function = std::function<bool(const value_type &)>;

          public /* LegacyIterator Types */:
            using value_type = maybe_add_const<archive::value_type>;
            using reference =
                const_toggle<archive::reference, archive::const_reference>;
            using difference_type = archive::difference_type;
            using pointer =
                const_toggle<archive::pointer, archive::const_pointer>;
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
                : current_archive_(rhs.current_archive_),
                  current_element_(rhs.current_element_),
                  current_front_idx_(rhs.current_front_idx_) {
                for (auto &[front_it, element_it] : rhs.front_begins_) {
                    front_begins_.emplace_back(front_it, element_it);
                }
                // advance_if_invalid(); // <-- assume we're copying from
                // valid
            }

            /// \brief Copy assignment
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            iterator_impl &operator=(const iterator_impl<rhs_is_const> &rhs) {
                current_archive_ = rhs.current_archive_;
                current_element_ = rhs.current_element_;
                current_front_idx_ = rhs.current_front_idx_;
                for (auto &[front_it, element_it] : rhs.front_begins_) {
                    front_begins_.emplace_back(front_it, element_it);
                }
                return *this;
            }

            /// \brief Destructor
            ~iterator_impl() = default;

          public /* LegacyForwardIterator Constructors */:
            /// \brief Default constructor
            /// You shouldn't be using this constructor very much
            /// because it refers to no archive
            iterator_impl()
                : front_begins_(0),
                  current_archive_(nullptr), current_element_{},
                  current_front_idx_(0) {}

          public /* ContainerConcept Constructors */:
            /// \brief Convert to const iterator
            // NOLINTNEXTLINE(google-explicit-constructor)
            operator iterator_impl<true>() {
                iterator_impl<true> b;
                b = *this;
                return b;
            }

          public /* SpatialContainer Concept Constructors */:
            /// \brief Move constructor
            /// Move constructors are important for spatial containers
            /// because iterators *might* contain large lists of predicates
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            // NOLINTNEXTLINE(google-explicit-constructor)
            iterator_impl(iterator_impl<rhs_is_const> &&rhs)
                : current_archive_(rhs.current_archive_),
                  current_element_(rhs.current_element_),
                  current_front_idx_(rhs.current_front_idx_) {
                for (auto &[front_it, element_it] : rhs.front_begins_) {
                    front_begins_.emplace_back(front_it, element_it);
                }
                // advance_if_invalid(); // <-- assume we're copying from
                // valid
            }

            /// \brief Move assignment
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            iterator_impl &operator=(iterator_impl<rhs_is_const> &&rhs) {
                current_archive_ = rhs.current_archive_;
                current_element_ = rhs.current_element_;
                current_front_idx_ = rhs.current_front_idx_;
                for (auto &[front_it, element_it] : rhs.front_begins_) {
                    front_begins_.emplace_back(front_it, element_it);
                }
                return *this;
            }

          private /* Internal Constructors / Used by Container */:
            /// \brief Construct from archive pointer for all archive elements
            /// Go thought all fronts storing their begin iterators
            /// This could be improved by only logically adding these iterators
            /// We would need to differentiate this from the case where
            /// no iterator should be added for a given front, which is the
            /// most common case
            explicit iterator_impl(archive_pointer ar)
                : front_begins_(0), current_archive_(ar),
                  current_element_(container_iterator()),
                  current_front_idx_(0) {
                if (current_archive_) {
                    auto front_it = ar->fronts_.begin();
                    for (; front_it != ar->fronts_.end(); ++front_it) {
                        auto first_el_it = unconst_reference(*front_it).begin();
                        front_begins_.emplace_back(front_it, first_el_it);
                    }
                    if (!front_begins_.empty()) {
                        current_element_ = front_begins_[0].second;
                    }
                    advance_if_invalid();
                }
            }

            /// \brief Construct an archive iterator for all elements
            /// \param ar Pointer to the archive
            /// \param begins Begin iterator of each internal pareto front
            /// \param current_iter Iterator to the current element in the
            /// current pareto front
            /// \param current_front Front with the current
            /// element
            iterator_impl(archive_pointer ar,
                          const std::vector<front_set_iterator> &begins,
                          container_iterator current_iter,
                          size_t current_front_idx)
                : current_archive_(ar), current_element_(current_iter),
                  current_front_idx_(current_front_idx) {
                auto it = ar->fronts_.begin();
                auto el_it = begins.begin();
                while (it != ar->fronts_.end() && el_it != begins.end()) {
                    front_begins_.emplace_back(it, *el_it);
                    ++it;
                    ++el_it;
                }
                advance_if_invalid();
            }

            /// \brief Construct from limited set of begins
            /// \param begins List of front index and begin iterators for all
            /// fronts \param ar Pointer to archive \param current_iter Current
            /// element in front \param current_front Index of front of current
            /// element \return
            iterator_impl(archive_pointer ar,
                          const fronts_and_elements_type &begins,
                          container_iterator current_iter,
                          size_t current_front_idx)
                : front_begins_(begins), current_archive_(ar),
                  current_element_(current_iter),
                  current_front_idx_(current_front_idx) {
                advance_if_invalid();
            }

            /// \brief Construct from a list of begins. Current iterator is set
            /// to first element of first front. \param begins Begin iterator of
            /// each front \param ar Pointer to the archive
            iterator_impl(archive_pointer ar,
                          const fronts_and_elements_type &begins)
                : iterator_impl(
                      ar, begins,
                      !begins.empty() ? begins[0].second
                      : ar->fronts_.empty()
                          ? typename archive::front_type::iterator()
                          : unconst_reference(*ar->fronts_.rbegin()).end(),
                      0) {}

            /// \brief Construct from a front iterator and its index in the
            /// archive.
            iterator_impl(archive_pointer ar, front_set_iterator front_index,
                          container_iterator current_iter)
                : iterator_impl(ar, fronts_and_elements_type{
                                        {front_index, current_iter}}) {}

          public /* LegacyIterator */:
            /// \brief Dereference iterator
            /// We make a conversion where
            ///     const std::pair<key_type, mapped_type>
            /// becomes
            ///     std::pair<const key_type, mapped_type>&
            /// like it's the case with maps.
            /// The user cannot change the key because it would mess
            /// the data structure. But the user CAN change the key.
            reference operator*() const { return current_element_.operator*(); }

            /// \brief Advance iterator
            iterator_impl &operator++() {
                current_element_.operator++();
                advance_if_invalid();
                return *this;
            }

          public /* LegacyInputIterator */:
            pointer operator->() const { return current_element_.operator->(); }

          public /* LegacyForwardIterator */:
            /// \brief Equality operator
            template <bool rhs_is_const>
            bool operator==(const iterator_impl<rhs_is_const> &rhs) const {
                bool a = is_end();
                bool b = rhs.is_end();
                if (a && b) {
                    return true;
                }
                if (a || b) {
                    return false;
                }
                if (current_archive_ != rhs.current_archive_) {
                    return false;
                }
                if (current_element_ != rhs.current_element_) {
                    return false;
                }
                if (current_front_idx_ != rhs.current_front_idx_) {

                    return false;
                }
                if (front_begins_.size() != rhs.front_begins_.size()) {
                    return false;
                }
                auto lhs_it = front_begins_.begin();
                auto rhs_it = front_begins_.begin();
                while (lhs_it != front_begins_.end()) {
                    if (lhs_it->first != rhs_it->first) {
                        return false;
                    }
                    if (lhs_it->second != rhs_it->second) {
                        return false;
                    }
                    ++lhs_it;
                    ++rhs_it;
                }
                return true;
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
                current_element_.operator++();
                advance_if_invalid();
                return tmp;
            }

          public /* LegacyBidirectionalIterator */:
            /// \brief Decrement iterator
            iterator_impl &operator--() {
                // if there are no pareto iterators in front_begins_
                if (front_begins_.empty() && is_end()) {
                    // this is an empty iterator representing end()
                    // so we need to populate it with the front_begins_
                    // iterators
                    iterator_impl begin_it = current_archive_->begin();
                    front_begins_ = begin_it.front_begins_;
                    current_front_idx_ = front_begins_.size();
                }
                return_to_previous_valid();
                return *this;
            }

            /// \brief Decrement iterator
            /// This is the expected return type for iterators
            iterator_impl operator--(int) { // NOLINT(cert-dcl21-cpp)
                auto tmp = *this;
                return_to_previous_valid();
                return tmp;
            }

          private /* Internal Functions */:
            /// \brief True if this is an iterator to end
            [[nodiscard]] bool is_end() const {
                if (!current_archive_ || front_begins_.empty()) {
                    return true;
                }
                if (current_front_idx_ == front_begins_.size()) {
                    return true;
                }
                return false;
            }

            /// \brief True if this is an iterator to first element
            [[nodiscard]] bool is_begin() const {
                // There are 0 elements
                if (!current_archive_ || front_begins_.empty()) {
                    // So it has to also be begin (besides being end)
                    return true;
                }
                if (current_front_idx_ != 0) {
                    return false;
                }
                return front_begins_[current_front_idx_].first->begin() ==
                       current_element_;
            }

            /// \brief Advance to the next valid element
            void advance_if_invalid() {
                if (!is_end()) {
                    while (current_element_ ==
                           unconst_reference(
                               *front_begins_[current_front_idx_].first)
                               .end()) {
                        ++current_front_idx_;
                        if (current_front_idx_ < front_begins_.size()) {
                            current_element_ =
                                front_begins_[current_front_idx_].second;
                        } else {
                            break;
                        }
                    }
                }
            }

            /// \brief Go to the previous valid element
            void return_to_previous_valid() {
                while (!is_begin()) {
                    // if we are past the last element of the last front
                    if (is_end()) {
                        // if the set of elements to iterate is not empty
                        if (!front_begins_.empty()) {
                            // go to end() of last front
                            current_front_idx_ = front_begins_.size() - 1;
                            current_element_ =
                                unconst_reference(
                                    *front_begins_[current_front_idx_].first)
                                    .end();
                        } else {
                            return;
                        }
                        // if end() of current front is same as begin()
                        if (current_element_ ==
                            front_begins_[current_front_idx_].second) {
                            // do one more iteration
                            continue;
                        } else {
                            // if current element is valid
                            --current_element_;
                            return;
                        }
                    } else if (current_element_ ==
                               front_begins_[current_front_idx_].second) {
                        // if we are at begin of the current front
                        // if this is not the first front (the begin of all
                        // elements)
                        if (current_front_idx_ != 0) {
                            // move to previous front
                            --current_front_idx_;
                            current_element_ =
                                unconst_reference(
                                    *front_begins_[current_front_idx_].first)
                                    .end();
                            // if last element is same as begin
                            if (current_element_ ==
                                front_begins_[current_front_idx_].second) {
                                // do one more iteration
                                continue;
                            } else {
                                // if current element is valid
                                --current_element_;
                                return;
                            }
                        } else {
                            // if this is the begin of first front
                            // return without changing iterator
                            return;
                        }
                    } else {
                        // if this is not a begin and this is not end()
                        current_element_.operator--();
                        return;
                    }
                }
            }

          private:
            /// Fronts where we have elements in which are interested and their
            /// begin's pair<front index, front begin iterator>
            fronts_and_elements_type front_begins_{};

            /// \brief Pointer to the archive to which this iterator belongs
            archive_pointer current_archive_;

            /// \brief Current element we are iterating
            /// This is an iterator to the current element in the current front
            /// For instance, one point in a kd-containers
            container_iterator current_element_;

            /// \brief Current front from the begins list we are iterating
            size_t current_front_idx_{0};

          public:
            /// \brief Let archive access the spatial private constructors
            /// Allow hiding private functions while allowing
            /// manipulation by logical owner of this kind of iterator
            friend class archive;
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
        explicit archive(const allocator_type &alloc =
                             placeholder_allocator<allocator_type>())
            : archive(default_capacity, alloc) {}

        /// \brief Copy constructor
        /// Copy constructors of AllocatorAwareContainers
        /// obtain their instances of the allocator by calling
        /// std::allocator_traits<allocator_type>::
        ///       select_on_container_copy_construction
        /// on the allocator of the container being copied.
        /// \param rhs
        archive(const archive &rhs)
            : fronts_(rhs.fronts_), is_minimization_(rhs.is_minimization_),
              size_(rhs.size_), capacity_(rhs.capacity_), alloc_(rhs.alloc_){};

        /// \brief Copy constructor data but use another allocator
        archive(const archive &rhs, const allocator_type &alloc)
            : fronts_(
                  rhs.fronts_,
                  front_set_allocator_type(
                      construct_allocator<front_set_allocator_type>(alloc))),
              is_minimization_(rhs.is_minimization_), size_(rhs.size_),
              capacity_(rhs.capacity_), alloc_(rhs.alloc_){};

        /// \brief Move constructor
        /// Move constructors obtain their instances of allocators
        /// by move-constructing from the allocator belonging to
        /// the old container
        archive(archive &&rhs) noexcept
            : fronts_(std::move(rhs.fronts_)),
              is_minimization_(std::move(rhs.is_minimization_)),
              size_(std::move(rhs.size_)), capacity_(std::move(rhs.capacity_)),
              alloc_(std::move(rhs.alloc_)) {}

        /// \brief Move constructor data but use new allocator
        archive(archive &&rhs, const allocator_type &alloc) noexcept
            : fronts_(
                  std::move(rhs.fronts_),
                  front_set_allocator_type(
                      construct_allocator<front_set_allocator_type>(alloc))),
              is_minimization_(std::move(rhs.is_minimization_)),
              size_(std::move(rhs.size_)), capacity_(std::move(rhs.capacity_)),
              alloc_(rhs.alloc_) {}

        /// \brief Destructor
        ~archive() = default;

      public /* Constructors: AssociativeContainer + AllocatorAwareContainer */:
        /// \brief Create container with custom comparison function
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        explicit archive(const dimension_compare &comp,
                         const allocator_type &alloc =
                             placeholder_allocator<allocator_type>())
            : archive(default_capacity, comp, alloc) {}

        /// \brief Construct with iterators + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        archive(InputIt first, InputIt last,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(default_capacity, first, last, comp, alloc) {}

        /// \brief Construct with list + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        archive(std::initializer_list<value_type> il,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(default_capacity, il, comp, alloc) {}

        /// \brief Construct with iterators
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        archive(InputIt first, InputIt last, const allocator_type &alloc)
            : archive(default_capacity, first, last, alloc) {}

        /// \brief Construct with initializer list
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        archive(std::initializer_list<value_type> il,
                const allocator_type &alloc)
            : archive(default_capacity, il, alloc) {}

      public /* Constructors: ParetoContainer + AllocatorAwareContainer */:
        /*
         * The pareto concept extends all constructors that take initial
         * values with
         * 1) an extra parameter to define minimization directions or
         * 2) replacing the initializer list of elements with an initializer
         *    list of directions.
         * These directions are fixed after the container is
         * constructed.
         */
        /// \brief Construct with iterators + directions + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt, class DirectionIt>
        archive(InputIt first, InputIt last, DirectionIt first_dir,
                DirectionIt last_dir,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(default_capacity, first, last, first_dir, last_dir, comp,
                      alloc) {}

        /// \brief Construct with list + direction + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class DirectionIt>
        archive(std::initializer_list<value_type> il, DirectionIt first_dir,
                DirectionIt last_dir,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(default_capacity, il, first_dir, last_dir, comp, alloc) {}

        /// \brief Construct with list + direction + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        archive(InputIt first, InputIt last, std::initializer_list<bool> il_dir,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(default_capacity, first, last, il_dir, comp, alloc) {}

        /// \brief Construct with list + direction list + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        archive(std::initializer_list<value_type> il,
                std::initializer_list<bool> il_dir,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(default_capacity, il, il_dir, comp, alloc) {}

        /// \brief Construct with direction list + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        archive(std::initializer_list<bool> il_dir,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(default_capacity, il_dir, comp, alloc) {}

        /// \brief Construct with iterators
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt, class DirectionIt>
        archive(InputIt first, InputIt last, DirectionIt first_dir,
                DirectionIt last_dir, const allocator_type &alloc)
            : archive(default_capacity, first, last, first_dir, last_dir,
                      alloc) {}

        /// \brief Construct with initializer list
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class DirectionIt>
        archive(std::initializer_list<value_type> il, DirectionIt first_dir,
                DirectionIt last_dir, const allocator_type &alloc)
            : archive(default_capacity, il, first_dir, last_dir, alloc) {}

        /// \brief Construct with initializer list
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        archive(InputIt first, InputIt last, std::initializer_list<bool> il_dir,
                const allocator_type &alloc)
            : archive(default_capacity, first, last, il_dir, alloc) {}

        /// \brief Construct with initializer list
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        archive(std::initializer_list<value_type> il,
                std::initializer_list<bool> il_dir, const allocator_type &alloc)
            : archive(default_capacity, il, il_dir, alloc) {}

        /// \brief Construct with initializer list
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        archive(std::initializer_list<bool> il_dir, const allocator_type &alloc)
            : archive(default_capacity, il_dir, alloc) {}

      public /* Constructors: ArchiveContainer + AllocatorAwareContainer */:
        /*
         * The constructors in the archive container concept reimplement ALL
         * constructor with an optional first parameter to define the
         * archive capacity.
         */

        /// \brief Create an empty container + capacity
        explicit archive(size_t capacity,
                         const allocator_type &alloc =
                             placeholder_allocator<allocator_type>())
            : fronts_(front_set_allocator_type(
                  construct_allocator<front_set_allocator_type>(alloc))),
              is_minimization_(), size_(0), capacity_(capacity),
              alloc_(construct_allocator<allocator_type>(alloc)) {
            initialize_directions();
        }

        /// \brief Create container with custom comparison function + capacity
        explicit archive(size_t capacity, const dimension_compare &comp,
                         const allocator_type &alloc =
                             placeholder_allocator<allocator_type>())
            : fronts_(front_set_allocator_type(
                  construct_allocator<front_set_allocator_type>(alloc))),
              capacity_(capacity),
              alloc_(construct_allocator<allocator_type>(alloc)), comp_(comp) {
            initialize_directions();
        }

        /// \brief Construct with iterators + comparison + capacity
        template <class InputIt>
        archive(size_t capacity, InputIt first, InputIt last,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : fronts_(front_set_allocator_type(
                  construct_allocator<front_set_allocator_type>(alloc))),
              capacity_(capacity),
              alloc_(construct_allocator<allocator_type>(alloc)), comp_(comp) {
            initialize_directions();
            // note: archive::insert(...) is very different from
            // front::insert(...)
            insert(first, last);
        }

        /// \brief Construct with list + comparison + capacity
        archive(size_t capacity, std::initializer_list<value_type> il,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(capacity, il.begin(), il.end(), comp, alloc) {}

        /// \brief Construct with iterators + capacity
        template <class InputIt>
        archive(size_t capacity, InputIt first, InputIt last,
                const allocator_type &alloc)
            : fronts_(front_set_allocator_type(
                  construct_allocator<front_set_allocator_type>(alloc))),
              capacity_(capacity),
              alloc_(construct_allocator<allocator_type>(alloc)) {
            initialize_directions();
            // note: this->insert(...) is very different from
            // fronts_.insert(...)
            insert(first, last);
        }

        /// \brief Construct with initializer list + capacity
        archive(size_t capacity, std::initializer_list<value_type> il,
                const allocator_type &alloc)
            : archive(capacity, il.begin(), il.end(), alloc) {}

        /// \brief Construct with iterators + directions + comparison + capacity
        template <class InputIt, class DirectionIt>
        archive(size_t capacity, InputIt first, InputIt last,
                DirectionIt first_dir, DirectionIt last_dir,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : fronts_(front_set_allocator_type(
                  construct_allocator<front_set_allocator_type>(alloc))),
              capacity_(capacity),
              alloc_(construct_allocator<allocator_type>(alloc)), comp_(comp) {
            initialize_directions(first_dir, last_dir);
            // note: this->insert(...) is very different from data_.insert(...)
            insert(first, last);
        }

        /// \brief Construct with list + direction + comparison + capacity
        template <class DirectionIt>
        archive(size_t capacity, std::initializer_list<value_type> il,
                DirectionIt first_dir, DirectionIt last_dir,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(capacity, il.begin(), il.end(), first_dir, last_dir, comp,
                      alloc) {}

        /// \brief Construct with list + direction + comparison + capacity
        template <class InputIt>
        archive(size_t capacity, InputIt first, InputIt last,
                std::initializer_list<bool> il_dir,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(capacity, first, last, il_dir.begin(), il_dir.end(), comp,
                      alloc) {}

        /// \brief Construct with list + direction list + comparison +
        /// capacity
        archive(size_t capacity, std::initializer_list<value_type> il,
                std::initializer_list<bool> il_dir,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(capacity, il.begin(), il.end(), il_dir.begin(),
                      il_dir.end(), comp, alloc) {}

        /// \brief Construct with direction list + comparison + capacity
        archive(size_t capacity, std::initializer_list<bool> il_dir,
                const dimension_compare &comp = dimension_compare(),
                const allocator_type &alloc =
                    placeholder_allocator<allocator_type>())
            : archive(capacity, {}, il_dir, comp, alloc) {}

        /// \brief Construct with iterators + capacity
        template <class InputIt, class DirectionIt>
        archive(size_t capacity, InputIt first, InputIt last,
                DirectionIt first_dir, DirectionIt last_dir,
                const allocator_type &alloc)
            : fronts_(front_set_allocator_type(
                  construct_allocator<front_set_allocator_type>(alloc))),
              capacity_(capacity),
              alloc_(construct_allocator<allocator_type>(alloc)) {
            initialize_directions(first_dir, last_dir);
            // note: this->insert(...) is very different from data_.insert(...)
            insert(first, last);
        }

        /// \brief Construct with initializer list + capacity
        template <class DirectionIt>
        archive(size_t capacity, std::initializer_list<value_type> il,
                DirectionIt first_dir, DirectionIt last_dir,
                const allocator_type &alloc)
            : archive(capacity, il.begin(), il.end(), first_dir, last_dir,
                      alloc) {}

        /// \brief Construct with initializer list + capacity
        template <class InputIt>
        archive(size_t capacity, InputIt first, InputIt last,
                std::initializer_list<bool> il_dir, const allocator_type &alloc)
            : archive(capacity, first, last, il_dir.begin(), il_dir.end(),
                      alloc) {}

        /// \brief Construct with initializer list + capacity
        archive(size_t capacity, std::initializer_list<value_type> il,
                std::initializer_list<bool> il_dir, const allocator_type &alloc)
            : archive(capacity, il.begin(), il.end(), il_dir.begin(),
                      il_dir.end(), alloc) {}

        /// \brief Construct with initializer list + capacity
        archive(size_t capacity, std::initializer_list<bool> il_dir,
                const allocator_type &alloc)
            : archive(capacity, {}, il_dir, alloc) {}

      public /* Assignment: Container + AllocatorAwareContainer */:
        /// \brief Copy assignment
        /// Copy-assignment will replace the allocator only if
        /// std::allocator_traits<allocator_type>::
        ///     propagate_on_container_copy_assignment::value is true
        /// \note if the allocators of the source and the target containers
        /// do not compare equal, copy assignment has to deallocate the
        /// target's memory using the old allocator and then allocate it
        /// using the new allocator before copying the elements
        archive &operator=(const archive &rhs) {
            if (&rhs == this) {
                return *this;
            }
            fronts_ = rhs.fronts_;
            is_minimization_ = rhs.is_minimization_;
            size_ = rhs.size_;
            capacity_ = rhs.capacity_;
            constexpr bool should_copy = std::allocator_traits<
                allocator_type>::propagate_on_container_copy_assignment::value;
            if constexpr (should_copy) {
                alloc_ = rhs.alloc_;
            }
            comp_ = rhs.comp_;
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
        archive &operator=(archive &&rhs) noexcept {
            if (&rhs == this) {
                return *this;
            }
            fronts_.clear();
            is_minimization_ = std::move(rhs.is_minimization_);
            size_ = rhs.size_;
            capacity_ = rhs.capacity_;
            const bool should_move = std::allocator_traits<
                allocator_type>::propagate_on_container_move_assignment::value;
            if constexpr (should_move) {
                alloc_ = std::move(rhs.alloc_);
                fronts_ = std::move(rhs.fronts_);
            } else {
                const bool same_memory_resources = alloc_ == rhs.alloc_;
                if (same_memory_resources) {
                    fronts_ = std::move(rhs.fronts_);
                } else {
                    fronts_ = rhs.fronts_;
                }
            }
            comp_ = std::move(rhs.comp_);
            return *this;
        }

      public /* Assignment: AssociativeContainer */:
        /// \brief Initializer list assignment
        archive &operator=(std::initializer_list<value_type> il) noexcept {
            fronts_.clear();
            insert(il.begin(), il.end());
            return *this;
        }

      public /* Non-Modifying Functions: AllocatorAwareContainer */:
        /// \brief Obtains a copy of the allocator
        /// The accessor get_allocator() obtains a copy of
        /// the allocator that was used to construct the
        /// container or installed by the most recent allocator
        /// replacement operation
        allocator_type get_allocator() const noexcept { return alloc_; }

      public /* Element Access / Map Concept */:
        /// \brief Access key (throw error if it doesn't exist)
        mapped_type &at(const key_type &k) {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("archive::at:  key not found");
            }
        }

        /// \brief Access key (throw error if it doesn't exist)
        const mapped_type &at(const key_type &k) const {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("archive::at:  key not found");
            }
        }

        /// \brief Access key (create key if it doesn't exist)
        mapped_type &operator[](const key_type &k) {
            auto it = find(k);
            if (it != end()) {
                // return reference to mapped type in the containers
                return it->second;
            } else {
                // include element in the containers with a default
                // mapped type
                auto [it2, ok] = insert(std::make_pair(k, mapped_type()));
                if (ok) {
                    // return reference to mapped type in the containers
                    return it2->second;
                } else {
                    // if item was not included in the containers
                    // return reference to a placeholder
                    // just to avoid throwing an error
                    // nothing was included in the containers because
                    // the element is dominated
                    static mapped_type placeholder;
                    return placeholder;
                }
            }
        }

        /// \brief Access key (create key if it doesn't exist)
        mapped_type &operator[](key_type &&k) {
            auto it = find(k);
            if (it != end()) {
                // return reference to mapped type in the containers
                return it->second;
            } else {
                // include element in the containers with a default
                // mapped type
                auto [it2, ok] =
                    insert(std::make_pair(std::move(k), mapped_type()));
                if (ok) {
                    // return reference to mapped type in the containers
                    return it2->second;
                } else {
                    // if item was not included in the containers
                    // return reference to a placeholder
                    // just to avoid throwing an error
                    // nothing was included in the containers because
                    // the element is dominated
                    static mapped_type placeholder;
                    return placeholder;
                }
            }
        }

        /// \brief Access key (create key if it doesn't exist)
        template <typename... Targs>
        mapped_type &operator()(const dimension_type &k, const Targs &...ks) {
            constexpr size_t m = sizeof...(Targs) + 1;
            assert(number_of_compile_dimensions == 0 ||
                   number_of_compile_dimensions == m);
            point_type p(m);
            copy_pack(p.begin(), k, ks...);
            return operator[](p);
        }

      public /* Non-Modifying Functions: Container Concept */:
        /// \brief Get iterator to first element
        const_iterator begin() const noexcept { return cbegin(); }

        /// \brief Get iterator to past-the-end element
        const_iterator end() const noexcept { return cend(); }

        /// \brief Get iterator to first element
        const_iterator cbegin() const noexcept {
            return (const_cast<archive *>(this))->begin();
        }

        /// \brief Get iterator to past-the-end element
        const_iterator cend() const noexcept {
            return (const_cast<archive *>(this))->end();
        }

        /// \brief Get iterator to first element
        iterator begin() noexcept { return iterator(this); }

        /// \brief Get iterator to past-the-end element
        iterator end() noexcept {
            return iterator(
                this, typename iterator::fronts_and_elements_type{},
                !fronts_.empty()
                    ? unconst_reference(*std::prev(fronts_.end())).end()
                    : typename front_type::iterator(),
                fronts_.size());
        }

      public /* Non-Modifying Functions: ReversibleContainer Concept */:
        /// \brief Get iterator to first element in reverse
        std::reverse_iterator<const_iterator> rbegin() const noexcept {
            return std::reverse_iterator(end());
        }

        /// \brief Get iterator to past-the-end element in reverse
        std::reverse_iterator<const_iterator> rend() const noexcept {
            return std::reverse_iterator(begin());
        }

        /// \brief Get iterator to first element in reverse
        std::reverse_iterator<iterator> rbegin() noexcept {
            return std::reverse_iterator(end());
        }

        /// \brief Get iterator to past-the-end element in reverse
        std::reverse_iterator<iterator> rend() noexcept {
            return std::reverse_iterator(begin());
        }

        /// \brief Get iterator to first element in reverse
        std::reverse_iterator<const_iterator> crbegin() const noexcept {
            return std::reverse_iterator(cend());
        }

        /// \brief Get iterator to past-the-end element in reverse
        std::reverse_iterator<const_iterator> crend() const noexcept {
            return std::reverse_iterator(cbegin());
        }

      public /* Archive Concept */:
        /// \brief Get an iterator to first front
        auto begin_front() const noexcept { return fronts_.begin(); }

        /// \brief Get an iterator to the past-the-end front
        auto end_front() const noexcept { return fronts_.end(); }

        /// \brief Get an iterator to first front
        auto cbegin_front() const noexcept { return fronts_.cbegin(); }

        /// \brief Get an iterator to the past-the-end front
        auto cend_front() const noexcept { return fronts_.cend(); }

        /// \brief Get an iterator to first front
        auto begin_front() noexcept { return fronts_.begin(); }

        /// \brief Get an iterator to the past-the-end front
        auto end_front() noexcept { return fronts_.end(); }

        /// \brief Get an iterator to first front
        auto rbegin_front() const noexcept { return fronts_.rbegin(); }

        /// \brief Get an iterator to the past-the-end front
        auto rend_front() const noexcept { return fronts_.rend(); }

        /// \brief Get an iterator to first front
        auto rbegin_front() noexcept { return fronts_.rbegin(); }

        /// \brief Get an iterator to the past-the-end front
        auto rend_front() noexcept { return fronts_.rend(); }

        /// \brief Get an iterator to first front
        auto crbegin_front() const noexcept {
            return std::reverse_iterator<
                typename front_set_type::const_iterator>(fronts_.cbegin());
        }

        /// \brief Get an iterator to the past-the-end front
        auto crend_front() const noexcept {
            return std::reverse_iterator<
                typename front_set_type::const_iterator>(fronts_.cend());
        }

      public /* Non-Modifying Functions: ReversibleContainer Concept */:
        /// \brief True if container is empty
        [[nodiscard]] bool empty() const noexcept { return fronts_.empty(); }

        /// \brief Get container size
        [[nodiscard]] size_t size() const noexcept {
            assert(size_ == total_front_sizes());
            return size_;
        }

        /// \brief Get container max size
        [[nodiscard]] size_type max_size() const noexcept {
            return std::numeric_limits<size_type>::max();
        }

      public /* Non-Modifying Functions / Capacity / Spatial Concept */:
        /// \brief Get number of dimensions of archive points
        size_type dimensions() const noexcept {
            if constexpr (number_of_compile_dimensions > 0) {
                // compile time
                return number_of_compile_dimensions;
            } else {
                // infer from minimization vector
                return is_minimization_.size();
            }
        }

        /// \brief Get maximum value in a given dimension
        dimension_type max_value(size_t dimension) const {
            return is_minimization(dimension) ? worst(dimension)
                                              : ideal(dimension);
        }

        /// \brief Get minimum value in a given dimension
        dimension_type min_value(size_t dimension) const {
            return is_minimization(dimension) ? ideal(dimension)
                                              : worst(dimension);
        }

      public /* Reference points / Pareto Concept */:
        /// \brief Ideal point in the archive
        point_type ideal() const { return fronts_.begin()->ideal(); }

        /// \brief Ideal value in an archive dimension
        dimension_type ideal(size_t d) const {
            return fronts_.begin()->ideal(d);
        }

        /// \brief The nadir point is the worst point among the
        /// non-dominated points. There is a difference
        /// between the nadir point and the worst point
        /// for archives.
        point_type nadir() const { return fronts_.begin()->nadir(); }

        /// \brief Nadir value in dimension d
        dimension_type nadir(size_t d) const {
            return fronts_.begin()->nadir(d);
        }

        /// \brief Worst point in the archive
        /// Worst is the same as nadir for fronts.
        /// In archives, worst != nadir because there are
        /// many fronts.
        point_type worst() const {
            auto front_it = fronts_.begin();
            point_type worst_point = front_it->worst();
            ++front_it;
            for (; front_it != fronts_.end(); ++front_it) {
                point_type tmp = front_it->worst();
                for (size_t j = 0; j < front_it->dimensions(); ++j) {
                    if (is_minimization(j)) {
                        if (tmp[j] > worst_point[j]) {
                            worst_point[j] = tmp[j];
                        }
                    } else {
                        if (tmp[j] < worst_point[j]) {
                            worst_point[j] = tmp[j];
                        }
                    }
                }
            }
            return worst_point;
        }

        /// \brief Worst value in archive dimension d
        dimension_type worst(size_t d) const {
            auto front_it = fronts_.begin();
            dimension_type worst_value = front_it->worst(d);
            ++front_it;
            for (; front_it != fronts_.end(); ++front_it) {
                dimension_type w = front_it->worst(d);
                if (is_minimization(d)) {
                    if (w > worst_value) {
                        worst_value = w;
                    }
                } else {
                    if (w < worst_value) {
                        worst_value = w;
                    }
                }
            }
            return worst_value;
        }

        /// \brief Check if all dimensions are minimization
        [[nodiscard]] bool is_minimization() const noexcept {
            return std::all_of(is_minimization_.begin(), is_minimization_.end(),
                               [](auto i) { return i == uint8_t(1); });
        }

        /// \brief Check if all dimensions are maximization
        [[nodiscard]] bool is_maximization() const noexcept {
            return std::all_of(is_minimization_.begin(), is_minimization_.end(),
                               [](auto i) { return i == uint8_t(0); });
        }

        /// \brief Check if i-th dimension is minimization
        [[nodiscard]] bool is_minimization(size_t dimension) const noexcept {
            return is_minimization_[dimension] > 0;
        }

        /// \brief Check if i-th dimension is maximization
        [[nodiscard]] bool is_maximization(size_t dimension) const noexcept {
            return is_minimization_[dimension] == 0;
        }

      public /* Non-modifiers / Archive Concept */:
        /// \brief Get container max size
        [[nodiscard]] size_t capacity() const noexcept { return capacity_; }

        /// \brief Get number of fronts in the archive
        size_type size_fronts() const noexcept { return fronts_.size(); }

      public /* Non-Modifying Functions / Reference points / Pareto Concept */:
        /// \brief Check if this archive weakly dominates a point
        /// A front a weakly dominates a solution b if a has at least
        /// one solution better than b in at least one objective and
        /// is at least as good as b in all other objectives.
        /// \see
        /// http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool dominates(const point_type &p) const {
            if (fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->dominates(p);
        }

        /// \brief Check if this archive strongly dominates a point
        /// A front a strongly dominates a solution b if a has a solution
        /// that is strictly better than b in all objectives.
        bool strongly_dominates(const point_type &p) const {
            if (fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->strongly_dominates(p);
        }

        /// \brief True if archive is partially dominated by p
        bool is_partially_dominated_by(const point_type &p) const {
            if (fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->is_partially_dominated_by(p);
        }

        /// \brief True if archive is completely dominated by p
        bool is_completely_dominated_by(const point_type &p) const {
            if (fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->is_completely_dominated_by(p);
        }

        /// \brief Check if this archive weakly dominates a point
        /// A solution a weakly dominates a solution b if a is better
        /// than b in at least one objective and is as good as b in
        /// all other objectives.
        /// \see
        /// http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool non_dominates(const point_type &p) const {
            if (fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->non_dominates(p);
        }

        /// \brief Check if this archive dominates another front
        bool dominates(const front_type &p) const {
            if (fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->dominates(p);
        }

        /// \brief Check if this archive weakly dominates another front
        bool strongly_dominates(const front_type &p) const {
            if (fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->strongly_dominates(p);
        }

        /// \brief Check if this archive weakly dominates another front
        bool is_partially_dominated_by(const front_type &p) const {
            if (fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->is_partially_dominated_by(p);
        }

        /// \brief Check if this archive weakly dominates another front
        bool is_completely_dominated_by(const front_type &p) const {
            if (fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->is_completely_dominated_by(p);
        }

        /// \brief Check if this archive weakly dominates another front
        bool non_dominates(const front_type &p) const {
            if (fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->non_dominates(p);
        }

        /// \brief Check if this archive dominates another front
        bool dominates(const archive &rhs) const {
            if (fronts_.empty()) {
                return false;
            }
            if (rhs.fronts_.empty()) {
                return true;
            }
            return fronts_.begin()->dominates(*rhs.fronts_.begin());
        }

        /// \brief Check if this archive weakly dominates another front
        bool strongly_dominates(const archive &rhs) const {
            if (fronts_.empty()) {
                return false;
            }
            if (rhs.fronts_.empty()) {
                return true;
            }
            return fronts_.begin()->strongly_dominates(*rhs.fronts_.begin());
        }

        /// \brief Check if this archive weakly dominates another front
        bool is_partially_dominated_by(const archive &rhs) const {
            if (fronts_.empty()) {
                return true;
            }
            if (rhs.fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->is_partially_dominated_by(
                *rhs.fronts_.begin());
        }

        /// \brief Check if this archive weakly dominates another front
        bool is_completely_dominated_by(const archive &rhs) const {
            if (fronts_.empty()) {
                return true;
            }
            if (rhs.fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->is_completely_dominated_by(
                *rhs.fronts_.begin());
        }

        /// \brief Check if this archive weakly dominates another front
        bool non_dominates(const archive &rhs) const {
            if (fronts_.empty()) {
                return rhs.fronts_.empty();
            }
            if (rhs.fronts_.empty()) {
                return false;
            }
            return fronts_.begin()->non_dominates(*rhs.fronts_.begin());
        }

      public /* Indicators / Pareto Concept */:
        /// \brief Get hypervolume from the nadir point
        /// If there is no specific reference point, we use
        /// the nadir point as reference.
        /// \param reference_point Reference point
        /// \return Hypervolume of this front
        dimension_type hypervolume() const {
            if (fronts_.empty()) {
                return dimension_type{0};
            }
            return fronts_.begin()->hypervolume();
        }

        /// \brief Get exact hypervolume
        /// Use the other hypervolume function if this takes too long.
        /// \param reference_point Reference point
        /// \return Hypervolume of this front
        dimension_type hypervolume(point_type reference_point) const {
            if (fronts_.empty()) {
                return dimension_type{0};
            }
            return fronts_.begin()->hypervolume(reference_point);
        }

        /// \brief Get hypervolume with monte-carlo simulation
        dimension_type hypervolume(size_t sample_size) const {
            if (fronts_.empty()) {
                return dimension_type{0};
            }
            return hypervolume(sample_size, nadir());
        }

        /// \brief Get hypervolume with monte-carlo simulation
        /// This function uses monte-carlo simulation as getting the
        /// exact indicator is too costly.
        /// \param reference_point Reference for the hyper-volume
        /// \param sample_size Number of samples for the simulation
        /// \return Hypervolume of the pareto front
        dimension_type hypervolume(size_t sample_size,
                                   const point_type &reference_point) const {
            if (fronts_.empty()) {
                return dimension_type{0};
            }
            return fronts_.begin()->hypervolume(sample_size, reference_point);
        }

        /// \brief Coverage indicator
        /// \see http://www.optimization-online.org/DB_FILE/2018/10/6887.pdf
        double coverage(const front_type &rhs) const {
            if (fronts_.empty()) {
                return dimension_type{0};
            }
            if (rhs.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->coverage(rhs);
        }

        /// \brief Coverage indicator
        /// \see http://www.optimization-online.org/DB_FILE/2018/10/6887.pdf
        double coverage(const archive &rhs) const {
            if (fronts_.empty()) {
                return dimension_type{0};
            }
            if (rhs.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->coverage(*rhs.fronts_.begin());
        }

        /// \brief Ratio of coverage indicators
        double coverage_ratio(const front_type &rhs) const {
            if (fronts_.empty()) {
                return dimension_type{0};
            }
            if (rhs.empty()) {
                return dimension_type{1};
            }
            return fronts_.begin()->coverage_ratio(rhs);
        }

        /// \brief Ratio of coverage indicators
        double coverage_ratio(const archive &rhs) const {
            if (fronts_.empty()) {
                return dimension_type{0};
            }
            if (rhs.empty()) {
                return dimension_type{1};
            }
            return fronts_.begin()->coverage_ratio(*rhs.fronts_.begin());
        }

        /// \brief Generational distance
        double gd(const front_type &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->gd(reference);
        }

        /// \brief Generational distance
        double gd(const archive &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference.fronts_.empty()) {
                return dimension_type{0};
            }
            return fronts_.begin()->gd(*reference.fronts_.begin());
        }

        /// \brief Standard deviation from the generational distance
        /// It measures the deformation of the Pareto set
        /// approximation according to a Pareto optimal solution set.
        double std_gd(const front_type &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->std_gd(reference);
        }

        /// \brief Standard deviation from the generational distance
        /// It measures the deformation of the Pareto set
        /// approximation according to a Pareto optimal solution set.
        double std_gd(const archive &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference.fronts_.empty()) {
                return dimension_type{0};
            }
            return fronts_.begin()->std_gd(*reference.fronts_.begin());
        }

        /// \brief Inverted generational distance
        double igd(const front_type &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->igd(reference);
        }

        /// \brief Inverted generational distance
        double igd(const archive &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference.fronts_.empty()) {
                return dimension_type{0};
            }
            return fronts_.begin()->igd(*reference.fronts_.begin());
        }

        /// \brief Standard deviation from the inverted generational distance
        double std_igd(const front_type &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->std_igd(reference);
        }

        /// \brief Standard deviation from the inverted generational distance
        double std_igd(const archive &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference.fronts_.empty()) {
                return dimension_type{0};
            }
            return fronts_.begin()->std_igd(*reference.fronts_.begin());
        }

        /// \brief Hausdorff indicator: max(GD,IGD)
        double hausdorff(const front_type &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->hausdorff(reference);
        }

        /// \brief Hausdorff indicator: max(GD,IGD)
        double hausdorff(const archive &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference.fronts_.empty()) {
                return dimension_type{0};
            }
            return fronts_.begin()->hausdorff(*reference.fronts_.begin());
        }

        /// \brief IGD+ indicator
        double igd_plus(const front_type &reference_front) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->igd_plus(reference_front);
        }

        /// \brief IGD+ indicator
        double igd_plus(const archive &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference.fronts_.empty()) {
                return dimension_type{0};
            }
            return fronts_.begin()->igd_plus(*reference.fronts_.begin());
        }

        /// \brief STD-IGD+ indicator
        double std_igd_plus(const front_type &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference.fronts_.empty()) {
                return dimension_type{0};
            }
            return fronts_.begin()->std_igd_plus(reference);
        }

        /// \brief STD-IGD+ indicator
        double std_igd_plus(const archive &reference) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference.fronts_.empty()) {
                return dimension_type{0};
            }
            return fronts_.begin()->std_igd_plus(*reference.fronts_.begin());
        }

        /// \brief Uniformity metric
        /// This is the minimal distance between two points of the Pareto front
        /// approximation. This measure is straightforward to compute and easy
        /// to understand. However, it does not really provide pertinent
        /// information on the repartition of the points along the Pareto
        /// front approximation.
        [[nodiscard]] double uniformity() const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->uniformity();
        }

        /// \brief Average distance between points
        [[nodiscard]] double average_distance() const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->average_distance();
        }

        /// \brief Average nearest distance between points
        [[nodiscard]] double average_nearest_distance(size_t k = 5) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->average_nearest_distance(k);
        }

        /// \brief Crowding distance of an element
        [[nodiscard]] double average_crowding_distance() const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return fronts_.begin()->average_crowding_distance();
        }

        /// \brief Crowding distance of an element
        double crowding_distance(const_iterator element, point_type worst_point,
                                 point_type ideal_point) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return element.front_begins_[element.current_front_idx_]
                .first->crowding_distance(element.current_element_, worst_point,
                                          ideal_point);
        }

        /// \brief Crowding distance of an element
        double crowding_distance(const_iterator element) const {
            if (fronts_.empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            return element.front_begins_[element.current_front_idx_]
                .first->crowding_distance(element.current_element_, worst(),
                                          ideal());
        }

        /// \brief Crowding distance of a point in the set
        double crowding_distance(const point_type &point) const {
            auto element = find(point);
            if (element != end()) {
                return crowding_distance(element, worst(), ideal());
            } else {
                auto nearest_element = find_nearest(point);
                return crowding_distance(nearest_element, worst(), ideal());
            }
        }

        /// \brief Direct conflict between objectives
        /// Use this conflict measure when:
        /// - Objectives are equally important
        /// - Objectives are in the same units
        /// This conflict measure is insensitive to summation or subtractions
        /// For instance, use this conflict measure when both objectives
        /// are measured in dollars.
        /// \see Freitas, Alan RR, Peter J. Fleming, and Frederico G. Guimarães.
        /// "Aggregation trees for visualization and dimension reduction in
        ///  many-objective optimization." Information Sciences 298 (2015):
        ///  288-314.
        /// Page 299
        /// Table 2
        dimension_type direct_conflict(const size_t a, const size_t b) const {
            dimension_type ideal_a = ideal(a);
            dimension_type ideal_b = ideal(b);
            dimension_type c_ab = 0;
            for (const auto &[x_i, value] : *this) {
                // distance to ideal
                const dimension_type x_line_ia =
                    is_minimization(a) ? x_i[a] - ideal_a : ideal_a - x_i[a];
                const dimension_type x_line_ib =
                    is_minimization(b) ? x_i[b] - ideal_b : ideal_b - x_i[b];
                c_ab += std::max(x_line_ia, x_line_ib) -
                        std::min(x_line_ia, x_line_ib);
            }
            return c_ab;
        }

        [[nodiscard]] double normalized_direct_conflict(const size_t a,
                                                        const size_t b) const {
            // max value in each term is max_a-min_a or max_b-min_b
            dimension_type worst_a = worst(a);
            dimension_type worst_b = worst(b);
            dimension_type ideal_a = ideal(a);
            dimension_type ideal_b = ideal(b);
            dimension_type range_a =
                is_minimization(a) ? worst_a - ideal_a : ideal_a - worst_a;
            dimension_type range_b =
                is_minimization(b) ? worst_b - ideal_b : ideal_b - worst_b;
            return static_cast<double>(direct_conflict(a, b)) /
                   (std::max(range_a, range_b) * size());
        }

        /// \brief Maxmin conflict between objectives
        /// Use this conflict measure when:
        /// - Objective importance is proportional to its range of values
        /// - Objectives are in comparable units
        /// This conflict measure is insensitive to linear normalizations.
        /// \see Freitas, Alan RR, Peter J. Fleming, and Frederico G. Guimarães.
        /// "Aggregation trees for visualization and dimension reduction in
        ///  many-objective optimization." Information Sciences 298 (2015):
        ///  288-314.
        /// Page 299
        /// Table 2
        [[nodiscard]] double maxmin_conflict(const size_t a,
                                             const size_t b) const {
            dimension_type worst_a = worst(a);
            dimension_type worst_b = worst(b);
            dimension_type ideal_a = ideal(a);
            dimension_type ideal_b = ideal(b);
            dimension_type range_a =
                is_minimization(a) ? worst_a - ideal_a : ideal_a - worst_a;
            dimension_type range_b =
                is_minimization(b) ? worst_b - ideal_b : ideal_b - worst_b;
            double c_ab = 0;
            for (const auto &[x_i, value] : *this) {
                const dimension_type x_line_ia =
                    static_cast<double>(is_minimization(a) ? x_i[a] - ideal_a
                                                           : ideal_a - x_i[a]) /
                    range_a;
                const dimension_type x_line_ib =
                    static_cast<double>(is_minimization(b) ? x_i[b] - ideal_b
                                                           : ideal_b - x_i[b]) /
                    range_b;
                c_ab += std::max(x_line_ia, x_line_ib) -
                        std::min(x_line_ia, x_line_ib);
            }
            return c_ab;
        }

        /// \brief Normalized maxmin conflict between two objectives ([0,1])
        [[nodiscard]] double normalized_maxmin_conflict(const size_t a,
                                                        const size_t b) const {
            return maxmin_conflict(a, b) / size();
        }

        /// \brief Non-parametric conflict between objectives
        /// This is the most general case of conflict
        /// Use this conflict measure when:
        /// - Objective importance is not comparable
        /// - Objectives are in any unit
        /// - When other measures are not appropriate
        /// - When you don't know what to use
        /// This conflict measure is insensitive to non-disruptive
        /// normalizations. \see Freitas, Alan RR, Peter J. Fleming, and
        /// Frederico G. Guimarães. "Aggregation trees for visualization and
        /// dimension reduction in
        ///  many-objective optimization." Information Sciences 298 (2015):
        ///  288-314.
        /// Page 299
        /// Table 2
        [[nodiscard]] double conflict(const size_t a, const size_t b) const {
            // get sorted values in objectives a and b
            std::vector<dimension_type> x_a;
            std::vector<dimension_type> x_b;
            x_a.reserve(size());
            x_b.reserve(size());
            for (const auto &[key, value] : *this) {
                x_a.emplace_back(key[a]);
                x_b.emplace_back(key[b]);
            }
            if (is_minimization(a)) {
                std::sort(x_a.begin(), x_a.end(), std::less<dimension_type>());
            } else {
                std::sort(x_a.begin(), x_a.end(),
                          std::greater<dimension_type>());
            }
            if (is_minimization(b)) {
                std::sort(x_b.begin(), x_b.end(), std::less<dimension_type>());
            } else {
                std::sort(x_b.begin(), x_b.end(),
                          std::greater<dimension_type>());
            }
            // get ranking of these values
            std::map<dimension_type, size_t> rankings_a;
            std::map<dimension_type, size_t> rankings_b;
            for (size_t i = 0; i < x_a.size(); ++i) {
                rankings_a[x_a[i]] = i + 1;
                rankings_b[x_b[i]] = i + 1;
            }
            // calculate conflict
            size_t c_ab = 0;
            for (const auto &[x_i, value] : *this) {
                const size_t x_line_ia = rankings_a[x_i[a]];
                const size_t x_line_ib = rankings_b[x_i[b]];
                c_ab += std::max(x_line_ia, x_line_ib) -
                        std::min(x_line_ia, x_line_ib);
            }
            return static_cast<double>(c_ab);
        }

        /// \brief Normalized conflict between two objectives
        /// This function returns non-paremetric conflict and is the default
        /// function when we don't know which type of conflict to use
        [[nodiscard]] double normalized_conflict(const size_t a,
                                                 const size_t b) const {
            double denominator = 0.;
            auto n = static_cast<double>(size());
            for (size_t i = 1; i <= size(); ++i) {
                denominator += abs(2. * i - n - 1);
            }
            return static_cast<double>(conflict(a, b)) / denominator;
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
        void swap(archive &rhs) noexcept {
            if (&rhs == this) {
                return;
            }
            std::swap(fronts_, rhs.fronts_);
            std::swap(is_minimization_, rhs.is_minimization_);
            std::swap(size_, rhs.size_);
            std::swap(capacity_, rhs.capacity_);
            const bool should_swap = std::allocator_traits<
                allocator_type>::propagate_on_container_swap::value;
            if constexpr (should_swap) {
                std::swap(alloc_, rhs.alloc_);
            }
            std::swap(comp_, rhs.comp_);
        }

      public /* Modifiers: Multimap Concept */:
        /// \brief Clear the front
        void clear() noexcept {
            fronts_.clear();
            size_ = 0;
        }

        /// \brief Insert element pair
        /// Insertion removes any point dominated by the point
        /// before inserting the element in the rtree
        /// \param v Pair value <point, value>
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        std::pair<iterator, bool> insert(const value_type &v) {
            maybe_adjust_dimensions(v);
            auto front_it = find_front(v.first);
            return try_insert(front_it, v);
        }

        /// \brief Move element pair to pareto front
        /// \param v Pair value <point, value>
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        std::pair<iterator, bool> insert(value_type &&v) {
            maybe_adjust_dimensions(v);
            auto front_it = find_front(v.first);
            return try_insert(front_it, std::move(v));
        }

        template <class P> std::pair<iterator, bool> insert(P &&v) {
            static_assert(std::is_constructible_v<value_type, P &&>);
            return emplace(std::forward<P>(v));
        }

        /// \brief Insert element with a hint
        /// It's still possible to implement hints for spatial
        /// maps. However, we need to come up with one strategy
        /// for each map container.
        std::pair<iterator, bool> insert(iterator, const value_type &v) {
            return insert(v);
        }

        std::pair<iterator, bool> insert(const_iterator, const value_type &v) {
            return insert(v);
        }

        std::pair<iterator, bool> insert(const_iterator, value_type &&v) {
            return insert(std::move(v));
        }

        template <class P>
        std::pair<iterator, bool> insert(const_iterator hint, P &&v) {
            static_assert(std::is_constructible_v<value_type, P &&>);
            return emplace_hint(hint, std::forward<P>(v));
        }

        /// \brief Insert list of elements in the front
        /// It's always more efficient to insert lots of elements
        ///     at once.
        /// Insertion removes any point dominated by the point
        ///     before inserting the element in the rtree
        /// \param first Iterator to first element
        /// \param last Iterator to last element
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        template <class InputIterator>
        size_t insert(InputIterator first, InputIterator last) {
            size_t c = 0;
            for (auto it = first; it != last; ++it) {
                auto res = insert(*it);
                c += res.second;
            }
            return c;
        }

        /// \brief Insert list of elements in the front
        /// It's always more efficient to insert lots of elements
        ///     at once.
        /// Insertion removes any point dominated by the point
        ///     before inserting the element in the rtree
        /// \param first Iterator to first element
        /// \param last Iterator to last element
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        size_t insert(std::initializer_list<value_type> il) {
            return insert(il.begin(), il.end());
        }

        /// \brief Create element and emplace it in the front
        /// Emplace becomes insert because the rtree does not have
        /// an emplace function
        template <class... Args>
        std::pair<iterator, bool> emplace(Args &&...args) {
            return insert(value_type(std::forward<Args>(args)...));
        }

        template <class... Args>
        std::pair<iterator, bool> emplace_hint(const_iterator, Args &&...args) {
            return insert(value_type(std::forward<Args>(args)...));
        }

        /// \brief Erase element pointed by iterator from the archive
        /// \warning The modification of the rtree may invalidate the iterators.
        iterator erase(const_iterator position) {
            return erase(create_unconst_iterator(position));
        }

        /// \brief Erase element pointed by iterator from the archive
        /// \warning The modification of the rtree may invalidate the iterators.
        iterator erase(const iterator &position) {
            iterator next_position = std::next(position);
            if (next_position != end()) {
                key_type next_key = next_position->first;
                erase_impl(position);
                return find(next_key);
            } else {
                erase_impl(position);
                return end();
            }
        }

        /// \brief Remove range of iterators from the archive
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
            size_t i = 0;
            for (auto &k : v) {
                iterator it = find(k);
                if (it != end()) {
                    next = erase(it);
                }
                ++i;
            }
            return next;
        }

        /// \brief Erase element from the archive
        /// \param v Point
        size_type erase(const key_type &point) {
            auto first_non_dominated = find_front(point);
            if (first_non_dominated != fronts_.end()) {
                return try_erase(first_non_dominated, point);
            } else {
                return 0;
            }
        }

        /// \brief Merge and move fronts
        void merge(archive &source) {
            insert(source.begin(), source.end());
            source.clear();
        }

        /// \brief Merge and move fronts
        void merge(front_type &source) {
            insert(source.begin(), source.end());
            source.clear();
        }

        /// \brief Merge and move fronts
        void merge(archive &&source) {
            insert(source.begin(), source.end());
            source.clear();
        }

        /// \brief Merge and move fronts
        void merge(front_type &&source) {
            insert(source.begin(), source.end());
            source.clear();
        }

        /// \brief Resize the archive
        /// If new size is more than the number of elements
        /// we currently have, adjust the capacity.
        /// If we need to remove elements, to make them fit
        /// in the new capacity by removing the most crowded
        /// elements in the last front.
        /// \param new_size
        void resize(size_t new_size) {
            size_t current_size = size();
            capacity_ = new_size;
            if (new_size < current_size) {
                prune(current_size - new_size);
            }
        }

      public /* Lookup / Multimap Concept */:
        /// \brief Returns the number of elements with key that compares
        /// equivalent to the specified argument.
        size_type count(const key_type &k) const {
            iterator it = (const_cast<archive *>(this))->find_intersection(k);
            iterator end = (const_cast<archive *>(this))->end();
            return static_cast<size_type>(std::distance(it, end));
        }

        /// \brief Returns the number of elements with key that compares
        /// equivalent to the specified argument.
        template <class L> size_type count(const L &k) const {
            iterator it = find_intersection(key_type{k}, key_type{k});
            return static_cast<size_type>(std::distance(it, end()));
        }

        /// \brief Find element by point
        const_iterator find(const key_type &k) const {
            return (const_cast<archive *>(this))->find(k);
        }

        /// \brief Find element by point
        iterator find(const key_type &k) {
            typename front_set_type::iterator front_it = find_front(k);
            if (front_it != fronts_.end()) {
                auto &pf = unconst_reference(*front_it);
                typename container_type::iterator it = pf.find(k);
                if (it != pf.end()) {
                    typename iterator::fronts_and_elements_type begins(
                        {{front_it, it}});
                    auto result = iterator(this, begins);
                    return result;
                }
            }
            return end();
        }

        /// \brief Finds an element with key equivalent to key
        template <class L> iterator find(const L &x) {
            return find(key_type{x});
        }

        /// \brief Finds an element with key equivalent to key
        template <class L> const_iterator find(const L &x) const {
            return find(key_type{x});
        }

        /// \brief Finds an element with key equivalent to key
        bool contains(const key_type &k) const { return find(k) != end(); }

        /// \brief Finds an element with key equivalent to key
        template <class L> bool contains(const L &x) const {
            return find(x) != end();
        }

      public /* Modifiers: Lookup / Spatial Concept */:
        /// \brief Get iterator to first element that passes a list of
        /// predicates
        const_iterator find(const predicate_list_type &ps) const noexcept {
            return (const_cast<archive *>(this))->find(ps);
        }

        /// \brief Get iterator to first element that passes a list of
        /// predicates
        iterator find(const predicate_list_type &ps) noexcept {
            typename iterator::fronts_and_elements_type begins;
            for (auto front_it = fronts_.begin(); front_it != fronts_.end();
                 ++front_it) {
                auto value_it = front_it->find(ps);
                if (value_it != front_it->end()) {
                    begins.emplace_back(front_it, value_it);
                }
            }
            return iterator(this, begins);
        }

        /// \brief Find intersection between points and query box
        iterator find_intersection(const key_type &k) {
            return find_intersection(k, k);
        }

        /// \brief Find intersection between points and query box
        const_iterator find_intersection(const key_type &k) const {
            return find_intersection(k, k);
        }

        /// \brief Find points in a hyperbox
        const_iterator find_intersection(const point_type &lb,
                                         const point_type &ub) const {
            return (const_cast<archive *>(this))->find_intersection(lb, ub);
        }

        /// \brief Find points in a hyperbox
        iterator find_intersection(const point_type &lb, const point_type &ub) {
            typename iterator::fronts_and_elements_type begins;
            for (auto front_it = fronts_.begin(); front_it != fronts_.end();
                 ++front_it) {
                auto value_it =
                    unconst_reference(*front_it).find_intersection(lb, ub);
                if (value_it != front_it->end()) {
                    begins.emplace_back(front_it, value_it);
                }
            }
            return iterator(this, begins);
        }

        /// \brief Find points within a hyperbox (intersection minus borders)
        const_iterator find_within(const point_type &lb,
                                   const point_type &ub) const {
            return (const_cast<archive *>(this))->find_within(lb, ub);
        }

        /// \brief Find points within a hyperbox (intersection minus borders)
        iterator find_within(const point_type &lb, const point_type &ub) {
            typename iterator::fronts_and_elements_type begins;
            for (auto front_it = fronts_.begin(); front_it != fronts_.end();
                 ++front_it) {
                auto value_it =
                    unconst_reference(*front_it).find_within(lb, ub);
                if (value_it != front_it->end()) {
                    begins.emplace_back(front_it, value_it);
                }
            }
            return iterator(this, begins);
        }

        /// \brief Find points disjointed of a hyperbox (intersection - borders)
        const_iterator find_disjoint(const point_type &lb,
                                     const point_type &ub) const {
            return (const_cast<archive *>(this))->find_disjoint(lb, ub);
        }

        /// \brief Find points disjointed of a hyperbox (intersection - borders)
        iterator find_disjoint(const point_type &lb, const point_type &ub) {
            typename iterator::fronts_and_elements_type begins;
            for (auto front_it = fronts_.begin(); front_it != fronts_.end();
                 ++front_it) {
                auto value_it =
                    unconst_reference(*front_it).find_disjoint(lb, ub);
                if (value_it != front_it->end()) {
                    begins.emplace_back(front_it, value_it);
                }
            }
            return iterator(this, begins);
        }

        /// \brief Find nearest point
        const_iterator find_nearest(const point_type &p) const {
            return (const_cast<archive *>(this))->find_nearest(p);
        }

        /// \brief Find nearest point
        iterator find_nearest(const point_type &p) {
            // each begin points to the nearest in each front
            typename iterator::fronts_and_elements_type nearests;
            for (auto front_it = fronts_.begin(); front_it != fronts_.end();
                 ++front_it) {
                auto it = unconst_reference(*front_it).find_nearest(p);
                if (it != front_it->end()) {
                    nearests.emplace_back(front_it, it);
                }
            }

            // check which begin has the nearest point
            auto best_it =
                std::min_element(nearests.begin(), nearests.end(),
                                 [p](const auto &ita, const auto &itb) {
                                     return p.distance(ita.second->first) <
                                            p.distance(itb.second->first);
                                 });

            // get only the nearest begin
            typename iterator::fronts_and_elements_type begins = {*best_it};
            return iterator(this, begins);
        }

        /// \brief Find k nearest points
        const_iterator find_nearest(const point_type &p, size_t k) const {
            return (const_cast<archive *>(this))->find_nearest(p, k);
        }

        /// \brief Find k nearest points
        iterator find_nearest(const point_type &p, size_t k) {
            // Store up to k * fronts() closest points with front iterators
            std::vector<unprotected_value_type> closest_candidates;

            // Each begin point to the nearest in each front
            for (auto front_it = fronts_.begin(); front_it != fronts_.end();
                 ++front_it) {
                auto it = front_it->find_nearest(p, k);
                auto front_end = unconst_reference(*front_it).end();
                for (; it != front_end; ++it) {
                    closest_candidates.emplace_back(*it);
                }
            }

            // Sort these points by distance
            auto dist_comp = [&p](const value_type &a, const value_type &b) {
                return p.distance(a.first) < p.distance(b.first);
            };
            std::partial_sort(closest_candidates.begin(),
                              closest_candidates.begin() + k,
                              closest_candidates.end(), dist_comp);

            // Find distance of the k-th closest
            double max_dist =
                p.distance((closest_candidates.begin() + k - 1)->first);

            // Create predicate with this as the max distance besides the
            // nearest k predicate
            typename iterator::fronts_and_elements_type begins;
            for (auto front_it = fronts_.begin(); front_it != fronts_.end();
                 ++front_it) {
                // Iterator to nearest k of each, excluding distances larger
                // than overall k-th
                auto it = typename front_type::iterator(
                    unconst_reference(*front_it).data_.find_nearest(
                        p, k, [p, max_dist](const value_type &v) {
                            return p.distance(v.first) <= max_dist;
                        }));
                if (it != front_it->end()) {
                    begins.emplace_back(front_it, it);
                }
            }
            // create concat iterator
            return iterator(this, begins);
        }

        /// \brief Get iterator to element with maximum value in a given
        /// dimension
        iterator max_element(size_t dimension) {
            if (is_minimization(dimension)) {
                return worst_element(dimension);
            } else {
                return ideal_element(dimension);
            }
        }

        /// \brief Get iterator to element with maximum value in a given
        /// dimension
        const_iterator max_element(size_t dimension) const {
            return (const_cast<archive *>(this))->max_element(dimension);
        }

        /// \brief Get iterator to element with minimum value in a given
        /// dimension
        iterator min_element(size_t dimension) {
            if (is_minimization(dimension)) {
                return ideal_element(dimension);
            } else {
                return worst_element(dimension);
            }
        }

        /// \brief Get iterator to element with minimum value in a given
        /// dimension
        const_iterator min_element(size_t dimension) const {
            return (const_cast<archive *>(this))->min_element(dimension);
        }

      public /* Modifiers: Lookup / Pareto Concept */:
        /// \brief Find points dominated by p
        /// If p is in the front, it dominates no other point
        /// Otherwise, it dominates the whole intersection between p and nadir
        const_iterator find_dominated(const point_type &p) const {
            return (const_cast<archive *>(this))->find_dominated(p);
        }

        iterator find_dominated(const point_type &p) {
            typename iterator::fronts_and_elements_type begins;
            for (auto front_it = fronts_.begin(); front_it != fronts_.end();
                 ++front_it) {
                auto value_it = unconst_reference(*front_it).find_dominated(p);
                if (value_it != front_it->end()) {
                    begins.emplace_back(front_it, value_it);
                }
            }
            return iterator(this, begins);
        }

        /// \brief Find nearest point excluding itself
        const_iterator find_nearest_exclusive(const point_type &p) const {
            return (const_cast<archive *>(this))->find_nearest_exclusive(p);
        }

        /// \brief Find nearest point excluding itself
        iterator find_nearest_exclusive(const point_type &p) {
            // each begin points to the nearest exclusive point in each front
            std::vector<std::pair<typename front_set_type::iterator,
                                  typename front_type::iterator>>
                nearest_iters;
            for (auto front_it = fronts_.begin(); front_it != fronts_.end();
                 ++front_it) {
                auto it =
                    unconst_reference(*front_it).find_nearest_exclusive(p);
                if (it != front_it->end()) {
                    nearest_iters.emplace_back(front_it, it);
                }
            }

            if (nearest_iters.empty()) {
                return iterator();
            }

            // check which begin has the nearest
            auto best_it =
                std::min_element(nearest_iters.begin(), nearest_iters.end(),
                                 [p](const auto &ita, const auto &itb) {
                                     return p.distance(ita.second->first) <
                                            p.distance(itb.second->first);
                                 });
            // get begins
            typename iterator::fronts_and_elements_type begins = {*best_it};
            return iterator(this, begins);
        }

        /// \brief Get iterator to element with best value in dimension d
        const_iterator ideal_element(size_t d) const {
            return (const_cast<archive *>(this))->ideal_element(d);
        }

        /// \brief Get iterator to element with best value in dimension d
        iterator ideal_element(size_t d) {
            typename iterator::fronts_and_elements_type b;
            b.emplace_back(
                fronts_.begin(),
                unconst_reference(*fronts_.begin()).ideal_element(d));
            return iterator(this, b);
        }

        /// \brief Element with nadir value in front dimension d
        const_iterator nadir_element(size_t d) const {
            return (const_cast<archive *>(this))->nadir_element(d);
        }

        /// \brief Element with nadir value in front dimension d
        iterator nadir_element(size_t d) {
            return iterator(
                this, fronts_.begin(),
                unconst_reference(*fronts_.begin()).nadir_element(d));
        }

        /// \brief Iterator to element with worst value in archive dimension d
        const_iterator worst_element(size_t d) const {
            return (const_cast<archive *>(this))->worst_element(d);
        }

        /// \brief Iterator to element with worst value in archive dimension d
        iterator worst_element(size_t d) {
            auto front_it = fronts_.begin();
            auto worst_element_it =
                unconst_reference(*front_it).worst_element(d);
            auto worst_front_it = front_it;
            ++front_it;
            for (; front_it != fronts_.end(); ++front_it) {
                auto it = unconst_reference(*front_it).worst_element(d);
                const bool is_worse =
                    is_minimization(d)
                        ? it->first[d] > worst_element_it->first[d]
                        : it->first[d] < worst_element_it->first[d];
                if (is_worse) {
                    worst_element_it = it;
                    worst_front_it = front_it;
                }
            }
            return iterator(this, {{worst_front_it, worst_element_it}});
        }

      public /* Lookup: ArchiveContainer */:
        /// \brief Find first front that does not dominate p
        typename front_set_type::iterator find_front(const point_type &p) {
            return fronts_.find(p);
        }

        /// \brief Find first front that does not dominate p
        typename front_set_type::const_iterator
        find_front(const point_type &p) const {
            return fronts_.find(p);
        }

      public /* Non-Modifying Functions: AssociativeContainer */:
        /// \brief Returns the function object that compares keys
        /// This function is here mostly to conform with the
        /// AssociativeContainer concepts. It's possible but not
        /// very useful.
        key_compare key_comp() const noexcept {
            return [](const key_type &a, const key_type &b) {
                return std::lexicographical_compare(a.begin(), a.end(),
                                                    b.begin(), b.end());
            };
        }

        /// \brief Returns the function object that compares values
        value_compare value_comp() const noexcept {
            return [](const value_type &a, const value_type &b) {
                return std::lexicographical_compare(
                    a.first.begin(), a.first.end(), b.first.begin(),
                    b.first.end());
            };
        }

        /// \brief Returns the function object that compares numbers
        /// This is the comparison operator for a single dimension
        dimension_compare dimension_comp() const noexcept { return comp_; }

      public /* For tests only */:
        /// \brief Set the number of dimensions
        /// This should only be used if the number of dimensions was not
        /// set at compile-time. Even at runtime, the dimension will be
        /// inferred from the first point you insert in the front,
        /// so there is no reason to worry about that.
        /// The only reason to use this function is the python interface,
        /// so that we can set the dimension at runtime even before
        /// inserting the first point. We used to have the dimension
        /// in the constructor, but this involved a lot of useless redundancy
        /// (we should use compile-time anyway) and was very ugly. The user
        /// would never use these constructors anyway
        void dimensions(size_t m) noexcept { maybe_adjust_dimensions(m); }

        /// \brief Calculate the total front sizes
        /// This should match size()
        [[nodiscard]] size_t total_front_sizes() const {
            return std::accumulate(fronts_.begin(), fronts_.end(), size_t{0},
                                   [](const size_t c, const front_type &pf) {
                                       return c + pf.size();
                                   });
        }

        /// \brief Check if archive passes the variants that define a front
        [[nodiscard]] bool check_invariants() const {
            auto last_front = std::prev(fronts_.end());
            for (auto front_it = fronts_.begin(); front_it != fronts_.end();
                 ++front_it) {
                if (!(*front_it).check_invariants()) {
                    // std::cerr << "Front does not pass its invariants" <<
                    // std::endl;
                    return false;
                }
                if (front_it->empty()) {
                    // std::cerr << "Archive contains an empty front" <<
                    // std::endl;
                    return false;
                }
                if (front_it != last_front) {
                    if (!(*std::next(front_it))
                             .is_completely_dominated_by((*front_it))) {
                        // std::cerr << "Fronts got unordered" << std::endl;
                        return false;
                    }
                }
            }
            if (size_ != total_front_sizes()) {
                std::cerr << "Archive and front size mismatch" << std::endl;
                std::cerr << "Archive size = " << size_ << std::endl;
                std::cerr << "Fronts size = " << total_front_sizes()
                          << std::endl;
                return false;
            }
            return true;
        }

        /// \brief Stream archive properties to an ostream
        friend std::ostream &operator<<(std::ostream &os, const archive &ar) {
            os << "Pareto archive (" << ar.size() << " elements - {";
            for (size_t i = 0; i < ar.is_minimization_.size() - 1; ++i) {
                os << (ar.is_minimization_[i] ? "minimization" : "maximization")
                   << ", ";
            }
            os << (ar.is_minimization_.back() ? "minimization" : "maximization")
               << "})";
            return os;
        }

      private /* functions */:
        /// \brief Try to insert the value v in one of the fronts from P^{lower
        /// bound} to P^{|A|} \param front_lower_bound_idx Index of the first
        /// front that might received v \param v Value to insert \return
        /// Iterator to the new value
        ///
        /// Insertion involves a somewhat unusual operation that requires us
        /// to remove the constness of the fronts. Because changing
        /// the elements in a set can affect their order, set iterators
        /// correctly return const elements. However, we can change the front
        /// elements in our algorithm without changing their order because we
        /// move elements around to keep their dominance relationship.
        ///
        /// This conflict arises because the set constraint is too restrictive
        /// for our use case. While all front members are involved in the
        /// less<> comparison, the order relationship is still not altered by
        /// our insertion algorithm. The usual solutions to this problem are:
        /// 1) using mutable elements in the front
        /// 2) using maps
        ///
        /// The first solution is already not very ellegant, besides being
        /// semantically incorrect (mutable does not mean "not involved in
        /// operator<"). This is even worse in our case, where all elements
        /// would need to be mutable to make this work. So it's even worse
        /// because we would be making all set elements unconst all the time.
        /// Removing constness only for this operation is much safer.
        ///
        /// The second solution also doesn't apply because we would need to
        /// reduce the whole front to a single element that represents it's
        /// order in a unidimensional way. This element doesn't exist. We could
        /// even sort fronts by their ideal points, but then the front_search
        /// algorithm would become impossible because we require a special
        /// lower bound search of fronts and points. std::lower_bound also
        /// doesn't work because it's O(n) on associative containers.
        ///
        /// Because the comparison depends on all front members while insertion
        /// still doesn't change the order relationship, this ultimately mean
        /// we would need a map where the front would be the key and mapped
        /// value. This obviously doesn't make sense, and would be equivalent to
        /// removing and inserting the fronts in the back in the set, which
        /// would considerably increase the asymptotic cost of our insertion
        /// algorithm.
        ///
        /// In this context, removing the constness of the front temporarily
        /// doesn't seem like a bad idea after all.
        ///
        /// The same comments can be applied to our erase algorithm.
        std::pair<iterator, bool>
        try_insert(typename front_set_type::iterator front_it,
                   const value_type &v) {
            const bool front_is_valid = front_it != fronts_.end();
            if (front_is_valid) {
                const bool can_solve_in_constant_time =
                    front_it->is_completely_dominated_by(v.first);
                if (can_solve_in_constant_time) {
                    // create a new front with v only
                    front_type tmp_pf({}, is_minimization_.begin(),
                                      is_minimization_.end(), comp_, alloc_);
                    auto [new_element_it, ok] = tmp_pf.insert(v);
                    assert(tmp_pf.size() == 1);
                    ++size_;

                    // emplace front (after creation so the hint works)
                    auto new_front_it =
                        fronts_.emplace_hint(front_it, std::move(tmp_pf));
                    // return iterator
                    const bool new_front_is_valid =
                        new_front_it != fronts_.end();
                    if (new_front_is_valid) {
                        // If inserting v made the archive exceed its max size
                        if (size() > capacity()) {
                            resize(capacity());

                            // Fix iterator if invalidated
                            // Iterator might be invalidated
                            // New item might even have been removed
                            // Look for item again to fix it
                            auto it3 = find(v.first);

                            auto end_id = end();
                            bool ok = it3 != end();
                            std::pair<iterator, bool> p;
                            p.first = it3;
                            p.second = ok;
                            return p;
                        }

                        typename iterator::fronts_and_elements_type begins;
                        begins.emplace_back(new_front_it, new_element_it);
                        iterator it2 =
                            iterator(this, begins, new_element_it, 0);
                        return std::make_pair(it2, true);
                    } else {
                        throw std::logic_error(
                            "The archive failed to create a new front");
                    }
                }

                // Get an unconst reference to the current front
                // We need to make the reference unconst to do that
                // Don't worry, we won't change the order relationship
                // between elements
                front_type &pf = unconst_reference(*front_it);

                // Get all points this solution dominates in front i
                auto dominated_it = front_it->find_dominated(v.first);
                auto dominated_end = front_it->cend();

                // Copy dominated solutions to temporary vector
                std::vector<value_type> dominated_solutions;
                std::copy(dominated_it, dominated_end,
                          std::back_inserter(dominated_solutions));

                // Plot the ones to be deleted
                std::vector<point_type> dominated_points;
                for (const auto &[k, v] : dominated_solutions) {
                    dominated_points.emplace_back(k);
                }

                // Remove dominated solutions from the current front
                pf.erase(dominated_it, dominated_end);
                size_ -= dominated_points.size();

                // Insert the new solution in this front
                // This has to come before reinserting the deleted
                // list to keep the order relationship in the set
                // Remember we have removed const from the front
                // and we have to maintain this order manually now
                auto [front_element_it, ok] = pf.insert(v);
                if (ok) {
                    ++size_;
                }

                // Recursively reinsert previously dominated solutions
                // into the next front. We calculate the next front inside
                // the loop because the pointer might be not pointing
                // to the next front after insertion.
                for (const auto &v2 : dominated_solutions) {
                    auto next_front_it = std::next(front_it);
                    try_insert(next_front_it, v2);
                }

                // Create archive iterator to this new solution
                iterator it2 =
                    iterator(this, typename iterator::fronts_and_elements_type(
                                       {{front_it, front_element_it}}));
                // If inserting v made the archive exceed its max size
                if (size() > capacity()) {
                    resize(capacity());

                    // Fix iterator if invalidated
                    // Iterator might be invalidated
                    // New item might even have been removed
                    // Look for item again to fix it
                    auto it3 = find(v.first);
                    return std::make_pair(it3, it3 != end());
                }
                return std::make_pair(it2, true);
            } else {
                // All fronts dominate v or there are no fronts yet at this
                // point
                bool there_is_space_in_the_archive = size() < capacity();
                if (there_is_space_in_the_archive) {
                    // create a new last front
                    front_type tmp_pf({}, is_minimization_.begin(),
                                      is_minimization_.end(), comp_, alloc_);
                    auto [new_front_element_it, ok] = tmp_pf.insert(v);
                    if (ok) {
                        ++size_;
                    } else {
                        throw std::logic_error(
                            "The new front failed to emplace an element");
                    }

                    // emplace it
                    // (we need to emplace AFTER constructing or the hint
                    // wouldn't work)
                    auto new_front_it =
                        fronts_.emplace_hint(fronts_.end(), std::move(tmp_pf));
                    const bool new_front_is_valid =
                        new_front_it != fronts_.end();
                    if (new_front_is_valid) {
                        iterator it2 = iterator(
                            this,
                            typename iterator::fronts_and_elements_type{
                                {new_front_it, new_front_element_it}},
                            new_front_element_it, 0);
                        return std::make_pair(it2, true);
                    } else {
                        throw std::logic_error(
                            "The archive failed to create a new front");
                    }
                }
            }
            return {end(), false};
        }

        size_type erase_impl(const iterator &position) {
            if (position.current_archive_ == this) {
                if (position.front_begins_[position.current_front_idx_].first !=
                    fronts_.end()) {
                    auto position_front =
                        position.front_begins_[position.current_front_idx_]
                            .first;
                    bool position_element_is_valid =
                        position.current_element_ !=
                        unconst_reference(*position_front).end();
                    if (position_element_is_valid) {
                        return try_erase(position_front, position->first);
                    }
                }
            }
            return 0;
        }

        /// \brief Try to erase element from front or subsequent fronts
        /// Tries to erase point from front front_idx or higher fronts
        /// The point is passed by value because if it is a reference to
        /// a point, it might be a reference to a point in the containers
        /// and the point would be invalidated as soon as we remove it
        /// from the containers.
        ///
        /// This function removes the constness of *front_it, but the
        /// order relationship is maintained by the end of the
        /// algorithm. See a better explanation in the documentation
        /// of the insert function.
        ///
        /// \param front_idx
        /// \param point
        /// \return
        size_type try_erase(typename front_set_type::iterator front_it,
                            const key_type &point_ref) {
            // Make a copy of the point because if the pointer reference
            // is a reference inside the a front, we might delete the
            // very parameter we need to make this work
            key_type point(point_ref);
            front_type &pf = unconst_reference(*front_it);
            size_t n_erased = pf.erase(point);
            if (n_erased == 0) {
                return 0;
            }
            size_ -= n_erased;

            const bool front_became_empty = pf.empty();
            if (front_became_empty) {
                fronts_.erase(front_it);
            } else {
                auto next_front = std::next(front_it);
                const bool there_is_a_next_front = next_front != fronts_.end();
                if (there_is_a_next_front) {
                    // Some elements from next front might not be dominated now
                    auto previously_dominated_it =
                        next_front->find_dominated(point);
                    typename front_type::const_iterator it_end(
                        next_front->end());

                    bool dominate_any_in_next_front =
                        previously_dominated_it != it_end;
                    if (dominate_any_in_next_front) {
                        // Copy these points because erasing them would
                        // invalidate front iterators
                        std::vector<value_type> previously_dominated;
                        std::copy(previously_dominated_it, it_end,
                                  std::back_inserter(previously_dominated));

                        // Move these elements to this front
                        for (const auto &v : previously_dominated) {
                            if (!pf.dominates(v.first)) {
                                auto [it, ok] = pf.insert(v);
                                if (ok) {
                                    ++size_;
                                    try_erase(next_front, v.first);
                                }
                            }
                        }
                    }
                }
            }
            return n_erased;
        }

        /// \brief Remove elements from the last archive fronts
        /// This function removes the elements without changing the
        /// max size
        void prune(size_t excess) {
            while (excess > 0) {
                const bool excess_larger_than_last_front =
                    excess >= fronts_.rbegin()->size();
                if (excess_larger_than_last_front) {
                    excess -= fronts_.rbegin()->size();
                    size_ -= fronts_.rbegin()->size();
                    fronts_.erase(std::prev(fronts_.end()));
                } else {
                    double max_linecurrent_archive_time_pruning =
                        2 * log2(static_cast<double>(capacity_));
                    if (excess > max_linecurrent_archive_time_pruning) {
                        prune_random(excess -
                                     max_linecurrent_archive_time_pruning);
                        excess = max_linecurrent_archive_time_pruning;
                    }
                    prune_crowded(excess);
                    excess = 0;
                }
            }
        }

        /// \brief Remove random elements from last front
        void prune_random(size_t n_to_remove) {
            front_type &last_front = unconst_reference(*fronts_.rbegin());
            box_type b(last_front.ideal(), last_front.nadir());
            auto &g = archive::generator();
            for (size_t i = 0; i < n_to_remove; ++i) {
                point_type r(dimensions());
                for (size_t j = 0; j < r.dimensions(); ++j) {
                    std::uniform_real_distribution<dimension_type> d(
                        b.min()[j], b.max()[j]);
                    r[j] = d(g);
                }
                auto it = last_front.find_nearest(r);
                last_front.erase(it);
                --size_;
            }
        }

        /// \brief Remove the most crowded elements from the last front
        void prune_crowded(size_t n_to_remove) {
            front_type &last_front = unconst_reference(*fronts_.rbegin());
            std::vector<std::pair<point_type, double>> candidates;
            candidates.reserve(last_front.size());
            for (const auto &[k, v] : last_front) {
                auto it = last_front.find_nearest(k, 3);
                double d = 0.0;
                for (; it != last_front.end(); ++it) {
                    d += k.distance(it->first);
                }
                // point k and crowding distance d
                candidates.emplace_back(k, d);
            }

            // smallest crowding distance comes first
            std::partial_sort(
                candidates.begin(), candidates.begin() + n_to_remove,
                candidates.end(), [](const auto &a, const auto &b) {
                    return a.second < b.second;
                });
            for (size_t i = 0; i < n_to_remove; ++i) {
                size_ -= last_front.erase(candidates[i].first);
            }
        }

        void maybe_resize(std::array<uint8_t, number_of_compile_dimensions> &v
                          [[maybe_unused]],
                          size_t n [[maybe_unused]]) {}

        void maybe_resize(std::vector<uint8_t> &v, size_t n) { v.resize(n); }

        /// \brief If the dimension is being set at runtime, this sets the
        /// dimension in case we don't already know it.
        inline void maybe_adjust_dimensions(const value_type &v) {
            if constexpr (number_of_compile_dimensions == 0) {
                if (empty() && is_minimization_.size() == 1) {
                    maybe_resize(is_minimization_, v.first.dimensions());
                    std::fill(is_minimization_.begin() + 1,
                              is_minimization_.end(),
                              *is_minimization_.begin());
                }
            }
        }

        /// \brief If the dimension is being set at runtime, this sets the
        /// dimension in case we don't already know it.
        inline void maybe_adjust_dimensions([[maybe_unused]] size_t s) {
            if constexpr (number_of_compile_dimensions == 0) {
                if (empty() && is_minimization_.size() == 1) {
                    maybe_resize(is_minimization_, s);
                    std::fill(is_minimization_.begin() + 1,
                              is_minimization_.end(),
                              *is_minimization_.begin());
                }
            }
        }

        iterator create_unconst_iterator(const_iterator position) {
            iterator it(const_cast<archive *>(position.current_archive_));
            std::vector<std::pair<typename front_set_type::iterator,
                                  typename container_type::iterator>>
                begins;
            for (const auto &item : position.front_begins_) {
                begins.emplace_back(item);
            }
            it.current_element_ =
                reinterpret_cast<decltype(it.current_element_)>(
                    position.current_element_);
            it.current_front_idx_ = position.current_front_idx_;
            return it;
        }

        static std::mt19937 &generator() {
            static std::mt19937 g(static_cast<unsigned int>(
                static_cast<unsigned int>(std::random_device()()) |
                static_cast<unsigned int>(
                    std::chrono::high_resolution_clock::now()
                        .time_since_epoch()
                        .count())));
            return g;
        }

        void initialize_directions(size_t target_size = 1, bool fill = true) {
            constexpr bool compile_time_dimension =
                number_of_compile_dimensions != 0;
            if constexpr (compile_time_dimension) {
                const bool target_is_a_placeholder_for_all_dimensions =
                    target_size == 1;
                const size_t expected_dimensions = number_of_compile_dimensions;
                const bool target_size_matches_dimensions =
                    target_size == expected_dimensions;
                const bool ok = target_is_a_placeholder_for_all_dimensions ||
                                target_size_matches_dimensions;
                if (!ok) {
                    throw std::invalid_argument(
                        "The number of minimization directions specified at "
                        "compile time does not match the number minimization "
                        "directions passed to the constructor");
                }
            }

            maybe_resize(is_minimization_, std::max(target_size, size_t{1}));
            if (fill) {
                std::fill(is_minimization_.begin(), is_minimization_.end(),
                          0x01);
            }
        }

        template <class DirectionIt>
        void initialize_directions(DirectionIt first, DirectionIt last) {
            auto d = std::distance(first, last);
            if (d < 0) {
                throw std::logic_error(
                    "The first iterator cannot come after the last iterator");
            } else {
                auto s = static_cast<size_t>(d);
                initialize_directions(s, false);
                const bool one_direction_for_all_dimensions = s == 1;
                if (one_direction_for_all_dimensions) {
                    std::fill(is_minimization_.begin(), is_minimization_.end(),
                              *first);
                } else {
                    if (s == is_minimization_.size()) {
                        std::copy(first, last, is_minimization_.begin());
                    } else {
                        throw std::logic_error("The iterators do not match the "
                                               "number of dimensions");
                    }
                }
            }
        }

      private:
        /// \brief Pareto fronts with archive solutions
        /// We keep the fronts in a set so we can delete an empty front
        /// in O(1) time and always find a front in O(log n) time
        front_set_type fronts_;

        /// \brief Whether each dimension is minimization or maximization
        /// We use uint8_t because bool to avoid the array specialization
        directions_type is_minimization_;

        /// \brief Number of elements in the archive
        /// Although we can calculate the archive size
        /// by accumulation the front sizes, we keep this
        /// pre-processed variable to avoid a worst-case
        /// O(|A|)=O(n)
        size_t size_{0};

        /// \brief Max number of elements in the archive
        size_t capacity_{default_capacity};

        /// \brief Allocator the pareto fronts will share
        /// We just have to use a copy of this allocator for each
        /// new front we create. All the rest is being managed
        /// by the internal data structures.
        allocator_type alloc_;

        /// \brief Key comparison (single dimension)
        dimension_compare comp_{std::less<dimension_type>()};
    };

    /// \brief Relational operator < for archives and archives
    /// This relationships are not as important as they are for archives
    /// but we still define them to make archives comformant with
    /// the archive container concept
    template <typename K, size_t M, typename T, typename C>
    bool operator<(const archive<K, M, T, C> &lhs,
                   const archive<K, M, T, C> &rhs) {
        return lhs.dominates(rhs);
    }

    /// \brief Relational operator > for archives and archives
    template <typename K, size_t M, typename T, typename C>
    bool operator>(const archive<K, M, T, C> &lhs,
                   const archive<K, M, T, C> &rhs) {
        return rhs < lhs;
    }

    /// \brief Relational operator <= for archives and archives
    template <typename K, size_t M, typename T, typename C>
    bool operator<=(const archive<K, M, T, C> &lhs,
                    const archive<K, M, T, C> &rhs) {
        return lhs < rhs || !(rhs < lhs);
    }

    /// \brief Relational operator >= for archives and archives
    template <typename K, size_t M, typename T, typename C>
    bool operator>=(const archive<K, M, T, C> &lhs,
                    const archive<K, M, T, C> &rhs) {
        return lhs > rhs || !(rhs > lhs);
    }

    /// \brief Relational operator < for archives and points
    template <typename K, size_t M, typename T, typename C>
    bool operator<(const archive<K, M, T, C> &lhs,
                   const typename archive<K, M, T, C>::key_type &rhs) {
        return lhs.dominates(rhs);
    }

    /// \brief Relational operator > for archives and points
    template <typename K, size_t M, typename T, typename C>
    bool operator>(const archive<K, M, T, C> &lhs,
                   const typename archive<K, M, T, C>::key_type &rhs) {
        // The idea here is that point < archive only if point completely
        // dominates archive because we are treating the point as a
        // archive with one element here so that sets of archives
        // know how to order elements. If point only partially dominated
        // archive, then we can't replace the archive. We should at most
        // insert the point in the archive. This is clearer when
        // we see the logic of archives.
        return lhs.is_completely_dominated_by(rhs);
    }

    /// \brief Relational operator <= for archives and points
    template <typename K, size_t M, typename T, typename C>
    bool operator<=(const archive<K, M, T, C> &lhs,
                    const typename archive<K, M, T, C>::key_type &rhs) {
        return lhs < rhs || !(rhs < lhs);
    }

    /// \brief Relational operator >= for archives and points
    template <typename K, size_t M, typename T, typename C>
    bool operator>=(const archive<K, M, T, C> &lhs,
                    const typename archive<K, M, T, C>::key_type &rhs) {
        return lhs > rhs || !(rhs > lhs);
    }

    /// \brief Relational operator < for points and archives
    template <typename K, size_t M, typename T, typename C>
    bool operator<(const typename archive<K, M, T, C>::key_type &lhs,
                   const archive<K, M, T, C> &rhs) {
        return rhs > lhs;
    }

    /// \brief Relational operator > for points and archives
    template <typename K, size_t M, typename T, typename C>
    bool operator>(const typename archive<K, M, T, C>::key_type &lhs,
                   const archive<K, M, T, C> &rhs) {
        return rhs < lhs;
    }

    /// \brief Relational operator <= for points and archives
    template <typename K, size_t M, typename T, typename C>
    bool operator<=(const typename archive<K, M, T, C>::key_type &lhs,
                    const archive<K, M, T, C> &rhs) {
        return rhs >= lhs;
    }

    /// \brief Relational operator >= for points and archives
    template <typename K, size_t M, typename T, typename C>
    bool operator>=(const typename archive<K, M, T, C>::key_type &lhs,
                    const archive<K, M, T, C> &rhs) {
        return rhs <= lhs;
    }

    /// \brief Equality operator
    template <class K, size_t M, class T, class C>
    bool operator==(const archive<K, M, T, C> &lhs,
                    const archive<K, M, T, C> &rhs) {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        if (lhs.dimensions() != rhs.dimensions()) {
            return false;
        }
        for (typename archive<K, M, T, C>::size_type i = 0;
             i < rhs.dimensions(); ++i) {
            if (lhs.is_minimization(i) != rhs.is_minimization(i)) {
                return false;
            }
        }
        return std::equal(
            lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
            [](const typename archive<K, M, T, C>::value_type &a,
               const typename archive<K, M, T, C>::value_type &b) {
                return a.first == b.first &&
                       mapped_type_custom_equality_operator(a.second, b.second);
            });
    }

    /// \brief Inequality operator
    template <class K, size_t M, class T, class C>
    bool operator!=(const archive<K, M, T, C> &lhs,
                    const archive<K, M, T, C> &rhs) {
        return !(lhs == rhs);
    }

} // namespace pareto

#endif // PARETO_FRONT_ARCHIVE_H
