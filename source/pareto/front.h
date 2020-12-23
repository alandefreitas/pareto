#ifndef PARETO_FRONTS_PARETO_FRONT_RTREE_H
#define PARETO_FRONTS_PARETO_FRONT_RTREE_H

#include <initializer_list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <ostream>
#include <random>
#include <thread>

#include <pareto/common/common.h>
#include <pareto/common/hypervolume.h>
#include <pareto/common/keywords.h>
#include <pareto/common/metaprogramming.h>

#include <pareto/spatial_map.h>

#ifdef BUILD_BOOST_TREE
#include <pareto/boost_tree.h>
#endif

namespace pareto {
    /// We need to make archive a friend because of a few functions
    /// that access the front data directly. To be removed once
    /// we finally deprecate boost_tree.
    template <typename K, size_t M, typename T, class Container> class archive;

    /// \class Pareto Front
    /// The fronts have their dimension set at compile time
    /// If we set the dimension to 0, then it's defined at runtime
    /// Defining at runtime is only useful for the python bindings
    /// When dimensions are set a runtime, we find out about the dimension
    /// when we insert the first element in the front. At this point,
    /// the front dimension is set and we cannot change it.
    /// \note See other container adaptors such as stack
    /// \see https://en.cppreference.com/w/cpp/container/stack
    template <typename K, size_t M, typename T,
              class Container = spatial_map<K, M, T>>
    class front {
      public /* SpatialAdapter Concept */:
        using container_type = Container;

      private /* Internal Types */:
        using point_type = typename container_type::key_type;

      public /* Container Concept */:
        using value_type = typename container_type::value_type;
        using reference = typename container_type::reference;
        using const_reference = typename container_type::const_reference;
        using iterator = typename container_type::iterator;
        using const_iterator = typename container_type::const_iterator;
        using pointer = typename container_type::pointer;
        using const_pointer = typename container_type::const_pointer;
        using difference_type = typename container_type::difference_type;
        using size_type = typename container_type::size_type;

      public /* ReversibleContainer Concept */:
        using reverse_iterator = typename container_type::reverse_iterator;
        using const_reverse_iterator =
            typename container_type::const_reverse_iterator;

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
        using directions_type = std::conditional_t<
            number_of_compile_dimensions == 0, std::vector<uint8_t>,
            std::array<uint8_t, number_of_compile_dimensions>>;

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
        explicit front() { initialize_directions(); }

        /// \brief Construct with allocator
        /// We have to replicate the function with and without the allocator
        /// to emulate a default value. This will make the internal container
        /// use its own allocator we can't set by default here.
        explicit front(const allocator_type &alloc) : data_(alloc) {
            initialize_directions();
        }

        /// \brief Copy constructor
        /// Copy constructors of AllocatorAwareContainers
        /// obtain their instances of the allocator by calling
        /// std::allocator_traits<allocator_type>::
        ///       select_on_container_copy_construction
        /// on the allocator of the container being copied.
        /// \param rhs
        front(const front &rhs)
            : data_(rhs.data_), is_minimization_(rhs.is_minimization_){};

        /// \brief Copy constructor data but use another allocator
        front(const front &rhs, const allocator_type &alloc)
            : data_(rhs.data_, alloc), is_minimization_(rhs.is_minimization_){};

        /// \brief Move constructor
        /// Move constructors obtain their instances of allocators
        /// by move-constructing from the allocator belonging to
        /// the old container
        front(front &&rhs) noexcept
            : data_(std::move(rhs.data_)),
              is_minimization_(std::move(rhs.is_minimization_)) {}

        /// \brief Move constructor data but use new allocator
        front(front &&rhs, const allocator_type &alloc) noexcept
            : data_(std::move(rhs.data_), alloc),
              is_minimization_(std::move(rhs.is_minimization_)) {}

        /// \brief Destructor
        ~front() = default;

      public /* Constructors: AssociativeContainer + AllocatorAwareContainer */:
        /// \brief Create container with custom comparison function
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        explicit front(const dimension_compare &comp) : data_(comp) {
            initialize_directions();
        }

        /// \brief Replicate above constructor with custom allocator
        /// This is only emulating the constructor above with the default
        /// parameter as the internal PMR container we cannot access from here
        explicit front(const dimension_compare &comp,
                       const allocator_type &alloc)
            : data_(comp, alloc) {
            initialize_directions();
        }

        /// \brief Construct with iterators + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        front(InputIt first, InputIt last,
              const dimension_compare &comp = dimension_compare())
            : data_(comp) {
            initialize_directions();
            // note: this->insert(...) is very different from data_.insert(...)
            insert(first, last);
        }

        /// \brief Replicate above constructor with custom allocator
        /// This is only emulating the constructor above with the default
        /// parameter as the internal PMR container we cannot access from here
        template <class InputIt>
        front(InputIt first, InputIt last, const dimension_compare &comp,
              const allocator_type &alloc)
            : data_(comp, alloc) {
            initialize_directions();
            // note: this->insert(...) is very different from data_.insert(...)
            insert(first, last);
        }

        /// \brief Construct with list + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        front(std::initializer_list<value_type> il,
              const dimension_compare &comp = dimension_compare())
            : front(il.begin(), il.end(), comp) {}

        /// \brief Replicate above constructor with custom allocator
        /// This is only emulating the constructor above with the default
        /// parameter as the internal PMR container we cannot access from here
        front(std::initializer_list<value_type> il,
              const dimension_compare &comp, const allocator_type &alloc)
            : front(il.begin(), il.end(), comp, alloc) {}

        /// \brief Construct with iterators
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        front(InputIt first, InputIt last, const allocator_type &alloc)
            : data_(alloc) {
            initialize_directions();
            // note: this->insert(...) is very different from data_.insert(...)
            insert(first, last);
        }

        /// \brief Construct with initializer list
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        front(std::initializer_list<value_type> il, const allocator_type &alloc)
            : front(il.begin(), il.end(), alloc) {}

      public /* Constructors: ParetoContainer + AllocatorAwareContainer */:
        /*
         * The pareto concept extends all constructors that take
         * initial values with
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
        front(InputIt first, InputIt last, DirectionIt first_dir,
              DirectionIt last_dir,
              const dimension_compare &comp = dimension_compare())
            : data_(comp) {
            initialize_directions(first_dir, last_dir);
            // note: this->insert(...) is very different from data_.insert(...)
            insert(first, last);
        }

        /// \brief Replicate above constructor with custom allocator
        /// This is only emulating the constructor above with the default
        /// parameter as the internal PMR container we cannot access from here
        template <class InputIt, class DirectionIt>
        front(InputIt first, InputIt last, DirectionIt first_dir,
              DirectionIt last_dir, const dimension_compare &comp,
              const allocator_type &alloc)
            : data_(comp, alloc) {
            initialize_directions(first_dir, last_dir);
            // note: this->insert(...) is very different from data_.insert(...)
            insert(first, last);
        }

        /// \brief Construct with list + direction + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class DirectionIt>
        front(std::initializer_list<value_type> il, DirectionIt first_dir,
              DirectionIt last_dir,
              const dimension_compare &comp = dimension_compare())
            : front(il.begin(), il.end(), first_dir, last_dir, comp) {}

        /// \brief Replicate above constructor with custom allocator
        /// This is only emulating the constructor above with the default
        /// parameter as the internal PMR container we cannot access from here
        template <class DirectionIt>
        front(std::initializer_list<value_type> il, DirectionIt first_dir,
              DirectionIt last_dir, const dimension_compare &comp,
              const allocator_type &alloc)
            : front(il.begin(), il.end(), first_dir, last_dir, comp, alloc) {}

        /// \brief Construct with list + direction + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        front(InputIt first, InputIt last, std::initializer_list<bool> il_dir,
              const dimension_compare &comp = dimension_compare())
            : front(first, last, il_dir.begin(), il_dir.end(), comp) {}

        /// \brief Replicate above constructor with custom allocator
        /// This is only emulating the constructor above with the default
        /// parameter as the internal PMR container we cannot access from here
        template <class InputIt>
        front(InputIt first, InputIt last, std::initializer_list<bool> il_dir,
              const dimension_compare &comp, const allocator_type &alloc)
            : front(first, last, il_dir.begin(), il_dir.end(), comp, alloc) {}

        /// \brief Construct with list + direction list + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        front(std::initializer_list<value_type> il,
              std::initializer_list<bool> il_dir,
              const dimension_compare &comp = dimension_compare())
            : front(il.begin(), il.end(), il_dir.begin(), il_dir.end(), comp) {}

        /// \brief Replicate above constructor with custom allocator
        /// This is only emulating the constructor above with the default
        /// parameter as the internal PMR container we cannot access from here
        front(std::initializer_list<value_type> il,
              std::initializer_list<bool> il_dir, const dimension_compare &comp,
              const allocator_type &alloc)
            : front(il.begin(), il.end(), il_dir.begin(), il_dir.end(), comp,
                    alloc) {}

        /// \brief Construct with direction list + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        front(std::initializer_list<bool> il_dir,
              const dimension_compare &comp = dimension_compare())
            : front({}, il_dir, comp) {}

        /// \brief Replicate above constructor with custom allocator
        /// This is only emulating the constructor above with the default
        /// parameter as the internal PMR container we cannot access from here
        front(std::initializer_list<bool> il_dir, const dimension_compare &comp,
              const allocator_type &alloc)
            : front({}, il_dir, comp, alloc) {}

        /// \brief Construct with iterators
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt, class DirectionIt>
        front(InputIt first, InputIt last, DirectionIt first_dir,
              DirectionIt last_dir, const allocator_type &alloc)
            : data_(alloc) {
            initialize_directions(first_dir, last_dir);
            // note: this->insert(...) is very different from data_.insert(...)
            insert(first, last);
        }

        /// \brief Construct with initializer list
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class DirectionIt>
        front(std::initializer_list<value_type> il, DirectionIt first_dir,
              DirectionIt last_dir, const allocator_type &alloc)
            : front(il.begin(), il.end(), first_dir, last_dir, alloc) {}

        /// \brief Construct with initializer list
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        front(InputIt first, InputIt last, std::initializer_list<bool> il_dir,
              const allocator_type &alloc)
            : front(first, last, il_dir.begin(), il_dir.end(), alloc) {}

        /// \brief Construct with initializer list
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        front(std::initializer_list<value_type> il,
              std::initializer_list<bool> il_dir, const allocator_type &alloc)
            : front(il.begin(), il.end(), il_dir.begin(), il_dir.end(), alloc) {
        }

        /// \brief Construct with initializer list
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        front(std::initializer_list<bool> il_dir, const allocator_type &alloc)
            : front({}, il_dir, alloc) {}

      public /* Non-Modifying Functions: AllocatorAwareContainer */:
        /// \brief Obtains a copy of the allocator
        /// The accessor get_allocator() obtains a copy of
        /// the allocator that was used to construct the
        /// container or installed by the most recent allocator
        /// replacement operation
        allocator_type get_allocator() const noexcept {
            return data_.get_allocator();
        }

      public /* Assignment: Container + AllocatorAwareContainer */:
        /// \brief Copy assignment
        /// Copy-assignment will replace the allocator only if
        /// std::allocator_traits<allocator_type>::
        ///     propagate_on_container_copy_assignment::value is true
        /// \note if the allocators of the source and the target containers
        /// do not compare equal, copy assignment has to deallocate the
        /// target's memory using the old allocator and then allocate it
        /// using the new allocator before copying the elements
        front &operator=(const front &rhs) {
            if (&rhs == this) {
                return *this;
            }
            data_ = rhs.data_;
            is_minimization_ = rhs.is_minimization_;
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
        front &operator=(front &&rhs) noexcept {
            if (&rhs == this) {
                return *this;
            }
            data_ = std::move(rhs.data_);
            is_minimization_ = std::move(rhs.is_minimization_);
            return *this;
        }

      public /* Assignment: AssociativeContainer */:
        /// \brief Initializer list assignment
        front &operator=(std::initializer_list<value_type> il) noexcept {
            data_.clear();
            insert(il.begin(), il.end());
            return *this;
        }

      public /* Element Access / Map Concept */:

        /// \brief Get reference to element at a given position, and throw error
        /// if it does not exist
        mapped_type &at(const key_type &k) {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("front::at: key not found");
            }
        }

        /// \brief Get reference to element at a given position, and throw error
        /// if it does not exist
        const mapped_type &at(const key_type &k) const {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("front::at: key not found");
            }
        }

        /// \brief Get reference to element at a given position, and create one
        /// if it does not exits
        mapped_type &operator[](const key_type &k) {
            auto it = find(k);
            if (it != end()) {
                // return reference to mapped type in the containers
                return it->second;
            } else {
                // include element in the containers with a default
                // mapped type
                auto [it2, ok] = this->insert(std::make_pair(k, mapped_type()));
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

        /// \brief Get reference to element at a given position, and create one
        /// if it does not exits
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

        template <typename... Targs>
        /// \brief Get reference to element at a given position, and create one
        /// if it does not exits
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
        const_iterator begin() const noexcept { return cbegin(); }

        /// \brief Get iterator to past-the-end element
        const_iterator end() const noexcept { return cend(); }

        /// \brief Get iterator to first element
        const_iterator cbegin() const noexcept { return data_.cbegin(); }

        /// \brief Get iterator to past-the-end element
        const_iterator cend() const noexcept { return data_.cend(); }

        /// \brief Get iterator to first element
        iterator begin() noexcept { return data_.begin(); }

        /// \brief Get iterator to past-the-end element
        iterator end() noexcept { return data_.end(); }

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
        /// \brief Get number of dimensions of elements in the front
        /// If the dimensions are set in compile time, we might not
        /// know the number of dimensions until the first item
        /// is inserted.
        [[nodiscard]] size_type dimensions() const noexcept {
            if constexpr (number_of_compile_dimensions > 0) {
                // compile time
                return number_of_compile_dimensions;
            } else {
                // run-time
                if (!data_.empty()) {
                    // infer from point
                    return data_.begin()->first.dimensions();
                } else {
                    // infer from minimization vector
                    return is_minimization_.size();
                }
            }
        }

        /// \brief Get maximum value in a given dimension
        dimension_type max_value(size_t dimension) const {
            return data_.max_value(dimension);
        }

        /// \brief Get minimum value in a given dimension
        dimension_type min_value(size_t dimension) const {
            return data_.min_value(dimension);
        }

      public /* Reference points / Pareto Concept */:
        /// \brief Ideal point in the front
        key_type ideal() const {
            key_type r(dimensions());
            for (size_t i = 0; i < r.dimensions(); ++i) {
                r[i] = ideal(i);
            }
            return r;
        }

        /// \brief Ideal value in a front dimension
        dimension_type ideal(size_t d) const {
            return is_minimization(d) ? data_.min_value(d) : data_.max_value(d);
        }

        /// \brief The nadir point is the worst point among the
        /// non-dominated points. There is a difference
        /// between the nadir point and the worst point
        /// for archives.
        key_type nadir() const {
            key_type r(dimensions());
            for (size_t i = 0; i < r.dimensions(); ++i) {
                r[i] = nadir(i);
            }
            return r;
        }

        /// \brief Nadir value in dimension d
        dimension_type nadir(size_t d) const {
            return is_minimization(d) ? data_.max_value(d) : data_.min_value(d);
        }

        /// \brief Worst point in the front
        /// Worst is the same as nadir for fronts.
        /// In archives, worst != nadir because there are
        /// many fronts.
        key_type worst() const { return nadir(); }

        /// \brief Worst value in front dimension d
        dimension_type worst(size_t d) const { return nadir(d); }

        /// \brief True if all dimensions are minimization
        [[nodiscard]] bool is_minimization() const noexcept {
            return std::all_of(is_minimization_.begin(), is_minimization_.end(),
                               [](auto i) { return i == uint8_t(1); });
        }

        /// \brief True if all dimensions are maximization
        [[nodiscard]] bool is_maximization() const noexcept {
            return std::all_of(is_minimization_.begin(), is_minimization_.end(),
                               [](auto i) { return i == uint8_t(0); });
        }

        /// \brief True if i-th dimension is minimization
        [[nodiscard]] bool is_minimization(size_t dimension) const noexcept {
            if constexpr (number_of_compile_dimensions > 0) {
                return is_minimization_[dimension] > 0;
            } else {
                if (dimension < is_minimization_.size()) {
                    return is_minimization_[dimension] == 0x01;
                } else {
                    return is_minimization_[0] > 0;
                }
            }
        }

        /// \brief True if i-th dimension is maximization
        [[nodiscard]] bool is_maximization(size_t dimension) const noexcept {
            if constexpr (number_of_compile_dimensions > 0) {
                return is_minimization_[dimension] == 0;
            } else {
                if (dimension < is_minimization_.size()) {
                    return is_minimization_[dimension] == 0;
                } else {
                    return is_minimization_[0] == 0;
                }
            }
        }

      public /* Non-Modifying Functions / Dominance / Pareto Concept */:
        /// \brief Check if this front weakly dominates a point
        /// A front a weakly dominates a solution p if it has at least
        /// one solution that dominates p.
        /// Some of these lists of conditions could be checked with a single
        /// predicate list, but we do not do it this way because boost_tree
        /// is very inefficient with predicate lists.
        /// \see
        /// http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool dominates(const point_type &p) const {
            // trivial case: front is empty
            if (empty()) {
                return false;
            }

            // trivial case: p is not behind ideal
            point_type ideal_point = ideal();
            const bool p_is_not_behind_ideal =
                !ideal_point.dominates(p, is_minimization_);
            if (p_is_not_behind_ideal) {
                return false;
            }

            // trivial case: p is in the front
            if (find(p) != end()) {
                return false;
            }

            // general case (removing trivial case 1)
            // points intersecting(ideal,p)
            const_iterator first_element_that_dominates =
                data_.find_intersection(ideal_point, p);
            return first_element_that_dominates != end();
        }

        /// \brief Check if this front strongly dominates a point
        /// A front a strongly dominates a solution b if a has a solution
        /// that is strictly better than b in all objectives.
        bool strongly_dominates(const point_type &p) const {
            // trivial case: front is empty
            if (empty()) {
                return false;
            }

            // trivial case:
            // p is not strictly behind ideal point
            point_type ideal_point = ideal();
            const bool ideal_strongly_dominates_p =
                ideal_point.strongly_dominates(p, is_minimization_);
            if (!ideal_strongly_dominates_p) {
                return false;
            }

            // general case
            // * p is behind ideal point (trivial case)
            // * points intersect(ideal, p+epsilon) != empty set
            // This works because:
            // * the border around p cannot strongly dominate p
            //   the epsilon removes this border
            // * the border around ideal *can* strongly dominate p
            //   the epsilon leaves this border there
            dimension_type epsilon =
                std::is_floating_point_v<dimension_type>
                    ? std::numeric_limits<dimension_type>::epsilon()
                    : static_cast<dimension_type>(1);
            auto p_line = p;
            for (size_t i = 0; i < p.dimensions(); ++i) {
                if (is_minimization_[i]) {
                    p_line[i] -= epsilon;
                } else {
                    p_line[i] += epsilon;
                }
            }
            const_iterator it = data_.find_intersection(ideal_point, p_line);
            return it != end();
        }

        /// \brief True if front is partially dominated by p
        bool is_partially_dominated_by(const point_type &p) const {
            // trivial case: front is empty
            if (empty()) {
                return true;
            }

            // trivial case: p is in the front
            if (find(p) != end()) {
                return false;
            }

            // get points in the intersection between worst and p
            // we already know p is not in the front, and any point
            // in this query that is not p is a point dominated by p
            const_iterator it = data_.find_intersection(worst(), p);
            // if there's someone p dominates, then it's partially dominated by
            // p
            return it != end();
        }

        /// \brief True if front is completely dominated by p
        bool is_completely_dominated_by(const point_type &p) const {
            // trivial case: front is empty
            if (empty()) {
                return true;
            }

            auto ideal_point = ideal();
            return p.dominates(ideal_point, is_minimization_);
        }

        /// \brief Check if this front non-dominates the point
        /// A solution a weakly dominates a solution b if a is better
        /// than b in at least one objective and is as good as b in
        /// all other objectives.
        /// \see
        /// http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool non_dominates(const point_type &p) const {
            // trivial case: front is empty
            if (empty()) {
                return true;
            }

            // Ensure pareto does not dominate p
            // Ensure p does not dominate anyone in the pareto
            return !dominates(p) && !is_partially_dominated_by(p);
        }

        /// \brief Check if this front dominates another front
        /// P1 dominates P2 if all points in P1 dominate or at least
        /// non-dominate P2
        /// To avoid both operations, it's easy to check if P1
        /// fails to dominate any point in P2
        bool dominates(const front &P2) const {
            // trivial case: front is empty
            if (empty()) {
                return false;
            }

            bool dominates_any = false;
            for (auto &[k, v] : P2) {
                if (!dominates(k)) {
                    if (find(k) != end()) {
                        return false;
                    }
                } else {
                    dominates_any = true;
                }
            }
            return dominates_any;
        }

        /// \brief Check if this front strongly dominates another front
        bool strongly_dominates(const front &p) const {
            // trivial case: front is empty
            if (empty()) {
                return false;
            }

            for (auto &[k, v] : p) {
                if (!strongly_dominates(k)) {
                    return false;
                }
            }
            return true;
        }

        /// \brief Check if this front is is_partially_dominated_by another
        /// front
        bool is_partially_dominated_by(const front &p) const {
            // trivial case: front is empty
            if (empty()) {
                return true;
            }

            for (auto &[k, v] : p) {
                if (is_partially_dominated_by(k)) {
                    return true;
                }
            }
            return false;
        }

        /// \brief Check if this front is is_partially_dominated_by another
        /// front
        bool is_completely_dominated_by(const front &p) const {
            // trivial case: front is empty
            if (empty()) {
                return true;
            }

            // trivial case: p's ideal point doesn't dominate this ideal point
            if (!p.ideal().dominates(this->ideal(), is_minimization_)) {
                return false;
            }

            for (auto &[k, v] : *this) {
                if (!p.dominates(k)) {
                    return false;
                }
            }
            return true;
        }

        /// \brief Check if this front non-dominates another front
        bool non_dominates(const front &p) const {
            // trivial case: front is empty
            if (empty()) {
                return true;
            }

            for (auto &[k, v] : p) {
                if (!non_dominates(k)) {
                    return false;
                }
            }
            return true;
        }

      public /* Indicators / Pareto Concept */:
        /// \brief Get hypervolume from the nadir point
        /// If there is no specific reference point, we use
        /// the nadir point as reference.
        /// \param reference_point Reference point
        /// \return Hypervolume of this front
        dimension_type hypervolume() const { return hypervolume(nadir()); }

        /// \brief Get exact hypervolume
        /// \note Use the other hypervolume function if this takes
        /// too long (m is too large).
        /// \param reference_point Reference point
        /// \return Hypervolume of this front
        dimension_type hypervolume(point_type reference_point) const {
            // reshape points
            std::vector<double> data;
            data.reserve(size() * dimensions());
            for (const auto &[k, v] : *this) {
                point<double, point_type::compile_dimensions,
                      typename point_type::coordinate_system_t>
                    inv = k;
                for (size_t i = 0; i < dimensions(); ++i) {
                    if (!is_minimization(i)) {
                        inv[i] = -inv[i];
                    }
                }
                data.insert(data.end(), inv.begin(), inv.end());
            }
            point_type inv_ref = reference_point;
            for (size_t i = 0; i < dimensions(); ++i) {
                if (!is_minimization(i)) {
                    inv_ref[i] = -inv_ref[i];
                }
            }
            std::vector<double> v_ref(inv_ref.begin(), inv_ref.end());
            int n = static_cast<int>(size());
            int d = static_cast<int>(dimensions());
            static std::mutex hv_mutex;
            hv_mutex.lock();
            double hv = fpli_hv(data.data(), d, n, v_ref.data());
            hv_mutex.unlock();
            return hv;
        }

        /// \brief Get hypervolume with monte-carlo simulation
        dimension_type hypervolume(size_t sample_size) const {
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
            double hv_upper_limit = 1;
            auto m = ideal();
            for (size_t i = 0; i < m.dimensions(); ++i) {
                hv_upper_limit *= abs(reference_point[i] - m[i]);
            }

            size_t max_monte_carlo_iter = sample_size;
            size_t hit = 0;
            size_t miss = 0;
            for (size_t i = 0; i < max_monte_carlo_iter; ++i) {
                point_type rand(dimensions());
                for (size_t j = 0; j < dimensions(); ++j) {
                    std::uniform_real_distribution<dimension_type> d(
                        m[j], reference_point[j]);
                    rand[j] = d(generator());
                }
                if (dominates(rand)) {
                    hit++;
                } else {
                    miss++;
                }
            }

            return hv_upper_limit * hit / static_cast<double>(hit + miss);
        }

        /// \brief Coverage indicator
        /// \see http://www.optimization-online.org/DB_FILE/2018/10/6887.pdf
        double coverage(const front &rhs) const {
            size_t hits = 0;
            for (const auto &[k, v] : rhs) {
                hits += dominates(k);
            }
            return static_cast<double>(hits) / rhs.size();
        }

        /// \brief Ratio of coverage indicators
        double coverage_ratio(const front &rhs) const {
            return coverage(rhs) / rhs.coverage(*this);
        }

        /// \brief Generational distance
        double gd(const front &reference) const {
            if (empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference.empty()) {
                return dimension_type{0};
            }
            double distances = 0.;
            for (const auto &[k, v] : *this) {
                distances += distance(k, reference.find_nearest(k)->first);
            }
            return distances / size();
        }

        /// \brief Standard deviation from the generational distance
        /// It measures the deformation of the Pareto set
        /// approximation according to a Pareto optimal solution set.
        double std_gd(const front &reference) const {
            if (empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference.empty()) {
                return dimension_type{0};
            }
            double _gd = gd(reference);
            double std_dev = 0.;
            for (const auto &[k, v] : *this) {
                double dist = distance(k, reference.find_nearest(k)->first);
                std_dev += pow(dist - _gd, 2.);
            }
            return sqrt(std_dev) / size();
        }

        /// \brief Inverted generational distance
        double igd(const front &reference) const { return reference.gd(*this); }

        /// \brief Standard deviation from the inverted generational distance
        double std_igd(const front &reference) const {
            return reference.std_gd(*this);
        }

        /// \brief Hausdorff indicator: max(GD,IGD)
        double hausdorff(const front &reference) const {
            return std::max(gd(reference), igd(reference));
        }

        /// \brief IGD+ indicator
        double igd_plus(const front &reference_front) const {
            if (empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference_front.empty()) {
                return dimension_type{0};
            }
            double distances = 0.;
            // for each element in the reference front
            for (const auto &item : reference_front) {
                auto min_it = std::min_element(
                    begin(), end(), [&](const auto &a, const auto &b) {
                        return a.first.distance_to_dominated_box(
                                   item.first, is_minimization_) <
                               b.first.distance_to_dominated_box(
                                   item.first, is_minimization_);
                    });
                distances += min_it->first.distance_to_dominated_box(
                    item.first, is_minimization_);
            }
            return distances / reference_front.size();
        }

        /// \brief STD-IGD+ indicator
        double std_igd_plus(const front &reference_front) const {
            if (empty()) {
                return std::numeric_limits<dimension_type>::max();
            }
            if (reference_front.empty()) {
                return dimension_type{0};
            }
            double _igd_plus = igd_plus(reference_front);
            double std_dev = 0.;
            for (const auto &item : reference_front) {
                auto min_it = std::min_element(
                    begin(), end(), [&](const auto &a, const auto &b) {
                        return a.first.distance_to_dominated_box(
                                   item.first, is_minimization_) <
                               b.first.distance_to_dominated_box(
                                   item.first, is_minimization_);
                    });
                auto distance = min_it->first.distance_to_dominated_box(
                    item.first, is_minimization_);
                std_dev += pow(distance - _igd_plus, 2.);
            }
            return sqrt(std_dev) / size();
        }

        /// \brief Uniformity metric
        /// This is the minimal distance between two points of the Pareto front
        /// approximation. This measure is straightforward to compute and easy
        /// to understand. However, it does not really provide pertinent
        /// information on the repartition of the points along the Pareto
        /// front approximation.
        [[nodiscard]] double uniformity() const {
            if (size() < 2) {
                return std::numeric_limits<double>::infinity();
            }
            auto ita = begin();
            auto itb = find_nearest_exclusive(ita->first);
            double min_distance = distance(ita->first, itb->first);
            for (; ita != end(); ++ita) {
                itb = find_nearest_exclusive(ita->first);
                min_distance =
                    std::min(min_distance, distance(ita->first, itb->first));
            }
            return min_distance;
        }

        /// \brief Average distance between points
        [[nodiscard]] double average_distance() const {
            double sum = 0.0;
            for (auto ita = begin(); ita != end(); ++ita) {
                auto itb = ita;
                itb++;
                for (; itb != end(); ++itb) {
                    sum += distance(ita->first, itb->first);
                }
            }
            return sum / (((size() - 1) * (size())) / 2);
        }

        /// \brief Average nearest distance between points
        [[nodiscard]] double average_nearest_distance(size_t k = 5) const {
            double sum = 0.0;
            for (auto ita = begin(); ita != end(); ++ita) {
                double nearest_avg = 0.0;
                for (auto itb = find_nearest(ita->first, k + 1); itb != end();
                     ++itb) {
                    nearest_avg += distance(ita->first, itb->first);
                }
                sum += nearest_avg / k;
            }
            return sum / size();
        }

        /// \brief Crowding distance of an element
        double crowding_distance(const_iterator element, point_type worst_point,
                                 point_type ideal_point) const {
            double sum = 0.0;
            // for each dimension
            for (size_t i = 0; i < dimensions(); ++i) {
                // find the closest elements in that dimension
                auto p1 = element->first;
                auto p2 = element->first;
                p1[i] = worst_point[i];
                p2[i] = ideal_point[i];
                box_type s(p1, p2);
                auto itb = data_.find_nearest(s, 3);
                while (itb != end()) {
                    sum += distance(itb->first, s);
                    ++itb;
                }
            }
            return sum;
        }

        /// \brief Crowding distance of an element
        double crowding_distance(const_iterator element) const {
            return crowding_distance(element, worst(), ideal());
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

        /// \brief Average crowding distance of all elements in the front
        [[nodiscard]] double average_crowding_distance() const {
            double sum = 0.0;
            const point_type worst_point = worst();
            const point_type ideal_point = ideal();
            // for each point
            for (auto ita = begin(); ita != end(); ++ita) {
                sum += crowding_distance(ita, worst_point, ideal_point);
            }
            return sum / size();
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

        /// \brief Normalized direct between two objectives ([0,1])
        [[nodiscard]] double normalized_direct_conflict(const size_t a,
                                                        const size_t b) const {
            // max value in each term is max_a-min_a or max_b-min_b
            dimension_type range_a =
                is_minimization(a) ? worst(a) - ideal(a) : ideal(a) - worst(a);
            dimension_type range_b =
                is_minimization(b) ? worst(b) - ideal(b) : ideal(b) - worst(b);
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
        void swap(front &other) noexcept {
            other.data_.swap(data_);
            std::swap(is_minimization_, other.is_minimization_);
        }

      public /* Modifiers: Multimap Concept */:
        /// \brief Clear the front
        void clear() noexcept { data_.clear(); }

        /// \brief Insert element pair
        /// Insertion removes any point dominated by the point
        /// before inserting the element in the rtree
        /// \param v Pair value <point, value>
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        std::pair<iterator, bool> insert(const value_type &v) {
            maybe_adjust_dimensions(v);
            if (!dominates(v.first)) {
                clear_dominated(v.first);
                return {data_.insert(v), true};
            }
            return {end(), false};
        }

        /// \brief Move element pair to pareto front
        /// \param v Pair value <point, value>
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        std::pair<iterator, bool> insert(value_type &&v) {
            maybe_adjust_dimensions(v);
            if (!dominates(v.first)) {
                clear_dominated(v.first);
                auto p = std::move(v);
                return {data_.insert(p), true};
            }
            return {end(), false};
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
        size_type insert(InputIterator first, InputIterator last) {
            size_t s = 0;
            for (auto it = first; it != last; ++it) {
                std::pair<iterator, bool> r = insert(*it);
                s += r.second;
            }
            return s;
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
            size_t s = 0;
            for (auto it = il.begin(); it != il.end(); ++it) {
                std::pair<iterator, bool> r = insert(*it);
                s += r.second;
            }
            return s;
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

        /// \brief Erase element pointed by iterator from the front
        /// \warning The modification of the rtree may invalidate the iterators.
        iterator erase(const_iterator position) {
            auto it = find(position->first);
            return data_.erase(it);
        }

        /// \brief Erase element pointed by iterator from the front
        /// \warning The modification of the rtree may invalidate the iterators.
        iterator erase(iterator position) {
            auto it = find(position->first);
            return data_.erase(it);
        }

        /// \brief Remove range of iterators from the front
        iterator erase(const_iterator first, const_iterator last) {
            return data_.erase(first, last);
        }

        /// \brief Erase element from the front
        /// \param v Point
        size_type erase(const key_type &point) { return data_.erase(point); }

        /// \brief Splices nodes from another container
        void merge(front &source) noexcept {
            insert(source.begin(), source.end());
        }

      public /* Lookup / Multimap Concept */:
        /// \brief Returns the number of elements with key that compares
        /// equivalent to the specified argument.
        size_type count(const key_type &k) const {
            iterator it = (const_cast<front *>(this))->find_intersection(k);
            iterator end = (const_cast<front *>(this))->end();
            return static_cast<size_type>(std::distance(it, end));
        }

        /// \brief Returns the number of elements with key that compares
        /// equivalent to the specified argument.
        template <class L> size_type count(const L &k) const {
            iterator it = find_intersection(key_type{k}, key_type{k});
            return static_cast<size_type>(std::distance(it, end()));
        }

        /// \brief Find element by point
        const_iterator find(const key_type &k) const { return data_.find(k); }

        /// \brief Find element by point
        iterator find(const key_type &k) { return data_.find(k); }

        /// \brief Finds an element with key equivalent to key
        template <class L> iterator find(const L &x) { return data_.find(x); }

        /// \brief Finds an element with key equivalent to key
        template <class L> const_iterator find(const L &x) const {
            return data_.find(x);
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
            return data_.find(ps);
        }

        /// \brief Get iterator to first element that passes a list of
        /// predicates
        iterator find(const predicate_list_type &ps) noexcept {
            return data_.find(ps);
        }

        /// \brief Find intersection between points and query box
        iterator find_intersection(const key_type &k) {
            return find_intersection(k, k);
        }

        /// \brief Find intersection between points and query box
        const_iterator find_intersection(const key_type &k) const {
            return find_intersection(k, k);
        }

        /// \brief Find points in a box
        const_iterator find_intersection(const point_type &lb,
                                         const point_type &ub) const {
            return data_.find_intersection(lb, ub);
        }

        /// \brief Find points in a box
        iterator find_intersection(const point_type &lb, const point_type &ub) {
            return data_.find_intersection(lb, ub);
        }

        /// \brief Find points within a box (intersection minus borders)
        const_iterator find_within(const point_type &lb,
                                   const point_type &ub) const {
            return data_.find_within(lb, ub);
        }

        /// \brief Find points within a box (intersection minus borders)
        iterator find_within(const point_type &lb, const point_type &ub) {
            return data_.find_within(lb, ub);
        }

        /// \brief Find points disjointed of a box (intersection - borders)
        const_iterator find_disjoint(const point_type &lb,
                                     const point_type &ub) const {
            return data_.find_disjoint(lb, ub);
        }

        /// \brief Find points disjointed of a box (intersection - borders)
        iterator find_disjoint(const point_type &lb, const point_type &ub) {
            return data_.find_disjoint(lb, ub);
        }

        /// \brief Find nearest point
        const_iterator find_nearest(const point_type &p) const {
            return data_.find_nearest(p);
        }

        /// \brief Find nearest point
        iterator find_nearest(const point_type &p) {
            return data_.find_nearest(p);
        }

        /// \brief Find k nearest points
        const_iterator find_nearest(const point_type &p, size_t k) const {
            return data_.find_nearest(p, k);
        }

        /// \brief Find k nearest points
        iterator find_nearest(const point_type &p, size_t k) {
            return data_.find_nearest(p, k);
        }

        /// \brief Find k nearest points
        const_iterator find_nearest(std::initializer_list<dimension_type> p,
                                    size_t k) const {
            return data_.find_nearest(point_type(p), k);
        }

        /// \brief Find k nearest points
        iterator find_nearest(std::initializer_list<dimension_type> p,
                              size_t k) {
            return data_.find_nearest(point_type(p), k);
        }

        /// \brief Find k nearest points
        const_iterator find_nearest(const box_type &b, size_t k) const {
            return data_.find_nearest(b, k);
        }

        /// \brief Find k nearest points
        iterator find_nearest(const box_type &b, size_t k) {
            return data_.find_nearest(b, k);
        }

        /// \brief Get iterator to element with maximum value in a given
        /// dimension
        iterator max_element(size_t dimension) {
            return data_.max_element(dimension);
        }

        /// \brief Get iterator to element with maximum value in a given
        /// dimension
        const_iterator max_element(size_t dimension) const {
            return data_.max_element(dimension);
        }

        /// \brief Get iterator to element with minimum value in a given
        /// dimension
        iterator min_element(size_t dimension) {
            return data_.min_element(dimension);
        }

        /// \brief Get iterator to element with minimum value in a given
        /// dimension
        const_iterator min_element(size_t dimension) const {
            return data_.min_element(dimension);
        }

      public /* Modifiers: Lookup / Pareto Concept */:
        /// \brief Find points dominated by p
        /// If p is in the front, it dominates no other point
        /// Otherwise, it dominates the whole intersection between p and nadir
        const_iterator find_dominated(const point_type &p) const {
            const bool p_is_in_the_front = find(p) != end();
            if (p_is_in_the_front) {
                return end();
            }

            point_type worst_point = worst();
            const bool p_dominates_worst =
                p.dominates(worst_point, is_minimization_);
            if (!p_dominates_worst) {
                return end();
            }

            return find_intersection(worst_point, p);
        }

        /// \brief Find points dominated by p
        /// If p is in the front, it dominates no other point
        /// Otherwise, it dominates the whole intersection between p and nadir
        iterator find_dominated(const point_type &p) {
            const bool p_is_in_the_front = find(p) != end();
            if (p_is_in_the_front) {
                return end();
            }

            point_type worst_point = worst();
            const bool p_dominates_worst =
                p.dominates(worst_point, is_minimization_);
            if (!p_dominates_worst) {
                return end();
            }

            return find_intersection(worst_point, p);
        }

        /// \brief Find nearest point excluding itself
        const_iterator find_nearest_exclusive(const point_type &p) const {
            auto itself = find_nearest(p);
            if (itself->first != p) {
                return itself;
            }
            for (size_t i = 1; i <= size(); ++i) {
                for (auto it = find_nearest(p, i); it != end(); ++it) {
                    if (itself->first != it->first ||
                        !mapped_type_custom_equality_operator(itself->second,
                                                              it->second)) {
                        return it;
                    }
                }
            }
            return end();
        }

        /// \brief Find nearest point excluding itself
        iterator find_nearest_exclusive(const point_type &p) {
            auto itself = find_nearest(p);
            if (itself->first != p) {
                return itself;
            }
            for (size_t i = 1; i <= size(); ++i) {
                for (auto it = find_nearest(p, i); it != end(); ++it) {
                    if (itself->first != it->first ||
                        !mapped_type_custom_equality_operator(itself->second,
                                                              it->second)) {
                        return it;
                    }
                }
            }
            return end();
        }

        /// \brief Get iterator to element with best value in dimension d
        const_iterator ideal_element(size_t d) const {
            return is_minimization(d) ? data_.min_element(d)
                                      : data_.max_element(d);
        }

        /// \brief Get iterator to element with best value in dimension d
        iterator ideal_element(size_t d) {
            return is_minimization(d) ? data_.min_element(d)
                                      : data_.max_element(d);
        }

        /// \brief Element with nadir value in front dimension d
        const_iterator nadir_element(size_t d) const {
            return is_minimization(d) ? data_.max_element(d)
                                      : data_.min_element(d);
        }

        /// \brief Element with nadir value in front dimension d
        iterator nadir_element(size_t d) {
            return is_minimization(d) ? data_.max_element(d)
                                      : data_.min_element(d);
        }

        /// \brief Iterator to element with worst value in front dimension d
        const_iterator worst_element(size_t d) const {
            return nadir_element(d);
        }

        /// \brief Iterator to element with worst value in front dimension d
        iterator worst_element(size_t d) { return nadir_element(d); }

      public /* Non-Modifying Functions: AssociativeContainer */:
        /// \brief Returns the function object that compares keys
        /// This function is here mostly to conform with the
        /// AssociativeContainer concepts. It's possible but not
        /// very useful.
        key_compare key_comp() const noexcept { return data_.key_comp(); }

        /// \brief Returns the function object that compares values
        value_compare value_comp() const noexcept { return data_.value_comp(); }

        /// \brief Returns the function object that compares numbers
        /// This is the comparison operator for a single dimension
        dimension_compare dimension_comp() const noexcept {
            return data_.dimension_comp();
        }

        /// \brief Stream front properties to an ostream
        friend std::ostream &operator<<(std::ostream &os, const front &pf) {
            os << "Pareto front (" << pf.size() << " elements - {";
            for (size_t i = 0; i < pf.is_minimization_.size() - 1; ++i) {
                os << (pf.is_minimization_[i] ? "minimization" : "maximization")
                   << ", ";
            }
            os << (pf.is_minimization_.back() ? "minimization" : "maximization")
               << "})";
            return os;
        }

      public /* Spatial Concept */:
        /// \brief Set the number of dimensions (if runtime)
        /// This is only for internal use and not part of the concepts.
        ///
        /// We make this public because we need it in the python bindings.
        /// You're not supposed to use this function directly.
        ///
        /// In general, you probably shouldn't be using this function
        /// directly.
        ///
        /// This should only be used if the number of dimensions was not
        /// set at compile-time. Even at runtime, you might only
        /// use this function before inserting the first point in the
        /// container.
        ///
        /// In most cases, the dimension will be inferred from in compile time
        /// or according to the first point you insert in the front,
        /// so there is no reason to worry about that.
        ///
        /// The special reason to use this function is the python interface,
        /// so that we can set the dimension at runtime even before
        /// inserting the first point. This guarantees an interface coherent
        /// with compile time dimensions.
        ///
        /// We used to have the dimension in the constructor, but this involved
        /// a lot of useless redundancy because 1) C++ users should use
        /// compile-time dimensions anyway, 2) the python bindings solve that
        /// for users, and 3) it's a very ugly syntax anyway.
        void dimensions(size_t m) noexcept { maybe_adjust_dimensions(m); }

        /// \brief Check if front passes the variants that define a front
        /// This is only for tests and it's not officially part of the concept
        [[nodiscard]] bool check_invariants() const {
            for (const value_type &item : data_) {
                for (const value_type &item2 : data_) {
                    if (item.first.dominates(item2.first, is_minimization_)) {
                        return false;
                    }
                }
            }
            return true;
        }
      private /* functions */:
        /// \brief Clear solutions are dominated by p
        /// Pareto-optimal front is the set F consisting of
        /// all non-dominated solutions x in the whole
        /// search space. No solution can dominate another
        /// solution. Note that this means two solutions
        /// might have the same values though.
        void clear_dominated(const point_type &p) {
            if (!empty()) {
                iterator it = find_dominated(p);
                erase(it, end());
            }
        }

        double distance(const point_type &p1, const point_type &p2) const {
#ifdef BUILD_BOOST_TREE
            if constexpr (number_of_compile_dimensions > 0) {
                return boost::geometry::distance(p1, p2);
            } else {
#endif
                double dist = 0.;
                for (size_t i = 0; i < dimensions(); ++i) {
                    dist += pow(p1[i] - p2[i], 2);
                }
                return sqrt(dist);
#ifdef BUILD_BOOST_TREE
            }
#endif
        }

        double distance(const point_type &p, const box_type &b) const {
#ifdef BUILD_BOOST_TREE
            if constexpr (number_of_compile_dimensions > 0) {
                boost::geometry::model::box<point_type> boost_box(b.first(),
                                                                  b.second());
                return boost::geometry::distance(p, boost_box);
            } else {
#endif
                double dist = 0.0;
                for (size_t i = 0; i < dimensions(); ++i) {
                    double di = std::max(
                        std::max(b.first()[i] - p[i], p[i] - b.second()[i]),
                        0.0);
                    dist += di * di;
                }
                return sqrt(dist);
#ifdef BUILD_BOOST_TREE
            }
#endif
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

        /// If the dimension is being set at runtime, this sets the
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

        /// If the dimension is being set at runtime, this sets the
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

        inline void maybe_adjust_dimensions(const point_type &position) {
            if constexpr (number_of_compile_dimensions == 0) {
                if (empty() && is_minimization_.size() == 1) {
                    maybe_resize(is_minimization_, position.dimensions());
                    std::fill(is_minimization_.begin() + 1,
                              is_minimization_.end(),
                              *is_minimization_.begin());
                }
            }
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
        /// \brief Container storing the pareto points
        container_type data_{};

        /// \brief Whether each dimension is minimization or maximization
        /// We use uint8_t instead of bool to avoid the array specialization
        directions_type is_minimization_;

      public:
        /// We won't need this when we finally deprecate boost tree
        template <class, size_t, class, class> friend class archive;
    };

    /// \brief Relational operator < for fronts and fronts
    /// We establish an order relationship for fronts that completely dominate
    /// one another.
    template <typename K, size_t M, typename T, typename C>
    bool operator<(const front<K, M, T, C> &lhs, const front<K, M, T, C> &rhs) {
        return lhs.dominates(rhs);
    }

    /// \brief Relational operator > for fronts and fronts
    /// We establish an order relationship for fronts that completely dominate
    /// one another.
    template <typename K, size_t M, typename T, typename C>
    bool operator>(const front<K, M, T, C> &lhs, const front<K, M, T, C> &rhs) {
        return rhs < lhs;
    }

    /// \brief Relational operator <= for fronts and fronts
    /// We establish an order relationship for fronts that completely dominate
    /// one another.
    template <typename K, size_t M, typename T, typename C>
    bool operator<=(const front<K, M, T, C> &lhs,
                    const front<K, M, T, C> &rhs) {
        return lhs < rhs || !(rhs < lhs);
    }

    /// \brief Relational operator >= for fronts and fronts
    /// We establish an order relationship for fronts that completely dominate
    /// one another.
    template <typename K, size_t M, typename T, typename C>
    bool operator>=(const front<K, M, T, C> &lhs,
                    const front<K, M, T, C> &rhs) {
        return lhs > rhs || !(rhs > lhs);
    }

    /// \brief Relational operator < for fronts and points
    /// We establish an order relationship for fronts that completely dominate
    /// one another
    /// stablish an order relationship for fronts that and points by replicating
    /// what would happen if that point was a front with a single element
    /// These relationships are very important for archives
    template <typename K, size_t M, typename T, typename C>
    bool operator<(const front<K, M, T, C> &lhs,
                   const typename front<K, M, T, C>::key_type &rhs) {
        return lhs.dominates(rhs);
    }

    /// \brief Relational operator > for fronts and points
    /// We establish an order relationship for fronts that completely dominate
    /// one another
    /// stablish an order relationship for fronts that and points by replicating
    /// what would happen if that point was a front with a single element
    /// These relationships are very important for archives
    template <typename K, size_t M, typename T, typename C>
    bool operator>(const front<K, M, T, C> &lhs,
                   const typename front<K, M, T, C>::key_type &rhs) {
        // The idea here is that point < front only if point completely
        // dominates front because we are treating the point as a
        // front with one element here so that sets of fronts
        // know how to order elements. If point only partially dominated
        // front, then we can't replace the front. We should at most
        // insert the point in the front. This is clearer when
        // we see the logic of archives.
        return lhs.is_completely_dominated_by(rhs);
    }

    /// \brief Relational operator <= for fronts and points
    /// We establish an order relationship for fronts that completely dominate
    /// one another.
    /// We establish an order relationship for fronts and points by replicating
    /// what would happen if that point was a front with a single element
    /// These relationships are very important for archives
    template <typename K, size_t M, typename T, typename C>
    bool operator<=(const front<K, M, T, C> &lhs,
                    const typename front<K, M, T, C>::key_type &rhs) {
        return lhs < rhs || !(rhs < lhs);
    }

    /// \brief Relational operator >= for fronts and points
    /// We establish an order relationship for fronts that completely dominate
    /// one another.
    /// We establish an order relationship for fronts and points by replicating
    /// what would happen if that point was a front with a single element
    /// These relationships are very important for archives
    template <typename K, size_t M, typename T, typename C>
    bool operator>=(const front<K, M, T, C> &lhs,
                    const typename front<K, M, T, C>::key_type &rhs) {
        return lhs > rhs || !(rhs > lhs);
    }

    /// \brief Relational operator < for points and fronts
    /// We establish an order relationship for fronts that completely dominate
    /// one another.
    /// We establish an order relationship for fronts and points by replicating
    /// what would happen if that point was a front with a single element
    /// These relationships are very important for archives
    template <typename K, size_t M, typename T, typename C>
    bool operator<(const typename front<K, M, T, C>::key_type &lhs,
                   const front<K, M, T, C> &rhs) {
        return rhs > lhs;
    }

    /// \brief Relational operator > for points and fronts
    /// We establish an order relationship for fronts that completely dominate
    /// one another.
    /// We establish an order relationship for fronts and points by replicating
    /// what would happen if that point was a front with a single element
    /// These relationships are very important for archives
    template <typename K, size_t M, typename T, typename C>
    bool operator>(const typename front<K, M, T, C>::key_type &lhs,
                   const front<K, M, T, C> &rhs) {
        return rhs < lhs;
    }

    /// \brief Relational operator <= for points and fronts
    /// We establish an order relationship for fronts that completely dominate
    /// one another.
    /// We establish an order relationship for fronts and points by replicating
    /// what would happen if that point was a front with a single element
    /// These relationships are very important for archives
    template <typename K, size_t M, typename T, typename C>
    bool operator<=(const typename front<K, M, T, C>::key_type &lhs,
                    const front<K, M, T, C> &rhs) {
        return rhs >= lhs;
    }

    /// \brief Relational operator >= for points and fronts
    /// We establish an order relationship for fronts that completely dominate
    /// one another.
    /// We establish an order relationship for fronts and points by replicating
    /// what would happen if that point was a front with a single element
    /// These relationships are very important for archives
    template <typename K, size_t M, typename T, typename C>
    bool operator>=(const typename front<K, M, T, C>::key_type &lhs,
                    const front<K, M, T, C> &rhs) {
        return rhs <= lhs;
    }

    /// \brief Equality operator
    template <class K, size_t M, class T, class C>
    bool operator==(const front<K, M, T, C> &lhs,
                    const front<K, M, T, C> &rhs) {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        if (lhs.dimensions() != rhs.dimensions()) {
            return false;
        }
        for (typename front<K, M, T, C>::size_type i = 0; i < rhs.dimensions();
             ++i) {
            if (lhs.is_minimization(i) != rhs.is_minimization(i)) {
                return false;
            }
        }
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                          [](const typename front<K, M, T, C>::value_type &a,
                             const typename front<K, M, T, C>::value_type &b) {
                              return a.first == b.first &&
                                     mapped_type_custom_equality_operator(
                                         a.second, b.second);
                          });
    }

    /// \brief Inequality operator
    template <class K, size_t M, class T, class C>
    bool operator!=(const front<K, M, T, C> &lhs,
                    const front<K, M, T, C> &rhs) {
        return !(lhs == rhs);
    }

} // namespace pareto

#endif // PARETO_FRONTS_PARETO_FRONT_RTREE_H
