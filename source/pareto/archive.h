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
    public /* types */:
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
        /// \class Archive iterator
        /// The archive iterator includes the begin iterator of each
        /// pareto front we want to iterate
        /// \tparam is_const
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

            /// \brief Default constructor for convenience
            iterator_impl() : begins_(0), ar_(nullptr), current_iter_(), current_front_(0) {}

            /// \brief Construct an archive iterator for all elements
            /// \param begins Begin iterator of each internal pareto front
            /// \param ar Pointer to the archive
            /// \param current_iter Iterator to the current element in the current pareto front
            /// \param current_front Front with the current element
            iterator_impl(const std::vector<pareto_iterator> &begins, archive_pointer ar, pareto_iterator current_iter,
                          size_t current_front)
                    : ar_(ar), current_iter_(current_iter), current_front_(current_front) {
                for (size_t i = 0; i < begins.size(); ++i) {
                    begins_.emplace_back(i, begins[i]);
                }
                advance_to_next_valid();
            }

            /// \brief Construct from limited set of begins
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

            /// \brief Construct from a list of begins. Current iterator is set to first element of first front.
            /// \param begins Begin iterator of each front
            /// \param ar Pointer to the archive
            iterator_impl(const std::vector<pareto_iterator> &begins, archive_pointer ar)
                    : iterator_impl(begins, ar, !begins.empty() ? begins[0] : ar->data_.back().end(), 0) {}

            /// \brief Construct from a front iterator and its index in the archive.
            iterator_impl(archive_pointer ar, size_t front_index, pareto_iterator current_iter)
                    : iterator_impl(std::vector<std::pair<size_t, pareto_iterator>>{{front_index, current_iter}}, ar) {}

            /// \brief Construct from a subset of begins (list of pair<front index, begin iterator>)
            /// \param begins
            /// \param ar
            iterator_impl(const std::vector<std::pair<size_t, pareto_iterator>> &begins, archive_pointer ar)
                    : iterator_impl(begins, ar, !begins.empty() ? begins[0].second : ar->data_.back().end(), 0) {}

            /// \brief Construct from archive pointer
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

            /// \brief Copy constructor if this is const_iterator other is non-const iterator (iterator_impl<false>)
            template<bool _constness = is_const>
            // NOLINTNEXTLINE(google-explicit-constructor): Iterators should be implicitly convertible
            iterator_impl(
                    const std::enable_if_t<_constness, iterator_impl<false>> &rhs)
                    : ar_(rhs.ar_), current_iter_(rhs.current_iter_), current_front_(rhs.current_front_) {
                for (auto&[index, it]: rhs.begins_) {
                    begins_.emplace_back(index, pareto_iterator(it));
                }
                advance_to_next_valid();
            }

            /// \brief Copy constructor if this is non-const iterator (iterator_impl<false>) other is non-const iterator (iterator_impl<false>)
            template<bool _constness = is_const>
            // NOLINTNEXTLINE(google-explicit-constructor): Iterators should be implicitly convertible
            iterator_impl(
                    const std::enable_if_t<!_constness, iterator_impl<false>> &rhs)
                    : begins_(rhs.begins_), ar_(rhs.ar_), current_iter_(rhs.current_iter_),
                      current_front_(rhs.current_front_) {
                advance_to_next_valid();
            }

            /// \brief Copy constructor if other is const_iterator
            /// SFINAE: "this" cannot be be iterator (iterator_impl<false>)
            template<bool _constness = is_const>
            // NOLINTNEXTLINE(google-explicit-constructor): Iterators should be implicitly convertible
            iterator_impl(
                    const std::enable_if_t<_constness, iterator_impl<true>> &rhs)
                    : begins_(rhs.begins_), ar_(rhs.ar_), current_iter_(rhs.current_iter_),
                      current_front_(rhs.current_front_) {
                advance_to_next_valid();
            }

            /// \brief Destructor
            ~iterator_impl() = default;

            /// \brief Copy assignment
            template<bool constness>
            iterator_impl &operator=(const iterator_impl<constness> &rhs) {
                begins_ = rhs.begins_;
                ar_ = rhs.ar_;
                current_iter_ = rhs.current_iter_;
                current_front_ = rhs.current_front_;
                advance_to_next_valid();
                return *this;
            }

            /// \brief Equality comparison
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

            /// \brief Inequality comparison
            bool operator!=(const iterator_impl &rhs) const {
                return !(this->operator==(rhs));
            }

            /// \brief Advance iterator
            iterator_impl &operator++() {
                current_iter_.operator++();
                advance_to_next_valid();
                return *this;
            }

            /// \brief Make copy and advance iterator
            iterator_impl operator++(int) { // NOLINT(cert-dcl21-cpp): This is the expected return type for iterators
                auto tmp = *this;
                current_iter_.operator++();
                advance_to_next_valid();
                return tmp;
            }

            /// \brief Go to previous element
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

            /// \brief Make copy and go to previous element
            iterator_impl operator--(int) { // NOLINT(cert-dcl21-cpp): This is the expected return type for iterators
                auto tmp = *this;
                return_to_previous_valid();
                return tmp;
            }

            /// \brief Dereference element
            reference operator*() const {
                return current_iter_.operator*();
            }

            /// \brief Dereference element
            pointer operator->() const {
                return current_iter_.operator->();
            }

        private:
            /// \brief True if this is an iterator to end
            [[nodiscard]] bool is_end() const {
                if (!ar_ || begins_.empty()) {
                    return true;
                }
                if (current_front_ == begins_.size()) {
                    return true;
                }
                return false;
            }

            /// \brief True if this is an iterator to first element
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

            /// \brief Advance to the next valid element
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

            /// \brief Go to the previous valid element
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

            /// \brief Pointer to the archive to which this iterator belongs
            archive_pointer ar_;

            /// \brief Current element we are iterating
            pareto_iterator current_iter_;

            /// \brief Current front from the begins list we are iterating
            size_t current_front_{0};
        };

        using iterator = iterator_impl<false>;
        using const_iterator = iterator_impl<true>;

    private:
        /*
        /// Is the internal type using the fast allocator
        constexpr static bool is_using_default_fast_allocator() {
            return pareto_front_type::is_using_default_fast_allocator();
        }
        */

    public /* constructors */:
        /// \brief Create a pareto archive with max_size from value range and is_minimization range
        /// This is the constructor all other constructors use as reference
        /// They are all just shortcuts to this constructor
        /// \tparam ValueIterator Iterator to values (pair of point<number> and element)
        /// \tparam IsMinimizationIterator Iterator to minimization directions (bool, or uint8_t)
        /// \param max_size Maximum number of elements in the archive
        /// \param value_begin Iterator to first pareto front candidate
        /// \param value_end Iterator to last + 1 pareto front candidate
        /// \param is_minimization_begin Iterator to first minimization direction
        /// \param is_minimization_end Iterator to last + 1 minimization direction
        template<class ValueIterator, class IsMinimizationIterator>
        archive(size_t max_size,
                ValueIterator value_begin,
                ValueIterator value_end,
                IsMinimizationIterator is_minimization_begin,
                IsMinimizationIterator is_minimization_end)
                : data_{}, is_minimization_{}, max_size_(max_size), alloc_(new node_allocator_type()) {
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

        /*
         * Constructors that provide elements and minimization directions
         */

        /// \brief Construct a pareto archive from a list of value pairs and determine whether each dimension is minimization
        template<class InputIterator>
        archive(size_t max_size, InputIterator first, InputIterator last, const std::vector<uint8_t> &is_minimization)
                : archive(max_size, first, last, is_minimization.begin(), is_minimization.end()) {}

        /// \brief Construct a pareto archive from a list of value pairs and determine whether each dimension is minimization
        template<typename IsMinimizationIterator>
        archive(size_t max_size, std::initializer_list<value_type> il,
                IsMinimizationIterator is_minimization_begin,
                IsMinimizationIterator is_minimization_end)
                : archive(max_size, il.begin(), il.end(), is_minimization_begin, is_minimization_end) {}

        /// \brief Construct a pareto archive from a list of value pairs and determine whether each dimension is minimization
        archive(size_t max_size, const std::vector<value_type> &v, const std::vector<uint8_t> &is_minimization)
                : archive(max_size, v.begin(), v.end(), is_minimization.begin(), is_minimization.end()) {}

        /// \brief Construct a pareto archive from a list of value pairs and determine whether each dimension is minimization
        archive(size_t max_size, const std::vector<value_type> &v, std::initializer_list<bool> is_minimization)
                : archive(max_size, v.begin(), v.end(), is_minimization.begin(), is_minimization.end()) {}

        /// \brief Construct a pareto archive from a list of value pairs and determine whether each dimension is minimization
        archive(size_t max_size, std::initializer_list<value_type> il, std::initializer_list<bool> is_minimization)
                : archive(max_size, il.begin(), il.end(), is_minimization.begin(), is_minimization.end()) {}

        /// \brief Construct a pareto archive from a list of value pairs and determine whether each dimension is minimization
        archive(size_t max_size, std::initializer_list<value_type> il, const std::vector<uint8_t> &is_minimization)
                : archive(max_size, il.begin(), il.end(), is_minimization) {}

        /*
         * Constructors that provide elements and a single minimization direction
         */

        /// \brief Construct a pareto archive from a list of value pairs and determine whether it is minimization
        template<class InputIterator>
        archive(size_t max_size, InputIterator first, InputIterator last, bool is_minimization)
                : archive(max_size, first, last,
                          std::vector<uint8_t>(std::max(number_of_compile_dimensions, size_t(1)), is_minimization)) {}

        /// \brief Construct a pareto archive from a list of value pairs and determine whether it is minimization
        archive(size_t max_size, const std::vector<value_type> &v, bool is_minimization)
                : archive(max_size, v.begin(), v.end(), is_minimization) {}

        /// \brief Construct a pareto archive from a list of value pairs and determine whether it is minimization
        archive(size_t max_size, std::initializer_list<value_type> il, bool is_minimization)
                : archive(max_size, il.begin(), il.end(), is_minimization) {}

        /*
         * Constructors that provide elements only
         */

        /// \brief Construct a pareto archive from a list of value pairs
        template<class InputIterator>
        archive(size_t max_size, InputIterator first, InputIterator last)
                : archive(max_size, first, last, true) {}

        /// \brief Construct a pareto archive from a list of value pairs
        archive(size_t max_size, const std::vector<value_type> &v)
                : archive(max_size, v.begin(), v.end(), true) {}

        /// \brief Construct a pareto archive from a list of value pairs
        explicit archive(size_t max_size, std::initializer_list<value_type> il)
                : archive(max_size, il.begin(), il.end(), true) {}

        /*
         * Constructors that provide minimization directions only
         */

        /// \brief Construct an empty archive and determine whether each dimension is minimization
        explicit archive(size_t max_size, const std::vector<uint8_t> &is_minimization)
                : archive(max_size, {}, is_minimization.begin(), is_minimization.end()) {}

        /// \brief Construct an empty archive and determine whether each dimension is minimization
        explicit archive(size_t max_size, const std::array<uint8_t, number_of_compile_dimensions> &is_minimization)
                : archive(max_size, {}, is_minimization.begin(), is_minimization.end()) {}

        /// \brief Construct an empty archive and determine whether each dimension is minimization
        explicit archive(size_t max_size, std::initializer_list<bool> is_minimization)
                : archive(max_size, {}, is_minimization.begin(), is_minimization.end()) {}

        /// \brief Construct an empty archive for max_size elements and determine whether directions are minimization
        explicit archive(size_t max_size, bool is_minimization)
                : archive(max_size,
                          std::vector<uint8_t>(std::max(number_of_compile_dimensions, size_t(1)), is_minimization)) {}

        /*
         * Constructors that don't provide elements or directions
         */

        /// \brief Empty minimization archive with space for max_size elements
        explicit archive(size_t max_size) : archive(max_size, true) {}

        /// \brief Empty archive with space for 1000 elements
        archive() : archive(1000) {}


        /// \brief Copy constructor
        archive(const archive &rhs) {
            data_ = rhs.data_;
            alloc_ = rhs.alloc_;
            is_minimization_ = rhs.is_minimization_;
            max_size_ = rhs.max_size_;
        }

        /// \brief Copy assignment
        archive &operator=(const archive &rhs) {
            alloc_ = rhs.alloc_;
            data_ = rhs.data_;
            is_minimization_ = rhs.is_minimization_;
            max_size_ = rhs.max_size_;
            return *this;
        }

        /// \brief Move constructor
        archive(archive &&m) noexcept {
            alloc_ = m.alloc_;
            data_ = std::move(m.data_);
            is_minimization_ = std::move(m.is_minimization_);
            max_size_ = std::move(m.max_size_);
        }

        /// \brief Move assignment
        archive &operator=(archive &&rhs) noexcept {
            alloc_ = rhs.alloc_;
            data_ = std::move(rhs.data_);
            is_minimization_ = std::move(rhs.is_minimization_);
            max_size_ = std::move(rhs.max_size_);
            return *this;
        }

    public /* iterators */:
        /// \brief Get an iterator to first point
        const_iterator begin() const noexcept {
            return const_iterator(this);
        }

        /// \brief Get an iterator to last + 1 point
        const_iterator end() const noexcept {
            return const_iterator(std::vector<std::pair<size_t, typename pareto_front_type::const_iterator>>(), this,
                                  !data_.empty() ? data_.back().end() : typename pareto_front_type::const_iterator(),
                                  data_.size());
        }

        /// \brief Get an iterator to first point
        iterator begin() noexcept {
            return iterator(this);
        }

        /// \brief Get an iterator to last + 1 point
        iterator end() noexcept {
            return iterator(std::vector<std::pair<size_t, typename pareto_front_type::iterator>>(), this,
                            !data_.empty() ? data_.back().end() : typename pareto_front_type::iterator(), data_.size());
        }

        /// \brief Get a reverse iterator to first point
        std::reverse_iterator<const_iterator> rbegin() const noexcept {
            return std::reverse_iterator(end());
        }

        /// \brief Get a reverse iterator to last + 1 point
        std::reverse_iterator<const_iterator> rend() const noexcept {
            return std::reverse_iterator(begin());
        }

        /// \brief Get a reverse iterator to first point
        std::reverse_iterator<iterator> rbegin() noexcept {
            return std::reverse_iterator(end());
        }

        /// \brief Get a reverse iterator to last + 1 point
        std::reverse_iterator<iterator> rend() noexcept {
            return std::reverse_iterator(begin());
        }

        /// \brief Get an iterator to first front
        typename std::vector<pareto_front_type>::const_iterator
        begin_front() const noexcept {
            return data_.begin();
        }

        /// \brief Get an iterator to the last + 1 front
        typename std::vector<pareto_front_type>::const_iterator
        end_front() const noexcept {
            return data_.end();
        }

        /// \brief Get an iterator to first front
        typename std::vector<pareto_front_type>::iterator
        begin_front() noexcept {
            return data_.begin();
        }

        /// \brief Get an iterator to the last + 1 front
        typename std::vector<pareto_front_type>::iterator
        end_front() noexcept {
            return data_.end();
        }

    public /* capacity */:
        /// \brief Check if archive is empty
        [[nodiscard]] bool empty() const noexcept {
            return data_.empty() ||
                    std::all_of(data_.begin(), data_.end(), [](const pareto_front_type &pf) { return pf.empty(); });
        }

        /// \brief Get archive size
        size_type size() const noexcept {
            size_t s = 0;
            for (const auto &pf: data_) {
                s += pf.size();
            }
            return s;
        }

        /// \brief Get maximum archive size
        size_type max_size() const noexcept {
            return max_size_;
        }

        /// \brief Get number of fronts in the archive
        size_type fronts() const noexcept {
            return data_.size();
        }

        /// \brief Get number of dimensions of archive points
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

    public /* element access */:
        /// \brief Access point (create point if it doesn't exist)
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

        /// \brief Access point (create point if it doesn't exist)
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

        /// \brief Access point (create point if it doesn't exist)
        template<typename... Targs>
        mapped_type &operator()(const number_type &k, const Targs &... ks) {
            constexpr size_t m = sizeof...(Targs) + 1;
            assert(number_of_compile_dimensions == 0 || number_of_compile_dimensions == m);
            point_type p(m);
            copy_pack(p.begin(), k, ks...);
            return operator[](p);
        }

        /// \brief Access point (throw error if it doesn't exist)
        mapped_type &at(const key_type &k) {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("archive::at:  key not found");
            }
        }

        /// \brief Access point (throw error if it doesn't exist)
        const mapped_type &at(const key_type &k) const {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("archive::at:  key not found");
            }
        }

    public /* relational operators */:
        /// \brief Equality operator
        bool operator==(const archive &rhs) const {
            return data_ == rhs.data_ &&
                   is_minimization_ == rhs.is_minimization_ &&
                   max_size_ == rhs.max_size_;
        }

        /// \brief Inequality operator
        bool operator!=(const archive &rhs) const {
            return !(this->operator==(rhs));
        }

    public /* modifiers */:
        /// \brief Emplace element
        /// Emplace becomes insert becomes the rtree does not have
        /// an emplace function
        template<class... Args>
        std::pair<iterator, bool> emplace(Args &&... args) {
            auto v = value_type(args...);
            return insert(v);
        }

        /// \brief Emplace elements
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
            return try_insert(find_front(v.first), v);
        }

        /// \brief Try to insert the value v in one of the fronts from P^{lower bound} to P^{|A|}
        /// \param front_lower_bound_idx Index of the first front that might received v
        /// \param v Value to insert
        /// \return Iterator to the new value
        std::pair<iterator, bool> try_insert(size_t front_idx, const value_type &v) {
            const bool front_is_valid = front_idx < data_.size();
            if (front_is_valid) {
                // Get all points this solution dominates in front i
                auto dominated_it = data_[front_idx].find_dominated(v.first);

                // Copy dominated solutions to vector
                std::vector<value_type> dominated_solutions;
                typename pareto_front_type::const_iterator front_end(data_[front_idx].end());
                std::copy(dominated_it, front_end, std::back_inserter(dominated_solutions));

                // Remove dominated solutions from the current front
                data_[front_idx].erase(dominated_it, front_end);

                // Recursively insert dominated solutions into the next front
                // Should we make this algorithm iterative?
                // The front might be |A| = n in the worst case,
                // so calling this too many times might cause a problem
                for (const auto &v2: dominated_solutions) {
                    try_insert(front_idx + 1, v2);
                }

                // Insert v in this front
                auto[pfit, ok] = data_[front_idx].insert(v);

                // Create archive iterator to this new solution
                std::vector<std::pair<size_t, pareto_front_iterator>> begins;
                begins.emplace_back(front_idx, pfit);
                iterator it2 = iterator(begins, this);

                // If inserting v made the archive exceed its max size
                if (size() > max_size()) {
                    resize(max_size());

                    // Fix iterator if invalidated
                    // Iterator might be invalidated
                    // New item might even have been removed
                    // Look for item again to fix it
                    auto it3 = find(v.first);
                    return std::make_pair(it3, it3 != end());
                }
                return std::make_pair(it2, true);
            } else {
                // If all fronts dominate v or there are no fronts yet
                bool there_is_space_in_the_archive = size() < max_size();
                if (there_is_space_in_the_archive) {
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

        /// \brief Erase element from the archive
        /// \param v Point
        size_type erase(const key_type &point) {
            return try_erase(find_front(point), point);
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
            size_t n_erased = data_[front_idx].erase(point);
            if (n_erased == 0) {
                return 0;
            }

            const bool front_became_empty = data_[front_idx].empty();
            if (front_became_empty) {
                data_.erase(data_.begin() + front_idx);
            } else if (front_idx + 1 < data_.size()) {
                // Some elements from next front might not be dominated now
                auto previously_dominated_it = data_[front_idx + 1].find_dominated(point);
                typename pareto_front_type::const_iterator it_end(data_[front_idx + 1].end());

                // Copy these points because erasing them would invalidate iterators
                std::vector<value_type> previously_dominated;
                std::copy(previously_dominated_it, it_end, std::back_inserter(previously_dominated));

                // Move these elements to this front
                for (const auto &v: previously_dominated) {
                    if (!data_[front_idx].dominates(v.first)) {
                        data_[front_idx].insert(v);
                        try_erase(front_idx + 1, v.first);
                    }
                }
            }
            return n_erased;
        }

        /// \brief Erase element pointed by iterator from the front
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

        /// \brief Erase element pointed by iterator from the front
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

        /// \brief Remove range of iterators from the front
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

        /// \brief Remove range of iterators from the front
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

        /// \brief Merge two fronts
        void merge(pareto_front_type &source) {
            insert(source.begin(), source.end());
        }

        /// \brief Merge and move fronts
        void merge(pareto_front_type &&source) {
            insert(source.begin(), source.end());
            source.clear();
        }

        /// \brief Swap the content of two fronts
        void swap(self_type &m) {
            m.data_.swap(data_);
            m.is_minimization_.swap(is_minimization_);
            std::swap(max_size_, m.max_size_);
        }

    public /* pareto operations */:
        /// \brief Find points in a hyperbox
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

        /// \brief Get points in a hyperbox
        std::vector<value_type> get_intersection(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<value_type> v;
            std::copy(find_intersection(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// \brief Find points within a hyperbox (intersection minus borders)
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

        /// \brief Get points within a hyperbox
        std::vector<value_type> get_within(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<value_type> v;
            std::copy(find_within(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// \brief Find points disjointed of a hyperbox (intersection - borders)
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

        /// \brief Get points disjointed of a hyperbox
        std::vector<value_type> get_disjoint(const point_type &min_corner, const point_type &max_corner) const {
            std::vector<value_type> v;
            std::copy(find_disjoint(min_corner, max_corner), end(), back_inserter(v));
            return v;
        }

        /// \brief Find nearest point
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

        /// \brief Get nearest point
        std::vector<value_type> get_nearest(const point_type &p) const {
            std::vector<value_type> v;
            std::copy(find_nearest(p), end(), back_inserter(v));
            return v;
        }

        /// \brief Find nearest point excluding itself
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

        /// \brief Get nearest point excluding itself
        std::vector<value_type> get_nearest_exclusive(const point_type &p) const {
            std::vector<value_type> v;
            std::copy(find_nearest(p), end(), back_inserter(v));
            return v;
        }

        /// \brief Find k nearest points
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

        /// \brief Get k nearest points
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

        /// \brief Coverage indicator
        /// \see http://www.optimization-online.org/DB_FILE/2018/10/6887.pdf
        double coverage(const archive &rhs) const {
            return data_[0].coverage(rhs.data_[0]);
        }

        /// \brief Coverage indicator
        /// \see http://www.optimization-online.org/DB_FILE/2018/10/6887.pdf
        double coverage(const pareto_front_type &rhs) const {
            return data_[0].coverage(rhs);
        }

        /// \brief Ratio of coverage indicators
        double coverage_ratio(const archive &rhs) const {
            return data_[0].coverage_ratio(rhs.data_[0]);
        }

        /// \brief Ratio of coverage indicators
        double coverage_ratio(const pareto_front_type &rhs) const {
            return data_[0].coverage_ratio(rhs);
        }

        /// \brief Generational distance
        double gd(const pareto_front_type &reference) const {
            return data_[0].gd(reference);
        }

        /// \brief Generational distance
        double gd(const archive &reference) const {
            return data_[0].gd(reference.data_[0]);
        }

        /// \brief Standard deviation from the generational distance
        /// It measures the deformation of the Pareto set
        /// approximation according to a Pareto optimal solution set.
        double std_gd(const pareto_front_type &reference) const {
            return data_[0].std_gd(reference);
        }

        /// \brief Standard deviation from the generational distance
        /// It measures the deformation of the Pareto set
        /// approximation according to a Pareto optimal solution set.
        double std_gd(const archive &reference) const {
            return data_[0].std_gd(reference.data_[0]);
        }

        /// \brief Inverted generational distance
        double igd(const pareto_front_type &reference) const {
            return data_[0].igd(reference);
        }

        /// \brief Inverted generational distance
        double igd(const archive &reference) const {
            return data_[0].igd(reference.data_[0]);
        }

        /// \brief Standard deviation from the inverted generational distance
        double std_igd(const pareto_front_type &reference) const {
            return data_[0].std_igd(reference);
        }

        /// \brief Standard deviation from the inverted generational distance
        double std_igd(const archive &reference) const {
            return data_[0].std_igd(reference.data_[0]);
        }

        /// \brief Hausdorff indicator: max(GD,IGD)
        double hausdorff(const pareto_front_type &reference) const {
            return data_[0].hausdorff(reference);
        }

        /// \brief Hausdorff indicator: max(GD,IGD)
        double hausdorff(const archive &reference) const {
            return data_[0].hausdorff(reference.data_[0]);
        }

        /// \brief IGD+ indicator
        double igd_plus(const pareto_front_type &reference_front) const {
            return data_[0].igd_plus(reference_front);
        }

        /// \brief IGD+ indicator
        double igd_plus(const archive &reference_front) const {
            return data_[0].igd_plus(reference_front.data_[0]);
        }

        /// \brief STD-IGD+ indicator
        double std_igd_plus(const pareto_front_type &reference_front) const {
            return data_[0].std_igd_plus(reference_front);
        }

        /// \brief STD-IGD+ indicator
        double std_igd_plus(const archive &reference_front) const {
            return data_[0].std_igd_plus(reference_front.data_[0]);
        }

        /// \brief Uniformity metric
        /// This is the minimal distance between two points of the Pareto front
        /// approximation. This measure is straightforward to compute and easy
        /// to understand. However, it does not really provide pertinent
        /// information on the repartition of the points along the Pareto
        /// front approximation.
        [[nodiscard]] double uniformity() const {
            return data_[0].uniformity();
        }

        /// \brief Average distance between points
        [[nodiscard]] double average_distance() const {
            return data_[0].average_distance();
        }

        /// \brief Average nearest distance between points
        [[nodiscard]] double average_nearest_distance(size_t k = 5) const {
            return data_[0].average_nearest_distance(k);
        }

        /// \brief Crowding distance of an element
        [[nodiscard]] double average_crowding_distance() const {
            return data_[0].average_crowding_distance();
        }

        /// \brief Crowding distance of an element
        double crowding_distance(const_iterator element, point_type worst_point, point_type ideal_point) const {
            return data_[element.current_front_].crowding_distance(element.current_iter_, worst_point, ideal_point);
        }

        /// \brief Crowding distance of an element
        double crowding_distance(const_iterator element) const {
            return data_[element.current_front_].crowding_distance(element.current_iter_, worst(), ideal());
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


        /// \brief Check if this archive weakly dominates a point
        /// A front a weakly dominates a solution b if a has at least
        /// one solution better than b in at least one objective and
        /// is at least as good as b in all other objectives.
        /// \see http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool dominates(const point_type &p) const {
            return data_[0].dominates(p);
        }

        /// \brief Check if this archive strongly dominates a point
        /// A front a strongly dominates a solution b if a has a solution
        /// that is strictly better than b in all objectives.
        bool strongly_dominates(const point_type &p) const {
            return data_[0].strongly_dominates(p);
        }

        /// \brief Check if this archive weakly dominates a point
        /// A solution a weakly dominates a solution b if a is better
        /// than b in at least one objective and is as good as b in
        /// all other objectives.
        /// \see http://www.cs.nott.ac.uk/~pszjds/research/files/dls_emo2009_1.pdf
        bool non_dominates(const point_type &p) const {
            return data_[0].non_dominates(p);
        }

        /// \brief True if archive is partially dominated by p
        bool is_partially_dominated_by(const point_type &p) const {
            return data_[0].is_partially_dominated_by(p);
        }

        /// \brief True if archive is completely dominated by p
        bool is_completely_dominated_by(const point_type &p) const {
            return data_[0].is_completely_dominated_by(p);
        }

        /// \brief Check if this archive dominates another front
        bool dominates(const pareto_front_type &p) const {
            return data_[0].dominates(p);
        }

        /// \brief Check if this archive dominates another front
        bool dominates(const self_type &p) const {
            return data_[0].dominates(p.data_[0]);
        }

        /// \brief Check if this archive weakly dominates another front
        bool strongly_dominates(const pareto_front_type &p) const {
            return data_[0].strongly_dominates(p);
        }

        /// \brief Check if this archive weakly dominates another front
        bool strongly_dominates(const self_type &p) const {
            return data_[0].strongly_dominates(p.data_[0]);
        }

        /// \brief Check if this archive weakly dominates another front
        bool non_dominates(const pareto_front_type &p) const {
            return data_[0].non_dominates(p);
        }

        /// \brief Check if this archive weakly dominates another front
        bool non_dominates(const self_type &p) const {
            return data_[0].non_dominates(p.data_[0]);
        }

        /// \brief Check if this archive weakly dominates another front
        bool is_partially_dominated_by(const pareto_front_type &p) const {
            return data_[0].is_partially_dominated_by(p);
        }

        /// \brief Check if this archive weakly dominates another front
        bool is_partially_dominated_by(const self_type &p) const {
            return data_[0].is_partially_dominated_by(p.data_[0]);
        }

        /// \brief Check if this archive weakly dominates another front
        bool is_completely_dominated_by(const pareto_front_type &p) const {
            return data_[0].is_completely_dominated_by(p);
        }

        /// \brief Check if this archive weakly dominates another front
        bool is_completely_dominated_by(const self_type &p) const {
            return data_[0].is_completely_dominated_by(p.data_[0]);
        }

        /// \brief Ideal point in the archive
        point_type ideal() const {
            return data_[0].ideal();
        }

        /// \brief Ideal value in an archive dimension
        number_type ideal(size_t d) const {
            return data_[0].ideal(d);
        }

        /// \brief Get iterator to element with best value in dimension d
        const_iterator dimension_ideal(size_t d) const {
            std::vector<std::pair<size_t, pareto_front_const_iterator>> begins;
            begins.emplace_back(0, data_[0].dimension_ideal(d));
            return const_iterator(begins, this);
        }

        /// \brief The nadir point is the worst point among the
        /// non-dominated points. There is a difference
        /// between the nadir point and the worst point
        /// for archives.
        point_type nadir() const {
            return data_[0].nadir();
        }

        /// \brief Nadir value in dimension d
        number_type nadir(size_t d) const {
            return data_[0].nadir(d);
        }

        /// \brief Element with nadir value in front dimension d
        const_iterator dimension_nadir(size_t d) const {
            return const_iterator(this, 0, data_[0].dimension_nadir(d));
        }

        /// \brief Worst point in the archive
        /// Worst is the same as nadir for fronts.
        /// In archives, worst != nadir because there are
        /// many fronts.
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

        /// \brief Worst value in archive dimension d
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

        /// \brief Iterator to element with worst value in archive dimension d
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

        /// \brief Find element by point
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

        /// \brief Find element by point
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

        /// \brief Find first front that does not dominate p
        size_t find_front(const point_type &p) {
            // This lambda returns {0,0,0,0,0,1,1,1,1,1,1} on the fronts
            // 0 if it does dominate p
            // 1 if it does not dominate p
            auto dominate_fn = [&p](const pareto_front_type &a, const bool &b) {
                return a.dominates(p) && b;
            };
            return std::lower_bound(data_.begin(), data_.end(), true, dominate_fn) - data_.begin();
        }

        /// \brief Check if front contains the point k
        bool contains(const key_type &k) const {
            return find(k) != end();
        }

        /// \brief Check if archive passes the variants that define a front
        [[nodiscard]] bool check_invariants() const {
            for (size_t i = 0; i < data_.size(); ++i) {
                if (!data_[i].check_invariants()) {
                    return false;
                }
                if (i < data_.size() - 1) {
                    if (!data_[i + 1].is_completely_dominated_by(data_[i])) {
                        return false;
                    }
                }
            }
            return true;
        }

        /// \brief Stream archive properties to an ostream
        friend std::ostream &operator<<(std::ostream &os, const archive &ar) {
            os << "Pareto archive (" << ar.size() << " elements - {";
            for (size_t i = 0; i < ar.is_minimization_.size() - 1; ++i) {
                os << (ar.is_minimization_[i] ? "minimization" : "maximization") << ", ";
            }
            os << (ar.is_minimization_.back() ? "minimization" : "maximization") << "})";
            return os;
        }

    private /* functions */:
        void maybe_resize(std::array<uint8_t, number_of_compile_dimensions> &v [[maybe_unused]],
                          size_t n [[maybe_unused]]) {}

        void maybe_resize(std::vector<uint8_t> &v, size_t n) {
            v.resize(n);
        }

        /// \brief If the dimension is being set at runtime, this sets the
        /// dimension in case we don't already know it.
        inline void maybe_adjust_dimensions(const value_type &v) {
            if constexpr (number_of_compile_dimensions == 0) {
                if (empty() && is_minimization_.size() == 1) {
                    maybe_resize(is_minimization_, v.first.dimensions());
                    std::fill(is_minimization_.begin() + 1, is_minimization_.end(), *is_minimization_.begin());
                }
            }
        }

        /// \brief If the dimension is being set at runtime, this sets the
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

        /// \brief Max number of elements in the archive
        size_t max_size_{1000};

        /// \brief Allocator the pareto fronts will share
        std::shared_ptr<node_allocator_type> alloc_;

    };

}


#endif //PARETO_FRONT_ARCHIVE_H
