#ifndef PARETO_FRONTS_PARETO_FRONT_RTREE_H
#define PARETO_FRONTS_PARETO_FRONT_RTREE_H

#include <random>
#include <map>
#include <initializer_list>
#include <ostream>
#include <thread>
#include <mutex>
#include <optional>

#ifdef BUILD_FRONTS_WITH_TRASE
#include <trase.hpp>
#endif

#include <pareto/hv-2.0rc2/hv.h>

#include <pareto/common.h>
#include <pareto/point.h>
#include <pareto/query_box.h>
#include <pareto/tree/vector_tree.h>
#include <pareto/tree/quad_tree.h>
#include <pareto/tree/kd_tree.h>

#ifdef BUILD_BOOST_TREE
#include <pareto/tree/boost_tree.h>
#endif

#include <pareto/tree/r_tree.h>
#include <pareto/tree/r_star_tree.h>

namespace pareto {
    struct vector_tree_tag {};
    struct quad_tree_tag {};
    struct kd_tree_tag {};
    struct boost_tree_tag {};
    struct r_tree_tag {};
    struct r_star_tree_tag {};

#ifdef BUILD_BOOST_TREE
    template <class nt, size_t ncd, class mt>
  using default_type_for_boost_tree_tag = boost_tree<nt,ncd,mt>;
#else
    template <class nt, size_t ncd, class mt>
    using default_type_for_boost_tree_tag = r_tree<nt,ncd,mt>;
#endif

    constexpr bool minimization = true;
    constexpr bool min = true;
    constexpr bool maximization = false;
    constexpr bool max = false;

    template <size_t NUMBER_OF_DIMENSIONS>
    using default_tag = std::conditional<NUMBER_OF_DIMENSIONS == 1, vector_tree_tag, kd_tree_tag>;

    template<typename NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, typename ELEMENT_TYPE, typename TAG>
    class archive;

    template<typename NUMBER_TYPE = double, size_t NUMBER_OF_DIMENSIONS = 2, typename ELEMENT_TYPE = unsigned, typename TAG = default_tag<NUMBER_OF_DIMENSIONS>>
    class front {
      public:
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

        using internal_type =
        std::conditional_t<
            std::is_same_v<TAG, vector_tree_tag>,
            vector_tree<number_type, number_of_compile_dimensions, mapped_type>,
            std::conditional_t<
                std::is_same_v<TAG, quad_tree_tag>,
                quad_tree<number_type, number_of_compile_dimensions, mapped_type>,
                std::conditional_t<
                    std::is_same_v<TAG, kd_tree_tag>,
                    kd_tree<number_type, number_of_compile_dimensions, mapped_type>,
                    std::conditional_t<
                        std::is_same_v<TAG, boost_tree_tag>,
                        std::conditional_t<number_of_compile_dimensions == 0,
                            r_tree<number_type, number_of_compile_dimensions, mapped_type>,
                            default_type_for_boost_tree_tag<number_type, number_of_compile_dimensions, mapped_type>
                        >,
                        std::conditional_t<
                            std::is_same_v<TAG, r_tree_tag>,
                            r_tree<number_type, number_of_compile_dimensions, mapped_type>,
                            // r*-tree is the only option left
                            r_star_tree<number_type, number_of_compile_dimensions, mapped_type>
                        >
                    >
                >
            >
        >;


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
        /// \brief Create an empty pareto set / minimization
        front()
            : front(true) {}

        /// \brief Create an empty pareto set and determine whether it is minimization
        /// Create an array indicating minimization/maximization for each dimension
        /// or, if number_of_compile_dimensions == 0, create a vector of size 1 with direction
        explicit front(bool is_minimization)
            : front(std::vector<uint8_t>(std::max(number_of_compile_dimensions,size_t(1)), is_minimization)) {}

        /// \brief Create an empty pareto set and determine whether each dimension is minimization
        explicit front(std::initializer_list<bool> is_minimization) {
            if constexpr (number_of_compile_dimensions > 0) {
                if (number_of_compile_dimensions != is_minimization_.size()) {
                    throw std::invalid_argument("Number of minimization directions should match the dimension defined for the object");
                }
            }
            maybe_resize(is_minimization_, is_minimization.size());
            std::copy(is_minimization.begin(), is_minimization.end(), is_minimization_.begin());
        }

        /// \brief Create an empty pareto set and determine whether each dimension is minimization
        explicit front(const std::vector<uint8_t>& is_minimization) {
            if constexpr (number_of_compile_dimensions > 0) {
                if (number_of_compile_dimensions != is_minimization_.size()) {
                    throw std::invalid_argument("Number of minimization directions should match the dimension");
                }
            }
            maybe_resize(is_minimization_, is_minimization.size());
            std::copy(is_minimization.begin(), is_minimization.end(), is_minimization_.begin());
        }

        /// \brief Create an empty pareto set and determine whether each dimension is minimization
        explicit front(const std::array<uint8_t, number_of_compile_dimensions>& is_minimization) {
            if constexpr (number_of_compile_dimensions > 0) {
                if (number_of_compile_dimensions != is_minimization_.size()) {
                    throw std::invalid_argument("Number of minimization directions should match the dimension");
                }
            }
            maybe_resize(is_minimization_, is_minimization.size());
            std::copy(is_minimization.begin(), is_minimization.end(), is_minimization_.begin());
        }

        /// \brief Private constructor using an external allocator
        /// Only archives should use this constructor to avoid one allocator
        /// per front because our fast allocator is not stateless.
        front(const internal_minimization_type& is_minimization, std::shared_ptr<node_allocator_type>& external_allocator)
            : data_(external_allocator) {
            if constexpr (number_of_compile_dimensions > 0) {
                if (number_of_compile_dimensions != is_minimization_.size()) {
                    throw std::invalid_argument("Number of minimization directions should match the dimension");
                }
            }
            maybe_resize(is_minimization_, is_minimization.size());
            std::copy(is_minimization.begin(), is_minimization.end(), is_minimization_.begin());
        }

        /// \brief Copy constructor
        front(const front &m) {
            data_ = m.data_;
            is_minimization_ = m.is_minimization_;
        }

        /// \brief Assignment
        front & operator=(const front &rhs) {
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
        front & operator=(front &&rhs) {
            data_ = std::move(rhs.data_);
            is_minimization_ = std::move(rhs.is_minimization_);
            return *this;
        }

        /// \brief Create a pareto set from a list of value pairs
        /// Each pair has a point and a value
        explicit front(std::initializer_list<value_type> il)
            : front(il.begin(), il.end()) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether dimensions are minimization
        front(std::initializer_list<value_type> il, bool is_minimization)
            : front(il.begin(), il.end(), is_minimization) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        front(std::initializer_list<value_type> il, std::vector<uint8_t> is_minimization)
            : front(il.begin(), il.end(), is_minimization) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        front(std::initializer_list<value_type> il, std::initializer_list<bool> is_minimization)
            : front(il.begin(), il.end(), init_list_to_vector(is_minimization)) {}

        /// \brief Create a pareto set from a list of value pairs
        front(const std::vector<value_type>& v)
            : front(v.begin(), v.end()) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether dimensions are minimization
        front(const std::vector<value_type>& v, bool is_minimization)
            : front(v.begin(), v.end(), is_minimization) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        front(const std::vector<value_type>& v, std::vector<uint8_t> is_minimization)
            : front(v.begin(), v.end(), is_minimization) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        front(const std::vector<value_type>& v, std::initializer_list<bool> is_minimization)
            : front(v.begin(), v.end(), init_list_to_vector(is_minimization)) {}

        /// \brief Create a pareto set from a list of value pairs
        template<class InputIterator>
        front(InputIterator first, InputIterator last)
            : front(first, last, true) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether it is minimization
        template<class InputIterator>
        front(InputIterator first, InputIterator last, bool is_minimization)
            : front(first, last, std::vector<uint8_t>(std::max(number_of_compile_dimensions,size_t(1)),is_minimization)) {}

        /// \brief Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        template<class InputIterator>
        front(InputIterator first, InputIterator last, std::vector<uint8_t> is_minimization)
            : data_(first, last) {
            if constexpr (number_of_compile_dimensions != 0) {
                if (is_minimization.size() != number_of_compile_dimensions) {
                    throw std::invalid_argument("The size specified at compile time does not match the number of minimization directions");
                }
            }
            maybe_resize(is_minimization_, is_minimization.size());
            std::copy(is_minimization.begin(), is_minimization.end(), is_minimization_.begin());
            clear_dominated();
        }

      public /* iterators */:
        const_iterator begin() const noexcept {
            return data_.begin();
        }

        const_iterator end() const noexcept {
            return data_.end();
        }

        iterator begin() noexcept {
            return data_.begin();
        }

        iterator end() noexcept {
            return data_.end();
        }

        std::reverse_iterator<const_iterator> rbegin() const noexcept {
            return std::reverse_iterator(data_.end());
        }

        std::reverse_iterator<const_iterator> rend() const noexcept {
            return std::reverse_iterator(data_.begin());
        }

        std::reverse_iterator<iterator> rbegin() noexcept {
            return std::reverse_iterator(data_.end());
        }

        std::reverse_iterator<iterator> rend() noexcept {
            return std::reverse_iterator(data_.begin());
        }

      public /* capacity */:
        bool empty() const noexcept {
            return data_.empty();
        }

        size_type size() const noexcept {
            return data_.size();
        }

        size_type dimensions() const noexcept {
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

        size_type is_minimization() const noexcept {
            return std::all_of(is_minimization_.begin(), is_minimization_.end(), [](auto i){return i == uint8_t(1);});
        }

        size_type is_maximization() const noexcept {
            return std::all_of(is_minimization_.begin(), is_minimization_.end(), [](auto i){return i == uint8_t(0);});
        }

        size_type is_minimization(size_t dimension) const noexcept {
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

        size_type is_maximization(size_t dimension) const noexcept {
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
        mapped_type& operator[](const key_type& k) {
            auto it = find(k);
            if (it != end()) {
                // return reference to mapped type in the tree
                return it->second;
            } else {
                // include element in the tree with a default
                // mapped type
                auto [it2, ok] = insert(std::make_pair(k, mapped_type()));
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

        mapped_type& operator[](key_type&& k) {
            auto it = find(k);
            if (it != end()) {
                // return reference to mapped type in the tree
                return it->second;
            } else {
                // include element in the tree with a default
                // mapped type
                auto [it2, ok] = insert(std::make_pair(std::move(k), mapped_type()));
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

        template <typename... Targs>
        mapped_type& operator()(const number_type& k, const Targs&... ks) {
            // constexpr size_t d = sizeof...(Targs) + 1;
            point_type p(k, ks...);
            return operator[](p);
        }

        mapped_type& at(const key_type& k){
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("front::at:  key not found");
            }
        }

        const mapped_type& at(const key_type& k) const {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("front::at:  key not found");
            }
        }

      public /* relational operators */:
        bool operator==(const front &rhs) const {
            return is_minimization_ == rhs.is_minimization_ &&
                   std::equal(data_.begin(), data_.end(), rhs.data_.begin(), rhs.data_.end(), [](const auto& a, const auto& b) {
                     return a.first == b.first && mapped_type_custom_equality_operator(a.second,b.second);
                   });
        }

        bool operator!=(const front &rhs) const {
            return is_minimization_ != rhs.is_minimization_ || !std::equal(data_.begin(), data_.end(), rhs.data_.begin(), rhs.data_.end(), [](const auto& a, const auto& b) {
              return a.first == b.first && mapped_type_custom_equality_operator(a.second,b.second);
            });;
        }

      public /* modifiers */:
        /// Emplace becomes insert becomes the rtree does not have
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

        /// Erase element pointed by iterator from the front
        /// \warning The modification of the rtree may invalidate the iterators.
        size_t erase(const_iterator position) {
            auto it = find(position->first);
            return data_.erase(it);
        }

        /// Erase element pointed by iterator from the front
        /// \warning The modification of the rtree may invalidate the iterators.
        size_t erase(iterator position) {
            auto it = find(position->first);
            return data_.erase(it);
        }

        /// Erase element from the front
        /// \param v Point
        size_type erase(const key_type &point) {
            auto it = find(point);
            if (it != end()) {
                return data_.erase(it);
            } else {
                return 0;
            }
        }

        /// Remove range of iterators from the front
        size_t erase(const_iterator first, const_iterator last) {
            return data_.erase(first, last);
        }

        /// Clear the front
        void clear() noexcept {
            data_.clear();
        }

        /// Merge two fronts
        void merge(self_type &source) {
            insert(source.begin(), source.end());
        }

        /// Merge and move fronts
        void merge(self_type &&source) {
            insert(source.begin(), source.end());
            source.clear();
        }

        /// Swap the content of two fronts
        void swap(self_type &m) {
            m.data_.swap(data_);
            m.is_minimization_.swap(is_minimization_);
        }

      public /* pareto operations */:
        /// Find points in a box
        const_iterator find_intersection(const point_type& min_corner, const point_type& max_corner) const {
            return data_.begin_intersection(min_corner, max_corner);
        }

        /// Get points in a box
        std::vector<value_type> get_intersection(const point_type& min_corner, const point_type& max_corner) const {
            std::vector<value_type> v;
            std::copy(find_intersection(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// Find points within a box (intersection minus borders)
        const_iterator find_within(const point_type& min_corner, const point_type& max_corner) const {
            return data_.begin_within(min_corner, max_corner);
        }

        /// Get points within a box
        std::vector<value_type> get_within(const point_type& min_corner, const point_type& max_corner) const {
            std::vector<value_type> v;
            std::copy(find_within(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// Find points disjointed of a box (intersection - borders)
        const_iterator find_disjoint(const point_type& min_corner, const point_type& max_corner) const {
            return data_.begin_disjoint(min_corner, max_corner);
        }

        /// Get points disjointed of a box
        std::vector<value_type> get_disjoint(const point_type& min_corner, const point_type& max_corner) const {
            std::vector<value_type> v;
            std::copy(find_disjoint(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// Find nearest point
        const_iterator find_nearest(const point_type& p) const {
            return data_.begin_nearest(p);
        }

        /// Get nearest point
        std::vector<value_type> get_nearest(const point_type& p) const {
            std::vector<value_type> v;
            std::copy(find_nearest(p), end(), back_inserter(v));
            return v;
        }

        /// Find nearest point excluding itself
        const_iterator find_nearest_exclusive(const point_type& p) const {
            auto itself = find_nearest(p);
            if (itself->first != p) {
                return itself;
            }
            for (size_t i = 1; i <= size(); ++i) {
                for (auto it = find_nearest(p,i); it != end(); ++it) {
                    if (itself->first != it->first || !mapped_type_custom_equality_operator(itself->second,it->second)) {
                        return it;
                    }
                }
            }
            return end();
        }

        /// Get nearest point excluding itself
        std::vector<value_type> get_nearest_exclusive(const point_type& p) const {
            std::vector<value_type> v;
            std::copy(find_nearest(p), end(), back_inserter(v));
            return v;
        }

        /// Find k nearest points
        const_iterator find_nearest(const point_type& p, size_t k) const {
            return data_.begin_nearest(p, k);
        }

        const_iterator find_nearest(std::initializer_list<number_type> p, size_t k) const {
            return data_.begin_nearest(point_type(p), k);
        }

        /// Get k nearest points
        std::vector<value_type> get_nearest(const point_type& p, size_t k) const {
            std::vector<value_type> v;
            std::copy(find_nearest(p, k), end(), back_inserter(v));
            return v;
        }

        /// Find k nearest points
        const_iterator find_nearest(const box_type& b, size_t k) const {
            return data_.begin_nearest(b, k);
        }

        /// Get k nearest points
        std::vector<value_type> get_nearest(const box_type& b, size_t k) const {
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
        number_type hypervolume(const point_type& reference_point, size_t sample_size) const {
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

            return hv_upper_limit * hit / (hit + miss);
        }

        /// \brief Get exact hypervolume
        /// Use the other hypervolume function if this takes too long.
        /// \param reference_point Reference point
        /// \return Hypervolume of this front
        number_type hypervolume(point_type reference_point) const {
            // reshape points
            std::vector<double> data;
            data.reserve(size() * dimensions());
            for (const auto& [k,v]: *this) {
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

        /// Coverage indicator
        /// \see http://www.optimization-online.org/DB_FILE/2018/10/6887.pdf
        double coverage(const front & rhs) const {
            size_t hits = 0;
            for (const auto& [k,v]: rhs) {
                hits += dominates(k);
            }
            return static_cast<double>(hits)/rhs.size();
        }

        /// Ratio of coverage indicators
        double coverage_ratio(const front & rhs) const {
            return coverage(rhs) / rhs.coverage(*this);
        }

        /// Generational distance
        double gd(const front & reference) const {
            double distances = 0.;
            for (const auto& [k,v]: *this) {
                distances += distance(k, reference.find_nearest(k)->first);
            }
            return distances/size();
        }

        /// Standard deviation from the generational distance
        /// It measures the deformation of the Pareto set
        /// approximation according to a Pareto optimal solution set.
        double std_gd(const front & reference) const {
            double _gd = gd(reference);
            double std_dev = 0.;
            for (const auto& [k,v]: *this) {
                double dist = distance(k, reference.find_nearest(k)->first);
                std_dev += pow(dist - _gd, 2.);
            }
            return sqrt(std_dev)/size();
        }

        /// Inverted generational distance
        double igd(const front & reference) const {
            return reference.gd(*this);
        }

        /// Standard deviation from the inverted generational distance
        double std_igd(const front & reference) const {
            return reference.std_gd(*this);
        }

        double hausdorff(const front & reference) const {
            return std::max(gd(reference), igd(reference));
        }

        double igd_plus(const front & reference_front) const {
            double distances = 0.;
            // for each element in the reference front
            for (const auto& item: reference_front) {
                auto min_it = std::min_element(begin(), end(), [&](const auto& a, const auto &b) {
                  return a.first.distance_to_dominated_box(item.first, is_minimization_) < b.first.distance_to_dominated_box(item.first, is_minimization_); });
                distances += min_it->first.distance_to_dominated_box(item.first, is_minimization_);
            }
            return distances/reference_front.size();
        }

        double std_igd_plus(const front & reference_front) const {
            double _igd_plus = igd_plus(reference_front);
            double std_dev = 0.;
            for (const auto& item: reference_front) {
                auto min_it = std::min_element(begin(), end(), [&](const auto& a, const auto &b) {
                  return a.first.distance_to_dominated_box(item.first, is_minimization_) < b.first.distance_to_dominated_box(item.first, is_minimization_); });
                auto distance = min_it->first.distance_to_dominated_box(item.first, is_minimization_);
                std_dev += pow(distance - _igd_plus, 2.);
            }
            return sqrt(std_dev)/size();
        }

        /// Uniformity metric
        /// This is the minimal distance between two points of the Pareto front
        /// approximation. This measure is straightforward to compute and easy
        /// to understand. However, it does not really provide pertinent
        /// information on the repartition of the points along the Pareto
        /// front approximation.
        double uniformity() {
            if (size() < 2) {
                return std::numeric_limits<double>::infinity();
            }
            auto ita = begin();
            auto itb = find_nearest_exclusive(ita->first);
            double min_distance = distance(ita->first,itb->first);
            for (; ita != end(); ++ita) {
                itb = find_nearest_exclusive(ita->first);
                min_distance = std::min(min_distance, distance(ita->first,itb->first));
            }
            return min_distance;
        }

        double average_distance() {
            double sum = 0.0;
            for (auto ita = begin(); ita != end(); ++ita) {
                auto itb = ita;
                itb++;
                for (; itb != end(); ++itb) {
                    sum += distance(ita->first,itb->first);
                }
            }
            return sum / (((size()-1) * (size())) / 2);
        }

        double average_nearest_distance(size_t k = 5) {
            double sum = 0.0;
            for (auto ita = begin(); ita != end(); ++ita) {
                double nearest_avg = 0.0;
                for (auto itb = find_nearest(ita->first, k+1); itb != end(); ++itb) {
                    nearest_avg += distance(ita->first,itb->first);
                }
                sum += nearest_avg / k;
            }
            return sum / size();
        }

        double crowding_distance(const_iterator element, point_type worst_point, point_type ideal_point) {
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

        double crowding_distance(const_iterator element) {
            return crowding_distance(element, worst(), ideal());
        }

        double crowding_distance(const point_type& point) {
            auto element = find(point);
            if (element != end()) {
                return crowding_distance(element, worst(), ideal());
            } else {
                auto nearest_element = find_nearest(point);
                return crowding_distance(nearest_element, worst(), ideal());
            }
        }

        double average_crowding_distance() {
            double sum = 0.0;
            const point_type worst_point = worst();
            const point_type ideal_point = ideal();
            // for each point
            for (auto ita = begin(); ita != end(); ++ita) {
                sum += crowding_distance(ita, worst_point, ideal_point);
            }
            return sum / size();
        }

        /// Direct conflict between objectives
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

        double normalized_direct_conflict(const size_t a, const size_t b) const {
            // max value in each term is max_a-min_a or max_b-min_b
            number_type range_a = is_minimization(a) ? worst(a) - ideal(a) : ideal(a) - worst(a);
            number_type range_b = is_minimization(b) ? worst(b) - ideal(b) : ideal(b) - worst(b);
            return static_cast<double>(direct_conflict(a,b))/(std::max(range_a,range_b)*size());
        }

        /// Maxmin conflict between objectives
        /// Use this conflict measure when:
        /// - Objective importance is proportional to its range of values
        /// - Objectives are in comparable units
        /// This conflict measure is insensitive to linear normalizations.
        /// \see Freitas, Alan RR, Peter J. Fleming, and Frederico G. Guimarães.
        /// "Aggregation trees for visualization and dimension reduction in
        ///  many-objective optimization." Information Sciences 298 (2015): 288-314.
        /// Page 299
        /// Table 2
        double maxmin_conflict(const size_t a, const size_t b) const {
            number_type worst_a = worst(a);
            number_type worst_b = worst(b);
            number_type ideal_a = ideal(a);
            number_type ideal_b = ideal(b);
            number_type range_a = is_minimization(a) ? worst_a - ideal_a : ideal_a - worst_a;
            number_type range_b = is_minimization(b) ? worst_b - ideal_b : ideal_b - worst_b;
            double c_ab = 0;
            for (const auto &[x_i, value]: *this) {
                const number_type x_line_ia = static_cast<double>(is_minimization(a) ? x_i[a] - ideal_a : ideal_a - x_i[a])/range_a;
                const number_type x_line_ib = static_cast<double>(is_minimization(b) ? x_i[b] - ideal_b : ideal_b - x_i[b])/range_b;
                c_ab += std::max(x_line_ia,x_line_ib) - std::min(x_line_ia,x_line_ib);
            }
            return c_ab;
        }

        double normalized_maxmin_conflict(const size_t a, const size_t b) const {
            return maxmin_conflict(a,b)/size();
        }

        /// Non-parametric conflict between objectives
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
        double conflict(const size_t a, const size_t b) const {
            // get sorted values in objectives a and b
            std::vector<number_type> x_a;
            std::vector<number_type> x_b;
            x_a.reserve(size());
            x_b.reserve(size());
            for (const auto& [key,value]: *this) {
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
                c_ab += std::max(x_line_ia,x_line_ib) - std::min(x_line_ia,x_line_ib);
            }
            return static_cast<double>(c_ab);
        }

        double normalized_conflict(const size_t a, const size_t b) const {
            double denominator = 0.;
            double n = static_cast<double>(size());
            for (size_t i = 1; i <= size(); ++i) {
                denominator += abs(2*i-n-1);
            }
            return static_cast<double>(conflict(a,b))/denominator;
        }

        /// \brief Check if this front weakly dominates a point
        /// A front a weakly dominates a solution p if it has at least
        /// one solution that dominates p
        /// \see http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool dominates(const point_type& p) const {
            const_iterator it = data_.begin_intersection(ideal(), p, [&p,this](const value_type& x) {
              return x.first.dominates(p, is_minimization_);});
            return it != end();
        }

        /// \brief Check if this front strongly dominates a point
        /// A front a strongly dominates a solution b if a has a solution
        /// that is strictly better than b in all objectives.
        bool strongly_dominates(const point_type& p) const {
            const_iterator it = data_.begin_intersection(ideal(), p, [&p,this](const value_type& x) {
              return x.first.strongly_dominates(p, is_minimization_);});
            return it != end();
        }

        /// \brief Check if this front non-dominates the point
        /// A solution a weakly dominates a solution b if a is better
        /// than b in at least one objective and is as good as b in
        /// all other objectives.
        /// \see http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool non_dominates(const point_type& p) const {
            // Ensure pareto does not dominate p
            // Ensure p does not dominate anyone in the pareto
            return !dominates(p) && !is_partially_dominated_by(p);
        }

        bool is_partially_dominated_by(const point_type& p) const {
            // get points in the intersection between worst and p that p dominates
            const_iterator it = data_.begin_intersection(worst(), p, [&p,this](const value_type& x) {
              return p.dominates(x.first, is_minimization_);});
            // if there's someone p dominates, then it's partially dominated by p
            return it != end();
        }

        bool is_completely_dominated_by(const point_type& p) const {
            // Get points outside the intersection between worst and p that p dominates.
            // These are the points p is certainly not going to dominate.
            const_iterator it = data_.begin_disjoint(worst(), p);
            for (; it != data_.end(); ++it) {
                if (!p.dominates(it->first, is_minimization_)) {
                    return false;
                }
            }
            // If there's no one outside the intersection, then we
            // check inside the intersection, because we are not sure
            // p dominates all points inside this intersection
            // get points in the intersection between worst and p that p dominates
            it = data_.begin_intersection(worst(), p, [&p,this](const value_type& x) {
              return !p.dominates(x.first, is_minimization_);});
            // if there's someone p dominates, then it's completely dominated by p
            return it == end();
        }


        /// \brief Check if this front dominates another front
        /// P1 dominates P2 if all points in P1 dominate or at least
        /// non-dominate P2
        /// To avoid both operations, it's easy to check if P1
        /// fails to dominate any point in P2
        bool dominates(const front & P2) const {
            if (empty()) {
                return false;
            }
            bool dominates_any = false;
            for (auto& [k,v]: P2) {
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

        /// Check if this front strongly dominates another front
        bool strongly_dominates(const front & p) const {
            if (empty()) {
                return false;
            }
            for (auto& [k,v]: p) {
                if (!strongly_dominates(k)) {
                    return false;
                }
            }
            return true;
        }

        /// Check if this front non-dominates another front
        bool non_dominates(const front & p) const {
            if (empty()) {
                return true;
            }
            for (auto& [k,v]: p) {
                if (!non_dominates(k)) {
                    return false;
                }
            }
            return true;
        }

        /// Check if this front is is_partially_dominated_by another front
        bool is_partially_dominated_by(const front & p) const {
            if (empty()) {
                return true;
            }
            for (auto& [k,v]: p) {
                if (is_partially_dominated_by(k)) {
                    return true;
                }
            }
            return false;
        }

        /// Check if this front is is_partially_dominated_by another front
        bool is_completely_dominated_by(const front & p) const {
            if (empty()) {
                return true;
            }
            for (auto& [k,v]: *this) {
                if (p.is_partially_dominated_by(k)) {
                    return false;
                }
            }
            return true;
        }

        point_type ideal() const {
            point_type r(dimensions());
            for (size_t i = 0; i < r.dimensions(); ++i) {
                r[i] = ideal(i);
            }
            return r;
        }

        number_type ideal(size_t d) const {
            return is_minimization(d) ? data_.min_value(d) : data_.max_value(d);
        }

        const_iterator dimension_ideal(size_t d) const {
            return is_minimization(d) ? data_.min_element(d) : data_.max_element(d);
        }

        /// The nadir point is the worst point among the
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

        number_type nadir(size_t d) const {
            return is_minimization(d) ? data_.max_value(d) : data_.min_value(d);
        }

        const_iterator dimension_nadir(size_t d) const {
            return is_minimization(d) ? data_.max_element(d) : data_.min_element(d);
        }

        point_type worst() const {
            return nadir();
        }

        number_type worst(size_t d) const {
            return nadir(d);
        }

        const_iterator dimension_worst(size_t d) const {
            return dimension_nadir(d);
        }

        const_iterator find(const key_type &k) const {
            return data_.find(k);
        }

        iterator find(const key_type& k) {
            return data_.find(k);
        }

        bool contains(const key_type &k) const {
            return find(k) != end();
        }

        friend std::ostream &operator<<(std::ostream &os, const front &pf) {
            os << "Pareto front (" << pf.size() << " elements - {";
            for (size_t i = 0; i < pf.is_minimization_.size() - 1; ++i) {
                os << (pf.is_minimization_[i] ? "minimization" : "maximization") << ", ";
            }
            os << (pf.is_minimization_.back() ? "minimization" : "maximization") << "})";
            return os;
        }

#ifdef BUILD_FRONTS_WITH_TRASE
        /// Setup a trace figure with the plot
        void setup_trase_figure(std::shared_ptr<trase::Figure>& fig, point_type min_point, point_type max_point, point_type worst_point, bool plot_ideal = true, bool plot_worst = true, bool plot_nadir = true, float point_color = 0.f, std::optional<point_type> extra_point = std::nullopt, std::optional<const_iterator> it = std::nullopt) const {
            std::shared_ptr<trase::Axis> ax = fig->axis();
            std::vector<trase::Vector<float, 3>> my_colors = {
                    {0.f, 0.0f, 1.f}, // points
                    {0.f, 0.7f, 0.f}, // ideal
                    {1.f, 0.f, 0.f}, // worst
                    {.7f, .7f, 0.f}, // nadir
                    {0.f, 0.f, 0.f}, // extra point
                    {1.f, 0.4f, 0.f} // extra set
            };
            trase::Colormap my_map(my_colors);
            if (dimensions() == 2) {
                // get x and y in order
                std::vector<const_iterator> v;
                for (auto it = begin(); it != end(); ++it) {
                    v.push_back(it);
                }
                std::sort(v.begin(), v.end(), [](const_iterator const& i, const_iterator const& j) { return i->first[0] < j->first[0]; } );
                std::vector<number_type> x;
                std::vector<number_type> y;
                std::vector<float> color;
                for (const auto& it : v) {
                    x.emplace_back(it->first[0]);
                    y.emplace_back(it->first[1]);
                    color.emplace_back(point_color);
                }
                // get max and min for both dimensions
                number_type x_min = min_point[0];
                number_type x_max = max_point[0];
                number_type x_range = x_max - x_min;
                number_type y_min = min_point[1];
                number_type y_max = max_point[1];
                number_type y_range = y_max - y_min;
                // plot x and y points
                auto data = trase::create_data().x(x).y(y).color(color).color(0,6).fill(color).fill(0,6);
                auto points = ax->points(data);
                trase::Colormap & cm = const_cast<trase::Colormap &>(points->get_colormap());
                cm = my_map;
                points->set_label("points");
                // plot dominated area
                std::vector<number_type> x_line;
                x_line.reserve(x.size()*2+2);
                std::vector<number_type> y_line;
                y_line.reserve(y.size()*2+2);
                // first line point outside the front
                if (is_minimization(0)) {
                    x_line.emplace_back(x[0]);
                    y_line.emplace_back(is_minimization(1) ? y_max + y_range*0.3 : y_min - y_range*0.3);
                } else {
                    x_line.emplace_back(x_min - x_range*0.3);
                    y_line.emplace_back(y[0]);
                }
                // all points in the front (but last)
                for (size_t i = 0; i < x.size() - 1; ++i) {
                    // front point
                    x_line.emplace_back(x[i]);
                    y_line.emplace_back(y[i]);
                    // line to next front
                    if (is_minimization(0)) {
                        x_line.emplace_back(x[i+1]);
                        y_line.emplace_back(y[i]);
                    } else {
                        x_line.emplace_back(x[i]);
                        y_line.emplace_back(y[i+1]);
                    }
                }
                // last point
                x_line.emplace_back(x.back());
                y_line.emplace_back(y.back());
                // last from last point to outside the plot
                if (is_minimization(0)) {
                    x_line.emplace_back(x_max + x_range*0.2);
                    y_line.emplace_back(y.back());
                } else {
                    x_line.emplace_back(x.back());
                    y_line.emplace_back(is_minimization(1) ? y_max + y_range * 0.2 : y_min - y_range * 0.2);
                }
                auto data_line = trase::create_data().x(x_line).y(y_line);
                auto plt = ax->line(data_line);
                trase::Vector<float,3> line_color = {0.f,0.f,0.f};
                plt->style().line_width(3-point_color*2).color(trase::RGBA(line_color));
                // plot reference points
                if (plot_ideal) {
                    auto k = ideal();
                    x.emplace_back(k[0]);
                    y.emplace_back(k[1]);
                    // color.emplace_back("#00FF00");
                    color.emplace_back(1);
                }
                if (plot_worst) {
                    auto k = worst_point;
                    x.emplace_back(k[0]);
                    y.emplace_back(k[1]);
                    // color.emplace_back("#FF0000");
                    color.emplace_back(2);
                }
                if (plot_nadir) {
                    auto k = nadir();
                    x.emplace_back(k[0]);
                    y.emplace_back(k[1]);
                    // color.emplace_back("#FFFF00");
                    color.emplace_back(3);
                }
                if (extra_point) {
                    auto k = *extra_point;
                    x.emplace_back(k[0]);
                    y.emplace_back(k[1]);
                    // color.emplace_back("#000000");
                    color.emplace_back(4);
                }
                if (it) {
                    while (*it != end()) {
                        auto k = (**it).first;
                        x.emplace_back(k[0]);
                        y.emplace_back(k[1]);
                        // color.emplace_back("#FFA500");
                        color.emplace_back(5);
                        ++(*it);
                    }
                }
                auto data_reference = trase::create_data().x(x).y(y).color(color).color(0,6).fill(color).fill(0,6);
                auto points_reference = ax->points(data_reference);
                trase::Colormap & cm_points_reference = const_cast<trase::Colormap &>(points_reference->get_colormap());
                cm_points_reference = my_map;
                points_reference->set_label("points");
                ax->xlabel("<tspan font-style=\"italic\">f</tspan><tspan baseline-shift=\"sub\" font-size=\"small\">1</tspan>");
                ax->ylabel("<tspan font-style=\"italic\">f</tspan><tspan baseline-shift=\"sub\" font-size=\"small\">2</tspan>");
            } else {
                auto include_line = [&my_colors, this](const point_type p, int color, float line_width, std::shared_ptr<trase::Axis> ax, const std::string& label = "") {
                    std::vector<number_type> x;
                    std::vector<number_type> y;
                    std::vector<double> colors;
                    for (size_t i = 0; i < p.dimensions(); ++i) {
                        y.emplace_back(p[i]);
                        x.emplace_back(i+1);
                        colors.emplace_back(color);
                    }
                    trase::DataWithAesthetic data = trase::create_data().x(x).y(y);
                    std::shared_ptr<trase::Geometry> plt_line = ax->line(data);
                    auto rgb_color_data = my_colors[color];
                    rgb_color_data *= 255;
                    plt_line->style().color(trase::RGBA(rgb_color_data)).line_width(line_width);
                    if (!label.empty()) {
                        plt_line->set_label(label);
                    }
                };
                for (const auto& [k,v] : *this) {
                    include_line(k, 0, 2, ax);
                }
                for (int i = 0; i < dimensions(); ++i) {
                    include_line(dimension_ideal(i)->first, 0, 4, ax);
                }
                include_line(ideal(), 1, 4, ax, "ideal");
                include_line(worst(), 2, 4, ax, "worst");
                include_line(nadir(), 3, 4, ax, "nadir");
                if (extra_point) {
                    include_line(*extra_point,4,6,ax, "reference");
                }
                if (it) {
                    while (*it != end()) {
                        include_line((**it).first,5,3,ax, "query");
                        ++(*it);
                    }
                }
                ax->xlabel("f(x)");
                ax->ylabel("x");
            }

        }

        /// Setup a trace figure with the plot
        void setup_trase_figure(std::shared_ptr<trase::Figure>& fig, std::optional<point_type> extra_point = std::nullopt, std::optional<const_iterator> it = std::nullopt) const {
            point_type min_point = ideal();
            point_type worst_point = worst();
            point_type max_point = worst_point;
            normalize_corners(min_point, max_point);
            float point_color = 0.f;
            setup_trase_figure(fig, min_point, max_point, worst_point, true, true, true, point_color, extra_point, it);
        }

        /// Plot the pareto front as a svg plot
        /// \param extra_point An extra point that will be plotted over the graph (default = nullopt)
        /// \param it An iterator to a set of points to be highlighted in the graph (default = nullopt)
        /// \return String with the plot in svg format
        std::string svg(std::optional<point_type> extra_point = std::nullopt, std::optional<const_iterator> it = std::nullopt) const {
            std::shared_ptr<trase::Figure> fig = trase::figure();
            // create a trace figure
            setup_trase_figure(fig, extra_point, it);
            // stream it to a stringstream
            std::stringstream ss;
            trase::BackendSVG backend(ss);
            fig->draw(backend);
            return ss.str();
        }
#endif

      private /* functions */:
        /// \brief Clear solutions are dominated by p
        /// Pareto-optimal front is the set F consisting of
        /// all non-dominated solutions x in the whole
        /// search space. No solution can dominate another
        /// solution. Note that this means two solutions
        /// might have the same values though.
        size_t clear_dominated(const point_type& p) {
            // The modification of the rtree may invalidate the iterators
            // https://www.boost.org/doc/libs/1_60_0/libs/geometry/doc/html/geometry/reference/spatial_indexes/boost__geometry__index__rtree/begin__.html
            // Remove doesn't take iterators pointing to values
            // Remove removes values equal to these passed as a range
            // https://www.boost.org/doc/libs/1_73_0/libs/geometry/doc/html/geometry/reference/spatial_indexes/group__rtree__functions/remove_rtree_________iterator__iterator_.html
            // We have to make only one query and get a copy of all ITEMS (not iterators)
            // we want to remove.
            if (!empty()) {
                // Query
                const_iterator it = data_.begin_intersection(p, worst(), [&p, this](const value_type& x) {
                  return p.dominates(x.first, is_minimization_);
                });
                // erase these elements
                return erase(it, end());
            }
            return 0;
        }

        /// \brief Clear all solutions dominated by some point
        size_t clear_dominated() {
            // The modification of the rtree may invalidate the iterators
            // https://www.boost.org/doc/libs/1_60_0/libs/geometry/doc/html/geometry/reference/spatial_indexes/boost__geometry__index__rtree/begin__.html
            // Remove doesn't take iterators pointing to values
            // Remove removes values equal to these passed as a range
            // https://www.boost.org/doc/libs/1_73_0/libs/geometry/doc/html/geometry/reference/spatial_indexes/group__rtree__functions/remove_rtree_________iterator__iterator_.html
            // We have to make only one query and get a copy of all ITEMS (not iterators)
            // we want to remove.
            if (size() > 1) {
                // Query everyone
                // Put them all in a separate list
                // because iterators will be invalidated
                auto it = find_intersection(ideal(), worst());
                auto it_end = end();
                std::vector<value_type> all;
                while (it != it_end) {
                    all.emplace_back(*it);
                    ++it;
                }
                // Iterate removing points they dominate
                size_t sum_removed = 0;
                for (const auto& [k,v]: all) {
                    // if k hasn't been removed yet
                    if (find(k) != end()) {
                        sum_removed += clear_dominated(k);
                    }
                }
                return sum_removed;
            }
            return 0;
        }


        double distance(const point_type& p1, const point_type& p2) const {
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

        double distance(const point_type& p, const box_type& b) const {
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
            static std::mt19937 g(
                static_cast<unsigned int>(static_cast<uint64_t>(std::random_device()()) | std::chrono::high_resolution_clock::now().time_since_epoch().count()));
            return g;
        }


        void normalize_corners(point_type& min_corner, point_type& max_corner) const {
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

        inline void maybe_adjust_dimensions(const point_type& position) {
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
