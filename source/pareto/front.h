#ifndef PARETO_FRONTS_PARETO_FRONT_RTREE_H
#define PARETO_FRONTS_PARETO_FRONT_RTREE_H

#include <random>
#include <map>
#include <initializer_list>
#include <ostream>
#include <thread>
#include <mutex>
#include <optional>

#include <pareto/hv-2.0rc2/hv.h>

#include <pareto/common/common.h>
#include <pareto/point.h>
#include <pareto/query/query_box.h>
#include <pareto/tree/vector_tree.h>
#include <pareto/tree/quad_tree.h>
#include <pareto/tree/kd_tree.h>

#include <pareto/common/metaprogramming.h>

#ifdef BUILD_BOOST_TREE
#include <pareto/tree/boost_tree.h>
#endif

#include <pareto/tree/r_tree.h>
#include <pareto/tree/r_star_tree.h>

namespace pareto {
    struct vector_tree_tag {
    };
    struct quad_tree_tag {
    };
    struct kd_tree_tag {
    };
    struct boost_tree_tag {
    };
    struct r_tree_tag {
    };
    struct r_star_tree_tag {
    };

#ifdef BUILD_BOOST_TREE
    template <class nt, size_t ncd, class mt>
  using default_type_for_boost_tree_tag = boost_tree<nt,ncd,mt>;
#else
    template<class nt, size_t ncd, class mt>
    using default_type_for_boost_tree_tag = r_tree<nt, ncd, mt>;
#endif

    constexpr bool minimization = true;
    constexpr bool min = true;
    constexpr bool maximization = false;
    constexpr bool max = false;

    template <size_t NUMBER_OF_DIMENSIONS>
    struct default_tag_value {
        using value = kd_tree_tag;
    };

    template <>
    struct default_tag_value<1> {
        using value = vector_tree_tag;
    };

    template<size_t NUMBER_OF_DIMENSIONS>
    using default_tag = typename default_tag_value<NUMBER_OF_DIMENSIONS>::value;

    template<typename NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, typename ELEMENT_TYPE, typename TAG>
    class archive;

    /// \class Pareto Front
    /// The fronts have their dimension set at compile time
    /// If we set the dimension to 0, then it's defined at runtime
    /// Defining at runtime is only useful for the python bindings
    /// When dimensions are set a runtime, we find out about the dimension
    /// when we insert the first element in the front. At this point,
    /// the front dimension is set and we cannot change it.
    template<typename NUMBER_TYPE = double, size_t NUMBER_OF_DIMENSIONS = 0, typename ELEMENT_TYPE = unsigned, typename TAG = default_tag<NUMBER_OF_DIMENSIONS>>
    class front {
    public /* types */:
        friend archive<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, TAG>;
        using self_type =
        front<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, TAG>;
        using number_type = NUMBER_TYPE;
        static constexpr size_t number_of_compile_dimensions = NUMBER_OF_DIMENSIONS;

        using point_type = point<number_type, number_of_compile_dimensions>;
        using box_type = query_box<number_type, number_of_compile_dimensions>;

        using key_type = point_type;
        using mapped_type = ELEMENT_TYPE;
        using value_type = std::pair<key_type, mapped_type>;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = value_type *;
        using const_pointer = value_type const *;
        using size_type = size_t;
        using predicate_list_type = predicate_list<number_type, number_of_compile_dimensions, mapped_type>;

        template<typename T>
        struct tag_struct_to_tree_type {
            using type = kd_tree<number_type, number_of_compile_dimensions, mapped_type>;
        };

        template<>
        struct tag_struct_to_tree_type<vector_tree_tag> {
            using type = vector_tree<number_type, number_of_compile_dimensions, mapped_type>;
        };

        template<>
        struct tag_struct_to_tree_type<quad_tree_tag> {
            using type = quad_tree<number_type, number_of_compile_dimensions, mapped_type>;
        };

        template<>
        struct tag_struct_to_tree_type<kd_tree_tag> {
            using type = kd_tree<number_type, number_of_compile_dimensions, mapped_type>;
        };

        template<>
        struct tag_struct_to_tree_type<boost_tree_tag> {
            using type = std::conditional_t<number_of_compile_dimensions == 0,
                    r_tree<number_type, number_of_compile_dimensions, mapped_type>,
                    default_type_for_boost_tree_tag<number_type, number_of_compile_dimensions, mapped_type>
            >;
        };

        template<>
        struct tag_struct_to_tree_type<r_tree_tag> {
            using type = r_tree<number_type, number_of_compile_dimensions, mapped_type>;
        };

        template<>
        struct tag_struct_to_tree_type<r_star_tree_tag> {
            using type = r_star_tree<number_type, number_of_compile_dimensions, mapped_type>;
        };

        using internal_type = typename tag_struct_to_tree_type<TAG>::type;

        /// Is the internal type using the fast allocator
        constexpr static bool is_using_default_fast_allocator() {
            return internal_type::is_using_default_fast_allocator();
        }

        using node_allocator_type = typename internal_type::node_allocator_type;

        using iterator = typename internal_type::iterator;
        using const_iterator = typename internal_type::const_iterator;

        using internal_minimization_type = std::conditional_t<number_of_compile_dimensions == 0,
                std::vector<uint8_t>, std::array<uint8_t, number_of_compile_dimensions>>;

        using difference_type = typename internal_type::difference_type;

    public /* constructors */:
        /// \brief Create a pareto set from value range and is_minimization range
        /// This is the constructor all other constructors use as reference
        /// They are all just shortcuts to this constructor
        /// \tparam ValueIterator Iterator to values (pair of point<number> and element)
        /// \tparam IsMinimizationIterator Iterator to minimization directions (bool, or uint8_t)
        /// \param value_begin Iterator to first pareto front candidate
        /// \param value_end Iterator to last + 1 pareto front candidate
        /// \param is_minimization_begin Iterator to first minimization direction
        /// \param is_minimization_end Iterator to last + 1 minimization direction
        template<class ValueIterator, class IsMinimizationIterator>
        front(ValueIterator value_begin,
              ValueIterator value_end,
              IsMinimizationIterator is_minimization_begin,
              IsMinimizationIterator is_minimization_end)
                : data_{} {
            long minimization_directions = std::distance(is_minimization_begin, is_minimization_end);
            long pareto_dimension =
                    minimization_directions == 0 ? number_of_compile_dimensions : minimization_directions;

            if constexpr (number_of_compile_dimensions != 0) {
                if (pareto_dimension != number_of_compile_dimensions) {
                    throw std::invalid_argument(
                            "The size specified at compile time does not match the number of minimization directions");
                }
            }

            if (pareto_dimension == 0) {
                if constexpr (number_of_compile_dimensions == 0) {
                    maybe_resize(is_minimization_, 1);
                }
                std::fill(is_minimization_.begin(), is_minimization_.begin(), true);
            } else {
                maybe_resize(is_minimization_, pareto_dimension);
                std::copy(is_minimization_begin, is_minimization_end, is_minimization_.begin());
            }

            while (value_begin != value_end) {
                insert(*value_begin);
                ++value_begin;
            }
        }

        /// \brief Create a pareto set from value list and is_minimization range
        /// Same as default, with initializer list instead of iterators for values
        template<class IsMinimizationIterator>
        front(std::initializer_list<value_type> values,
              IsMinimizationIterator is_minimization_begin,
              IsMinimizationIterator is_minimization_end)
                : front(values.begin(), values.end(), is_minimization_begin, is_minimization_end) {}

        /// \brief Create a pareto set from value range and is_minimization list
        /// Same as default, with initializer list instead of iterators for values
        template<class ValueIterator>
        front(ValueIterator value_begin,
              ValueIterator value_end,
              std::initializer_list<bool> is_minimization)
                : front(value_begin, value_end, is_minimization.begin(), is_minimization.end()) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether it is minimization
        template<class InputIterator>
        front(InputIterator first, InputIterator last, bool is_minimization)
                : front(first, last,
                        std::vector<uint8_t>(std::max(number_of_compile_dimensions, size_t(1)), is_minimization)) {}

        /// \brief Create a pareto set from a list of value pairs
        /// Each pair has a point and a value
        front(std::initializer_list<value_type> il)
                : front(il.begin(), il.end(), {}) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether dimensions are minimization
        front(std::initializer_list<value_type> il, bool is_minimization)
                : front(il.begin(), il.end(), is_minimization) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        front(std::initializer_list<value_type> il, const std::vector<uint8_t> &is_minimization)
                : front(il.begin(), il.end(), is_minimization.begin(), is_minimization.end()) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        front(std::initializer_list<value_type> il, std::initializer_list<bool> is_minimization)
                : front(il.begin(), il.end(), is_minimization.begin(), is_minimization.end()) {}

        /// \brief Create a pareto set from a list of value pairs
        explicit front(const std::vector<value_type> &v)
                : front(v.begin(), v.end(), {}) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether dimensions are minimization
        front(const std::vector<value_type> &v, bool is_minimization)
                : front(v.begin(), v.end(), is_minimization) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        front(const std::vector<value_type> &v, const std::vector<uint8_t> &is_minimization)
                : front(v.begin(), v.end(), is_minimization) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        front(const std::vector<value_type> &v, std::initializer_list<bool> is_minimization)
                : front(v.begin(), v.end(), init_list_to_vector(is_minimization)) {}

        /// \brief Create a pareto set from a list of value pairs
        template<class InputIterator>
        front(InputIterator first, InputIterator last)
                : front(first, last, true) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        template<class InputIterator>
        front(InputIterator first, InputIterator last, std::vector<uint8_t> is_minimization)
                : front(first, last, is_minimization.begin(), is_minimization.end()) {}

        /// \brief Create an empty pareto set and determine whether each dimension is minimization
        explicit front(const std::vector<uint8_t> &is_minimization) :
                front({}, is_minimization.begin(), is_minimization.end()) {}

        /// \brief Create an empty pareto set and determine whether each dimension is minimization
        /// Convert the initializer list and use the constructor above
        front(std::initializer_list<bool> is_minimization)
                : front({}, is_minimization.begin(), is_minimization.end()) {}

        /// \brief Create an empty pareto set and determine whether each dimension is minimization
        explicit front(const std::array<uint8_t, number_of_compile_dimensions> &is_minimization)
                : front({}, is_minimization.begin(), is_minimization.end()) {}

        /// \brief Create an empty pareto set and determine whether it is minimization (all objectives)
        /// Create an array indicating minimization/maximization for each dimension
        /// or, if number_of_compile_dimensions == 0, create a vector of size 1 with this direction
        /// The minimization_ array gets expanded when we find out about the dimension
        explicit front(bool is_minimization)
                : front(std::vector<uint8_t>(std::max(number_of_compile_dimensions, size_t(1)), is_minimization)) {}

        /// \brief Create an empty pareto set / minimization
        /// Call constructor that takes direction to be minimization
        front() : front(true) {}

        /// \brief Copy constructor
        front(const front &m) {
            data_ = m.data_;
            is_minimization_ = m.is_minimization_;
        }

        /// \brief Copy assignment
        front &operator=(const front &rhs) {
            data_ = rhs.data_;
            is_minimization_ = rhs.is_minimization_;
            return *this;
        }

        /// \brief Move constructor
        front(front &&m) noexcept {
            data_ = std::move(m.data_);
            is_minimization_ = std::move(m.is_minimization_);
        }

        /// \brief Move assignment
        front &operator=(front &&rhs) noexcept {
            data_ = std::move(rhs.data_);
            is_minimization_ = std::move(rhs.is_minimization_);
            return *this;
        }

        /// \brief Private special constructor using an external allocator
        /// Only archives should use this constructor to avoid one allocator
        /// per front because our fast allocator is not stateless.
        front(const internal_minimization_type &is_minimization,
              std::shared_ptr<node_allocator_type> &external_allocator)
                : data_(external_allocator) {
            if constexpr (number_of_compile_dimensions > 0) {
                if (number_of_compile_dimensions != is_minimization_.size()) {
                    throw std::invalid_argument("Number of minimization directions should match the dimension");
                }
            }
            maybe_resize(is_minimization_, is_minimization.size());
            std::copy(is_minimization.begin(), is_minimization.end(), is_minimization_.begin());
        }

    public /* iterators */:

        /// \brief Get iterator to first element
        const_iterator begin() const noexcept {
            return data_.begin();
        }

        /// \brief Get iterator to first element that passes a list of predicates
        const_iterator begin(const predicate_list_type& ps) const noexcept {
            return data_.begin(ps);
        }

        /// \brief Get iterator to last + 1 element
        const_iterator end() const noexcept {
            return data_.end();
        }

        /// \brief Get iterator to first element
        iterator begin() noexcept {
            return data_.begin();
        }

        /// \brief Get iterator to first element that passes a list of predicates
        iterator begin(const predicate_list_type& ps) noexcept {
            return data_.begin(ps);
        }

        /// \brief Get iterator to last + 1 element
        iterator end() noexcept {
            return data_.end();
        }

        /// \brief Get iterator to first element in reverse
        std::reverse_iterator<const_iterator> rbegin() const noexcept {
            return std::reverse_iterator(data_.end());
        }

        /// \brief Get iterator to last + 1 element in reverse
        std::reverse_iterator<const_iterator> rend() const noexcept {
            return std::reverse_iterator(data_.begin());
        }

        /// \brief Get iterator to first element in reverse
        std::reverse_iterator<iterator> rbegin() noexcept {
            return std::reverse_iterator(data_.end());
        }

        /// \brief Get iterator to last + 1 element in reverse
        std::reverse_iterator<iterator> rend() noexcept {
            return std::reverse_iterator(data_.begin());
        }

    public /* capacity */:
        /// \brief Check if front is empty
        [[nodiscard]] bool empty() const noexcept {
            return data_.empty();
        }

        /// \brief Get number of elements in the front
        [[nodiscard]] size_type size() const noexcept {
            return data_.size();
        }

        /// \brief Get number of dimensions of elements in the front
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
        void dimensions(size_t m) noexcept {
            maybe_adjust_dimensions(m);
        }

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
                    return is_minimization_[dimension] > 0;
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

    public /* element access */:
        /// \brief Get reference to element at a given position, and create one if it does not exits
        mapped_type &operator[](const key_type &k) {
            auto it = find(k);
            if (it != end()) {
                // return reference to mapped type in the tree
                return it->second;
            } else {
                // include element in the tree with a default
                // mapped type
                auto[it2, ok] = insert(std::make_pair(k, mapped_type()));
                if (ok) {
                    // return reference to mapped type in the tree
                    return it2->second;
                } else {
                    // if item was not included in the tree
                    // return reference to a placeholder
                    // just to avoid throwing an error
                    // nothing was included in the tree because
                    // the element is dominated
                    static mapped_type placeholder;
                    return placeholder;
                }
            }
        }

        /// \brief Get reference to element at a given position, and create one if it does not exits
        mapped_type &operator[](key_type &&k) {
            auto it = find(k);
            if (it != end()) {
                // return reference to mapped type in the tree
                return it->second;
            } else {
                // include element in the tree with a default
                // mapped type
                auto[it2, ok] = insert(std::make_pair(std::move(k), mapped_type()));
                if (ok) {
                    // return reference to mapped type in the tree
                    return it2->second;
                } else {
                    // if item was not included in the tree
                    // return reference to a placeholder
                    // just to avoid throwing an error
                    // nothing was included in the tree because
                    // the element is dominated
                    static mapped_type placeholder;
                    return placeholder;
                }
            }
        }

        template<typename... Targs>
        /// \brief Get reference to element at a given position, and create one if it does not exits
        mapped_type &operator()(const number_type &x1, const Targs &... xs) {
            constexpr size_t m = sizeof...(Targs) + 1;
            assert(number_of_compile_dimensions == 0 || number_of_compile_dimensions == m);
            point_type p(m);
            copy_pack(p.begin(), x1, xs...);
            return operator[](p);
        }

        /// \brief Get reference to element at a given position, and throw error if it does not exist
        mapped_type &at(const key_type &k) {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("front::at:  key not found");
            }
        }

        /// \brief Get reference to element at a given position, and throw error if it does not exist
        const mapped_type &at(const key_type &k) const {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("front::at:  key not found");
            }
        }

    public /* relational operators */:
        /// \brief Equality operator
        bool operator==(const front &rhs) const {
            return is_minimization_ == rhs.is_minimization_ &&
                   std::equal(data_.begin(), data_.end(), rhs.data_.begin(), rhs.data_.end(),
                              [](const auto &a, const auto &b) {
                                  return a.first == b.first && mapped_type_custom_equality_operator(a.second, b.second);
                              });
        }

        /// \brief Inequality operator
        bool operator!=(const front &rhs) const {
            return is_minimization_ != rhs.is_minimization_ ||
                   !std::equal(data_.begin(), data_.end(), rhs.data_.begin(), rhs.data_.end(),
                               [](const auto &a, const auto &b) {
                                   return a.first == b.first &&
                                          mapped_type_custom_equality_operator(a.second, b.second);
                               });
        }

    public /* modifiers */:
        /// \brief Create element and emplace it in the front
        /// Emplace becomes insert because the rtree does not have
        /// an emplace function
        template<class... Args>
        std::pair<iterator, bool> emplace(Args &&... args) {
            auto v = value_type(args...);
            return insert(v);
        }

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
                return data_.insert(v);
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
                return data_.insert(p);
            }
            return {end(), false};
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
        template<class InputIterator>
        void insert(InputIterator first, InputIterator last) {
            for (auto it = first; it != last; ++it) {
                std::pair<iterator, bool> r = insert(*it);
            }
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

        /// \brief Erase element pointed by iterator from the front
        /// \warning The modification of the rtree may invalidate the iterators.
        size_t erase(const_iterator position) {
            auto it = find(position->first);
            return data_.erase(it);
        }

        /// \brief Erase element pointed by iterator from the front
        /// \warning The modification of the rtree may invalidate the iterators.
        size_t erase(iterator position) {
            auto it = find(position->first);
            return data_.erase(it);
        }

        /// \brief Erase element from the front
        /// \param v Point
        size_type erase(const key_type &point) {
            auto it = find(point);
            if (it != end()) {
                return data_.erase(it);
            } else {
                return 0;
            }
        }

        /// \brief Remove range of iterators from the front
        size_t erase(const_iterator first, const_iterator last) {
            return data_.erase(first, last);
        }

        /// \brief Remove range of iterators from the front
        size_t erase(iterator first, iterator last) {
            return data_.erase(first, last);
        }

        /// \brief Remove range of iterators from the front
        size_t erase(const_iterator first, iterator last) {
            return data_.erase(first, const_iterator(last));
        }

        /// \brief Remove range of iterators from the front
        size_t erase(iterator first, const_iterator last) {
            return data_.erase(const_iterator(first), last);
        }

        /// \brief Clear the front
        void clear() noexcept {
            data_.clear();
        }

        /// \brief Merge two fronts
        void merge(self_type &source) {
            insert(source.begin(), source.end());
        }

        /// \brief Merge and move fronts
        void merge(self_type &&source) {
            insert(source.begin(), source.end());
            source.clear();
        }

        /// \brief Swap the content of two fronts
        void swap(self_type &m) {
            m.data_.swap(data_);
            m.is_minimization_.swap(is_minimization_);
        }

    public /* pareto operations */:
        /// \brief Find points in a box
        const_iterator find_intersection(const point_type &min_corner, const point_type &max_corner) const {
            return data_.begin_intersection(min_corner, max_corner);
        }

        /// \brief Get points in a box
        std::vector<value_type> get_intersection(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<value_type> v;
            std::copy(find_intersection(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// \brief Find points dominated by p
        /// If p is in the front, it dominates no other point
        /// Otherwise, it dominates the whole intersection between p and nadir
        const_iterator find_dominated(const point_type &p) const {
            const bool p_is_in_the_front = find(p) != end();
            if (p_is_in_the_front) {
                return end();
            }

            point_type worst_point = worst();
            const bool p_dominates_worst = p.dominates(worst_point, is_minimization_);
            if (!p_dominates_worst) {
                return end();
            }

            return find_intersection(worst_point, p);
        }

        /// \brief Find points within a box (intersection minus borders)
        const_iterator find_within(const point_type &min_corner, const point_type &max_corner) const {
            return data_.begin_within(min_corner, max_corner);
        }

        /// \brief Get points within a box
        std::vector<value_type> get_within(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<value_type> v;
            std::copy(find_within(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// \brief Find points disjointed of a box (intersection - borders)
        const_iterator find_disjoint(const point_type &min_corner, const point_type &max_corner) const {
            return data_.begin_disjoint(min_corner, max_corner);
        }

        /// \brief Get points disjointed of a box
        std::vector<value_type> get_disjoint(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<value_type> v;
            std::copy(find_disjoint(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// \brief Find nearest point
        const_iterator find_nearest(const point_type &p) const {
            return data_.begin_nearest(p);
        }

        /// \brief Get nearest point
        std::vector<value_type> get_nearest(const point_type &p) const {
            std::vector<value_type> v;
            std::copy(find_nearest(p), end(), back_inserter(v));
            return v;
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
                        !mapped_type_custom_equality_operator(itself->second, it->second)) {
                        return it;
                    }
                }
            }
            return end();
        }

        /// \brief Get nearest point excluding itself
        std::vector<value_type> get_nearest_exclusive(const point_type &p) const {
            std::vector<value_type> v;
            std::copy(find_nearest(p), end(), back_inserter(v));
            return v;
        }

        /// \brief Find k nearest points
        const_iterator find_nearest(const point_type &p, size_t k) const {
            return data_.begin_nearest(p, k);
        }

        /// \brief Find k nearest points
        const_iterator find_nearest(std::initializer_list<number_type> p, size_t k) const {
            return data_.begin_nearest(point_type(p), k);
        }

        /// \brief Get k nearest points
        std::vector<value_type> get_nearest(const point_type &p, size_t k) const {
            std::vector<value_type> v;
            std::copy(find_nearest(p, k), end(), back_inserter(v));
            return v;
        }

        /// \brief Find k nearest points
        const_iterator find_nearest(const box_type &b, size_t k) const {
            return data_.begin_nearest(b, k);
        }

        /// \brief Get k nearest points
        std::vector<value_type> get_nearest(const box_type &b, size_t k) const {
            std::vector<value_type> v;
            std::copy(find_nearest(b, k), end(), back_inserter(v));
            return v;
        }

        /// \brief Get hypervolume with monte-carlo simulation
        /// This function uses monte-carlo simulation as getting the
        /// exact indicator is too costly.
        /// \param reference_point Reference for the hyper-volume
        /// \param sample_size Number of samples for the simulation
        /// \return Hypervolume of the pareto front
        number_type hypervolume(const point_type &reference_point, size_t sample_size) const {
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
                    std::uniform_real_distribution<number_type> d(m[j], reference_point[j]);
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

        /// \brief Get exact hypervolume
        /// Use the other hypervolume function if this takes too long.
        /// \param reference_point Reference point
        /// \return Hypervolume of this front
        number_type hypervolume(point_type reference_point) const {
            // reshape points
            std::vector<double> data;
            data.reserve(size() * dimensions());
            for (const auto&[k, v]: *this) {
                point<double, point_type::compile_dimensions, typename point_type::coordinate_system_t> inv = k;
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

        /// \brief Get hypervolume from the nadir point
        /// If there is no specific reference point, we use
        /// the nadir point as reference.
        /// \param reference_point Reference point
        /// \return Hypervolume of this front
        number_type hypervolume() const {
            return hypervolume(nadir());
        }

        /// \brief Coverage indicator
        /// \see http://www.optimization-online.org/DB_FILE/2018/10/6887.pdf
        double coverage(const front &rhs) const {
            size_t hits = 0;
            for (const auto&[k, v]: rhs) {
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
            double distances = 0.;
            for (const auto&[k, v]: *this) {
                distances += distance(k, reference.find_nearest(k)->first);
            }
            return distances / size();
        }

        /// \brief Standard deviation from the generational distance
        /// It measures the deformation of the Pareto set
        /// approximation according to a Pareto optimal solution set.
        double std_gd(const front &reference) const {
            double _gd = gd(reference);
            double std_dev = 0.;
            for (const auto&[k, v]: *this) {
                double dist = distance(k, reference.find_nearest(k)->first);
                std_dev += pow(dist - _gd, 2.);
            }
            return sqrt(std_dev) / size();
        }

        /// \brief Inverted generational distance
        double igd(const front &reference) const {
            return reference.gd(*this);
        }

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
            double distances = 0.;
            // for each element in the reference front
            for (const auto &item: reference_front) {
                auto min_it = std::min_element(begin(), end(), [&](const auto &a, const auto &b) {
                    return a.first.distance_to_dominated_box(item.first, is_minimization_) <
                            b.first.distance_to_dominated_box(item.first, is_minimization_);
                });
                distances += min_it->first.distance_to_dominated_box(item.first, is_minimization_);
            }
            return distances / reference_front.size();
        }

        /// \brief STD-IGD+ indicator
        double std_igd_plus(const front &reference_front) const {
            double _igd_plus = igd_plus(reference_front);
            double std_dev = 0.;
            for (const auto &item: reference_front) {
                auto min_it = std::min_element(begin(), end(), [&](const auto &a, const auto &b) {
                    return a.first.distance_to_dominated_box(item.first, is_minimization_) <
                            b.first.distance_to_dominated_box(item.first, is_minimization_);
                });
                auto distance = min_it->first.distance_to_dominated_box(item.first, is_minimization_);
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
                min_distance = std::min(min_distance, distance(ita->first, itb->first));
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
                for (auto itb = find_nearest(ita->first, k + 1); itb != end(); ++itb) {
                    nearest_avg += distance(ita->first, itb->first);
                }
                sum += nearest_avg / k;
            }
            return sum / size();
        }

        /// \brief Crowding distance of an element
        double crowding_distance(const_iterator element, point_type worst_point, point_type ideal_point) const {
            double sum = 0.0;
            // for each dimension
            for (size_t i = 0; i < dimensions(); ++i) {
                // find the closest elements in that dimension
                auto p1 = element->first;
                auto p2 = element->first;
                p1[i] = worst_point[i];
                p2[i] = ideal_point[i];
                box_type s(p1, p2);
                auto itb = data_.begin_nearest(s, 3);
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
        ///  many-objective optimization." Information Sciences 298 (2015): 288-314.
        /// Page 299
        /// Table 2
        number_type direct_conflict(const size_t a, const size_t b) const {
            number_type ideal_a = ideal(a);
            number_type ideal_b = ideal(b);
            number_type c_ab = 0;
            for (const auto &[x_i, value]: *this) {
                // distance to ideal
                const number_type x_line_ia = is_minimization(a) ? x_i[a] - ideal_a : ideal_a - x_i[a];
                const number_type x_line_ib = is_minimization(b) ? x_i[b] - ideal_b : ideal_b - x_i[b];
                c_ab += std::max(x_line_ia, x_line_ib) - std::min(x_line_ia, x_line_ib);
            }
            return c_ab;
        }

        /// \brief Normalized direct between two objectives ([0,1])
        [[nodiscard]] double normalized_direct_conflict(const size_t a, const size_t b) const {
            // max value in each term is max_a-min_a or max_b-min_b
            number_type range_a = is_minimization(a) ? worst(a) - ideal(a) : ideal(a) - worst(a);
            number_type range_b = is_minimization(b) ? worst(b) - ideal(b) : ideal(b) - worst(b);
            return static_cast<double>(direct_conflict(a, b)) / (std::max(range_a, range_b) * size());
        }

        /// \brief Maxmin conflict between objectives
        /// Use this conflict measure when:
        /// - Objective importance is proportional to its range of values
        /// - Objectives are in comparable units
        /// This conflict measure is insensitive to linear normalizations.
        /// \see Freitas, Alan RR, Peter J. Fleming, and Frederico G. Guimarães.
        /// "Aggregation trees for visualization and dimension reduction in
        ///  many-objective optimization." Information Sciences 298 (2015): 288-314.
        /// Page 299
        /// Table 2
        [[nodiscard]] double maxmin_conflict(const size_t a, const size_t b) const {
            number_type worst_a = worst(a);
            number_type worst_b = worst(b);
            number_type ideal_a = ideal(a);
            number_type ideal_b = ideal(b);
            number_type range_a = is_minimization(a) ? worst_a - ideal_a : ideal_a - worst_a;
            number_type range_b = is_minimization(b) ? worst_b - ideal_b : ideal_b - worst_b;
            double c_ab = 0;
            for (const auto &[x_i, value]: *this) {
                const number_type x_line_ia =
                        static_cast<double>(is_minimization(a) ? x_i[a] - ideal_a : ideal_a - x_i[a]) / range_a;
                const number_type x_line_ib =
                        static_cast<double>(is_minimization(b) ? x_i[b] - ideal_b : ideal_b - x_i[b]) / range_b;
                c_ab += std::max(x_line_ia, x_line_ib) - std::min(x_line_ia, x_line_ib);
            }
            return c_ab;
        }

        /// \brief Normalized maxmin conflict between two objectives ([0,1])
        [[nodiscard]] double normalized_maxmin_conflict(const size_t a, const size_t b) const {
            return maxmin_conflict(a, b) / size();
        }

        /// \brief Non-parametric conflict between objectives
        /// This is the most general case of conflict
        /// Use this conflict measure when:
        /// - Objective importance is not comparable
        /// - Objectives are in any unit
        /// - When other measures are not appropriate
        /// - When you don't know what to use
        /// This conflict measure is insensitive to non-disruptive normalizations.
        /// \see Freitas, Alan RR, Peter J. Fleming, and Frederico G. Guimarães.
        /// "Aggregation trees for visualization and dimension reduction in
        ///  many-objective optimization." Information Sciences 298 (2015): 288-314.
        /// Page 299
        /// Table 2
        [[nodiscard]] double conflict(const size_t a, const size_t b) const {
            // get sorted values in objectives a and b
            std::vector<number_type> x_a;
            std::vector<number_type> x_b;
            x_a.reserve(size());
            x_b.reserve(size());
            for (const auto&[key, value]: *this) {
                x_a.emplace_back(key[a]);
                x_b.emplace_back(key[b]);
            }
            if (is_minimization(a)) {
                std::sort(x_a.begin(), x_a.end(), std::less<number_type>());
            } else {
                std::sort(x_a.begin(), x_a.end(), std::greater<number_type>());
            }
            if (is_minimization(b)) {
                std::sort(x_b.begin(), x_b.end(), std::less<number_type>());
            } else {
                std::sort(x_b.begin(), x_b.end(), std::greater<number_type>());
            }
            // get ranking of these values
            std::map<number_type, size_t> rankings_a;
            std::map<number_type, size_t> rankings_b;
            for (size_t i = 0; i < x_a.size(); ++i) {
                rankings_a[x_a[i]] = i + 1;
                rankings_b[x_b[i]] = i + 1;
            }
            // calculate conflict
            size_t c_ab = 0;
            for (const auto &[x_i, value]: *this) {
                const size_t x_line_ia = rankings_a[x_i[a]];
                const size_t x_line_ib = rankings_b[x_i[b]];
                c_ab += std::max(x_line_ia, x_line_ib) - std::min(x_line_ia, x_line_ib);
            }
            return static_cast<double>(c_ab);
        }

        /// \brief Normalized conflict between two objectives
        /// This function returns non-paremetric conflict and is the default
        /// function when we don't know which type of conflict to use
        [[nodiscard]] double normalized_conflict(const size_t a, const size_t b) const {
            double denominator = 0.;
            auto n = static_cast<double>(size());
            for (size_t i = 1; i <= size(); ++i) {
                denominator += abs(2. * i - n - 1);
            }
            return static_cast<double>(conflict(a, b)) / denominator;
        }

        /// \brief \brief Check if this front weakly dominates a point
        /// A front a weakly dominates a solution p if it has at least
        /// one solution that dominates p.
        /// Some of these lists of conditions could be checked with a single
        /// predicate list, but we do not do it this way because boost_tree
        /// is very inefficient with predicate lists.
        /// \see http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool dominates(const point_type &p) const {
            // trivial case: front is empty
            if (empty()) {
                return false;
            }

            // trivial case: p is not behind ideal
            point_type ideal_point = ideal();
            const bool p_is_not_behind_ideal = !ideal_point.dominates(p, is_minimization_);
            if (p_is_not_behind_ideal) {
                return false;
            }

            // trivial case: p is in the front
            if (find(p) != end()) {
                return false;
            }

            // general case (removing trivial case 1)
            // points intersecting(ideal,p)
            const_iterator first_element_that_dominates = data_.begin_intersection(ideal_point, p);
            return first_element_that_dominates != end();
        }

        /// \brief \brief Check if this front strongly dominates a point
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
            const bool ideal_strongly_dominates_p = ideal_point.strongly_dominates(p, is_minimization_);
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
            number_type epsilon = std::is_floating_point_v<number_type> ? std::numeric_limits<number_type>::epsilon()
                                                                        : static_cast<number_type>(1);
            auto p_line = p;
            for (size_t i = 0; i < p.dimensions(); ++i) {
                if (is_minimization_[i]) {
                    p_line[i] -= epsilon;
                } else {
                    p_line[i] += epsilon;
                }
            }
            const_iterator it = data_.begin_intersection(ideal_point, p_line);
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
            const_iterator it = data_.begin_intersection(worst(), p);
            // if there's someone p dominates, then it's partially dominated by p
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

        /// \brief \brief Check if this front non-dominates the point
        /// A solution a weakly dominates a solution b if a is better
        /// than b in at least one objective and is as good as b in
        /// all other objectives.
        /// \see http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool non_dominates(const point_type &p) const {
            // trivial case: front is empty
            if (empty()) {
                return true;
            }

            // Ensure pareto does not dominate p
            // Ensure p does not dominate anyone in the pareto
            return !dominates(p) && !is_partially_dominated_by(p);
        }


        /// \brief \brief Check if this front dominates another front
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
            for (auto&[k, v]: P2) {
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

            for (auto&[k, v]: p) {
                if (!strongly_dominates(k)) {
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

            for (auto&[k, v]: p) {
                if (!non_dominates(k)) {
                    return false;
                }
            }
            return true;
        }

        /// \brief Check if this front is is_partially_dominated_by another front
        bool is_partially_dominated_by(const front &p) const {
            // trivial case: front is empty
            if (empty()) {
                return true;
            }

            for (auto&[k, v]: p) {
                if (is_partially_dominated_by(k)) {
                    return true;
                }
            }
            return false;
        }

        /// \brief Check if this front is is_partially_dominated_by another front
        bool is_completely_dominated_by(const front &p) const {
            // trivial case: front is empty
            if (empty()) {
                return true;
            }

            for (auto&[k, v]: *this) {
                if (!p.dominates(k)) {
                    return false;
                }
            }
            return true;
        }

        /// \brief Ideal point in the front
        point_type ideal() const {
            point_type r(dimensions());
            for (size_t i = 0; i < r.dimensions(); ++i) {
                r[i] = ideal(i);
            }
            return r;
        }

        /// \brief Ideal value in a front dimension
        number_type ideal(size_t d) const {
            return is_minimization(d) ? data_.min_value(d) : data_.max_value(d);
        }

        /// \brief Get iterator to element with best value in dimension d
        const_iterator dimension_ideal(size_t d) const {
            return is_minimization(d) ? data_.min_element(d) : data_.max_element(d);
        }

        /// \brief The nadir point is the worst point among the
        /// non-dominated points. There is a difference
        /// between the nadir point and the worst point
        /// for archives.
        point_type nadir() const {
            point_type r(dimensions());
            for (size_t i = 0; i < r.dimensions(); ++i) {
                r[i] = nadir(i);
            }
            return r;
        }

        /// \brief Nadir value in dimension d
        number_type nadir(size_t d) const {
            return is_minimization(d) ? data_.max_value(d) : data_.min_value(d);
        }

        /// \brief Element with nadir value in front dimension d
        const_iterator dimension_nadir(size_t d) const {
            return is_minimization(d) ? data_.max_element(d) : data_.min_element(d);
        }

        /// \brief Worst point in the front
        /// Worst is the same as nadir for fronts.
        /// In archives, worst != nadir because there are
        /// many fronts.
        point_type worst() const {
            return nadir();
        }

        /// \brief Worst value in front dimension d
        number_type worst(size_t d) const {
            return nadir(d);
        }

        /// \brief Iterator to element with worst value in front dimension d
        const_iterator dimension_worst(size_t d) const {
            return dimension_nadir(d);
        }

        /// \brief Find element by point
        const_iterator find(const key_type &k) const {
            return data_.find(k);
        }

        /// \brief Find element by point
        iterator find(const key_type &k) {
            return data_.find(k);
        }

        /// \brief Check if front contains the point k
        bool contains(const key_type &k) const {
            return find(k) != end();
        }

        /// \brief Check if front passes the variants that define a front
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

        /// \brief Stream front properties to an ostream
        friend std::ostream &operator<<(std::ostream &os, const front &pf) {
            os << "Pareto front (" << pf.size() << " elements - {";
            for (size_t i = 0; i < pf.is_minimization_.size() - 1; ++i) {
                os << (pf.is_minimization_[i] ? "minimization" : "maximization") << ", ";
            }
            os << (pf.is_minimization_.back() ? "minimization" : "maximization") << "})";
            return os;
        }

    private /* functions */:
        /// \brief Clear solutions are dominated by p
        /// Pareto-optimal front is the set F consisting of
        /// all non-dominated solutions x in the whole
        /// search space. No solution can dominate another
        /// solution. Note that this means two solutions
        /// might have the same values though.
        size_t clear_dominated(const point_type &p) {
            if (!empty()) {
                const_iterator it = find_dominated(p);
                return erase(it, const_iterator(end()));
            }
            return 0;
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
                boost::geometry::model::box<point_type> boost_box(b.first(), b.second());
                return boost::geometry::distance(p, boost_box);
            } else {
#endif
            double dist = 0.0;
            for (size_t i = 0; i < dimensions(); ++i) {
                double di = std::max(std::max(b.first()[i] - p[i], p[i] - b.second()[i]), 0.0);
                dist += di * di;
            }
            return sqrt(dist);
#ifdef BUILD_BOOST_TREE
            }
#endif
        }

        static std::mt19937 &generator() {
            static std::mt19937 g(static_cast<unsigned int>(static_cast<unsigned int>(std::random_device()()) |
                                                            static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count())));
            return g;
        }


        void normalize_corners(point_type &min_corner, point_type &max_corner) const {
            for (size_t i = 0; i < min_corner.dimensions(); ++i) {
                if (min_corner[i] > max_corner[i]) {
                    std::swap(min_corner[i], max_corner[i]);
                }
            }
        }

        /// If the dimension is being set at runtime, this sets the
        /// dimension in case we don't already know it.
        inline void maybe_adjust_dimensions(const value_type &v) {
            if constexpr (number_of_compile_dimensions == 0) {
                if (empty() && is_minimization_.size() == 1) {
                    maybe_resize(is_minimization_, v.first.dimensions());
                    std::fill(is_minimization_.begin() + 1, is_minimization_.end(), *is_minimization_.begin());
                }
            }
        }

        /// If the dimension is being set at runtime, this sets the
        /// dimension in case we don't already know it.
        inline void maybe_adjust_dimensions([[maybe_unused]] size_t s) {
            if constexpr (number_of_compile_dimensions == 0) {
                if (empty() && is_minimization_.size() == 1) {
                    maybe_resize(is_minimization_, s);
                    std::fill(is_minimization_.begin() + 1, is_minimization_.end(), *is_minimization_.begin());
                }
            }
        }

        inline void maybe_adjust_dimensions(const point_type &position) {
            if constexpr (number_of_compile_dimensions == 0) {
                if (empty() && is_minimization_.size() == 1) {
                    maybe_resize(is_minimization_, position.dimensions());
                    std::fill(is_minimization_.begin() + 1, is_minimization_.end(), *is_minimization_.begin());
                }
            }
        }

    private:
        /// \brief r-tree representing the pareto points
        internal_type data_;

        /// \brief Whether each dimension is minimization or maximization
        /// We use uint8_t because bool to avoid the array specialization
        internal_minimization_type is_minimization_;
    };

}

#endif //PARETO_FRONTS_PARETO_FRONT_RTREE_H
