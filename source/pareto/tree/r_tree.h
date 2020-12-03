#ifndef PARETO_FRONT_R_TREE_H
#define PARETO_FRONT_R_TREE_H

#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <vector>
#include <queue>
#include <map>
#include <forward_list>

#include <pareto/point.h>
#include <pareto/query/query_box.h>
#include <pareto/query/predicates.h>
#include <pareto/memory/memory_pool.h>

namespace pareto {
    template<typename NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, typename ELEMENT_TYPE, typename TAG>
    class front;

    struct r_tree_tag;

    /// \class r_tree
    /// Implementation of an R-Tree with an interface that matches the pareto front.
    /// We could always use boost r-trees but:
    /// - boost's r-tree doesn't work well with our allocator, which is an
    ///   important part of our design.
    /// - boost r-tree is also much more generic than we need because we only store
    ///   points. In our experiments, our r-tree was up to 5 times faster that boost's
    /// - At last, we need to be able to also define dimensions at runtime because of
    ///   our python bindings. Boost's r-tree only allows us to define the dimension
    ///   at compile time.
    ///
    /// On the other hand, defining the dimension at runtime is a VERY bad idea. These
    /// trees seem to be up to 50 times slower, and there no much to do about it.
    /// With runtime dimension, every new point involves an allocation, and we cannot
    /// use our fast allocator because we don't know the dimension (our fast allocator
    /// only works for structs of fixed size).
    ///
    /// We could come up with a new kind of allocator for this case, but this is a
    /// completely new task in itself. The C++17 standard defines unsynchronized_pool_resource
    /// which could be helpful, but it seems that most standard libraries simply don't
    /// implement it. It seems like that's not really part of their plans, as
    /// they have implemented much of C++20 already.
    ///
    /// Our current solution is to provide compile-time dimensions for low
    /// dimensions in our python bindings and fall back to run-time dimensions for
    /// high dimensions: 1) these applications are rare and 2) the application using
    /// the tree might be expensive enough to amortize the cost of storing the tree.
    /// 3) A third option is for python users to recompile the library with more
    /// compile-time dimensions as they need for their tasks.
    ///
    /// This implementation used
    /// https://github.com/nushoin/RTree/blob/master/RTree.h
    /// as reference for correctness, but the design is completely different.
    template<class NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, class ELEMENT_TYPE, template<typename> class ALLOCATOR = fast_memory_pool>
    class r_tree {
    public:
        friend front<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, r_tree_tag>;
        using self_type = r_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>;
        using number_type = NUMBER_TYPE;
        static constexpr size_t number_of_compile_dimensions_ = NUMBER_OF_DIMENSIONS;
        using point_type = point<number_type, number_of_compile_dimensions_>;
        using distance_type = typename point_type::distance_type;
        using key_type = point_type;
        using mapped_type = ELEMENT_TYPE;
        using value_type = std::pair<key_type, mapped_type>;
        using box_type = query_box<number_type, number_of_compile_dimensions_>;

        template<typename VALUE_TYPE>
        using allocator_type = ALLOCATOR<VALUE_TYPE>;

        using difference_type = size_t;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = value_type *;
        using const_pointer = value_type const *;

        // Better split classification, may be slower on some systems
        static constexpr bool rtree_use_spherical_volume_ = true;

        // Max and min number of elements in a node
        static constexpr size_t tmaxnodes_ = 8;
        static constexpr size_t tminnodes_ = tmaxnodes_ / 2;
        static constexpr size_t maxnodes_ = tmaxnodes_;
        static constexpr size_t minnodes_ = tminnodes_;
        static_assert(maxnodes_ > minnodes_);
        static_assert(minnodes_ > 0);

    private /* internal node types */:
        struct rtree_node;
        using box_and_node = std::pair<box_type, rtree_node *>;

        /// \brief This represents a box_and_node or a value_type
        /// The two kinds of data we might have on a node
        class branch_variant {
        public:
            using variant_type = std::variant<box_and_node, value_type>;

            branch_variant() = default;

            explicit branch_variant(const variant_type &v) : data_(v) {}

            explicit branch_variant(const box_and_node &v) : data_(variant_type(v)) {}

            explicit branch_variant(const value_type &v) : data_(variant_type(v)) {}

            bool is_branch() const {
                return std::holds_alternative<box_and_node>(data_);
            }

            bool is_value() const {
                return std::holds_alternative<value_type>(data_);
            }

            const box_and_node &as_branch() const {
                return std::get<box_and_node>(data_);
            }

            const rtree_node *as_node() const {
                return std::get<box_and_node>(data_).second;
            }

            const value_type &as_value() const {
                return std::get<value_type>(data_);
            }

            box_and_node &as_branch() {
                return std::get<box_and_node>(data_);
            }

            rtree_node *&as_node() {
                return std::get<box_and_node>(data_).second;
            }

            value_type &as_value() {
                return std::get<value_type>(data_);
            }

            box_type rectangle() const {
                if (is_branch()) {
                    return as_branch().first;
                } else {
                    return box_type(as_value().first);
                }
            }

            distance_type distance(const point_type &p) const {
                if (is_branch()) {
                    return as_branch().first.distance(p);
                } else {
                    return as_value().first.distance(p);
                }
            }

            distance_type distance(const box_type &b) const {
                if (is_branch()) {
                    return as_branch().first.distance(b);
                } else {
                    return b.distance(as_value().first);
                }
            }

            distance_type distance(const nearest <number_type, number_of_compile_dimensions_> &b) const {
                if (b.has_reference_box()) {
                    return distance(b.reference_box());
                } else {
                    return distance(b.reference_point());
                }
            }

            void set_parent(rtree_node *parent) {
                if (is_branch()) {
                    as_branch().second->parent_ = parent;
                }
            }

            rtree_node *node_pointer() {
                return as_branch().second;
            }

            mapped_type &mapped_value() {
                return as_value().second;
            }

            point_type &point_value() {
                return as_value().first;
            }

            const mapped_type &mapped_value() const {
                return as_value().second;
            }

            const point_type &point_value() const {
                return as_value().first;
            }

            bool operator==(const branch_variant &rhs) const {
                if (is_branch() != rhs.is_branch()) {
                    return false;
                }
                if (is_branch()) {
                    return as_branch() == rhs.as_branch();
                } else {
                    return point_value() == rhs.point_value() &&
                           mapped_type_custom_equality_operator(mapped_value(), rhs.mapped_value());
                }
            }

            bool operator!=(const branch_variant &rhs) const {
                return !(*this == rhs);
            }

        private:
            variant_type data_;
        };

        /// \brief Node
        /// Each branch might have a rtree_node for each branch level
        /// The number of children is fixed because we need to make the
        /// size of a node constant. This makes it possible to use
        /// allocators efficiently.
        struct rtree_node {
            rtree_node() : rtree_node(0, 0) {}

            rtree_node(size_t count, size_t level)
                    : parent_(nullptr), count_(count), level_(level) {}

            /// An internal node, contains other nodes
            bool is_internal_node() const {
                return (level_ > 0);
                // or return std::holds_alternative<node_array>(data_)
            }

            /// A leaf, contains data
            bool is_leaf_node() const {
                return (level_ == 0);
                // or return std::holds_alternative<value_array>(data_)
            }

            branch_variant &operator[](size_t index) {
                return branches_[index];
            }

            const box_type rectangle(size_t index) const {
                return branches_[index].rectangle();
            }

            /// Parent node
            /// Keeping track of parent nodes is important for more efficient
            /// iterators. A link to the parent node allows us to eliminate
            /// stacks and easily start the iterator from any node. This makes
            /// the use of iterators with predicates much easier because we just
            /// have to move forward until a predicate is valid.
            rtree_node *parent_;

            /// Count / Number of elements in this node
            size_t count_ = 0;

            /// Leaf is zero, others positive
            size_t level_ = std::numeric_limits<size_t>::max();

            /// Nodes branches
            /// This might be:
            /// - A list of other nodes
            /// - A list of data points
            /// The parents level determines this.
            /// If the parents level is 0, then this is data.
            using branches_array = std::array<branch_variant, maxnodes_>;
            branches_array branches_;
        };

    public:
        using node_allocator_type = allocator_type<rtree_node>;

    private:
        /// Check if using the fast allocator
        constexpr static bool is_using_default_fast_allocator() {
            return std::is_same_v<node_allocator_type, fast_memory_pool < rtree_node>>;
        }

        /// \brief A link list of nodes for reinsertion after a delete operation
        using list_allocator_type = allocator_type<rtree_node *>;
        using node_list = std::forward_list<rtree_node *, list_allocator_type>;

        /// \brief Variables for finding a split partition
        /// These are the variables the function to split nodes needs
        struct partition_vars {
            constexpr partition_vars() = default;

            static constexpr int NOT_TAKEN = -1;

            std::array<int, maxnodes_ + 1> partition_;

            size_t total_{0};

            size_t min_fill_{0};

            std::pair<int, int> count_;

            std::pair<box_type, box_type> cover_;

            std::pair<number_type, number_type> area_;

            std::array<branch_variant, maxnodes_ + 1> branch_buffer_{};

            size_t branch_count_;

            box_type cover_split_;

            number_type cover_split_area_;
        };

    public /* iterators */:

        /// \brief Iterator is not erase safe. Erase elements will invalidate the iterators.
        /// Because iterator and const_iterator are almost the same,
        /// we define iterator as iterator<false> and const_iterator as iterator<true>
        /// \see https://stackoverflow.com/questions/2150192/how-to-avoid-code-duplication-implementing-const-and-non-const-iterators
        template<bool is_const = false>
        class iterator_impl {
        public:
            // friend const_iterator;
            friend r_tree;
            using difference_type = typename self_type::difference_type;
            using value_type = std::conditional_t<is_const, const std::pair<key_type, mapped_type>, std::pair<const key_type, mapped_type>>;
            using reference = value_type &;
            using const_reference = value_type const &;
            using pointer = value_type *;
            using const_pointer = value_type const *;
            using pointer_type = std::conditional_t<is_const, const rtree_node *, rtree_node *>;
            using iterator_category = std::forward_iterator_tag;
            using predicate_variant_type = predicate_variant<number_type, number_of_compile_dimensions_, mapped_type>;
            enum class iterator_tag {
                begin,
                end
            };
            using predicate_list_type = predicate_list <number_type, number_of_compile_dimensions_, mapped_type>;

            /// Default
            explicit iterator_impl() : iterator_impl(nullptr, 0) {}

            /// This is also the end iterator
            iterator_impl(pointer_type root_, iterator_tag is_end)
                    : iterator_impl(root_, is_end == iterator_tag::begin ? 0 : root_->count_) {}

            /// This is the begin iterator
            explicit iterator_impl(pointer_type root_)
                    : iterator_impl(root_, 0) {}

            /// This is the begin iterator
            iterator_impl(pointer_type root_, size_t index) : current_node_(root_),
                                                              current_branch_(index),
                                                              nearest_predicate_(nullptr),
                                                              nearest_queue_{},
                                                              nearest_points_iterated_(0) {
                advance_if_invalid();
            }

            /// This is the begin iterator
            iterator_impl(pointer_type root_, std::initializer_list<predicate_variant_type> predicate_list)
                    : iterator_impl(root_, predicate_list.begin(), predicate_list.end()) {}

            /// This is the begin iterator
            iterator_impl(pointer_type root_, const predicate_list_type &predicate_list)
                    : iterator_impl(root_, predicate_list.begin(), predicate_list.end()) {}

            /// This is the begin iterator
            template<class predicate_iterator_type>
            iterator_impl(pointer_type root_, predicate_iterator_type predicate_begin,
                          predicate_iterator_type predicate_end)
                    : current_node_(root_), current_branch_(0),
                      predicates_(predicate_begin, predicate_end), nearest_predicate_(nullptr),
                      nearest_queue_{}, nearest_points_iterated_(0) {
                sort_predicates();
                initialize_nearest_algorithm();
                advance_if_invalid();
            }

            template<bool constness>
            iterator_impl(const iterator_impl<constness> &rhs)
                    : current_node_(rhs.current_node_), current_branch_(rhs.current_branch_),
                      predicates_(rhs.predicates_), nearest_predicate_(rhs.nearest_predicate_),
                      nearest_points_iterated_(rhs.nearest_points_iterated_) {
                if (rhs.nearest_predicate_ != nullptr) {
                    nearest_predicate_ = predicates_.get_nearest();
                }
                nearest_queue_.reserve(rhs.nearest_queue_.size());
                for (const auto&[a, b, c]: rhs.nearest_queue_) {
                    nearest_queue_.emplace_back(std::make_tuple(a, b, c));
                }
                advance_if_invalid();
            }

            iterator_impl(const iterator_impl &rhs)
                    : current_node_(rhs.current_node_), current_branch_(rhs.current_branch_),
                      predicates_(rhs.predicates_), nearest_predicate_(rhs.nearest_predicate_),
                      nearest_points_iterated_(rhs.nearest_points_iterated_) {
                if (rhs.nearest_predicate_ != nullptr) {
                    nearest_predicate_ = predicates_.get_nearest();
                }
                nearest_queue_.reserve(rhs.nearest_queue_.size());
                for (const auto&[a, b, c]: rhs.nearest_queue_) {
                    nearest_queue_.emplace_back(std::make_tuple(a, b, c));
                }
                advance_if_invalid();
            }

            ~iterator_impl() = default;

            template<bool constness>
            iterator_impl &operator=(const iterator_impl<constness> &rhs) {
                current_node_ = rhs.current_node_;
                current_branch_ = rhs.current_branch_;
                predicates_ = rhs.predicates_;
                if (rhs.nearest_predicate_ != nullptr) {
                    nearest_predicate_ = predicates_.get_nearest();
                }
                nearest_points_iterated_ = rhs.nearest_points_iterated_;
                nearest_queue_ = rhs.nearest_queue_;
                return *this;
            }

            /// The equality operator ignores the predicates
            /// It only matters if they point to the same element here
            bool operator==(const iterator_impl &rhs) const {
                if (current_node_ != rhs.current_node_) {
                    return false;
                }
                return current_branch_ == rhs.current_branch_;
            }

            bool operator!=(const iterator_impl &rhs) const {
                return !(*this == rhs);
            }

            /// Find the next data element
            iterator_impl &operator++() {
                advance_to_next_valid();
                return *this;
            }

            const iterator_impl operator++(int) {
                auto tmp = *this;
                advance_to_next_valid();
                return tmp;
            }

            iterator_impl &operator--() {
                return_to_previous_valid();
                return *this;
            }

            const iterator_impl operator--(int) {
                auto tmp = *this;
                return_to_previous_valid();
                return tmp;
            }

            template<bool _constness = is_const>
            std::enable_if_t<_constness, reference>
            operator*() const {
                assert(is_not_null());
                return current_node_->branches_[current_branch_].as_value();
            }

            template<bool _constness = is_const>
            std::enable_if_t<!_constness, reference>
            operator*() const {
                assert(is_not_null());
                std::pair<key_type, mapped_type> &non_const_ref = current_node_->branches_[current_branch_].as_value();
                std::pair<key_type, mapped_type> *non_const_ptr = &non_const_ref;
                auto *const_ptr = reinterpret_cast<std::pair<const key_type, mapped_type> *>(non_const_ptr);
                std::pair<const key_type, mapped_type> &const_ref = *const_ptr;
                return const_ref;
            }

            template<bool _constness = is_const>
            std::enable_if_t<_constness, pointer>
            operator->() const {
                return &operator*();
            }

            template<bool _constness = is_const>
            std::enable_if_t<!_constness, pointer>
            operator->() const {
                return &operator*();
            }

        private:
            void advance_if_invalid() {
                if (is_end()) {
                    return;
                }
                if (current_node_->is_leaf_node()) {
                    // if there is no nearest predicate
                    if (nearest_predicate_ == nullptr) {
                        // we check if it passes other predicates
                        if (!predicates_.pass_predicate(current_node_->branches_[current_branch_].as_value())) {
                            // advance if current is not valid
                            advance_to_next_valid(false);
                        }
                    } else {
                        // if there is a nearest predicate
                        if (nearest_points_iterated_ == 0 ||
                            !predicates_.pass_predicate(current_node_->branches_[current_branch_].as_value())) {
                            // if we haven't iterated nearest points, we advance even
                            // if it might pass the predicate by coincidence
                            advance_to_next_valid(false);
                        }
                    }
                } else {
                    advance_to_next_valid(true);
                }
            }

            /// Is iterator_impl at the end
            bool is_end() const {
                if (current_node_ == nullptr) {
                    return true;
                }
                // We represent the "end" by keeping the index of one branch
                // after the last branch in the root node
                return current_node_->count_ <= current_branch_;
            }

            /// Is iterator_impl at the begin
            bool is_begin() const {
                auto root = current_node_;
                while (root->parent_ != nullptr) {
                    root = root->parent_;
                }
                return *this == iterator_impl(root, predicates_);
            }

            /// Is iterator_impl pointing to valid data
            bool is_not_null() const {
                return !is_end();
            }

            /// Reset iterator
            void init() {
                while (current_node_->parent_ != nullptr) {
                    current_node_ = current_node_->parent_;
                }
                current_branch_ = 0;
            }

            void initialize_nearest_algorithm() {
                for (auto &p: predicates_) {
                    if (p.is_nearest()) {
                        nearest_predicate_ = &p.as_nearest();
                        break;
                    }
                }
                if (nearest_predicate_ == nullptr) {
                    return;
                }

                // If this is the end iterator
                if (is_end()) {
                    // We already pre-process all the results
                    // Create an usual iterator
                    auto root = current_node_;
                    while (root->parent_ != nullptr) {
                        root = root->parent_;
                    }
                    iterator_impl it = iterator_impl(root, predicates_);
                    iterator_impl end = iterator_impl(root, iterator_tag::end);
                    // Iterate until the end so we have the nearest points pre-processed
                    while (it != end) {
                        ++it;
                    }
                    // Copy the pre-processed results for nearest points
                    nearest_set_ = it.nearest_set_;
                    nearest_points_iterated_ = it.nearest_points_iterated_;
                }

                // Initialize nearest queue
                // https://dl.acm.org/doi/pdf/10.1145/320248.320255 (page 278)
                // The algorithm basically enqueues nodes by potential distance in a priority queue
                // If first in the queue is an internal node, enqueue all its children.
                // If first in the queue is a leaf node, report the element as i-th nearest.

                // 1. Queue <- NewPriorityQueue()
                // This is already done when we create the iterator_impl with the
                // nearest_queue_ member variable.

                // 2. Enqueue(Queue, R-tree.RootNode, 0)
                // This line is the algorithm is just to ensure there's something to
                // dequeue at the first iteration. For this reason, the distance doesn't matter.
                // It's just easier to Enqueue all branches of R-tree.RootNode
                // already, as would always happen at the first iteration anyway.
                for (size_t i = 0; i < current_node_->count_; ++i) {
                    nearest_queue_.emplace_back(current_node_, i,
                                                current_node_->branches_[i].distance(*nearest_predicate_));
                    std::push_heap(nearest_queue_.begin(), nearest_queue_.end(), queue_comp);
                }

                // 3. while not IsEmpty(Queue) do
                // From this point the algorithm is implemented inside the
                // "advance_to_next_valid" function so that we can iteractively
                // return the nearest points.
            }

            void advance_to_next_valid_through_nearest() {
                // If we already iterated all nearest points the predicate asked for
                if (nearest_points_iterated_ >= nearest_predicate_->k()) {
                    advance_to_end();
                    return;
                }

                // If we already iterated this nearest point in the past
                // we should have the pre-processed results
                while (nearest_points_iterated_ < nearest_set_.size()) {
                    bool passed_predicate;
                    std::tie(current_node_, current_branch_, passed_predicate) = nearest_set_[nearest_points_iterated_];
                    ++nearest_points_iterated_;
                    if (passed_predicate) {
                        return;
                    }
                }

                // We go to the real algorithm after dealing with the trivial cases
                // The first steps were executed in 'initialize_nearest_algorithm'
                // The advance step starts the loop that looks for more nearest elements
                // 3. while not IsEmpty(Queue) do
                while (!nearest_queue_.empty()) {
                    // 4. Element <- Dequeue(Queue)
                    auto[element_node, element_index, distance] = nearest_queue_.front();
                    std::conditional_t<is_const, const branch_variant, branch_variant> &element = element_node->branches_[element_index];
                    std::pop_heap(nearest_queue_.begin(), nearest_queue_.end(), queue_comp);
                    nearest_queue_.pop_back();
                    // 5. if element is an object or its bounding rectangle
                    // The way we represent the tree, it's never a bounding rectangle
                    // because we only represent points in our tree. So we only test
                    // if it's an object (a value).
                    if (element.is_value()) {
                        // 6. if Element is the bounding rectangle
                        //     7. That never happens in our tree
                        // 8. else
                        // 9.     Report Element
                        // in our version, we only report it if it also passes
                        // the other predicates
                        if (predicates_.pass_predicate(element.as_value())) {
                            ++nearest_points_iterated_;
                            current_node_ = element_node;
                            current_branch_ = element_index;
                            // put it in the pre-processed set of results
                            nearest_set_.emplace_back(current_node_, current_branch_, true);
                            return;
                        } else {
                            // if it doesn't pass the predicates, we just count it
                            // as one more nearest point we would have reported
                            ++nearest_points_iterated_;
                            nearest_set_.emplace_back(current_node_, current_branch_, false);
                            if (nearest_points_iterated_ >= nearest_predicate_->k()) {
                                advance_to_end();
                                return;
                            }
                        }
                    } else {
                        // 11. else if Element is a leaf node then
                        // 15. else /* Element is a non-leaf node*/
                        // These two conditions have blocks enqueuing all child elements.
                        // So they are equivalent to us because we use variants
                        // for the branches.
                        // 12. for each entry (Object, Rect) in leaf node Element do
                        // 16. for each entry (Node,   Rect) in      node Element do
                        for (size_t i = 0; i < element.as_node()->count_; ++i) {
                            // 13. Enqueue(Queue, [Object], Dist(QueryObject, Rect))
                            // 17. Enqueue(Queue, Node,     Dist(QueryObject, Rect))
                            nearest_queue_.emplace_back(element.as_node(), i,
                                                        element.as_node()->branches_[i].distance(*nearest_predicate_));
                            std::push_heap(nearest_queue_.begin(), nearest_queue_.end(), queue_comp);
                        }
                    }
                }
                // Nothing else to report
                advance_to_end();
            }

            void return_to_previous_valid_through_nearest() {
                // If previous point is already iterated we get the results from there
                while (nearest_points_iterated_ == 0 || nearest_points_iterated_ - 1 < nearest_set_.size()) {
                    // If everything is iterated
                    if (nearest_points_iterated_ == 0) {
                        return_to_begin();
                        return;
                    } else {
                        // if nearest_points_iterated_ - 1 is already pre-processed
                        bool passed_predicate;
                        std::tie(current_node_, current_branch_, passed_predicate) = nearest_set_[
                                nearest_points_iterated_ - 1];
                        --nearest_points_iterated_;
                        if (passed_predicate) {
                            return;
                        }
                    }
                }

                // If we got here, we have a problem
                throw std::logic_error("We should have the pre-processed results for the nearest points");
            }

            void advance_to_end() {
                while (current_node_->parent_ != nullptr) {
                    go_to_parent();
                }
                current_branch_ = current_node_->count_;
            }

            void return_to_begin() {
                while (current_node_->parent_ != nullptr) {
                    current_node_ = current_node_->parent_;
                }
                current_branch_ = 0;
            }

            void advance_to_next_valid_depth_first(bool first_time_in_this_branch = false) {
                while (!is_end()) {
                    // this should usually be the case at the start
                    // it might only be an internal node at the next iterations
                    if (current_node_->is_leaf_node()) {
                        // iterate branches
                        for (size_t index = current_branch_ + !first_time_in_this_branch;
                             index < current_node_->count_; ++index) {
                            // if value passes the predicate
                            if (predicates_.pass_predicate(current_node_->branches_[index].as_value())) {
                                // found a value branch in a node
                                // point to it and return
                                current_branch_ = index;
                                return;
                            }
                        }
                        // No more data, go to previous level / parent node
                        go_to_parent();
                        first_time_in_this_branch = false;
                    } else {
                        // the node is internal
                        // iterate from next branch
                        // current branch as been iterated, as in the first part of this "if"
                        bool predicate_might_pass = false;
                        for (size_t index = current_branch_ + !first_time_in_this_branch;
                             index < current_node_->count_; ++index) {
                            // If branch might have nodes that pass all predicates
                            if (predicates_.might_pass_predicate(current_node_->branches_[index].as_branch().first)) {
                                // Found a value branch in a node
                                // Point to it and continue looking until we find a value branch
                                current_node_ = current_node_->branches_[index].as_branch().second;
                                current_branch_ = 0;
                                predicate_might_pass = true;
                                first_time_in_this_branch = true;
                                break;
                            }
                        }
                        // No more branches that could pass the predicates, go to previous level / parent node
                        if (!predicate_might_pass) {
                            go_to_parent();
                            first_time_in_this_branch = false;
                        }
                    }
                }
            }

            void return_to_previous_valid_depth_first(bool first_time_in_this_branch = false) {
                while (!is_begin()) {
                    if (current_node_->is_leaf_node()) {
                        // move to previous value branch
                        // if moving to previous element and this is the first branch
                        if (!first_time_in_this_branch && current_branch_ == 0) {
                            // go to parent
                            go_to_parent(false);
                            // don't iterate the current branch on the next iteration
                            // because we've already done that
                            first_time_in_this_branch = false;
                            continue;
                        }
                        for (size_t index = current_branch_ - !first_time_in_this_branch;
                             index >= 0; --index) {
                            // if value passes the predicate
                            if (predicates_.pass_predicate(current_node_->branches_[index].as_value())) {
                                // found a value branch in a node
                                // point to it and return
                                current_branch_ = index;
                                return;
                            }
                        }
                        // No more data, go to previous level / parent node
                        go_to_parent(false);
                        first_time_in_this_branch = false;
                    } else {
                        // the node is internal
                        // move to previous value branch
                        // if moving to previous element and this is the first branch
                        if (!first_time_in_this_branch && current_branch_ == 0) {
                            // go to parent
                            go_to_parent(false);
                            // don't iterate the current branch on the next iteration
                            // because we've already done that
                            first_time_in_this_branch = false;
                            continue;
                        }
                        // iterate from previous branch
                        // current branch as been iterated, as in the first part of this "if"
                        bool predicate_might_pass = false;
                        for (size_t index = current_branch_ - !first_time_in_this_branch; index >= 0; --index) {
                            // If branch might have nodes that pass all predicates
                            if (predicates_.might_pass_predicate(current_node_->branches_[index].as_branch().first)) {
                                // Found a value branch in a node
                                // Point to it and continue looking until we find a value branch
                                current_node_ = current_node_->branches_[index].as_branch().second;
                                current_branch_ = current_node_->count_ - 1;
                                predicate_might_pass = true;
                                first_time_in_this_branch = true;
                                break;
                            }
                        }
                        // No more branches that could pass the predicates, go to previous level / parent node
                        if (!predicate_might_pass) {
                            go_to_parent(false);
                            first_time_in_this_branch = false;
                        }
                    }
                }
            }

            /// \brief Find the next data element in the tree (For internal use only)
            /// If we have a nearest predicate, the way we iterate the tree
            /// is completely different. We need to use the nearest queue
            /// to decide which branches will be iterated first. We iterate even the
            /// branches that may fail other predicates because this is the only way
            /// for us to count how many nearest points we have visited.
            ///
            /// If there is no nearest predicate, we use a depth-first search in which
            /// we ignore branches that have no potential to pass the predicates. For small
            /// query boxes (as small as a branch), this leads to log(n) queries.
            void advance_to_next_valid(bool first_time_in_this_branch = false) {
                if (nearest_predicate_ != nullptr) {
                    advance_to_next_valid_through_nearest();
                } else {
                    advance_to_next_valid_depth_first(first_time_in_this_branch);
                }
            }

            void return_to_previous_valid(bool first_time_in_this_branch = false) {
                if (nearest_predicate_ != nullptr) {
                    return_to_previous_valid_through_nearest();
                } else {
                    return_to_previous_valid_depth_first(first_time_in_this_branch);
                }
            }

            void go_to_parent(bool move_right_if_root = true) {
                if (current_node_->parent_ == nullptr) {
                    if (move_right_if_root) {
                        // if no parent, we move to the end / move right
                        current_branch_ = current_node_->count_;
                    } else {
                        // if no parent, we move to begin / move left
                        *this = iterator_impl(current_node_->parent_, predicates_);
                    }
                } else {
                    // Find index of current branch in the parent
                    // This takes NODEMAX/2 operations on average
                    // It's more efficient than redundantly storing the branch index
                    // on every child node.
                    current_branch_ = 0;
                    for (const auto &parent_branch: current_node_->parent_->branches_) {
                        if (parent_branch.as_branch().second == current_node_) {
                            break;
                        }
                        ++current_branch_;
                    }
                    current_node_ = current_node_->parent_;
                    assert(current_branch_ < current_node_->count_);
                }
            }

            void sort_predicates() {
                // If there is any disjoint predicate
                number_type volume_root = 0.;
                if (predicates_.size() > 1 && predicates_.contains_disjoint()) {
                    // find root node
                    auto root = current_node_;
                    while (root->parent_ != nullptr) {
                        root = root->parent_;
                    }
                    // calculate root node minimum bounding rectangle
                    box_type rect = root->rectangle(0);
                    for (size_t index = 1; index < root->count_; ++index) {
                        rect = rect.combine(root->rectangle(index));
                    }
                    // calculate volume of root minimum bounding rectangle
                    volume_root = rect.volume();
                }
                predicates_.sort(volume_root);
            }

            /// Stack as we are doing iteration instead of recursion
            pointer_type current_node_;

            /// Top of stack index
            size_t current_branch_;

            /// Predicate constraining the search area
            predicate_list_type predicates_;

            /// Pointer to a nearest predicate
            nearest <number_type, number_of_compile_dimensions_> *nearest_predicate_ = nullptr;

            // Pair with branch (node or object) and distance to the reference point
            // The branch is represented by the node and its position in the node
            // tuple<node, branch_index, distance>
            using queue_element = std::tuple<pointer_type, size_t, distance_type>;

            // Function to compare queue_elements by their distance to the reference point
            static const std::function<bool(const queue_element &, const queue_element &)> queue_comp;

            // Queue <- NewPriorityQueue()
            std::vector<queue_element> nearest_queue_;

            // Number of nearest points we have iterated so far
            size_t nearest_points_iterated_{0};

            // Set of nearest values we have already found
            // tuple<node, branch index, passed predicate>
            std::vector<std::tuple<pointer_type, size_t, bool>> nearest_set_;

            // Allow hiding of non-public functions while allowing manipulation by logical owner
            friend class r_tree;
        };

        using iterator = iterator_impl<false>;
        using const_iterator = iterator_impl<true>;

    public:
        /// Construct
        r_tree() : size_(0),
                   dimensions_(number_of_compile_dimensions_),
                   alloc_(new node_allocator_type()),
                   list_alloc_() {
            if constexpr (number_of_compile_dimensions_ != 0) {
                initialize_unit_sphere_volume();
            }
            root_ = allocate_rtree_node();
            root_->level_ = 0;
        }

        /// \brief Constructor that shares an external allocator
        r_tree(std::shared_ptr<node_allocator_type> &external_alloc) :
                size_(0),
                dimensions_(number_of_compile_dimensions_),
                alloc_(external_alloc),
                list_alloc_() {
            if constexpr (number_of_compile_dimensions_ != 0) {
                initialize_unit_sphere_volume();
            }
            root_ = allocate_rtree_node();
            root_->level_ = 0;
        }

        /// \brief Construct from points
        template<class InputIterator>
        r_tree(InputIterator first, InputIterator last) : r_tree() {
            // sort points and bulk insert
            std::vector<value_type> v(first, last);
            std::sort(v.begin(), v.end());
            bulk_insert(v, root_);
        }

        /// \brief Copy constructor
        r_tree(const r_tree &other) :
                size_(other.size_),
                dimensions_(other.dimensions_),
                unit_sphere_volume_(other.unit_sphere_volume_),
                alloc_(other.alloc_),
                list_alloc_(other.list_alloc_) {
            root_ = allocate_rtree_node();
            rtree_node *root_parent = nullptr;
            copy_recursive(root_, root_parent, other.root_);
        }

        /// \brief Copy constructor
        r_tree &operator=(const r_tree &other) {
            remove_all_records(root_);
            size_ = other.size_;
            dimensions_ = other.dimensions_;
            unit_sphere_volume_ = other.unit_sphere_volume_;
            alloc_ = other.alloc_;
            list_alloc_ = list_allocator_type();
            root_ = allocate_rtree_node();
            rtree_node *root_parent = nullptr;
            copy_recursive(root_, root_parent, other.root_);
            return *this;
        }

        /// \brief Destructor
        virtual ~r_tree() {
            remove_all_records(root_);
        }

    public /* iterators */:

        const_iterator begin() const noexcept {
            if (root_) {
                return const_iterator(root_);
            } else {
                return end();
            }
        }

        const_iterator end() const noexcept {
            return const_iterator(root_, const_iterator::iterator_tag::end);
        }

        iterator begin() noexcept {
            if (root_) {
                return iterator(root_);
            } else {
                return end();
            }
        }

        iterator end() noexcept {
            return iterator(root_, iterator::iterator_tag::end);
        }

        std::reverse_iterator<const_iterator> rbegin() const noexcept {
            return std::reverse_iterator<const_iterator>(end());
        }

        std::reverse_iterator<const_iterator> rend() const noexcept {
            return std::reverse_iterator<const_iterator>(begin());
        }

        std::reverse_iterator<iterator> rbegin() noexcept {
            return std::reverse_iterator<iterator>(end());
        }

        std::reverse_iterator<iterator> rend() noexcept {
            return std::reverse_iterator<iterator>(begin());
        }

        bool operator==(const self_type &rhs) const {
            return std::equal(begin(), end(), rhs.begin(), rhs.end());
        }

        bool operator!=(const self_type &rhs) const {
            return !(rhs == *this);
        }

        const_iterator find(const point_type &p) const {
            const_iterator it = begin_intersection(p, p);
            it.predicates_.clear();
            return it;
        }

        iterator find(const point_type &p) {
            iterator it = begin_intersection(p, p);
            it.predicates_.clear();
            return it;
        }

        iterator find(const value_type &v) {
            iterator it = begin_intersection(v.first, v.first,
                                             [&v](const value_type &x) {
                                                 return mapped_type_custom_equality_operator(x.second, v.second);;
                                             });
            it.predicates_.clear();
            return it;
        }

        const_iterator find(const value_type &v) const {
            const_iterator it = begin_intersection(v.first, v.first,
                                                   [&v](const value_type &x) {
                                                       return mapped_type_custom_equality_operator(x.second, v.second);;
                                                   });
            it.predicates_.clear();
            return it;
        }

        iterator begin_intersection(const point_type &min_corner, const point_type &max_corner) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return iterator(root_, {intersects(min_corner_, max_corner_)});
        }

        const_iterator begin_intersection(const point_type &min_corner, const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return const_iterator(root_, {intersects(min_corner_, max_corner_)});
        }

        iterator begin_intersection(const point_type &min_corner, const point_type &max_corner,
                                    std::function<bool(const value_type &)> fn) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return iterator(root_, {intersects(min_corner_, max_corner_), satisfies(fn)});
        }

        const_iterator begin_intersection(const point_type &min_corner, const point_type &max_corner,
                                          std::function<bool(const value_type &)> fn) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return const_iterator(root_, {intersects(min_corner_, max_corner_), satisfies(fn)});
        }

        iterator begin_within(const point_type &min_corner, const point_type &max_corner) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return iterator(root_, {within(min_corner_, max_corner_)});
        }

        const_iterator begin_within(const point_type &min_corner, const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return const_iterator(root_, {within(min_corner_, max_corner_)});
        }

        iterator begin_disjoint(const point_type &min_corner, const point_type &max_corner) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return iterator(root_, {disjoint(min_corner_, max_corner_)});
        }

        const_iterator begin_disjoint(const point_type &min_corner, const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return const_iterator(root_, {disjoint(min_corner_, max_corner_)});
        }

        /// \brief  Find the point closest to this point
        /// \see Hjaltason, Gísli R., and Hanan Samet. "Distance browsing in spatial databases." ACM Transactions on Database Systems (TODS) 24.2 (1999): 265-318.
        /// \see https://dl.acm.org/doi/pdf/10.1145/320248.320255
        /// \see https://stackoverflow.com/questions/45816632/nearest-neighbor-algorithm-in-r-tree
        iterator begin_nearest(const point_type &p) {
            return iterator(root_, {nearest(p)});
        }

        const_iterator begin_nearest(const point_type &p) const {
            return const_iterator(root_, {nearest(p)});
        }

        iterator begin_nearest(const point_type &p, size_t k) {
            return iterator(root_, {nearest(p, k)});
        }

        const_iterator begin_nearest(const point_type &p, size_t k) const {
            return const_iterator(root_, {nearest(p, k)});
        }

        iterator begin_nearest(const box_type &b, size_t k) {
            return iterator(root_, {nearest(b, k)});
        }

        const_iterator begin_nearest(const box_type &b, size_t k) const {
            return const_iterator(root_, {nearest(b, k)});
        }

        iterator begin_nearest(const point_type &p, size_t k, std::function<bool(const value_type &)> fn) {
            return iterator(root_, {nearest(p, k), satisfies(fn)});
        }

        const_iterator begin_nearest(const point_type &p, size_t k, std::function<bool(const value_type &)> fn) const {
            return const_iterator(root_, {nearest(p, k), satisfies(fn)});
        }

    public /* non-modifying functions */:
        bool empty() const noexcept {
            return root_->count_ == 0;
        }

        size_t size() const noexcept {
            return size_;
        }

        size_t dimensions() const noexcept {
            if constexpr (number_of_compile_dimensions_ != 0) {
                return number_of_compile_dimensions_;
            } else {
                return dimensions_;
            }
        }

        number_type max_value(size_t dimension) const {
            auto max_it = std::max_element(root_->branches_.begin(), root_->branches_.begin() + root_->count_,
                                           [&dimension](const branch_variant &a, const branch_variant &b) {
                                               return (a.is_value() ? a.as_value().first[dimension]
                                                                    : a.as_branch().first.second()[dimension]) <
                                                      (b.is_value() ? b.as_value().first[dimension]
                                                                    : b.as_branch().first.second()[dimension]);
                                           });
            if (max_it != root_->branches_.begin() + root_->count_) {
                return max_it->is_value() ? max_it->as_value().first[dimension]
                                          : max_it->as_branch().first.second()[dimension];
            } else {
                return std::numeric_limits<number_type>::min();
            }
        }


        iterator max_element(size_t dimension) {
            auto[node, index] = recursive_max_element(root_, dimension);
            return iterator(node, index);
        }

        const_iterator max_element(size_t dimension) const {
            auto[node, index] = recursive_max_element(root_, dimension);
            return const_iterator(node, index);
        }

        iterator min_element(size_t dimension) {
            auto[node, index] = recursive_min_element(root_, dimension);
            return iterator(node, index);
        }

        const_iterator min_element(size_t dimension) const {
            auto[node, index] = recursive_min_element(root_, dimension);
            return const_iterator(node, index);
        }

        number_type min_value(size_t dimension) const {
            auto min_it = std::min_element(root_->branches_.begin(), root_->branches_.begin() + root_->count_,
                                           [&dimension](const branch_variant &a, const branch_variant &b) {
                                               return (a.is_value() ? a.as_value().first[dimension]
                                                                    : a.as_branch().first.first()[dimension]) <
                                                      (b.is_value() ? b.as_value().first[dimension]
                                                                    : b.as_branch().first.first()[dimension]);
                                           });
            if (min_it != root_->branches_.begin() + root_->count_) {
                return min_it->is_value() ? min_it->as_value().first[dimension]
                                          : min_it->as_branch().first.first()[dimension];
            } else {
                return std::numeric_limits<number_type>::min();
            }
        }

    public /* modifying functions */:
        /// Insert entry
        /// Insertion erases any point dominated by the point
        /// before inserting the element in the rtree
        /// \param v Pair value <point, value>
        /// \return iterator to the new element
        /// \return True if insertion happened successfully
        std::pair<iterator, bool> insert(const value_type &v) {
            if constexpr (number_of_compile_dimensions_ == 0) {
                if (dimensions_ == 0) {
                    dimensions_ = v.first.dimensions();
                    initialize_unit_sphere_volume();
                }
            }
            auto[node_split, destination_node, value_index] = insert_branch(branch_variant(v), root_, 0);
            ++size_;
            return {iterator(destination_node, value_index), true};
        }

        /// \brief Insert list of elements
        template<class Inputiterator>
        void insert(Inputiterator first, Inputiterator last) {
            while (first != last) {
                insert(*first);
                ++first;
            }
        }

        /// \brief Erase element
        size_t erase(iterator position) {
            // erase_query_box will erase only one element
            // the first element in the box that has position->second
            // will be removed.
            // this is ok for what we need here
            auto s = erase_query_box_bottom_up(position);
            size_ -= s;
            return s;
        }

        size_t erase(const value_type &v) {
            auto s = erase_query_box(box_type(v.first), v.second, root_);
            size_ -= s;
            return s;
        }

        /// \brief Remove range of iterators from the front
        size_t erase(const_iterator first, const_iterator last) {
            // get copy of all elements in the query
            std::vector<value_type> v(first, last);
            // remove using these copies
            size_t s = 0;
            for (auto begin = v.begin(); begin != v.end(); ++begin) {
                s += erase(*begin);
            }
            return s;
        }

        /// \brief erase all entries from tree
        void clear() {
            // Delete all existing nodes
            remove_all_records(root_);
            list_alloc_ = list_allocator_type();
            root_ = allocate_rtree_node();
            root_->level_ = 0;
            size_ = 0;
        }

        /// \brief Swap the content of two fronts
        void swap(self_type &m) {
            std::swap(root_, m.root_);
            std::swap(size_, m.size_);
            std::swap(unit_sphere_volume_, m.unit_sphere_volume_);
        }

    private:
        /// \brief Insert a value into a tree node
        /// insert_branch provides for splitting the root;
        /// insert_branch_recursive does the recursion.
        /// \param value Value to be inserted
        /// \param a_root Node where we should insert the value (usually root node)
        /// \param a_level Number of steps up from the leaf level to insert e.g. a data rectangle goes in at level = 0.
        /// \return return 1 if root was split, 0 if it was not.
        /// \return Pointer to the node containing the value we inserted
        /// \return Index of the element we inserted in the node that contains it
        std::tuple<bool, rtree_node *, size_t>
        insert_branch(const branch_variant &branch, rtree_node *&root_node, size_t a_level) {
            assert(root_node);
            assert(a_level >= 0 && a_level <= root_node->level_);

            // Create new node pointer to put new node in case this node splits
            rtree_node *new_rtree_node = nullptr;

            // If the root split
            auto result_tuple = insert_branch_recursive(branch, root_node, new_rtree_node, static_cast<int>(a_level));
            bool root_was_split = std::get<0>(result_tuple);
            if (root_was_split) {
                // Grow tree taller and new root
                // Mark that it has a new level
                rtree_node *new_root = allocate_rtree_node();
                new_root->level_ = root_node->level_ + 1;

                // Create a new branch with the root node
                // Add old root node as a child of the new root
                box_and_node branch_with_root_node;
                branch_with_root_node.first = minimum_bounding_rectangle(root_node);
                branch_with_root_node.second = root_node;
                add_rtree_branch(branch_variant(branch_with_root_node), new_root);

                // Add the split node as a child of the new root
                box_and_node branch_with_new_node;
                branch_with_new_node.first = minimum_bounding_rectangle(new_rtree_node);
                branch_with_new_node.second = new_rtree_node;
                add_rtree_branch(branch_variant(branch_with_new_node), new_root);

                // Set the new root as the root node
                root_node = new_root;
            }

            return result_tuple;
        }

        /// \brief Inserts a new data rectangle into the index structure.
        /// Recursively descends tree, propagates splits back up.
        /// Returns 0 if node was not split.  Old node updated.
        /// If node was split, returns 1 and sets the pointer pointed to by
        /// new_node to point to the new node.  Old node updated to become one of two.
        /// The level argument specifies the number of steps up from the leaf
        /// level to insert; e.g. a data rectangle goes in at level = 0.
        /// \param value value to be inserted
        /// \param a_node Node where we are inserting the element
        /// \param a_new_rtree_node Pointer where we store the new node in case the node splits
        /// \param a_level Level at which we are inserting the new (0 represent the leaf node)
        /// \return True if node was split
        /// \return Pointer to the node containing the value we inserted
        /// \return Index of the element we inserted in the node that contains it
        std::tuple<bool, rtree_node *, size_t>
        insert_branch_recursive(const branch_variant &branch, rtree_node *&parent_node,
                                rtree_node *&maybe_new_tree_node, int target_level) {
            assert(parent_node);
            assert(target_level >= 0 && target_level <= static_cast<int>(parent_node->level_));

            // Recurse until we reach the correct level for the new record. Data records
            // will always be called with target_level == 0 (leaf)
            if (static_cast<int>(parent_node->level_) > target_level) {
                // Still above level for insertion, go down tree recursively
                // Find the optimal branch for this record
                size_t index = pick_rtree_branch(branch.rectangle(), parent_node);

                // Recursively insert this record into the picked branch
                rtree_node *other_rtree_node = nullptr;
                auto[child_was_split, insertion_branch, insertion_index] = insert_branch_recursive(branch,
                                                                                                   parent_node->branches_[index].as_branch().second,
                                                                                                   other_rtree_node,
                                                                                                   target_level);

                if (!child_was_split) {
                    // Child was not split.
                    // Merge the bounding box of the new record with the existing bounding box
                    parent_node->branches_[index].as_branch().first = branch.rectangle().combine(
                            parent_node->branches_[index].as_branch().first);
                    parent_node->branches_[index].set_parent(parent_node);
                    return std::make_tuple(child_was_split, insertion_branch, insertion_index);
                } else {
                    // Child was split. The old branches are now re-partitioned to two nodes
                    // so we have to re-calculate the bounding boxes of each node
                    parent_node->branches_[index].as_branch().first = minimum_bounding_rectangle(
                            parent_node->branches_[index].as_branch().second);
                    box_and_node branch_with_new_tree;
                    branch_with_new_tree.first = minimum_bounding_rectangle(other_rtree_node);
                    branch_with_new_tree.second = other_rtree_node;
                    branch_with_new_tree.second->parent_ = parent_node;

                    // The old node is already a child of parent_node. Now add the newly-created
                    // node to parent_node as well. parent_node might be split because of that.
                    auto[branch_was_split, branch_node, branch_index] = add_rtree_branch(
                            branch_variant(branch_with_new_tree), parent_node, maybe_new_tree_node);
                    // we return if the immediate branch split, but propagate up the directions to the new value
                    return std::make_tuple(branch_was_split, insertion_branch, insertion_index);
                }
            } else if (static_cast<int>(parent_node->level_) == target_level) {
                // We have reached level for insertion. Add branch, split if necessary
                return add_rtree_branch(branch, parent_node, maybe_new_tree_node);
            } else {
                throw std::logic_error("Should never occur. Target level cannot be less than the node level.");
            }
        }

        rtree_node *allocate_rtree_node() {
            auto p = alloc_->allocate(1);
            new(p) rtree_node(0, 0);
            return p;
        }

        void deallocate_rtree_node(rtree_node *p) {
            p->~rtree_node();
            alloc_->deallocate(p, 1);
        }

        /// \brief Find the smallest rectangle that includes all rectangles in branches of a node.
        box_type minimum_bounding_rectangle(rtree_node *a_node) {
            assert(a_node);
            box_type rect = a_node->rectangle(0);
            for (size_t index = 1; index < a_node->count_; ++index) {
                rect = rect.combine(a_node->rectangle(index));
            }
            return rect;
        }

        /// \brief Add a branch to a node. Split the node if necessary.
        /// Returns 0 if node not split. Old node updated.
        /// Returns 1 if node split, sets a_new_rtree_node to address of new node.
        /// Old node updated, becomes one of two.
        /// \param a_branch Branch
        /// \param a_node Node where we should put the branch
        /// \param a_new_rtree_node New node, if the node was split
        /// \return True if node was split
        /// \return Pointer to the node containing the value we inserted
        /// \return Index of the element we inserted in the node that contains it
        std::tuple<bool, rtree_node *, size_t>
        add_rtree_branch(const branch_variant &branch_to_insert, rtree_node *&parent_node,
                         rtree_node *&maybe_new_tree) {
            assert(parent_node);
            // Split won't be necessary
            if (parent_node->count_ < maxnodes_) {
                parent_node->branches_[parent_node->count_] = branch_to_insert;
                parent_node->branches_[parent_node->count_].set_parent(parent_node);
                ++parent_node->count_;
                return std::make_tuple(false, parent_node, parent_node->count_ - 1);
            } else {
                // If we need to split the node
                auto[node_with_value, value_index] = split_rtree_node(parent_node, branch_to_insert, maybe_new_tree);
                return std::make_tuple(true, node_with_value, value_index);
            }
        }

        /// \brief \return True if node was split
        /// \return Pointer to the node containing the value we inserted
        /// \return Index of the element we inserted in the node that contains it
        std::tuple<bool, rtree_node *, size_t>
        add_rtree_branch(const branch_variant &branch_to_insert, rtree_node *parent_node) {
            assert(parent_node);
            // Split won't be necessary
            if (parent_node->count_ < maxnodes_) {
                parent_node->branches_[parent_node->count_] = branch_to_insert;
                parent_node->branches_[parent_node->count_].set_parent(parent_node);
                ++parent_node->count_;
                return std::make_tuple(false, parent_node, parent_node->count_ - 1);
            } else {
                throw std::logic_error(
                        "You should use the other add_rtree_branch function is a split might be necessary");
            }
        }

        /// \brief Pick a branch.  Pick the one that will need the smallest increase
        /// in area to accomodate the new rectangle.  This will result in the
        /// least total area for the covering rectangles in the current node.
        /// In case of a tie, pick the one which was smaller before, to get
        /// the best resolution when searching.
        /// \param new_rectangle Rectangle of the new element we are putting in the node
        /// \param parent_node Node where we will put an element with the new rectangle
        /// \return Index of the child most appropriate to put the element
        size_t pick_rtree_branch(const box_type &new_rectangle, rtree_node *parent_node) {
            assert(parent_node);

            // true if it's the first time we are going to find a better increase size
            bool first_time = true;
            // by how much the box would increase the minimum bounding rectangle
            number_type increase;
            // the best/minimal increase we found
            auto best_increase = static_cast<number_type>(-1);
            // variable to keep the volume we calculate for the query box
            number_type volume;
            // best volume of the new box
            number_type best_volume = 0;
            // index of the branch most appropriate for the new element
            size_t best = 0;
            // variable to keep the combination of both query boxes
            box_type combined_query_box;

            // for each branch in this parent node
            for (size_t index = 0; index < parent_node->count_; ++index) {
                box_type &current_query_box = parent_node->branches_[index].as_branch().first;
                volume = calculate_query_box_volume(current_query_box);
                combined_query_box = new_rectangle.combine(current_query_box);
                increase = calculate_query_box_volume(combined_query_box) - volume;
                if ((increase < best_increase) || first_time) {
                    best = index;
                    best_volume = volume;
                    best_increase = increase;
                    first_time = false;
                } else if ((increase == best_increase) && (volume < best_volume)) {
                    // if the increase is no better, we compare the volumes
                    // smaller volumes are always better
                    best = index;
                    best_volume = volume;
                    best_increase = increase;
                }
            }
            return best;
        }

        size_t pick_rtree_branch(const point_type &p, rtree_node *a_node) {
            box_type b(p);
            return pick_rtree_branch(b, a_node);
        }

        /// \brief Split a node.
        /// Divides the nodes branches and the extra one between two nodes.
        /// Old node is one of the new ones, and one really new one is created.
        /// Tries more than one method for choosing a partition, uses best result.
        /// \param a_node Node to be split (will become one of the two new nodes)
        /// \param a_branch Branch we are inserting in a_node that would make it exceed its capacity
        /// \param a_new_rtree_node Pointer where we will store the new node
        /// \return Pointer to the node containing the value we inserted
        /// \return Index of the element we inserted in the node that contains it
        std::tuple<rtree_node *, size_t>
        split_rtree_node(rtree_node *&old_node, const branch_variant &branch_to_insert, rtree_node *&new_tree_node) {
            assert(old_node);

            // Could just use local here, but member or external is faster since it is reused
            partition_vars par_vars;

            // Load all the branches into a buffer, initialize old node
            // New branch goes to last position of par_vars.branch_buffer_
            get_rtree_branches(old_node, branch_to_insert, par_vars);

            // Calculate an appropriate partition for the nodes
            choose_partition(par_vars, minnodes_);

            // Create a new node to hold (about) half of the branches
            new_tree_node = allocate_rtree_node();
            new_tree_node->level_ = old_node->level_;
            new_tree_node->parent_ = old_node->parent_;
            // Put branches from buffer into 2 nodes according to the chosen partition
            old_node->count_ = 0;
            auto result_tuple = load_rtree_nodes(old_node, new_tree_node, par_vars);
            assert((old_node->count_ + new_tree_node->count_) == par_vars.total_);
            return result_tuple;
        }

        /// \brief The exact volume of the bounding sphere for the given box_type
        number_type query_box_spherical_volume(const box_type &region_to_erase) {
            auto sum_of_squares = static_cast<number_type>(0.);
            number_type radius;

            for (size_t index = 0; index < dimensions(); ++index) {
                number_type half_extent = (static_cast<number_type>(region_to_erase.max()[index]) -
                                           static_cast<number_type>(region_to_erase.min()[index])) *
                                          static_cast<number_type>(0.5);
                sum_of_squares += half_extent * half_extent;
            }

            radius = static_cast<number_type>(sqrt(sum_of_squares));

            // Pow maybe slow, so test for common dims like 2,3 and just use x*x, x*x*x.
            if (dimensions() == 3) {
                return (radius * radius * radius * unit_sphere_volume_);
            } else if (dimensions() == 2) {
                return (radius * radius * unit_sphere_volume_);
            } else {
                return (number_type) (pow(radius, dimensions()) * unit_sphere_volume_);
            }
        }

        /// \brief Calculate the n-dimensional volume of a rectangle
        number_type query_box_volume(const box_type &region_to_erase) {
            auto volume = static_cast<number_type>(1.0);

            for (size_t index = 0; index < NUMBER_OF_DIMENSIONS; ++index) {
                volume *= region_to_erase.max()[index] - region_to_erase.min()[index];
            }

            assert(volume >= static_cast<number_type>(0));

            return volume;
        }

        /// \brief Use one of the methods to calculate retangle volume
        number_type calculate_query_box_volume(const box_type &region_to_erase) {
            if constexpr (rtree_use_spherical_volume_) {
                return query_box_spherical_volume(region_to_erase); // Slower but helps certain merge cases
            } else {
                return query_box_volume(region_to_erase); // Faster but can cause poor merges
            }
        }

        /// \brief Load branch buffer with branches from full node plus the extra branch.
        void get_rtree_branches(rtree_node *parent_node, const branch_variant &branch_to_insert,
                                partition_vars &partition_vars) {
            assert(parent_node);
            assert(parent_node->count_ == maxnodes_);

            // Load the branch buffer
            for (size_t index = 0; index < maxnodes_; ++index) {
                partition_vars.branch_buffer_[index] = parent_node->branches_[index];
            }

            // Add the branch to the buffer
            partition_vars.branch_buffer_[maxnodes_] = branch_to_insert;
            partition_vars.branch_count_ = maxnodes_ + 1;

            // Calculate rect containing all in the set
            partition_vars.cover_split_ = partition_vars.branch_buffer_[0].rectangle();

            for (size_t index = 1; index < maxnodes_ + 1; ++index) {
                partition_vars.cover_split_ = partition_vars.cover_split_.combine(
                        partition_vars.branch_buffer_[index].rectangle());
            }
            partition_vars.cover_split_area_ = calculate_query_box_volume(partition_vars.cover_split_);
        }

        /// \brief Method #0 for choosing a partition:
        /// As the seeds for the two groups, pick the two rects that would waste the
        /// most area if covered by a single rectangle, i.e. evidently the worst pair
        /// to have in the same group.
        /// Of the remaining, one at a time is chosen to be put in one of the two groups.
        /// The one chosen is the one with the greatest difference in area expansion
        /// depending on which group - the rect most strongly attracted to one group
        /// and repelled from the other.
        /// If one group gets too full (more would force other group to violate min
        /// fill requirement) then other group gets the rest.
        /// These last are the ones that can go in either group most easily.
        void choose_partition(partition_vars &a_par_vars, size_t a_min_fill) {
            number_type biggest_diff;
            int group, chosen = 0, better_group = 0;

            init_partition_variables(a_par_vars, a_par_vars.branch_count_, a_min_fill);
            pick_seeds(a_par_vars);
            // while (not all nodes have groups && groups have less than minimum number of branches (4))
            while (((a_par_vars.count_.first + a_par_vars.count_.second) < static_cast<int>(a_par_vars.total_))
                   && (a_par_vars.count_.first < static_cast<int>(a_par_vars.total_ - a_par_vars.min_fill_))
                   && (a_par_vars.count_.second < static_cast<int>(a_par_vars.total_ - a_par_vars.min_fill_))) {
                biggest_diff = static_cast<number_type>(-1);
                // for each branch
                for (size_t index = 0; index < a_par_vars.total_; ++index) {
                    // if branch is not in a group yet
                    if (partition_vars::NOT_TAKEN == a_par_vars.partition_[index]) {
                        box_type current_query_box = a_par_vars.branch_buffer_[index].rectangle();
                        box_type rect_0 = current_query_box.combine(a_par_vars.cover_.first);
                        box_type rect_1 = current_query_box.combine(a_par_vars.cover_.second);
                        number_type growth_0 = calculate_query_box_volume(rect_0) - a_par_vars.area_.first;
                        number_type growth_1 = calculate_query_box_volume(rect_1) - a_par_vars.area_.second;
                        number_type diff = growth_1 - growth_0;
                        // assign branch to group that causes smallest growth
                        if (diff >= 0) {
                            group = 0;
                        } else {
                            group = 1;
                            diff = -diff;
                        }
                        // save the group that included the element with the biggest difference
                        if (diff > biggest_diff) {
                            biggest_diff = diff;
                            chosen = static_cast<int>(index);
                            better_group = group;
                        } else if ((diff == biggest_diff) &&
                                   (((group == 0) ? a_par_vars.count_.first : a_par_vars.count_.second) <
                                    (better_group == 0 ? a_par_vars.count_.first : a_par_vars.count_.second))) {
                            chosen = static_cast<int>(index);
                            better_group = group;
                        }
                    }
                }
                classify(chosen, better_group, a_par_vars);
            }

            // If one group too full, put remaining rects in the other
            if ((a_par_vars.count_.first + a_par_vars.count_.second) < static_cast<int>(a_par_vars.total_)) {
                // first group is full
                if (a_par_vars.count_.first >= static_cast<int>(a_par_vars.total_ - a_par_vars.min_fill_)) {
                    group = 1;
                } else {
                    group = 0;
                }
                // classify nodes not taken
                for (int index = 0; index < static_cast<int>(a_par_vars.total_); ++index) {
                    if (partition_vars::NOT_TAKEN == a_par_vars.partition_[index]) {
                        classify(index, group, a_par_vars);
                    }
                }
            }

            assert((a_par_vars.count_.first + a_par_vars.count_.second) == static_cast<int>(a_par_vars.total_));
            assert((a_par_vars.count_.first >= static_cast<int>(a_par_vars.min_fill_)) &&
                   (a_par_vars.count_.second >= static_cast<int>(a_par_vars.min_fill_)));
        }

        // Copy branches from the buffer into two nodes according to the partition.
        /// \return Pointer to the node containing the branch
        /// \return Index of the last branch in the node that now contains it
        std::tuple<rtree_node *, size_t>
        load_rtree_nodes(rtree_node *a_nodeA, rtree_node *a_nodeB, partition_vars &a_par_vars) {
            assert(a_nodeA);
            assert(a_nodeB);

            rtree_node *node_with_last_branch;
            size_t last_branch_index;

            // for each branch to include in the nodes
            for (size_t index = 0; index < a_par_vars.total_; ++index) {
                assert(a_par_vars.partition_[index] == 0 || a_par_vars.partition_[index] == 1);
                // node where we should put the branch (0 or 1)
                int target_rtree_node_index = a_par_vars.partition_[index];
                // It is assured that add_rtree_branch here will not cause a node split.
                bool node_was_split;
                if (index != a_par_vars.total_ - 1) {
                    std::tie(node_was_split, std::ignore, std::ignore) = add_rtree_branch(
                            a_par_vars.branch_buffer_[index], (target_rtree_node_index == 0 ? a_nodeA : a_nodeB));
                } else {
                    std::tie(node_was_split, node_with_last_branch, last_branch_index) = add_rtree_branch(
                            a_par_vars.branch_buffer_[index], (target_rtree_node_index == 0 ? a_nodeA : a_nodeB));
                }
                assert(!node_was_split);
            }
            return std::make_tuple(node_with_last_branch, last_branch_index);
        }

        // Initialize a partition_vars structure.
        void init_partition_variables(partition_vars &a_par_vars, size_t a_maxquery_boxs, size_t a_min_fill) {
            a_par_vars.count_.first = a_par_vars.count_.second = 0;
            a_par_vars.area_.first = a_par_vars.area_.second = (number_type) 0;
            a_par_vars.total_ = a_maxquery_boxs;
            a_par_vars.min_fill_ = a_min_fill;
            for (size_t index = 0; index < a_maxquery_boxs; ++index) {
                a_par_vars.partition_[index] = partition_vars::NOT_TAKEN;
            }
        }

        void pick_seeds(partition_vars &a_par_vars) {
            size_t seed0 = 0, seed1 = 0;
            number_type worst, waste;
            number_type area[maxnodes_ + 1];
            // calculate volume of each branch
            for (size_t index = 0; index < a_par_vars.total_; ++index) {
                area[index] = calculate_query_box_volume(a_par_vars.branch_buffer_[index].rectangle());
            }
            // worst area possible (whole partition area)
            worst = -a_par_vars.cover_split_area_ - 1;
            // for each pair of branches
            for (size_t indexA = 0; indexA < a_par_vars.total_ - 1; ++indexA) {
                for (size_t indexB = indexA + 1; indexB < a_par_vars.total_; ++indexB) {
                    // combine box
                    box_type onequery_box = a_par_vars.branch_buffer_[indexA].rectangle().combine(
                            a_par_vars.branch_buffer_[indexB].rectangle());
                    waste = calculate_query_box_volume(onequery_box) - area[indexA] - area[indexB];
                    // store the branches that would create the worst box as seed
                    if (waste > worst) {
                        worst = waste;
                        seed0 = indexA;
                        seed1 = indexB;
                    }
                }
            }

            classify(static_cast<int>(seed0), 0, a_par_vars);
            classify(static_cast<int>(seed1), 1, a_par_vars);
        }

        /// \brief Put a branch in one of the groups.
        void classify(int a_index, int a_group, partition_vars &a_par_vars) {
            assert(partition_vars::NOT_TAKEN == a_par_vars.partition_[a_index]);

            a_par_vars.partition_[a_index] = a_group;

            // Calculate combined rect
            if ((a_group == 0 ? a_par_vars.count_.first : a_par_vars.count_.second) == 0) {
                (a_group == 0 ? a_par_vars.cover_.first
                              : a_par_vars.cover_.second) = a_par_vars.branch_buffer_[a_index].rectangle();
            } else {
                (a_group == 0 ? a_par_vars.cover_.first
                              : a_par_vars.cover_.second) = a_par_vars.branch_buffer_[a_index].rectangle().combine(
                        (a_group == 0 ? a_par_vars.cover_.first : a_par_vars.cover_.second));
            }

            // Calculate volume of combined rect
            (a_group == 0 ? a_par_vars.area_.first : a_par_vars.area_.second) = calculate_query_box_volume(
                    (a_group == 0 ? a_par_vars.cover_.first : a_par_vars.cover_.second));

            ++(a_group == 0 ? a_par_vars.count_.first : a_par_vars.count_.second);
        }

        /// \brief Delete a data rectangle from an index structure.
        /// Pass in a pointer to a box_type, the tid of the record, ptr to ptr to root node.
        /// Returns 1 if record not found, 0 if success.
        /// erase_query_box provides for eliminating the root.
        /// \param region_to_erase
        /// \param mapped_type_to_erase
        /// \param a_root
        /// \return Number of elements erased
        size_t erase_query_box(const box_type &region_to_erase, const mapped_type &mapped_type_to_erase,
                               rtree_node *&root_node) {
            assert(root_node);

            // List of elements we need to reinsert in the tree after removing some branches
            node_list reinsert_list(list_alloc_);

            // Call the recursive erase function. It should return if we found the item
            if (size_t n = erase_query_box_recursive(region_to_erase, mapped_type_to_erase, root_node, reinsert_list)) {
                apply_reinsert_list(root_node, reinsert_list);
                // Return number of elements we erased
                return n;
            } else {
                return 0;
            }
        }

        void apply_reinsert_list(rtree_node *&root_node, node_list &reinsert_list) {
            // Found and deleted a data item
            // Reinsert any branches from eliminated nodes
            // While there are items to reinsert
            while (!reinsert_list.empty()) {
                // Get item from the list
                rtree_node *temp_rtree_node = reinsert_list.front();

                // This list node has less than minnodes_, so
                // For each branch of this node, put it in the root
                for (size_t index = 0; index < temp_rtree_node->count_; ++index) {
                    insert_branch(temp_rtree_node->branches_[index], root_node, temp_rtree_node->level_);
                }

                // Deallocate the original
                deallocate_rtree_node(temp_rtree_node);

                // Next branch
                reinsert_list.pop_front();
            }

            // Check for redundant root (not leaf, 1 child) and eliminate
            while (root_node->count_ == 1 && root_node->is_internal_node()) {
                auto root_parent = root_node->parent_;
                auto root_child = root_node->branches_[0].as_node();
                deallocate_rtree_node(root_node);
                root_node = root_child;
                root_node->parent_ = root_parent;
            }
        }

        size_t erase_query_box_bottom_up(iterator &node_to_erase) {
            // reinsert list in case removal needs to reorganize branches
            node_list reinsert_list(list_alloc_);

            // parent node so we have a reference after (maybe) removing the branch
            rtree_node *parent_node = node_to_erase.current_node_;
            size_t branch_index = node_to_erase.current_branch_;

            // Remove leaf branch
            parent_node->branches_[branch_index] = parent_node->branches_[parent_node->count_ - 1];
            --parent_node->count_;

            // Go up the tree adjusting the rectangles or eliminating branches
            rtree_node *child_node = parent_node;
            parent_node = parent_node->parent_;
            while (parent_node != nullptr) {
                // Look for branch position of child node in the parent
                for (branch_index = 0; branch_index < parent_node->count_; ++branch_index) {
                    if (parent_node->branches_[branch_index].as_node() == child_node) {
                        break;
                    }
                }
                assert(branch_index != parent_node->count_);

                // We have the position of parent_node in the parent_node
                // So we adjust the rectangle for this branch because we just removed an element
                adjust_rectangle_or_eliminate_branch(parent_node, branch_index, reinsert_list);

                // Go to next parent node
                child_node = parent_node;
                parent_node = parent_node->parent_;
            }

            // We created the reinsertion list, so we apply it to the root node
            apply_reinsert_list(root_, reinsert_list);

            // Return that we removed the element
            return 1;
        }

        /// \brief If branch has minimum number of elements, we update its minimum bounding rectangle
        /// If branch does not have minimum number of elements, put its branches on reinsertion list
        /// \param parent_node Node from where we might update or remove the branch
        /// \param index Branch index in the node
        /// \param reinsert_list List with elements to reinsert / removed branches
        void adjust_rectangle_or_eliminate_branch(rtree_node *&parent_node, size_t index, node_list &reinsert_list) {
            // If it erased any points from the branch,
            // If branch still has the minimum number of elements
            if (parent_node->branches_[index].as_node()->count_ >= minnodes_) {
                // Just update the bounding rectangle
                parent_node->branches_[index].as_branch().first = minimum_bounding_rectangle(
                        parent_node->branches_[index].as_node());
            } else {
                // If branch doesn't have the minimum number of elements anymore
                // Eliminate the node from the tree.
                // Put the elements in a reinsert_list.
                reinsert_list.emplace_front(parent_node->branches_[index].as_node());
                // Erase node from the tree
                // erase element by swapping with the last element to prevent gaps in array
                parent_node->branches_[index] = parent_node->branches_[parent_node->count_ - 1];
                --parent_node->count_;
            }
        }

        /// \brief Delete a rectangle from non-root part of an index structure.
        /// Called by erase_query_box.  Descends tree recursively,
        /// merges branches on the way back up.
        /// Returns 1 if record not found, 0 if success.
        /// \param region_to_erase
        /// \param mapped_type_to_erase
        /// \param parent_node
        /// \param reinsert_list
        /// \return Number of elements we have erased
        size_t erase_query_box_recursive(const box_type &region_to_erase, const mapped_type &mapped_type_to_erase,
                                         rtree_node *&parent_node, node_list &reinsert_list) {
            assert(parent_node);
            assert(parent_node->level_ >= 0);

            // If parent is not a leaf node
            if (parent_node->is_internal_node()) {
                // for each branch
                for (size_t index = 0; index < parent_node->count_; ++index) {
                    // if the query boxes overlap
                    if (parent_node->branches_[index].as_branch().first.overlap(region_to_erase)) {
                        // call the erase function recursively to erase from the branch
                        if (size_t n = erase_query_box_recursive(region_to_erase, mapped_type_to_erase,
                                                                 parent_node->branches_[index].as_node(),
                                                                 reinsert_list)) {
                            // count branches and 1. adjust rectangle or 2. eliminate branch
                            adjust_rectangle_or_eliminate_branch(parent_node, index, reinsert_list);
                            // Must return after this call as count has changed
                            return n;
                        }
                    }
                }
                return 0;
            } else {
                // If parent is a leaf node, for each branch...
                for (size_t index = 0; index < parent_node->count_; ++index) {
                    // If the value key is in the region to erase
                    // If the value mapped type has the mapped type we want to erase
                    if (region_to_erase.contains(parent_node->branches_[index].as_value().first)) {
                        // Remove leaf branch
                        parent_node->branches_[index] = parent_node->branches_[parent_node->count_ - 1];
                        --parent_node->count_;
                        // Must return after this call as count has changed
                        return 1;
                    }
                }
                // No leaf is in the region or passes the predicate
                // Return 0 elements returned
                return 0;
            }
        }

        /// \brief Count number of elements
        void count_recursive(const rtree_node *parent_node, size_t &counter) const {
            // not a leaf node
            if (parent_node->is_internal_node()) {
                // call the function recursively for all branches
                for (size_t index = 0; index < parent_node->count_; ++index) {
                    count_recursive(parent_node->branches_[index].as_branch().second, counter);
                }
            } else {
                // A leaf node
                counter += parent_node->count_;
            }
        }

        void copy_recursive(rtree_node *current, rtree_node *current_parent, const rtree_node *other) {
            current->level_ = other->level_;
            current->count_ = other->count_;
            current->parent_ = current_parent;

            // Not a leaf node
            if (current->is_internal_node()) {
                // for each branch
                for (size_t index = 0; index < current->count_; ++index) {
                    std::pair<box_type, rtree_node *> &current_rtree_branch = current->branches_[index].as_branch();
                    const std::pair<box_type, rtree_node *> &other_rtree_branch = other->branches_[index].as_branch();
                    current_rtree_branch.first = other_rtree_branch.first;
                    current_rtree_branch.second = allocate_rtree_node();
                    copy_recursive(current_rtree_branch.second, current, other_rtree_branch.second);
                }
            } else {
                // A leaf node
                for (size_t index = 0; index < current->count_; ++index) {
                    const value_type &other_rtree_branch = other->branches_[index].as_value();
                    current->branches_[index] = branch_variant(
                            std::make_pair(other_rtree_branch.first, other_rtree_branch.second));
                }
            }
        }

        void initialize_unit_sphere_volume() {
            constexpr long double pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933446128475648233786783165271201909145648566923460348610454326648213393607260249141273;

            // Precomputed volumes of the unit spheres for the first few dimensions
            constexpr double UNIT_SPHERE_VOLUMES[] = {
                    1.000000, // 0 dimensions
                    2.000000, // 1 dimensions
                    3.141592653589793116, // 2 dimensions
                    4.1887902047863905253, // 3 dimensions
                    4.9348022005446789962, // 4 dimensions
                    5.2637890139143239665, // 5 dimensions
                    5.1677127800499693677, // 6 dimensions
                    4.7247659703314006663, // 7 dimensions
                    4.058712126416767596, // 8 dimensions
                    3.2985089027387064498, // 9 dimensions
                    2.5501640398773450791, // 10 dimensions
                    1.8841038793898998716, // 11 dimensions
                    1.335262768854589277, // 12 dimensions
                    0.91062875478328297874, // 13 dimensions
                    0.59926452932079199432, // 14 dimensions
                    0.38144328082330442031, // 15 dimensions
                    0.23533063035889315029, // 16 dimensions
                    0.14098110691713899856, // 17 dimensions
                    0.082145886611128204891, // 18 dimensions
                    0.046621601030088527517, // 19 dimensions
                    0.025806891390014050774, // 20 dimensions
                    0.013949150409020994595, // 21 dimensions
                    0.0073704309457143478423, // 22 dimensions
                    0.0038106563868521231911, // 23 dimensions
                    0.0019295743094039222772, // 24 dimensions
                    0.00095772240882317240772, // 25 dimensions
                    0.00046630280576761233688, // 26 dimensions
                    0.00022287212472127398847, // 27 dimensions
                    0.00010463810492484565049, // 28 dimensions
                    4.8287822738917413415e-05, // 29 dimensions
                    2.191535344783020374e-05, // 30 dimensions
                    9.7871399467373613697e-06, // 31 dimensions
                    4.3030695870329447526e-06, // 32 dimensions
                    1.8634670882621389836e-06, // 33 dimensions
                    7.952054001475508376e-07, // 34 dimensions
                    3.3452882941089706486e-07, // 35 dimensions
                    1.3878952462213763426e-07, // 36 dimensions
                    5.6808287183311750687e-08, // 37 dimensions
                    2.2948428997269856493e-08, // 38 dimensions
                    9.1522306501595595658e-09, // 39 dimensions
                    3.6047307974624982221e-09, // 40 dimensions
                    1.4025649060732004594e-09, // 41 dimensions
                    5.3926646626081237818e-10, // 42 dimensions
                    2.0494360953964766978e-10, // 43 dimensions
                    7.7007071306013460981e-11, // 44 dimensions
                    2.8615526139108097209e-11, // 45 dimensions
                    1.0518471716932053807e-11, // 46 dimensions
                    3.8254607105203703415e-12, // 47 dimensions
                    1.3768647280377399728e-12, // 48 dimensions
                    4.9053221488845654961e-13, // 49 dimensions
                    1.7302192458361089539e-13, // 50 dimensions
            };

            if (dimensions() <= 50) {
                unit_sphere_volume_ = (number_type) UNIT_SPHERE_VOLUMES[NUMBER_OF_DIMENSIONS];
            } else {
                // number_type v0 = 1.;
                // number_type v1 = 2.;
                size_t i = 2;
                number_type vi_minus_2 = static_cast<number_type>(1.);
                number_type vi_minus_1 = static_cast<number_type>(2.);
                number_type vi = static_cast<number_type>(
                        (static_cast<number_type>(2.) * static_cast<number_type>(pi)) / static_cast<number_type>(i) *
                        vi_minus_2);
                while (i < number_of_compile_dimensions_) {
                    ++i;
                    vi_minus_2 = vi_minus_1;
                    vi_minus_1 = vi;
                    vi = static_cast<number_type>(
                            (static_cast<number_type>(2.) * static_cast<number_type>(3.14159265359)) /
                            static_cast<number_type>(i) * vi_minus_2);
                }
                unit_sphere_volume_ = vi;
            }
        }

        std::pair<rtree_node *, size_t> recursive_max_element(rtree_node *parent_node, size_t dimension) const {
            auto max_it = std::max_element(parent_node->branches_.begin(),
                                           parent_node->branches_.begin() + parent_node->count_,
                                           [&dimension](const branch_variant &a, const branch_variant &b) {
                                               return (a.is_value() ? a.as_value().first[dimension]
                                                                    : a.as_branch().first.second()[dimension]) <
                                                      (b.is_value() ? b.as_value().first[dimension]
                                                                    : b.as_branch().first.second()[dimension]);
                                           });
            if (max_it->is_value()) {
                return std::make_pair(parent_node, max_it - parent_node->branches_.begin());
            }
            return recursive_max_element(max_it->as_node(), dimension);
        }

        std::pair<rtree_node *, size_t> recursive_min_element(rtree_node *parent_node, size_t dimension) const {
            auto min_it = std::min_element(parent_node->branches_.begin(),
                                           parent_node->branches_.begin() + parent_node->count_,
                                           [&dimension](const branch_variant &a, const branch_variant &b) {
                                               return (a.is_value() ? a.as_value().first[dimension]
                                                                    : a.as_branch().first.first()[dimension]) <
                                                      (b.is_value() ? b.as_value().first[dimension]
                                                                    : b.as_branch().first.first()[dimension]);
                                           });
            if (min_it->is_value()) {
                return std::make_pair(parent_node, min_it - parent_node->branches_.begin());
            }
            return recursive_min_element(min_it->as_node(), dimension);
        }

        void remove_all_records(rtree_node *node) {
            assert(node);
            assert(node->level_ >= 0);
            if (node->is_internal_node()) {
                for (size_t index = 0; index < node->count_; ++index) {
                    assert(node->branches_[index].as_node() != node->parent_);
                    remove_all_records(node->branches_[index].as_node());
                }
            }
            deallocate_rtree_node(node);
        }

        /// \brief Bulk insertion inserts the median before other elements
        template<class InputIterator>
        void bulk_insert(InputIterator l_begin, InputIterator l_end, const value_type &v, InputIterator r_begin,
                         InputIterator r_end) {
            bulk_insert(l_begin, l_end, v, r_begin, r_end, root_);
        }

        void bulk_insert(const std::vector<value_type> &v, rtree_node *&node) {
            // bulk insert ranges {1, median - 1}, median, { median + 1, end()}
            if (!v.empty()) {
                if (v.size() == 1) {
                    insert_branch(branch_variant(v[0]), node, 0);
                } else {
                    size_t median_pos = v.size() / 2;
                    bulk_insert(v.begin(), v.begin() + median_pos, v[median_pos], v.begin() + median_pos + 1, v.end(),
                                node);
                }
            }
        }

        template<class InputIterator>
        void bulk_insert(InputIterator l_begin, InputIterator l_end, const value_type &v, InputIterator r_begin,
                         InputIterator r_end, rtree_node *&node) {
            insert_branch(branch_variant(v), node, 0);
            size_t l_size = std::distance(l_begin, l_end);
            if (l_size != 0) {
                if (l_size == 1) {
                    insert_branch(branch_variant(*l_begin), node, 0);
                } else {
                    size_t l_median_pos = l_size / 2;
                    bulk_insert(l_begin, l_begin + l_median_pos, *(l_begin + l_median_pos), l_begin + l_median_pos + 1,
                                l_end, node);
                }
            }
            size_t r_size = std::distance(r_begin, r_end);
            if (r_size != 0) {
                if (r_size == 1) {
                    insert_branch(branch_variant(*r_begin), node, 0);
                } else {
                    size_t r_median_pos = r_size / 2;
                    bulk_insert(r_begin, r_begin + r_median_pos, *(r_begin + r_median_pos), r_begin + r_median_pos + 1,
                                r_end, node);
                }
            }
        }

        /// \brief Root of tree
        rtree_node *root_;

        /// \brief Number of elements in the tree
        /// We cache the size at each insertion/removal.
        /// Otherwise it would cost us O(n log(n)) to find out the size
        size_t size_{0};

        /// \brief Number of dimensions of this rtree
        /// This is only needed when the dimension will be defined at runtime
        size_t dimensions_{0};

        /// \brief Unit sphere constant for required number of dimensions
        number_type unit_sphere_volume_;

        /// \brief Node allocator
        /// It's fundamental to allocate our nodes with an efficient allocator
        /// to have a tree that can compete with vectors
        /// This is a shared ptr because trees can also share the same
        /// allocator. The last one to use it should delete the allocator.
        /// This is an important component of archives, where it's
        /// unreasonable to create an allocator for every new front.
        std::shared_ptr<node_allocator_type> alloc_;

        /// \brief Allocator for lists
        list_allocator_type list_alloc_;

    };

    // MSVC hack
    template<class NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, class ELEMENT_TYPE, template<typename> class ALLOCATOR>
    template<bool constness>
    const std::function<bool(
            const typename r_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>::template iterator_impl<constness>::queue_element &,
            const typename r_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>::template iterator_impl<constness>::queue_element &)> r_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>::iterator_impl<constness>::queue_comp = [](
            const typename r_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>::template iterator_impl<constness>::queue_element &a,
            const typename r_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>::template iterator_impl<constness>::queue_element &b) -> bool {
        return std::get<2>(a) > std::get<2>(b);
    };
}


#endif //PARETO_FRONT_R_TREE_H
