//
// Created by Alan Freitas on 2020-05-20.
//

#ifndef PARETO_FRONT_ARCHIVE_H
#define PARETO_FRONT_ARCHIVE_H

#include <vector>

#include <pareto/front.h>

namespace pareto {

    template<typename NUMBER_TYPE = double, size_t NUMBER_OF_DIMENSIONS = 2, typename ELEMENT_TYPE = unsigned, typename TAG = default_tag<NUMBER_OF_DIMENSIONS>>
    class archive {
    public /* constructors */:
        using self_type = archive<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, TAG>;
        using pareto_front_type =
            front<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, TAG>;
        static constexpr size_t number_of_compile_dimensions = NUMBER_OF_DIMENSIONS;
        using number_type = typename pareto_front_type::number_type;
        using point_type = typename pareto_front_type::point_type;
        using box_type = typename pareto_front_type::box_type;
        using key_type = typename pareto_front_type::key_type;
        using mapped_type = typename pareto_front_type::mapped_type;
        using value_type = typename pareto_front_type::value_type;
        using reference = typename pareto_front_type::reference;
        using const_reference = typename pareto_front_type::const_reference;
        using pointer = typename pareto_front_type::pointer;
        using const_pointer = typename pareto_front_type::const_pointer;
        using size_type = typename pareto_front_type::size_type;
        using internal_type = typename pareto_front_type::internal_type;
        using internal_minimization_type = typename pareto_front_type::internal_minimization_type;
        using difference_type = typename pareto_front_type::difference_type;
        using node_allocator_type = typename pareto_front_type::node_allocator_type;

        using pareto_front_iterator = typename pareto_front_type::iterator;
        using pareto_front_const_iterator = typename pareto_front_type::const_iterator;

    public /* iterators */:
        template<bool is_const = false>
        class iterator_impl {
        public:
            friend archive;
            using pareto_iterator = std::conditional_t<is_const, typename pareto_front_type::const_iterator, typename pareto_front_type::iterator>;
            using difference_type = typename pareto_iterator::difference_type;
            using value_type = typename pareto_iterator::value_type;
            using reference = typename pareto_iterator::reference;
            using const_reference = const reference;
            using pointer = typename pareto_iterator::pointer;
            using const_pointer = value_type const *;
            using point_type = typename archive::point_type;
            using iterator_category = typename pareto_iterator::iterator_category;
            using archive_pointer = std::conditional_t<is_const, const archive *, archive *>;

            /// Default constructor for convenience
            iterator_impl() : begins_(0), ar_(nullptr), current_iter_(), current_front_(0) {}

            /// Construct an archive iterator
            /// \param begins Begin iterator of each internal pareto fron
            /// \param ar Pointer to the archive
            /// \param current_iter Iterator to the current element in the pareto front
            /// \param current_front Front of the current element
            iterator_impl(const std::vector<pareto_iterator> &begins, archive_pointer ar, pareto_iterator current_iter,
                     size_t current_front)
                    : ar_(ar), current_iter_(current_iter), current_front_(current_front) {
                for (size_t i = 0; i < begins.size(); ++i) {
                    begins_.emplace_back(i, begins[i]);
                }
                advance_to_next_valid();
            }

            /// Construct from limited set of begins
            /// \param begins List of front index and begin iterators for all fronts
            /// \param ar Pointer to archive
            /// \param current_iter Current element in front
            /// \param current_front Index of front of current element
            /// \return
            iterator_impl(const std::vector<std::pair<size_t, pareto_iterator>> &begins, archive_pointer ar,
                     pareto_iterator current_iter, size_t current_front)
                    : begins_(begins), ar_(ar), current_iter_(current_iter), current_front_(current_front) {
                advance_to_next_valid();
            }

            /// Construct from a list of begins. Current iterator is set to first element of first front.
            /// \param begins Begin iterator of each front
            /// \param ar Pointer to the archive
            iterator_impl(const std::vector<pareto_iterator> &begins, archive_pointer ar)
                    : iterator_impl(begins, ar, !begins.empty() ? begins[0] : ar->data_.back().end(), 0) {}

            /// Construct from a front iterator and its index in the archive.
            iterator_impl(archive_pointer ar, size_t front_index, pareto_iterator current_iter)
                    : iterator_impl(std::vector<std::pair<size_t, pareto_iterator>>{{front_index, current_iter}}, ar)
            {}

            /// Construct from a subset of begins (list of pair<front index, begin iterator>)
            /// \param begins
            /// \param ar
            iterator_impl(const std::vector<std::pair<size_t, pareto_iterator>> &begins, archive_pointer ar)
                    : iterator_impl(begins, ar, !begins.empty() ? begins[0].second : ar->data_.back().end(), 0) {}

            /// Construct from archive pointer
            explicit iterator_impl(archive_pointer ar) {
                if (ar) {
                    for (size_t i = 0; i < ar->data_.size(); ++i) {
                        pareto_iterator it = ar->data_[i].begin();
                        begins_.emplace_back(i, it);
                    }
                    ar_ = ar;
                    current_iter_ = begins_[0].second;
                    current_front_ = 0;
                    advance_to_next_valid();
                } else {
                    *this = iterator_impl();
                }
            }

            /// Copy constructor if this is const_iterator other is non-const iterator (iterator_impl<false>)
            template<bool _constness = is_const>
            iterator_impl(const std::enable_if_t<_constness, iterator_impl<false>> &rhs) // NOLINT(google-explicit-constructor): Iterators should be implicitly convertible
                    : ar_(rhs.ar_), current_iter_(rhs.current_iter_), current_front_(rhs.current_front_) {
                for (auto& [index, it]: rhs.begins_) {
                    begins_.emplace_back(index, pareto_iterator(it));
                }
                advance_to_next_valid();
            }

            /// Copy constructor if this is non-const iterator (iterator_impl<false>) other is non-const iterator (iterator_impl<false>)
            template<bool _constness = is_const>
            iterator_impl(const std::enable_if_t<!_constness, iterator_impl<false>> &rhs) // NOLINT(google-explicit-constructor): Iterators should be implicitly convertible
                    : begins_(rhs.begins_), ar_(rhs.ar_), current_iter_(rhs.current_iter_),
                      current_front_(rhs.current_front_) {
                advance_to_next_valid();
            }

            /// Copy constructor if other is const_iterator
            /// SFINAE: "this" cannot be be iterator (iterator_impl<false>)
            template<bool _constness = is_const>
            iterator_impl(const std::enable_if_t<_constness, iterator_impl<true>> &rhs) // NOLINT(google-explicit-constructor): Iterators should be implicitly convertible
                    : begins_(rhs.begins_), ar_(rhs.ar_), current_iter_(rhs.current_iter_),
                      current_front_(rhs.current_front_) {
                advance_to_next_valid();
            }

            ~iterator_impl() = default;

            template <bool constness>
            iterator_impl &operator=(const iterator_impl<constness> &rhs) {
                begins_ = rhs.begins_;
                ar_ = rhs.ar_;
                current_iter_ = rhs.current_iter_;
                current_front_ = rhs.current_front_;
                advance_to_next_valid();
                return *this;
            }

            bool operator==(const iterator_impl &rhs) const {
                bool a = is_end();
                bool b = rhs.is_end();
                if (a && b) {
                    return true;
                }
                if (a || b) {
                    return false;
                }
                return begins_ == rhs.begins_ &&
                       ar_ == rhs.ar_ &&
                       current_iter_ == rhs.current_iter_ &&
                       current_front_ == rhs.current_front_;
            }

            bool operator!=(const iterator_impl &rhs) const {
                return !(this->operator==(rhs));
            }

            iterator_impl &operator++() {
                current_iter_.operator++();
                advance_to_next_valid();
                return *this;
            }

            iterator_impl operator++(int) { // NOLINT(cert-dcl21-cpp): This is the expected return type for iterators
                auto tmp = *this;
                current_iter_.operator++();
                advance_to_next_valid();
                return tmp;
            }

            iterator_impl &operator--() {
                // if there are no pareto iterators in begins_
                if (begins_.empty() && is_end()) {
                    // this is an empty iterator representing end()
                    // so we need to populate it with the begins_ iterators
                    iterator_impl begin_it = ar_->begin();
                    begins_ = begin_it.begins_;
                    current_front_ = begins_.size();
                }
                return_to_previous_valid();
                return *this;
            }

            iterator_impl operator--(int) { // NOLINT(cert-dcl21-cpp): This is the expected return type for iterators
                auto tmp = *this;
                return_to_previous_valid();
                return tmp;
            }

            reference operator*() const {
                return current_iter_.operator*();
            }

            pointer operator->() const {
                return current_iter_.operator->();
            }

        private:
            [[nodiscard]] bool is_end() const {
                if (!ar_ || begins_.empty()) {
                    return true;
                }
                if (current_front_ == begins_.size()) {
                    return true;
                }
                return false;
            }

            [[nodiscard]] bool is_begin() const {
                // There are 0 elements
                if (!ar_ || begins_.empty()) {
                    // So it has to also be begin (besides being end)
                    return true;
                }
                if (current_front_ != 0) {
                    return false;
                }
                return (ar_->data_[begins_[current_front_].first].begin() == current_iter_);
            }

            void advance_to_next_valid() {
                if (!is_end()) {
                    while (current_iter_ == ar_->data_[begins_[current_front_].first].end()) {
                        ++current_front_;
                        if (current_front_ < begins_.size()) {
                            current_iter_ = begins_[current_front_].second;
                        } else {
                            break;
                        }
                    }
                }
            }

            void return_to_previous_valid() {
                while (!is_begin()) {
                    // if we are past the last element of the last front
                    if (is_end()) {
                        // if the set of elements to iterate is not empty
                        if (!begins_.empty()) {
                            // go to end() of last front
                            current_front_ = begins_.back().first;
                            current_iter_ = ar_->data_[begins_[current_front_].first].end();
                        } else {
                            return;
                        }
                        // if end() of current front is same as begin()
                        if (current_iter_ == begins_[current_front_].second) {
                            // do one more iteration
                            continue;
                        } else {
                            // if current element is valid
                            --current_iter_;
                            return;
                        }
                    } else if (current_iter_ == begins_[current_front_].second) {
                        // if we are at begin of the current front
                        // if this is not the first front (the begin of all elements)
                        if (current_front_ != 0) {
                            // move to previous front
                            --current_front_;
                            current_iter_ = ar_->data_[begins_[current_front_].first].end();
                            // if last element is same as begin
                            if (current_iter_ == begins_[current_front_].second) {
                                // do one more iteration
                                continue;
                            } else {
                                // if current element is valid
                                --current_iter_;
                                return;
                            }
                        } else {
                            // if this is the begin of first front
                            // return without changing iterator
                            return;
                        }
                    } else {
                        // if this is not a begin and this is not end()
                        current_iter_.operator--();
                        return;
                    }
                }
            }

        private:
            /// Fronts where we have elements in which are interested and their begin's
            /// pair<front index, front begin iterator>
            std::vector<std::pair<size_t, pareto_iterator>> begins_;

            /// Pointer to the archive to which this iterator belongs
            archive_pointer ar_;

            /// Current element we are iterating
            pareto_iterator current_iter_;

            /// Current front from the begins list we are iterating
            size_t current_front_{0};
        };

        using iterator = iterator_impl<false>;
        using const_iterator = iterator_impl<true>;

    private:
        /// Is the internal type using the fast allocator
        constexpr static bool is_using_default_fast_allocator() {
            return pareto_front_type::is_using_default_fast_allocator();
        }

    public /* constructors */:
        /// Empty archive with room for 1000 elements
        archive() : archive(1000) {}

        /// Empty minimization archive
        explicit archive(size_t max_size) : archive(max_size, true) {}

        /// Create an empty pareto set and determine whether it is minimization
        explicit archive(size_t max_size, bool is_minimization)
                : archive(max_size, std::vector<uint8_t>(std::max(number_of_compile_dimensions, size_t(1)), is_minimization)) {}

        /// Create an empty pareto set and determine whether each dimension is minimization
        explicit archive(size_t max_size, std::initializer_list<bool> is_minimization) :
            max_size_(max_size),
            alloc_(new node_allocator_type())
        {
            if constexpr (number_of_compile_dimensions > 0) {
                if (number_of_compile_dimensions != is_minimization_.size()) {
                    throw std::invalid_argument("Number of minimization directions should match the dimension defined for the object");
                }
            }
            maybe_resize(is_minimization_, is_minimization.size());
            std::copy(is_minimization.begin(), is_minimization.end(), is_minimization_.begin());
        }

        /// Create an empty pareto set and determine whether each dimension is minimization
        explicit archive(size_t max_size, const std::vector<uint8_t> &is_minimization) :
            max_size_(max_size), alloc_(new node_allocator_type()) {
            if constexpr (number_of_compile_dimensions > 0) {
                if (number_of_compile_dimensions != is_minimization_.size()) {
                    throw std::invalid_argument("Number of minimization directions should match the dimension");
                }
            }
            maybe_resize(is_minimization_, is_minimization.size());
            std::copy(is_minimization.begin(), is_minimization.end(), is_minimization_.begin());
        }

        /// Create an empty pareto set and determine whether each dimension is minimization
        explicit archive(size_t max_size, const std::array<uint8_t, number_of_compile_dimensions> &is_minimization)
            : max_size_(max_size),
              alloc_(new node_allocator_type()) {
            if constexpr (number_of_compile_dimensions > 0) {
                if (number_of_compile_dimensions != is_minimization_.size()) {
                    throw std::invalid_argument("Number of minimization directions should match the dimension");
                }
            }
            maybe_resize(is_minimization_, is_minimization.size());
            std::copy(is_minimization.begin(), is_minimization.end(), is_minimization_.begin());
        }

        /// Copy constructor
        archive(const archive &rhs) {
            data_ = rhs.data_;
            alloc_ = rhs.alloc_;
            is_minimization_ = rhs.is_minimization_;
            max_size_ = rhs.max_size_;
        }

        /// Assignment
        archive &operator=(const archive &rhs) {
            alloc_ = rhs.alloc_;
            data_ = rhs.data_;
            is_minimization_ = rhs.is_minimization_;
            max_size_ = rhs.max_size_;
            return *this;
        }

        /// Move constructor
        archive(archive &&m) noexcept {
            alloc_ = m.alloc_;
            data_ = std::move(m.data_);
            is_minimization_ = std::move(m.is_minimization_);
            max_size_ = std::move(m.max_size_);
        }

        /// Move assignment
        archive &operator=(archive &&rhs) noexcept {
            alloc_ = rhs.alloc_;
            data_ = std::move(rhs.data_);
            is_minimization_ = std::move(rhs.is_minimization_);
            max_size_ = std::move(rhs.max_size_);
            return *this;
        }

        /// Create a pareto set from a list of value pairs
        explicit archive(size_t max_size, std::initializer_list<value_type> il)
                : archive(max_size, il.begin(), il.end()) {}

        /// Create a pareto set from a list of value pairs and determine whether it is minimization
        archive(size_t max_size, std::initializer_list<value_type> il, bool is_minimization)
                : archive(max_size, il.begin(), il.end(), is_minimization) {}

        /// Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        archive(size_t max_size, std::initializer_list<value_type> il, const std::vector<uint8_t>& is_minimization)
                : archive(max_size, il.begin(), il.end(), is_minimization) {}

        /// Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        archive(size_t max_size, std::initializer_list<value_type> il, std::initializer_list<bool> is_minimization)
                : archive(max_size, il.begin(), il.end(), init_list_to_vector(is_minimization)) {}

        /// Create a pareto set from a list of value pairs
        archive(size_t max_size, const std::vector<value_type> &v)
                : archive(max_size, v.begin(), v.end()) {}

        /// Create a pareto set from a list of value pairs and determine whether it is minimization
        archive(size_t max_size, const std::vector<value_type> &v, bool is_minimization)
                : archive(max_size, v.begin(), v.end(), is_minimization) {}

        /// Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        archive(size_t max_size, const std::vector<value_type> &v, const std::vector<uint8_t>& is_minimization)
                : archive(max_size, v.begin(), v.end(), is_minimization) {}

        /// Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        archive(size_t max_size, const std::vector<value_type> &v, std::initializer_list<bool> is_minimization)
                : archive(max_size, v.begin(), v.end(), init_list_to_vector(is_minimization)) {}

        /// Create a pareto set from a list of value pairs
        template<class InputIterator>
        archive(size_t max_size, InputIterator first, InputIterator last)
                : archive(max_size, first, last, true) {}

        /// Create a pareto set from a list of value pairs and determine whether it is minimization
        template<class InputIterator>
        archive(size_t max_size, InputIterator first, InputIterator last, bool is_minimization)
                : archive(max_size, first, last, std::vector<uint8_t>(std::max(number_of_compile_dimensions,size_t(1)), is_minimization)) {}

        /// Create a pareto set from a list of value pairs and determine whether each dimension is minimization
        template<class InputIterator>
        archive(size_t max_size, InputIterator first, InputIterator last, std::vector<uint8_t> is_minimization)
                : max_size_(max_size), alloc_(new node_allocator_type()) {
            if constexpr (number_of_compile_dimensions != 0) {
                if (is_minimization.size() != number_of_compile_dimensions) {
                    throw std::invalid_argument(
                            "The size specified at compile time does not match the number of minimization directions");
                }
            }
            maybe_resize(is_minimization_, is_minimization.size());
            std::copy(is_minimization.begin(), is_minimization.end(), is_minimization_.begin());
            while (first != last) {
                insert(*first);
                ++first;
            }
        }

    public /* iterators */:
        const_iterator begin() const noexcept {
            return const_iterator(this);
        }

        const_iterator end() const noexcept {
            return const_iterator(std::vector<std::pair<size_t, typename pareto_front_type::const_iterator>>(), this, !data_.empty() ? data_.back().end() : typename pareto_front_type::const_iterator(), data_.size());
        }

        iterator begin() noexcept {
            return iterator(this);
        }

        iterator end() noexcept {
            return iterator(std::vector<std::pair<size_t, typename pareto_front_type::iterator>>(), this, !data_.empty() ? data_.back().end() : typename pareto_front_type::iterator(), data_.size());
        }

        std::reverse_iterator<const_iterator> rbegin() const noexcept {
            return std::reverse_iterator(end());
        }

        std::reverse_iterator<const_iterator> rend() const noexcept {
            return std::reverse_iterator(begin());
        }

        std::reverse_iterator<iterator> rbegin() noexcept {
            return std::reverse_iterator(end());
        }

        std::reverse_iterator<iterator> rend() noexcept {
            return std::reverse_iterator(begin());
        }

        typename std::vector<pareto_front_type>::const_iterator
        begin_front() const noexcept {
            return data_.begin();
        }

        typename std::vector<pareto_front_type>::const_iterator
        end_front() const noexcept {
            return data_.end();
        }

        typename std::vector<pareto_front_type>::iterator
        begin_front() noexcept {
            return data_.begin();
        }

        typename std::vector<pareto_front_type>::iterator
        end_front() noexcept {
            return data_.end();
        }

    public /* capacity */:
        [[nodiscard]] bool empty() const noexcept {
            return data_.empty() ||
                   std::all_of(data_.begin(), data_.end(), [](const pareto_front_type &pf) { return pf.empty(); });
        }

        size_type size() const noexcept {
            size_t s = 0;
            for (const auto& pf: data_) {
                s += pf.size();
            }
            return s;
        }

        size_type max_size() const noexcept {
            return max_size_;
        }

        size_type fronts() const noexcept {
            return data_.size();
        }

        size_type dimensions() const noexcept {
            return is_minimization_.size();
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
            return is_minimization_[dimension] > 0;
        }

        size_type is_maximization(size_t dimension) const noexcept {
            return is_minimization_[dimension] == 0;
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
            constexpr size_t m = sizeof...(Targs) + 1;
            assert(number_of_compile_dimensions == 0 || number_of_compile_dimensions == m);
            point_type p(m);
            copy_pack(p.begin(), k, ks...);
            return operator[](p);
        }

        mapped_type& at(const key_type& k){
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("archive::at:  key not found");
            }
        }

        const mapped_type& at(const key_type& k) const {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("archive::at:  key not found");
            }
        }

    public /* relational operators */:
        bool operator==(const archive &rhs) const {
            return data_ == rhs.data_ &&
                   is_minimization_ == rhs.is_minimization_ &&
                   max_size_ == rhs.max_size_;
        }

        bool operator!=(const archive &rhs) const {
            return !(this->operator==(rhs));
        }

    public /* modifiers */:
        /// Emplace becomes insert becomes the rtree does not have
        /// an emplace function
        template<class... Args>
        std::pair<iterator, bool> emplace(Args &&... args) {
            auto v = value_type(args...);
            return insert(v);
        }

        /// Emplace becomes insert becomes the rtree does not have
        /// an emplace function
        template<class InputIterator>
        void emplace(InputIterator first, InputIterator last) {
            insert(first, last);
        }

        /// \brief Insert element pair
        /// Insertion removes any point dominated by the point
        /// before inserting the element in the rtree
        /// \param v Pair value <point, value>
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        std::pair<iterator, bool> insert(const value_type &v) {
            maybe_adjust_dimensions(v);
            return try_insert(0, v);
        }

        void normalize_corners(point_type &min_corner, point_type &max_corner) const {
            for (size_t i = 0; i < min_corner.dimensions(); ++i) {
                if (min_corner[i] > max_corner[i]) {
                    std::swap(min_corner[i], max_corner[i]);
                }
            }
        }

        std::pair<iterator, bool> try_insert(size_t front_idx, const value_type &v) {
            // Find first front that does not dominate v.first
            auto it = std::lower_bound(data_.begin() + front_idx, data_.end(), true,
                                       [&v](const auto &a, const bool &b) {
                                           return !a.dominates(v.first) < b;
                                       });
            // if the front it does not dominate the solution
            if (it != data_.end()) {
                size_t i = it - data_.begin();
                // get all points this solution dominates in front i
                auto dominated_it = data_[i].data_.begin_intersection(v.first, data_[i].worst(),
                                                                      [&v, this](const value_type &x) {
                                                                          return v.first.dominates(x.first,
                                                                                                   is_minimization_);
                                                                      });
                // This does not work on some compilers
                // std::vector<value_type> dominated_solutions(dominated_it, data_[i].data_.end());
                std::vector<value_type> dominated_solutions;
                auto begin_it = data_[i].data_.end();
                auto end_it = data_[i].data_.end();
                while (begin_it != end_it) {
                    dominated_solutions.emplace_back(*begin_it);
                    ++begin_it;
                }
                // remove all of them from the current front
                data_[i].erase(dominated_it, typename pareto_front_type::const_iterator(data_[i].data_.end()));
                // recursively insert all of them to higher fronts i+1, i+2, ...
                for (const auto &v2: dominated_solutions) {
                    try_insert(i + 1, v2);
                }
                // insert v in this front
                auto [pfit, ok] = data_[i].insert(v);
                std::vector<std::pair<size_t, pareto_front_iterator>> begins;
                begins.emplace_back(i, pfit);
                iterator it2 = iterator(begins, this);
                // if inserting v made the archive exceed its max size
                if (size() > max_size()) {
                    resize(max_size());
                    // iterator might be invalidated
                    // item might even have been removed
                    // look for item again
                    auto it3 = find(v.first);
                    return std::make_pair(it3, it3 != end());
                }
                return std::make_pair(it2, true);
            }
            // if all fronts dominate v
            // if there's room for more elements in the archive
            if (size() < max_size()) {
                // create a new last front and put it there
                data_.emplace_back(pareto_front_type(is_minimization_, alloc_));
                auto[pfit, ok] = data_.back().insert(v);
                if (ok) {
                    std::vector<std::pair<size_t, pareto_front_iterator>> begins;
                    begins.emplace_back(data_.size() - 1, pfit);
                    iterator it2 = iterator(begins, this, pfit, 0);
                    return std::make_pair(it2, true);
                }
            }
            return {end(), false};
        }

        /// \brief Resize the archive
        /// If new size is more than the number of elements
        /// we currently have, adjust the max_size.
        /// If we need to remove elements, to make them fit
        /// in the new max_size by removing the most crowded
        /// elements in the last front.
        /// \param new_size
        void resize(size_t new_size) {
            size_t current_size = size();
            if (current_size < new_size) {
                max_size_ = new_size;
            } else {
                size_t excess = current_size - new_size;
                while (excess > 0) {
                    if (excess >= data_.back().size()) {
                        // if excess is more than the last front
                        // remove it completely
                        excess -= data_.back().size();
                        data_.pop_back();
                    } else {
                        // if excess is less than last front
                        // remove the most crowded elements
                        std::vector<std::pair<point_type, double>> candidates;
                        candidates.reserve(data_.back().size());
                        for (const auto&[k, v] : data_.back()) {
                            auto it = data_.back().find_nearest(k, 3);
                            double d = 0.0;
                            for (; it != data_.back().end(); ++it) {
                                d += k.distance(it->first);
                            }
                            // point k and crowding distance d
                            candidates.emplace_back(k, d);
                        }
                        // smallest crowding distance comes first
                        std::partial_sort(candidates.begin(), candidates.begin() + excess, candidates.end(),
                                          [](const auto &a, const auto &b) {
                                              return a.second < b.second;
                                          });
                        for (size_t i = 0; i < excess; ++i) {
                            data_.back().erase(candidates[i].first);
                        }
                        excess = 0;
                    }
                }
            }
        }


        /// \brief Move element pair to pareto front
        /// \param v Pair value <point, value>
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        std::pair<iterator, bool> insert(value_type &&v) {
            auto p = std::move(v);
            return insert(p);
        }

        /// \brief Insert point and respective value in the front
        /// Insertion removes any point dominated by the point
        /// before inserting the element in the rtree
        /// \param position Point
        /// \param v Value
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        std::pair<iterator, bool> insert(point_type position, const mapped_type &v) {
            auto pv = std::make_pair(position, v);
            return insert(pv);
        }

        /// \brief Insert point and move respective value to the front
        /// Insertion removes any point dominated by the point
        /// before inserting the element in the rtree
        /// \param Vector with the point
        /// \param v Value
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        std::pair<iterator, bool> insert(const point_type &position, mapped_type &&v) {
            auto p = std::make_pair(position, std::move(v));
            return insert(p);
        }

        /// \brief Insert point and move respective value to the front
        /// Insertion removes any point dominated by the point
        /// before inserting the element in the rtree
        /// \param position point
        /// \param p Value
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        template<class V>
        std::pair<iterator, bool> insert(const point_type &position, V &&p) {
            return insert(position, mapped_type(p));
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

        /// Erase element from the archive
        /// \param v Point
        size_type erase(const key_type &point) {
            return try_erase(0, point);
        }

        /// \brief Try to erase element from front or subsequent fronts
        /// Tries to erase point from front front_idx or higher fronts
        /// The point is passed by value because if it is a reference to
        /// a point, it might be a reference to a point in the tree
        /// and the point would be invalidated as soon as we remove it
        /// from the tree.
        /// \param front_idx
        /// \param point
        /// \return
        size_type try_erase(size_t front_idx, key_type point) {
            // Find first front that does not dominate the point
            // The point cannot be in a front that dominates it,
            // or it would be there already.
            auto lower_bound_it = std::lower_bound(data_.begin() + front_idx, data_.end(), true,
                                                   [&point](const auto &a, const bool &b) {
                                                       return !a.dominates(point) < b;
                                                   });
            // Find first front partially dominated by the point.
            // Point cannot be in a front it dominates, or it would be
            // there already.
            auto upper_bound_it = std::lower_bound(lower_bound_it, data_.end(), true,
                                                   [&point](const auto &a, const bool &b) {
                                                       return a.is_partially_dominated_by(point) < b;
                                                   });
            size_t c = 0;
            for (size_t i = lower_bound_it - data_.begin(); i < static_cast<size_t>(upper_bound_it - data_.begin()); ++i) {
                c = data_[i].erase(point);
                // if we could erase in this front
                if (c != 0) {
                    // if front became empty
                    if (data_[i].empty()) {
                        // just erase the front and we're all set
                        data_.erase(data_.begin() + i);
                    }
                    // if front is not empty because of this removal, and there are other fronts
                    else if (i + 1 < data_.size()) {
                        // Some elements from next front might not be
                        // dominated in this front now. Move these
                        // elements from next front to this front.
                        // This happens recursively to trigger the same
                        // effect on the next fronts.
                        // Get points that might now not be dominated by the current front
                        auto it = data_[i + 1].find_intersection(point, data_[i + 1].worst());
                        // Copy these points because erasing the points would invalidate iterators
                        std::vector<value_type> intersection;
                        auto it_end = data_[i + 1].end();
                        while (it != it_end) {
                            intersection.emplace_back(*it);
                            ++it;
                        }
                        // For each of these points
                        for (auto p_it = intersection.begin(); p_it != intersection.end(); ++p_it) {
                            // If this front doesn't dominate the point
                            if (!data_[i].dominates(p_it->first)) {
                                // Insert it in this front
                                data_[i].insert(*p_it);
                                // Trigger the same process recursively
                                // to remove it from the following fronts
                                try_erase(i + 1, p_it->first);
                            }
                        }
                    }
                    return c;
                }
            }
            return c;
        }


        /// Erase element pointed by iterator from the front
        /// \warning The modification of the rtree may invalidate the iterators.
        size_t erase(const_iterator position) {
            if (position.ar_ == this) {
                if (position.current_front_ < data_.size()) {
                    if (position.current_iter_ != data_[position.current_front_].end()) {
                        return try_erase(position.current_front_, position->first);
                    }
                }
            }
            return 0;
        }

        /// Erase element pointed by iterator from the front
        /// \warning The modification of the rtree may invalidate the iterators.
        size_t erase(const iterator &position) {
            if (position.ar_ == this) {
                if (position.begins_[position.current_front_].first < data_.size()) {
                    if (position.current_iter_ != data_[position.begins_[position.current_front_].first].end()) {
                        return try_erase(position.begins_[position.current_front_].first, position->first);
                    }
                }
            }
            return 0;
        }

        /// Remove range of iterators from the front
        size_t erase(const_iterator first, const_iterator last) {
            // we need to make a copy because erasing
            // invalidates the iterators
            std::vector<point_type> tmp;
            while (first != last) {
                tmp.emplace_back(first->first);
                ++first;
            }
            size_t c = 0;
            for (const point_type &p: tmp) {
                c += erase(p);
            }
            return c;
        }

        /// Remove range of iterators from the front
        size_t erase(iterator first, iterator last) {
            // we need to make a copy because erasing
            // invalidates the iterators
            std::vector<point_type> tmp;
            while (first != last) {
                tmp.emplace_back(first->first);
                ++first;
            }
            size_t c = 0;
            for (const point_type &p: tmp) {
                c += erase(p);
            }
            return c;
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

        /// Merge two fronts
        void merge(pareto_front_type &source) {
            insert(source.begin(), source.end());
        }

        /// Merge and move fronts
        void merge(pareto_front_type &&source) {
            insert(source.begin(), source.end());
            source.clear();
        }

        /// Swap the content of two fronts
        void swap(self_type &m) {
            m.data_.swap(data_);
            m.is_minimization_.swap(is_minimization_);
            std::swap(max_size_, m.max_size_);
        }

    public /* pareto operations */:
        /// Find points in a box
        const_iterator find_intersection(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<std::pair<size_t, typename pareto_front_type::const_iterator>> begins;
            for (size_t i = 0; i < fronts(); ++i) {
                auto it = data_[i].find_intersection(min_corner, max_corner);
                if (it != data_[i].end()) {
                    begins.emplace_back(i, it);
                }
            }
            return const_iterator(begins, this);
        }

        /// Get points in a box
        std::vector<value_type> get_intersection(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<value_type> v;
            std::copy(find_intersection(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// Find points within a box (intersection minus borders)
        const_iterator find_within(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<std::pair<size_t, typename pareto_front_type::const_iterator>> begins;
            for (size_t i = 0; i < fronts(); ++i) {
                auto it = data_[i].find_within(min_corner, max_corner);
                if (it != data_[i].end()) {
                    begins.emplace_back(i, it);
                }
            }
            return const_iterator(begins, this);
        }

        /// Get points within a box
        std::vector<value_type> get_within(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<value_type> v;
            std::copy(find_within(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// Find points disjointed of a box (intersection - borders)
        const_iterator find_disjoint(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<std::pair<size_t, typename pareto_front_type::const_iterator>> begins;
            for (size_t i = 0; i < fronts(); ++i) {
                auto it = data_[i].find_disjoint(min_corner, max_corner);
                if (it != data_[i].end()) {
                    begins.emplace_back(i, it);
                }
            }
            return const_iterator(begins, this);
        }

        /// Get points disjointed of a box
        std::vector<value_type> get_disjoint(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<value_type> v;
            std::copy(find_disjoint(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// Find nearest point
        const_iterator find_nearest(const point_type &p) const {
            // each begin points to the nearest in each front
            std::vector<std::pair<size_t, typename pareto_front_type::const_iterator>> nearests;
            for (size_t i = 0; i < fronts(); ++i) {
                auto it = data_[i].find_nearest(p);
                if (it != data_[i].end()) {
                    nearests.emplace_back(i, it);
                }
            }

            // check which begin has the nearest point
            auto best_it = std::min_element(nearests.begin(), nearests.end(),
                                            [p](const auto &ita, const auto &itb) {
                                                return p.distance(ita.second->first) < p.distance(itb.second->first);
                                            });

            // get only the nearest begin
            std::vector begins = {*best_it};
            return const_iterator(begins, this);
        }

        /// Get nearest point
        std::vector<value_type> get_nearest(const point_type &p) const {
            std::vector<value_type> v;
            std::copy(find_nearest(p), end(), back_inserter(v));
            return v;
        }

        /// Find nearest point excluding itself
        const_iterator find_nearest_exclusive(const point_type &p) const {
            // each begin points to the nearest exclusive point in each front
            std::vector<std::pair<size_t, typename pareto_front_type::const_iterator>> nearest_iters;
            for (size_t i = 0; i < fronts(); ++i) {
                auto it = data_[i].find_nearest_exclusive(p);
                if (it != data_[i].end()) {
                    nearest_iters.emplace_back(i, it);
                }
            }
            // check which begin has the nearest
            auto best_it = std::min_element(nearest_iters.begin(), nearest_iters.end(),
                                            [p](const auto &ita, const auto &itb) {
                                                return p.distance(ita.second->first) < p.distance(itb.second->first);
                                            });
            // get begins
            std::vector begins = {*best_it};
            return const_iterator(begins, this);
        }

        /// Get nearest point excluding itself
        std::vector<value_type> get_nearest_exclusive(const point_type &p) const {
            std::vector<value_type> v;
            std::copy(find_nearest(p), end(), back_inserter(v));
            return v;
        }

        /// Find k nearest points
        const_iterator find_nearest(const point_type &p, size_t k) const {
            // each begin point to the nearest in each front
            std::vector<std::pair<size_t, typename pareto_front_type::const_iterator>> begins;
            for (size_t i = 0; i < fronts(); ++i) {
                auto it = data_[i].find_nearest(p, k);
                if (it != data_[i].end()) {
                    begins.emplace_back(i, it);
                }
            }
            // we now got the k * fronts() nearest with these iterators
            // Let's put all of them in a vector
            std::vector<value_type> v(const_iterator(begins, this), end());
            // Sort these points by distance
            std::partial_sort(v.begin(), v.begin() + k, v.end(), [&p](const value_type &a, const value_type &b) {
                return p.distance(a.first) < p.distance(b.first);
            });
            // Find distance of the k-th closest
            double d = p.distance((v.begin() + k - 1)->first);
            // Create predicate with this as the max distance besides the nearest k predicate
            begins.clear();
            for (size_t i = 0; i < fronts(); ++i) {
                // Iterator to nearest k of each, excluding distances larger than overall k-th
                auto it = typename pareto_front_type::const_iterator(data_[i].data_.begin_nearest(p, k, [p, d](
                        const value_type &v) { return p.distance(v.first) <= d; }));
                if (it != data_[i].end()) {
                    begins.emplace_back(i, it);
                }
            }
            // create concat iterator
            return const_iterator(begins, this);
        }

        /// Get k nearest points
        std::vector<value_type> get_nearest(const point_type &p, size_t k) const {
            std::vector<value_type> v;
            std::copy(find_nearest(p, k), end(), back_inserter(v));
            return v;
        }

        /// \brief Get hypervolume with monte-carlo simulation
        /// This function uses monte-carlo simulation as getting the
        /// exact indicator is too costly.
        /// \param reference_point Reference for the hyper-volume
        /// \param sample_size Number of samples for the simulation
        /// \return Hypervolume of the pareto front
        number_type hypervolume(const point_type &reference_point, size_t sample_size) const {
            return data_[0].hypervolume(reference_point, sample_size);
        }

        /// \brief Get exact hypervolume
        /// Use the other hypervolume function if this takes too long.
        /// \param reference_point Reference point
        /// \return Hypervolume of this front
        number_type hypervolume(point_type reference_point) const {
            return data_[0].hypervolume(reference_point);
        }

        /// \brief Get hypervolume from the nadir point
        /// If there is no specific reference point, we use
        /// the nadir point as reference.
        /// \param reference_point Reference point
        /// \return Hypervolume of this front
        number_type hypervolume() const {
            return data_[0].hypervolume();
        }

        /// Coverage indicator
        /// \see http://www.optimization-online.org/DB_FILE/2018/10/6887.pdf
        double coverage(const archive &rhs) const {
            return data_[0].coverage(rhs.data_[0]);
        }

        /// Coverage indicator
        /// \see http://www.optimization-online.org/DB_FILE/2018/10/6887.pdf
        double coverage(const pareto_front_type &rhs) const {
            return data_[0].coverage(rhs);
        }

        /// Ratio of coverage indicators
        double coverage_ratio(const archive &rhs) const {
            return data_[0].coverage_ratio(rhs.data_[0]);
        }

        /// Ratio of coverage indicators
        double coverage_ratio(const pareto_front_type &rhs) const {
            return data_[0].coverage_ratio(rhs);
        }

        /// Generational distance
        double gd(const pareto_front_type &reference) const {
            return data_[0].gd(reference);
        }

        /// Generational distance
        double gd(const archive &reference) const {
            return data_[0].gd(reference.data_[0]);
        }

        /// Standard deviation from the generational distance
        /// It measures the deformation of the Pareto set
        /// approximation according to a Pareto optimal solution set.
        double std_gd(const pareto_front_type &reference) const {
            return data_[0].std_gd(reference);
        }

        /// Standard deviation from the generational distance
        /// It measures the deformation of the Pareto set
        /// approximation according to a Pareto optimal solution set.
        double std_gd(const archive &reference) const {
            return data_[0].std_gd(reference.data_[0]);
        }

        /// Inverted generational distance
        double igd(const pareto_front_type &reference) const {
            return data_[0].igd(reference);
        }

        /// Inverted generational distance
        double igd(const archive &reference) const {
            return data_[0].igd(reference.data_[0]);
        }

        /// Standard deviation from the inverted generational distance
        double std_igd(const pareto_front_type &reference) const {
            return data_[0].std_igd(reference);
        }

        /// Standard deviation from the inverted generational distance
        double std_igd(const archive &reference) const {
            return data_[0].std_igd(reference.data_[0]);
        }

        double hausdorff(const pareto_front_type &reference) const {
            return data_[0].hausdorff(reference);
        }

        double hausdorff(const archive &reference) const {
            return data_[0].hausdorff(reference.data_[0]);
        }

        double igd_plus(const pareto_front_type &reference_front) const {
            return data_[0].igd_plus(reference_front);
        }

        double igd_plus(const archive &reference_front) const {
            return data_[0].igd_plus(reference_front.data_[0]);
        }

        double std_igd_plus(const pareto_front_type &reference_front) const {
            return data_[0].std_igd_plus(reference_front);
        }

        double std_igd_plus(const archive &reference_front) const {
            return data_[0].std_igd_plus(reference_front.data_[0]);
        }

        /// Uniformity metric
        /// This is the minimal distance between two points of the Pareto front
        /// approximation. This measure is straightforward to compute and easy
        /// to understand. However, it does not really provide pertinent
        /// information on the repartition of the points along the Pareto
        /// front approximation.
        double uniformity() {
            return data_[0].uniformity();
        }

        double average_distance() {
            return data_[0].average_distance();
        }

        double average_nearest_distance(size_t k = 5) {
            return data_[0].average_nearest_distance(k);
        }

        double average_crowding_distance() {
            return data_[0].average_crowding_distance();
        }

        double crowding_distance(const_iterator element, point_type worst_point, point_type ideal_point) {
            return data_[element.current_front_].crowding_distance(element.current_iter_, worst_point, ideal_point);
        }

        double crowding_distance(const_iterator element) {
            return data_[element.current_front_].crowding_distance(element.current_iter_, worst(), ideal());
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

        [[nodiscard]] double normalized_direct_conflict(const size_t a, const size_t b) const {
            // max value in each term is max_a-min_a or max_b-min_b
            number_type worst_a = worst(a);
            number_type worst_b = worst(b);
            number_type ideal_a = ideal(a);
            number_type ideal_b = ideal(b);
            number_type range_a = is_minimization(a) ? worst_a - ideal_a : ideal_a - worst_a;
            number_type range_b = is_minimization(b) ? worst_b - ideal_b : ideal_b - worst_b;
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
        [[nodiscard]] double maxmin_conflict(const size_t a, const size_t b) const {
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

        [[nodiscard]] double normalized_maxmin_conflict(const size_t a, const size_t b) const {
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
        [[nodiscard]] double conflict(const size_t a, const size_t b) const {
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

        [[nodiscard]] double normalized_conflict(const size_t a, const size_t b) const {
            double denominator = 0.;
            auto n = static_cast<double>(size());
            for (size_t i = 1; i <= size(); ++i) {
                denominator += abs(2.*i-n-1);
            }
            return static_cast<double>(conflict(a,b))/denominator;
        }


        /// \brief Check if this front weakly dominates a point
        /// A front a weakly dominates a solution b if a has at least
        /// one solution better than b in at least one objective and
        /// is at least as good as b in all other objectives.
        /// \see http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool dominates(const point_type &p) const {
            return data_[0].dominates(p);
        }

        /// \brief Check if this front strongly dominates a point
        /// A front a strongly dominates a solution b if a has a solution
        /// that is strictly better than b in all objectives.
        bool strongly_dominates(const point_type &p) const {
            return data_[0].strongly_dominates(p);
        }

        /// \brief Check if this front weakly dominates a point
        /// A solution a weakly dominates a solution b if a is better
        /// than b in at least one objective and is as good as b in
        /// all other objectives.
        /// \see http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool non_dominates(const point_type &p) const {
            return data_[0].non_dominates(p);
        }

        bool is_partially_dominated_by(const point_type &p) const {
            return data_[0].is_partially_dominated_by(p);
        }

        bool is_completely_dominated_by(const point_type &p) const {
            return data_[0].is_completely_dominated_by(p);
        }

        /// Check if this front dominates another front
        bool dominates(const pareto_front_type &p) const {
            return data_[0].dominates(p);
        }

        /// Check if this front dominates another front
        bool dominates(const self_type &p) const {
            return data_[0].dominates(p.data_[0]);
        }

        /// Check if this front weakly dominates another front
        bool strongly_dominates(const pareto_front_type &p) const {
            return data_[0].strongly_dominates(p);
        }

        /// Check if this front weakly dominates another front
        bool strongly_dominates(const self_type &p) const {
            return data_[0].strongly_dominates(p.data_[0]);
        }

        /// Check if this front weakly dominates another front
        bool non_dominates(const pareto_front_type &p) const {
            return data_[0].non_dominates(p);
        }

        /// Check if this front weakly dominates another front
        bool non_dominates(const self_type &p) const {
            return data_[0].non_dominates(p.data_[0]);
        }

        /// Check if this front weakly dominates another front
        bool is_partially_dominated_by(const pareto_front_type &p) const {
            return data_[0].is_partially_dominated_by(p);
        }

        /// Check if this front weakly dominates another front
        bool is_partially_dominated_by(const self_type &p) const {
            return data_[0].is_partially_dominated_by(p.data_[0]);
        }

        /// Check if this front weakly dominates another front
        bool is_completely_dominated_by(const pareto_front_type &p) const {
            return data_[0].is_completely_dominated_by(p);
        }

        /// Check if this front weakly dominates another front
        bool is_completely_dominated_by(const self_type &p) const {
            return data_[0].is_completely_dominated_by(p.data_[0]);
        }

        point_type ideal() const {
            return data_[0].ideal();
        }

        number_type ideal(size_t d) const {
            return data_[0].ideal(d);
        }

        const_iterator dimension_ideal(size_t d) const {
            std::vector<std::pair<size_t, pareto_front_const_iterator>> begins;
            begins.emplace_back(0, data_[0].dimension_ideal(d));
            return const_iterator(begins, this);
        }

        /// The nadir point is the worst point among the
        /// non-dominated points. There is a difference
        /// between the nadir point and the worst point
        /// for archives.
        point_type nadir() const {
            return data_[0].nadir();
        }

        number_type nadir(size_t d) const {
            return data_[0].nadir(d);
        }

        const_iterator dimension_nadir(size_t d) const {
            return const_iterator(this, 0, data_[0].dimension_nadir(d));
        }

        point_type worst() const {
            point_type worst_point = data_[0].worst();
            for (size_t i = 1; i < data_.size(); ++i) {
                point_type tmp = data_[i].worst();
                for (size_t j = 0; j < data_[i].dimensions(); ++j) {
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

        number_type worst(size_t d) const {
            number_type worst_value = data_[0].worst(d);
            for (size_t i = 1; i < data_.size(); ++i) {
                number_type w = data_[i].worst(d);
                if (is_minimization(i)) {
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

        const_iterator dimension_worst(size_t d) const {
            auto worst_it = data_[0].dimension_worst(d);
            size_t worst_front = 0;
            for (size_t i = 1; i < data_.size(); ++i) {
                auto it = data_[i].dimension_worst(d);
                const bool is_worse = is_minimization(d) ?
                           it->first[d] > worst_it->first[d] :
                           it->first[d] < worst_it->first[d];
                if (is_worse) {
                    worst_it = it;
                    worst_front = i;
                }
            }
            std::vector<std::pair<size_t, pareto_front_const_iterator>> begins;
            begins.emplace_back(worst_front, worst_it);
            return const_iterator(begins, this);
        }

        const_iterator find(const key_type &k) const {
            for (size_t i = 0; i < fronts(); ++i) {
                auto it = data_[i].find(k);
                if (it != data_[i].end()) {
                    std::vector<std::pair<size_t, pareto_front_const_iterator>> begins;
                    begins.emplace_back(i, it);
                    return const_iterator(begins, this);
                }
            }
            return end();
        }

        iterator find(const key_type &k) {
            for (size_t i = 0; i < fronts(); ++i) {
                auto it = data_[i].find(k);
                if (it != data_[i].end()) {
                    std::vector<std::pair<size_t, pareto_front_iterator>> begins;
                    begins.emplace_back(i, it);
                    return iterator(begins, this);
                }
            }
            return end();
        }

        bool contains(const key_type &k) const {
            return find(k) != end();
        }

        friend std::ostream &operator<<(std::ostream &os, const archive &ar) {
            os << "Archive front (" << ar.size() << " elements - {";
            for (size_t i = 0; i < ar.is_minimization_.size() - 1; ++i) {
                os << (ar.is_minimization_[i] ? "minimization" : "maximization") << ", ";
            }
            os << (ar.is_minimization_.back() ? "minimization" : "maximization") << "})";
            return os;
        }

#ifdef BUILD_FRONTS_WITH_TRASE
        std::string svg(std::optional<point_type> extra_point = std::nullopt,
                        std::optional<const_iterator> it = std::nullopt) const {
            std::shared_ptr<trase::Figure> fig = trase::figure();
            point_type min_point = ideal();
            point_type worst_point = worst();
            point_type max_point = worst_point;
            normalize_corners(min_point, max_point);
            for (size_t i = 0; i < data_.size(); ++i) {
                float point_color = static_cast<float>(i)/1.4/data_.size();
                if (it) {
                    auto pareto_it = std::find_if(it->begins_.begin(), it->begins_.end(), [&i](const auto& begin) {
                        return begin.first == i;
                    });
                    if (pareto_it != it->begins_.end()) {
                        if (i == 0) {
                            data_[i].setup_trase_figure(fig, min_point, max_point, worst_point, true, true, true, point_color, extra_point, pareto_it->second);
                        } else {
                            data_[i].setup_trase_figure(fig, min_point, max_point, worst_point, false, false, false, point_color, std::nullopt, pareto_it->second);
                        }
                        continue;
                    }
                }
                if (i == 0) {
                    data_[i].setup_trase_figure(fig, min_point, max_point, worst_point, true, true, true, point_color, extra_point);
                } else {
                    data_[i].setup_trase_figure(fig, min_point, max_point, worst_point, false, false, false, point_color);
                }
            }
            std::stringstream ss;
            trase::BackendSVG backend(ss);
            fig->draw(backend);
            return ss.str();
        }
#endif

    private /* functions */:
//        /// \brief Clear solutions are dominated by p
//        /// Pareto-optimal front is the set F consisting of
//        /// all non-dominated solutions x in the whole
//        /// search space. No solution can dominate another
//        /// solution. Note that this means two solutions
//        /// might have the same values though.
//        size_t clear_dominated(point_type p) {
//            // The modification of the rtree may invalidate the iterators
//            // https://www.boost.org/doc/libs/1_60_0/libs/geometry/doc/html/geometry/reference/spatial_indexes/boost__geometry__index__rtree/begin__.html
//            // Remove doesn't take iterators pointing to values
//            // Remove removes values equal to these passed as a range
//            // https://www.boost.org/doc/libs/1_73_0/libs/geometry/doc/html/geometry/reference/spatial_indexes/group__rtree__functions/remove_rtree_________iterator__iterator_.html
//            // We have to make only one query and get a copy of all ITEMS (not iterators)
//            // we want to remove.
//            if (!empty()) {
//                // Query
//                const_iterator it = data_.begin_intersection(p, worst(), [&p, this](const value_type &x) {
//                    return p.dominates(x.first, is_minimization_);
//                });
//                // erase these elements
//                return erase(it, end());
//            }
//            return 0;
//        }
//
//        /// \brief Clear all solutions dominated by some point
//        size_t clear_dominated() {
//            // The modification of the rtree may invalidate the iterators
//            // https://www.boost.org/doc/libs/1_60_0/libs/geometry/doc/html/geometry/reference/spatial_indexes/boost__geometry__index__rtree/begin__.html
//            // Remove doesn't take iterators pointing to values
//            // Remove removes values equal to these passed as a range
//            // https://www.boost.org/doc/libs/1_73_0/libs/geometry/doc/html/geometry/reference/spatial_indexes/group__rtree__functions/remove_rtree_________iterator__iterator_.html
//            // We have to make only one query and get a copy of all ITEMS (not iterators)
//            // we want to remove.
//            if (size() > 1) {
//                // Query everyone
//                // Put them all in a separate list
//                // because iterators will be invalidated
//                auto it = find_intersection(ideal(), worst());
//                std::vector<value_type> all(it, end());
//                // Iterate removing points they dominate
//                size_t sum_removed = 0;
//                for (const auto&[k, v]: all) {
//                    // if k hasn't been removed yet
//                    if (find(k) != end()) {
//                        sum_removed += clear_dominated(k);
//                    }
//                }
//                return sum_removed;
//            }
//            return 0;
//        }

        void maybe_resize(std::array<uint8_t, number_of_compile_dimensions> &v [[maybe_unused]], size_t n [[maybe_unused]]) {}

        void maybe_resize(std::vector<uint8_t> &v, size_t n) {
            v.resize(n);
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

    private:
        /// \brief Pareto fronts with archive solutions
        std::vector<pareto_front_type> data_;

        /// \brief Whether each dimension is minimization or maximization
        /// We use uint8_t because bool to avoid the array specialization
        internal_minimization_type is_minimization_;

        /// Max number of elements in the archive
        size_t max_size_{1000};

        /// Allocator the pareto fronts will share
        std::shared_ptr<node_allocator_type> alloc_;

    };

}


#endif //PARETO_FRONT_ARCHIVE_H
