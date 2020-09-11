//
// Created by Alan Freitas on 2020-06-03.
//

#ifndef PARETO_FRONT_QUAD_TREE_H
#define PARETO_FRONT_QUAD_TREE_H

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
#include <sstream>

#include <pareto_front/point.h>
#include <pareto_front/query_box.h>
#include <pareto_front/predicates.h>
#include <pareto_front/memory_pool.h>

namespace pareto {
    template<typename NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, typename ELEMENT_TYPE, typename TAG>
    class front;
    struct quad_tree_tag;

    /// \class quad_tree
    /// Implementation of a Point Quad-Tree with an interface that matches the pareto front.
    /// Quad-trees are the classic benchmark for Pareto Fronts.
    /// If points have n dimensions, each node potencially has 2^n children.
    /// We determine the child index by comparing the points in each dimension.
    ///
    /// In our quad-tree variant, we keep the minimum bounding rectangle of all
    /// tree branches, so that we can more easily query the data. This is the
    /// only adaptation we include. Unlike an R-Tree, this is not a fundamental
    /// part of the data structure, it's only a way to have pre-processed data
    /// for queries. The query for which this is most useful is the max/min
    /// query, which would need to iterate many nodes without this information.
    ///
    /// This implementation used
    /// https://github.com/danshapero/quadtree
    /// as reference for correctness, but the design is completely different.
    /// \see https://en.wikipedia.org/wiki/Quadtree#Point_quadtree
    template<class NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, class ELEMENT_TYPE, template<typename> class ALLOCATOR = default_fast_memory_pool>
    class quad_tree {
    public:
        friend front<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, quad_tree_tag>;
        using self_type = quad_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>;
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
        /// Node
        /// Each branch might have a quadtree_node for each branch level
        /// The number of children is fixed because we need to make the
        /// size of a node constant. This makes it possible to use
        /// allocators efficiently.
        struct quadtree_node {
            quadtree_node() : quadtree_node(nullptr) {}

            quadtree_node(quadtree_node* parent)
                    : parent_(parent) {}

            quadtree_node(quadtree_node* parent, const value_type& value)
                    : value_(value), parent_(parent), bounds_(box_type(value.first)) {}

            /// An internal node, contains other nodes
            bool is_internal_node() const {
                return !children_.empty();
            }

            /// A leaf, contains data only
            bool is_leaf_node() const {
                return children_.empty();
            }

            /// Value: a quad-tree node holds a value itself.
            value_type value_;

            /// Node children
            /// Unlike other data structures, we use a map instead of
            /// an array because a quad_tree node can potentially have
            /// 2^n children. We cannot waste space with an exponential
            /// number of children that don't exist on every node.
            using children_map = std::map<size_t, quadtree_node*>;
            children_map children_;


            /// Parent node
            /// The quad-tree nodes only need values and branches. All other
            /// member elements are conveniences for faster queries.
            /// Keeping track of parent nodes is important for more efficient
            /// iterators. A link to the parent node allows us to eliminate
            /// stacks and easily start the iterator from any node. This makes
            /// the use of iterators with predicates much easier because we just
            /// have to move forward until a predicate is valid.
            quadtree_node *parent_;

            /// Minimal Bounding Rectangle
            /// The bounds are useful to make queries faster
            /// We can eliminate branches by only checking their
            /// bounds.
            /// The bounds of a point quad-tree are implicitly defined
            /// by the points in the node and its children.
            box_type bounds_;

        };

        using node_allocator_type = allocator_type<quadtree_node>;

        /// Check if using the fast allocator
        constexpr static bool is_using_default_fast_allocator() {
            return std::is_same_v<node_allocator_type, default_fast_memory_pool<quadtree_node>>;
        }

    public /* iterators */:

        /// Iterator is not erase safe. Erase elements will invalidate the iterators.
        /// Because iterator and const_iterator are almost the same,
        /// we define iterator as iterator<false> and const_iterator as iterator<true>
        /// \see https://stackoverflow.com/questions/2150192/how-to-avoid-code-duplication-implementing-const-and-non-const-iterators
        template<bool is_const = false>
        class iterator_impl {
        public:
            // friend const_iterator;
            friend quad_tree;
            using difference_type = typename self_type::difference_type;
            using value_type = std::conditional_t<is_const, const std::pair<key_type, mapped_type>, std::pair<const key_type, mapped_type>>;
            using reference = value_type &;
            using const_reference = value_type const &;
            using pointer = value_type *;
            using const_pointer = value_type const *;
            using node_pointer_type = std::conditional_t<is_const, const quadtree_node *, quadtree_node *>;
            using context_pointer_type = std::conditional_t<is_const, const quad_tree *, quad_tree *>;
            using iterator_category = std::forward_iterator_tag;
            using query_predicate_type = query_predicate<number_type, number_of_compile_dimensions_, mapped_type>;
            enum class iterator_tag {
                begin,
                end
            };

            /// Default
            explicit iterator_impl() : iterator_impl(nullptr, nullptr) {}

            /// Begin or end iterator
            iterator_impl(context_pointer_type context, iterator_tag is_end)
                    : iterator_impl(context, is_end == iterator_tag::begin ? context->root_ : nullptr) {}

            /// Begin or end iterator
            iterator_impl(context_pointer_type context)
                    : iterator_impl(context, iterator_tag::begin) {}

            /// Tree and root node
            /// When the current pointer == nullptr, we are at end().
            /// We need the context so that a.end() != b.end().
            iterator_impl(context_pointer_type context, node_pointer_type root_)
            : context_(context), current_node_(root_), nearest_predicate_(nullptr),
              nearest_queue_{}, nearest_points_iterated_(0) {
                advance_if_invalid();
            }

            /// Iterator with predicate initializer list
            iterator_impl(context_pointer_type context, node_pointer_type root_, std::initializer_list<query_predicate_type> predicate_list)
                    : iterator_impl(context, root_, predicate_list.begin(), predicate_list.end()) {}

            /// Iterator with predicate vector
            iterator_impl(context_pointer_type context, node_pointer_type root_, const std::vector<query_predicate_type> &predicate_list)
                    : iterator_impl(context, root_, predicate_list.begin(), predicate_list.end()) {}

            /// Iterator with iterators to predicates
            template<class predicate_iterator_type>
            iterator_impl(context_pointer_type context, node_pointer_type root_, predicate_iterator_type predicate_begin, predicate_iterator_type predicate_end)
                    : context_(context), current_node_(root_),
                      predicates_(predicate_begin, predicate_end), nearest_predicate_(nullptr),
                      nearest_queue_{}, nearest_points_iterated_(0) {
                sort_predicates();
                normalize_nearest_queries();
                advance_if_invalid();
            }

            /// Copy constructor
            template<bool constness>
            iterator_impl(const iterator_impl<constness> &rhs)
                    : context_(rhs.context_), current_node_(rhs.current_node_),
                      predicates_(rhs.predicates_), nearest_predicate_(nullptr),
                      nearest_points_iterated_(rhs.nearest_points_iterated_) {
                if (rhs.nearest_predicate_ != nullptr) {
                    for (auto &p: predicates_) {
                        if (p.is_nearest()) {
                            nearest_predicate_ = &p.as_nearest();
                            break;
                        }
                    }
                }
                nearest_queue_.reserve(rhs.nearest_queue_.size());
                for (const auto&[a, b, c]: rhs.nearest_queue_) {
                    nearest_queue_.emplace_back(std::make_tuple(a, b, c));
                }
                advance_if_invalid();
            }

            iterator_impl(const iterator_impl &rhs)
                    : context_(rhs.context_), current_node_(rhs.current_node_),
                      predicates_(rhs.predicates_), nearest_predicate_(nullptr),
                      nearest_points_iterated_(rhs.nearest_points_iterated_) {
                if (rhs.nearest_predicate_ != nullptr) {
                    for (auto &p: predicates_) {
                        if (p.is_nearest()) {
                            nearest_predicate_ = &p.as_nearest();
                            break;
                        }
                    }
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
                context_ = rhs.context_;
                current_node_ = rhs.current_node_;
                predicates_ = nullptr;
                if (rhs.nearest_predicate_ != nullptr) {
                    for (auto &p: predicates_) {
                        if (p.is_nearest()) {
                            nearest_predicate_ = &p.as_nearest();
                            break;
                        }
                    }
                }
                nearest_points_iterated_ = rhs.nearest_points_iterated_;
                nearest_queue_ = rhs.nearest_queue_;
                return *this;
            }

            /// The equality operator ignores the predicates
            /// It only matters if they point to the same element here
            bool operator==(const iterator_impl &rhs) const {
                return (current_node_ == rhs.current_node_) && (context_ == rhs.context_);
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
                return current_node_->value_;
            }

            template<bool _constness = is_const>
            std::enable_if_t<!_constness, reference>
            operator*() const {
                assert(is_not_null());
                std::pair<key_type, mapped_type> &non_const_ref = current_node_->value_;
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
                const bool need_to_iterate_to_nearest = nearest_predicate_ != nullptr ? nearest_points_iterated_ == 0 : false;
                if (need_to_iterate_to_nearest || !passes_predicates(current_node_->value_)) {
                    // advance if current is not valid
                    advance_to_next_valid(false);
                }
            }

            /// Is iterator_impl at the end
            bool is_end() const {
                // We represent the "end" by keeping a pointer to nullptr
                return current_node_ == nullptr || context_ == nullptr;
            }

            /// Is iterator_impl at the begin
            bool is_begin() const {
                // if there's no object, this can be both begin and end
                if (context_ == nullptr) {
                    return true;
                }
                // in the general case, begin is the root node (depth-first)
                return current_node_ == context_->root_;
            }

            /// Is iterator_impl pointing to valid data
            bool is_not_null() const {
                return !is_end();
            }

            bool passes_predicates(const box_type &b) const {
                for (const auto &p: predicates_) {
                    if (!p.pass_predicate(b)) {
                        return false;
                    }
                }
                return true;
            }

            bool passes_predicates(const value_type &pnt) const {
                for (const auto &p: predicates_) {
                    if (!p.pass_predicate(pnt)) {
                        return false;
                    }
                }
                return true;
            }

            bool might_pass_predicates(const box_type &b) const {
                for (const auto &p: predicates_) {
                    if (!p.might_pass_predicate(b)) {
                        return false;
                    }
                }
                return true;
            }

            void normalize_nearest_queries() {
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
                    iterator_impl it = iterator_impl(context_, root, predicates_);
                    iterator_impl end = iterator_impl(context_, iterator_tag::end);
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
                // This line in the algorithm is just to ensure there's something to
                // dequeue at the first iteration. For this reason, the distance doesn't matter.
                // It's just easier to Enqueue all branches of quad-tree.RootNode
                // already, as would always happen at the first iteration anyway.
                // Because this is a quad-tree, we also enqueue the node value.
                nearest_queue_.emplace_back(current_node_, true,
                                            nearest_predicate_->distance(current_node_->value_.first));
                std::push_heap(nearest_queue_.begin(), nearest_queue_.end(), queue_comp);
                for (auto& child: current_node_->children_) {
                    nearest_queue_.emplace_back(child.second, false,
                                                nearest_predicate_->distance(current_node_->bounds_));
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
                    bool is_value;
                    bool passed_predicate;
                    std::tie(current_node_, is_value, passed_predicate) = nearest_set_[nearest_points_iterated_];
                    assert(is_value);
                    ++nearest_points_iterated_;
                    if (passed_predicate) {
                        return;
                    }
                }

                // We go to the real algorithm after dealing with the trivial cases
                // The first steps (1 and 2) were executed in 'normalize_nearest_queries'
                // The advance step starts the loop that looks for more nearest elements
                // 3. while not IsEmpty(Queue) do
                while (!nearest_queue_.empty()) {
                    // 4. Element <- Dequeue(Queue)
                    auto [element_node, is_value, distance] = nearest_queue_.front();
                    auto &element = element_node->value_;
                    std::pop_heap(nearest_queue_.begin(), nearest_queue_.end(), queue_comp);
                    nearest_queue_.pop_back();
                    // 5. if element is an object or its bounding rectangle
                    // The way we represent the tree, it's never a bounding rectangle
                    // because we only represent points in our tree. So we only test
                    // if it's an object (a value).
                    if (is_value) {
                        // 6. if Element is the bounding rectangle
                        //     7. That never happens in our tree
                        // 8. else
                        // 9.     Report Element
                        // in our version, we only report it if it also passes
                        // the other predicates
                        if (passes_predicates(element)) {
                            ++nearest_points_iterated_;
                            current_node_ = element_node;
                            // put it in the pre-processed set of results
                            nearest_set_.emplace_back(current_node_, true, true);
                            return;
                        } else {
                            // if it doesn't pass the predicates, we just count it
                            // as one more nearest point we would have reported
                            ++nearest_points_iterated_;
                            nearest_set_.emplace_back(current_node_, true, false);
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
                        nearest_queue_.emplace_back(element_node, true,
                                                    nearest_predicate_->distance(element_node->value_.first));
                        std::push_heap(nearest_queue_.begin(), nearest_queue_.end(), queue_comp);
                        for (auto& child: element_node->children_) {
                            // 13. Enqueue(Queue, [Object], Dist(QueryObject, Rect))
                            // 17. Enqueue(Queue, Node,     Dist(QueryObject, Rect))
                            nearest_queue_.emplace_back(child.second, false,
                                                        nearest_predicate_->distance(child.second->bounds_));
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
                        bool is_value;
                        bool passed_predicate;
                        std::tie(current_node_, is_value, passed_predicate) = nearest_set_[nearest_points_iterated_ - 1];
                        assert(is_value);
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
                current_node_ = nullptr;
            }

            void return_to_begin() {
                if (context_ != nullptr) {
                    current_node_ = context_->root_;
                }
            }

            void advance_to_next_valid_depth_first(bool first_time_in_this_branch = false) {
                // this should usually be the case at the start
                while (!is_end()) {
                    // return if first time
                    // if we haven't checked the current node yet
                    if (first_time_in_this_branch) {
                        if (passes_predicates(current_node_->value_)) {
                            // found a valid value in current node
                            // point to it (already does) and return
                            return;
                        }
                        first_time_in_this_branch = false;
                    } else {
                        // if we already checked this node,
                        // go to first valid child if there are children, stop iterating
                        bool predicate_might_pass = false;
                        // if it has children, try to go to its first child
                        // that might pass the predicate
                        for (auto& child: current_node_->children_) {
                            // If node children might have nodes that pass all predicates
                            if (might_pass_predicates(child.second->bounds_)) {
                                // Found a child that might pass predicates
                                // Point to it and continue looking until we find a value_type
                                // that actually passes the predicate
                                current_node_ = child.second;
                                predicate_might_pass = true;
                                first_time_in_this_branch = true;
                                break;
                            }
                        }

                        // go to right sibling, stop iterating
                        // No more branches that could pass the predicates to test
                        // Go to previous level / parent node
                        // In the parent node we look for the next sibling that might
                        // pass the predicates. If no siblings can pass the predicates,
                        // we go to the next parent again and look for more siblings.
                        // We do that until we reach the end.
                        if (!predicate_might_pass) {
                            // the function already goes to end() if no right sibling, stop iterating
                            go_to_sibling(true);
                            first_time_in_this_branch = true;
                        }
                    }
                }
            }

            void return_to_previous_valid_depth_first(bool first_time_in_this_branch = false) {
                // this should usually be the case at the start
                while (!is_begin()) {
                    // return if first time
                    if (first_time_in_this_branch) {
                        if (passes_predicates(current_node_->value_)) {
                            // found a valid value in current node
                            // point to it (already does) and return
                            return;
                        }
                        first_time_in_this_branch = false;
                    } else {
                        // go to left sibling and then to rightmost valid element, stop iterating
                        // if this is end(), left sibling will be the root node
                        auto previous_node = current_node_;
                        // this function already goes to parent if there are no left siblings, stop iterating
                        go_to_sibling(false);
                        // go_to_sibling goes to parent when there is no sibling
                        // go_to_sibling goes to root when it's end()
                        bool there_was_sibling = previous_node == nullptr || previous_node->parent_ != current_node_;
                        if (there_was_sibling) {
                            // go to rightmost valid element in the sibling
                            // we will visit this node later when we go to the parent
                            bool children_might_pass_predicate = true;
                            while (children_might_pass_predicate) {
                                children_might_pass_predicate = false;
                                for (auto child_it = current_node_->children_.rbegin(); child_it != current_node_->children_.rend(); ++child_it) {
                                    auto& child = *child_it;
                                    if (might_pass_predicates(child.second->bounds_)) {
                                        current_node_ = child.second;
                                        children_might_pass_predicate = true;
                                        break;
                                    }
                                }
                            }
                        }
                        first_time_in_this_branch = true;
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

            void go_to_sibling(bool move_right = true) {
                // if we are at the "end" element and we are moving right
                if (!move_right && current_node_ == nullptr) {
                    // the root is considered the
                    current_node_ = context_->root_;
                    return;
                }
                while (current_node_->parent_ != nullptr) {
                    // Move to parent and find index of current branch there
                    auto branch_address = current_node_;
                    current_node_ = current_node_->parent_;
                    size_t branch_index = current_node_->value_.first.quadrant(branch_address->value_.first);
                    auto branch_it = current_node_->children_.find(branch_index);
                    assert(branch_it != current_node_->children_.end());
                    // Look for next sibling that might pass the predicates
                    if (move_right) {
                        ++branch_it;
                        for (; branch_it != current_node_->children_.end(); ++branch_it) {
                            if (might_pass_predicates(branch_it->second->bounds_)) {
                                current_node_ = branch_it->second;
                                return;
                            }
                        }
                    } else {
                        // if not at first child yet
                        while (branch_it != current_node_->children_.begin()) {
                            // go to previous child (which might also be the first child)
                            --branch_it;
                            // try to pass the predicate there
                            if (might_pass_predicates(branch_it->second->bounds_)) {
                                current_node_ = branch_it->second;
                                return;
                            }
                        }
                        // if we got to or were at the beginning
                        if (branch_it == current_node_->children_.begin()) {
                            // go to parent and take it from there
                            current_node_ = branch_it->second->parent_;
                            return;
                        }
                    }
                }
                // If we went through all nodes and got to the root node
                // there are no siblings, we just go to end
                if (current_node_->parent_ == nullptr) {
                    if (move_right) {
                        // if no parent, we move to the end / move right
                        current_node_ = nullptr;
                    } else {
                        // if no parent, we move to begin / move left
                        *this = iterator_impl(context_, context_->root_, predicates_);
                    }
                }
            }

            void sort_predicates() {
                // handle the most trivial cases
                // there is nothing to sort if there are less than 2 elements
                if (predicates_.size() < 2) {
                    return;
                }
                // if it's a query box and a predicate, just swap or not
                if (predicates_.size() == 2) {
                    if (predicates_[0].is_intersects() || predicates_[0].is_within() || predicates_[0].is_disjoint()) {
                        if (predicates_[1].is_satisfies() || predicates_[1].is_nearest()) {
                            return;
                        }
                    }
                    if (predicates_[0].is_satisfies() || predicates_[0].is_nearest()) {
                        if (predicates_[1].is_intersects() || predicates_[1].is_within() ||
                            predicates_[1].is_disjoint()) {
                            std::swap(predicates_[0], predicates_[1]);
                            return;
                        }
                    }
                }
                // for the more general case
                number_type volume_root = 0.;
                // if there is any disjoint predicate, we need the root volume
                if (std::find_if(predicates_.begin(), predicates_.end(),
                                 [](const auto &x) { return x.is_disjoint(); }) != predicates_.end()) {
                    // find root node
                    auto root = current_node_;
                    while (root->parent_ != nullptr) {
                        root = root->parent_;
                    }
                    // calculate volume of root minimum bounding rectangle
                    volume_root = root->bounds_.volume();
                }
                // sort predicates by how restrictive they are
                std::sort(predicates_.begin(), predicates_.end(), [&volume_root](const auto &a, const auto &b) {
                    return a.is_more_restrictive(b, volume_root);
                });
            }

            /// Stack as we are doing iteration instead of recursion
            context_pointer_type context_;

            /// Stack as we are doing iteration instead of recursion
            node_pointer_type current_node_;

            /// Predicate constraining the search area
            std::vector<query_predicate<number_type, number_of_compile_dimensions_, mapped_type>> predicates_;

            /// Pointer to a nearest predicate
            nearest<number_type, number_of_compile_dimensions_> *nearest_predicate_ = nullptr;

            // Pair with branch (node or object) and distance to the reference point
            // The branch is represented by the node and a boolean marking
            // if the pointer represents itself or its value
            // tuple<node, represents value, distance>
            using queue_element = std::tuple<node_pointer_type, bool, distance_type>;

            // Function to compare queue_elements by their distance to the reference point
            static const std::function<bool(const queue_element&, const queue_element&)> queue_comp;

            // Queue <- NewPriorityQueue()
            std::vector<queue_element> nearest_queue_;

            // Number of nearest points we have iterated so far
            size_t nearest_points_iterated_{0};

            // Set of nearest values we have already found
            // Some pointers represent the value of a node
            // Some pointers represent the node itself with its bounding box
            // tuple<node, represents value, passed predicate>
            std::vector<std::tuple<node_pointer_type, bool, bool>> nearest_set_;

            // Allow hiding of non-public functions while allowing manipulation by logical owner
            friend class quad_tree;
        };

        using iterator = iterator_impl<false>;
        using const_iterator = iterator_impl<true>;

    public:
        /// Construct
        quad_tree() : root_(nullptr), size_(0), dimensions_(number_of_compile_dimensions_), alloc_(new node_allocator_type()) {}

        /// Constructor that shares an external allocator
        quad_tree(std::shared_ptr<node_allocator_type>& external_alloc) :
                root_(nullptr),
                size_(0),
                dimensions_(number_of_compile_dimensions_),
                alloc_(external_alloc) {}

        /// Construct from points
        template<class InputIterator>
        quad_tree(InputIterator first, InputIterator last) : quad_tree() {
            // sort points and bulk insert
            std::vector<value_type> v(first, last);
            std::sort(v.begin(), v.end());
            bulk_insert(v, root_);

        }

        /// Copy constructor
        quad_tree(const quad_tree &other) :
                size_(other.size_),
                dimensions_(other.dimensions_),
                alloc_(other.alloc_) {
            root_ = allocate_quadtree_node();
            copy_recursive(root_, nullptr, other.root_);
        }

        /// Copy constructor
        quad_tree &operator=(const quad_tree &other) {
            remove_all_records();
            size_ = other.size_;
            dimensions_ = other.dimensions_;
            alloc_ = other.alloc_;
            if (other.root_ != nullptr) {
                root_ = allocate_quadtree_node();
                copy_recursive(root_, nullptr, other.root_);
            } else {
                root_ = nullptr;
            }
            return *this;
        }

        /// Destructor
        virtual ~quad_tree() {
            remove_all_records();
        }

    public /* iterators */:

        const_iterator begin() const noexcept {
            return root_ ? const_iterator(this, root_) : end();
        }

        const_iterator end() const noexcept {
            return const_iterator(this, const_iterator::iterator_tag::end);
        }

        iterator begin() noexcept {
            return root_ ? iterator(this, root_) : end();
        }

        iterator end() noexcept {
            return iterator(this, iterator::iterator_tag::end);
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

        /// Equality
        /// \warning This operator tells us if the trees are equal
        /// and not if they contain the same elements
        /// Two trees might contain the same elements in different
        /// trees if the insertion order was different
        /// If that's what you want to test, you need a very complicated
        /// algorithm or you need to move your elements to another
        /// sorted container before comparing.
        /// We implement it this way because this operation is the only
        /// one we need for our front application.
        bool operator==(const self_type &rhs) const {
            return std::equal(begin(), end(), rhs.begin(), rhs.end(), [](const auto& a, const auto& b) {
                return a.first == b.first && mapped_type_custom_equality_operator(a.second, b.second);
            });
        }

        bool operator!=(const self_type &rhs) const {
            return !std::equal(begin(), end(), rhs.begin(), rhs.end(), [](const auto& a, const auto& b) {
                return a.first == b.first && mapped_type_custom_equality_operator(a.second, b.second);
            });
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

        iterator begin_intersection(const point_type &min_corner, const point_type &max_corner) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return iterator(this, root_, {intersects(min_corner_, max_corner_)});
        }

        const_iterator begin_intersection(const point_type &min_corner, const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return const_iterator(this, root_, {intersects(min_corner_, max_corner_)});
        }

        iterator begin_intersection(const point_type &min_corner, const point_type &max_corner,
                                    std::function<bool(const value_type &)> fn) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return iterator(this, root_, {intersects(min_corner_, max_corner_), satisfies(fn)});
        }

        const_iterator begin_intersection(const point_type &min_corner, const point_type &max_corner,
                                          std::function<bool(const value_type &)> fn) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return const_iterator(this, root_, {intersects(min_corner_, max_corner_), satisfies(fn)});
        }

        iterator begin_within(const point_type &min_corner, const point_type &max_corner) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return iterator(this, root_, {within(min_corner_, max_corner_)});
        }

        const_iterator begin_within(const point_type &min_corner, const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return const_iterator(this, root_, {within(min_corner_, max_corner_)});
        }

        iterator begin_disjoint(const point_type &min_corner, const point_type &max_corner) {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return iterator(this, root_, {disjoint(min_corner_, max_corner_)});
        }

        const_iterator begin_disjoint(const point_type &min_corner, const point_type &max_corner) const {
            point_type min_corner_ = min_corner;
            point_type max_corner_ = max_corner;
            normalize_corners(min_corner_, max_corner_);
            return const_iterator(this, root_, {disjoint(min_corner_, max_corner_)});
        }

        /// Find the point closest to this point
        /// We adapted the algorithm a little so it would work for quad-trees
        /// instead of r-trees
        /// \see Hjaltason, Gísli R., and Hanan Samet. "Distance browsing in spatial databases." ACM Transactions on Database Systems (TODS) 24.2 (1999): 265-318.
        /// \see https://dl.acm.org/doi/pdf/10.1145/320248.320255
        /// \see https://stackoverflow.com/questions/45816632/nearest-neighbor-algorithm-in-r-tree
        iterator begin_nearest(const point_type &p) {
            return iterator(this, root_, {nearest(p)});
        }

        const_iterator begin_nearest(const point_type &p) const {
            return const_iterator(this, root_, {nearest(p)});
        }

        iterator begin_nearest(const point_type &p, size_t k) {
            return iterator(this, root_, {nearest(p, k)});
        }

        const_iterator begin_nearest(const point_type &p, size_t k) const {
            return const_iterator(this, root_, {nearest(p, k)});
        }

        iterator begin_nearest(const box_type &b, size_t k) {
            return iterator(this, root_, {nearest(b, k)});
        }

        const_iterator begin_nearest(const box_type &b, size_t k) const {
            return const_iterator(this, root_, {nearest(b, k)});
        }

        iterator begin_nearest(const point_type &p, size_t k, std::function<bool(const value_type &)> fn) {
            return iterator(this, root_, {nearest(p, k), satisfies(fn)});
        }

        const_iterator begin_nearest(const point_type &p, size_t k, std::function<bool(const value_type &)> fn) const {
            return const_iterator(this, root_, {nearest(p, k), satisfies(fn)});
        }

    public /* non-modifying functions */:
        bool empty() const noexcept {
            return root_ == nullptr;
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
            return root_ ? root_->bounds_.max()[dimension] : std::numeric_limits<number_type>::max();
        }

        number_type min_value(size_t dimension) const {
            return root_ ? root_->bounds_.min()[dimension] : std::numeric_limits<number_type>::min();
        }


        iterator max_element(size_t dimension) {
            if (empty()) {
                return end();
            }
            return iterator(this, recursive_max_element(root_, dimension));
        }

        const_iterator max_element(size_t dimension) const {
            if (empty()) {
                return end();
            }
            return const_iterator(this, recursive_max_element(root_, dimension));
        }

        iterator min_element(size_t dimension) {
            if (empty()) {
                return end();
            }
            return iterator(this, recursive_min_element(root_, dimension));
        }

        const_iterator min_element(size_t dimension) const {
            if (empty()) {
                return end();
            }
            return const_iterator(this, recursive_min_element(root_, dimension));
        }

    public /* Modifying functions */:
        /// Insert entry
        /// \param v Pair value <point, value>
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        std::pair<iterator, bool> insert(const value_type &v) {
            quadtree_node* destination_node = insert_branch(v, root_);
            return {iterator(this, destination_node), true};
        }

        /// \brief Insert list of elements
        template<class Inputiterator>
        void insert(Inputiterator first, Inputiterator last) {
            while (first != last) {
                insert(*first);
                ++first;
            }
        }

        /// Erase element
        size_t erase(iterator position) {
            // we will remove this node and all nodes under it
            quadtree_node* node_to_remove = position.current_node_;

            // if this is empty
            if (node_to_remove == nullptr) {
                // we can't remove anything
                return 0;
            }

            // List of elements we need to reinsert in the tree after removing the node
            std::vector<value_type> reinsert_list;

            // copy and erase all elements under node_to_remove to the reinsert_list
            move_to_reinsert(node_to_remove, reinsert_list, false);

            // remove the root
            quadtree_node* current_node = node_to_remove->parent_;
            if (node_to_remove != root_) {
                // find it in the parent node and erase it from the children
                auto it = current_node->children_.find(current_node->value_.first.quadrant(node_to_remove->value_.first));
                assert(it != current_node->children_.end());
                current_node->children_.erase(it);
            } else {
                // set root node to nullptr
                root_ = nullptr;
            }
            // deallocate node
            deallocate_quadtree_node(node_to_remove);
            assert(size_ > 0);
            --size_;

            // reinsert all elements from the vector under the node
            std::sort(reinsert_list.begin(), reinsert_list.end());
            bulk_insert(reinsert_list, current_node == nullptr ? root_ : current_node);

            // if current node was not root, recalculate bounds for parent nodes up to the root
            while (current_node != nullptr) {
                current_node->bounds_ = minimum_bounding_rectangle(current_node);
                current_node = current_node->parent_;
            }

            return 1;
        }

        size_t erase(const value_type &v) {
            // Look for the item
            iterator it = find(v.first);
            // erase if found
            return !it.is_end() ? erase(it) : 0;
        }

        /// Remove range of iterators from the front
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

        /// erase all entries from tree
        void clear() {
            // Delete all existing nodes
            remove_all_records();
            root_ = nullptr;
            size_ = 0;
        }

        /// Swap the content of two fronts
        void swap(self_type &m) {
            std::swap(root_, m.root_);
            std::swap(size_, m.size_);
            std::swap(dimensions_, m.dimensions_);
        }

    protected:

        /// Bulk insertion inserts the median before other elements
        template<class InputIterator>
        void bulk_insert(InputIterator l_begin, InputIterator l_end, const value_type& v, InputIterator r_begin, InputIterator r_end) {
            bulk_insert(l_begin, l_end, v, r_begin, r_end, root_);
        }

        void bulk_insert(const std::vector<value_type> &v, quadtree_node*& node) {
            // bulk insert ranges {1, median - 1}, median, { median + 1, end()}
            if (!v.empty()) {
                if (v.size() == 1) {
                    insert_branch(v[0], node);
                } else {
                    size_t median_pos = v.size() / 2;
                    bulk_insert(v.begin(), v.begin() + median_pos, v[median_pos], v.begin() + median_pos + 1, v.end(), node);
                }
            }
        }

        template<class InputIterator>
        void bulk_insert(InputIterator l_begin, InputIterator l_end, const value_type& v, InputIterator r_begin, InputIterator r_end, quadtree_node*& node) {
            insert_branch(v, node);
            size_t l_size = std::distance(l_begin, l_end);
            if (l_size != 0) {
                if (l_size == 1) {
                    insert_branch(*l_begin, node);
                } else {
                    size_t l_median_pos = l_size / 2;
                    bulk_insert(l_begin, l_begin + l_median_pos, *(l_begin + l_median_pos), l_begin + l_median_pos + 1, l_end, node);
                }
            }
            size_t r_size = std::distance(r_begin, r_end);
            if (r_size != 0) {
                if (r_size == 1) {
                    insert_branch(*r_begin, node);
                } else {
                    size_t r_median_pos = r_size / 2;
                    bulk_insert(r_begin, r_begin + r_median_pos, *(r_begin + r_median_pos), r_begin + r_median_pos + 1, r_end, node);
                }
            }
        }

        /// \brief Insert a value into a tree node
        /// insert_branch provides for splitting the root;
        /// insert_branch_recursive does the recursion.
        /// \param value Value to be inserted
        /// \param a_root Node where we should insert the value (usually root node)
        /// \param a_level Number of steps up from the leaf level to insert e.g. a data rectangle goes in at level = 0.
        /// \return return 1 if root was split, 0 if it was not.
        /// \return Pointer to the node containing the value we inserted
        /// \return Index of the element we inserted in the node that contains it
        quadtree_node* insert_branch(const value_type &v, quadtree_node *&root_node) {
            if constexpr (number_of_compile_dimensions_ == 0) {
                if (dimensions_ == 0) {
                    dimensions_ = v.first.dimensions();
                }
            }
            /// If root node is empty, put the value there
            if (root_node == nullptr) {
                root_node = allocate_quadtree_node(nullptr, v);
                ++size_;
                return root_node;
            }

            /// Find the region that would contain the point P.
            quadtree_node* current = root_node;
            size_t quadrant = current->value_.first.quadrant(v.first);
            auto quadrant_it = current->children_.find(quadrant);
            while (quadrant_it != current->children_.end()) {
                current = quadrant_it->second;
                quadrant = current->value_.first.quadrant(v.first);
                quadrant_it = current->children_.find(quadrant);
            }

            /// Add point where you fall off the tree.
            /// The element would be in current->children_[quadrant]
            quadtree_node* new_node = allocate_quadtree_node(current, v);
            current->children_.emplace(quadrant, new_node);

            /// Adjust the minimum bounds up to the root
            current->bounds_.stretch(v.first);
            while (current->parent_ != nullptr) {
                current = current->parent_;
                current->bounds_.stretch(v.first);
            }

            ++size_;
            return new_node;
        }

        template <class... Args>
        quadtree_node *allocate_quadtree_node(Args&&... args) {
            auto p = alloc_->allocate(1);
            new(p) quadtree_node(args...);
            return p;
        }

        void deallocate_quadtree_node(quadtree_node *p) {
            p->~quadtree_node();
            alloc_->deallocate(p, 1);
        }

        /// Find the smallest rectangle that includes all rectangles in branches of a node.
        box_type minimum_bounding_rectangle(quadtree_node *a_node) {
            assert(a_node);
            box_type rect = box_type(a_node->value_.first);
            for (auto& [quadrant, child]: a_node->children_) {
                rect.stretch(child->bounds_);
            }
            return rect;
        }

        void copy_recursive(quadtree_node *current, quadtree_node *current_parent, const quadtree_node *other) {
            current->value_ = other->value_;
            current->bounds_ = other->bounds_;
            current->parent_ = current_parent;
            for (const auto& [quadrant, child]: other->children_) {
                auto l_child = allocate_quadtree_node();
                current->children_.emplace(quadrant,l_child);
                copy_recursive(l_child, current, child);
            }
        }

        void move_to_reinsert(quadtree_node*& node_to_remove, std::vector<value_type>& reinsert_list, bool move_root) {
            for (auto &[quadrant, child_node]: node_to_remove->children_) {
                if (child_node->children_.empty()) {
                    reinsert_list.emplace_back(child_node->value_);
                    deallocate_quadtree_node(child_node);
                    --size_;
                } else {
                    move_to_reinsert(child_node, reinsert_list, true);
                }
            }
            if (move_root) {
                --size_;
                reinsert_list.emplace_back(node_to_remove->value_);
                deallocate_quadtree_node(node_to_remove);
            }
        }

        quadtree_node * recursive_max_element(quadtree_node *parent_node, size_t dimension) const {
            if (parent_node->children_.empty()) {
                return parent_node;
            }
            if (parent_node->value_.first[dimension] >= parent_node->bounds_.second()[dimension]) {
                return parent_node;
            }
            // we could iterate only though the elements that have quadrant[dimension] == 1
            // but both algorithms would be O(n) on average and calculating these indexes
            // would involve loops with assignments and comparisons that would
            // cost more than checking the invalid elements
            auto max_it = std::max_element(parent_node->children_.begin(),
                                           parent_node->children_.end(),
                                           [&dimension](const typename quadtree_node::children_map::value_type &a,
                                                        const typename quadtree_node::children_map::value_type &b) {
                                               return a.second->bounds_.second()[dimension] < b.second->bounds_.second()[dimension];
                                           });
            assert(max_it != parent_node->children_.end());
            if (parent_node->value_.first[dimension] >= max_it->second->bounds_.second()[dimension]) {
                return parent_node;
            } else {
                return recursive_max_element(max_it->second, dimension);
            }
        }

        quadtree_node * recursive_min_element(quadtree_node *parent_node, size_t dimension) const {
            if (parent_node->children_.empty()) {
                return parent_node;
            }
            if (parent_node->value_.first[dimension] <= parent_node->bounds_.first()[dimension]) {
                return parent_node;
            }
            auto min_it = std::min_element(parent_node->children_.begin(),
                                           parent_node->children_.end(),
                                           [&dimension](const typename quadtree_node::children_map::value_type &a,
                                                        const typename quadtree_node::children_map::value_type &b) {
                                               return a.second->bounds_.first()[dimension] < b.second->bounds_.first()[dimension];
                                           });
            assert(min_it != parent_node->children_.end());
            if (parent_node->value_.first[dimension] <= min_it->second->bounds_.first()[dimension]) {
                return parent_node;
            } else {
                return recursive_min_element(min_it->second, dimension);
            }
        }

        void remove_all_records() {
            if (root_ == nullptr) {
                return;
            }
            remove_all_records(root_);
        }

        void remove_all_records(quadtree_node *node) {
            assert(node);
            if (node->is_internal_node()) {
                for (auto& child: node->children_) {
                    assert(child.second != node->parent_);
                    assert(child.second->parent_ == node);
                    remove_all_records(child.second);
                }
            }
            deallocate_quadtree_node(node);
        }


        std::string to_string() const {
            std::string str;
            // auto current = root_;
            str += to_string(root_, 0);
            return str;
        }

        std::string to_string(const quadtree_node* current, size_t level) const {
            std::string str(level*2,' ');
            std::stringstream ss;
            ss << current->value_.first;
            // ss << current->value_.first << ": " << current->value_.second;
            assert(current->bounds_.intersects(current->value_.first));
            str += ss.str() + "\n";
            for (const auto& child: current->children_) {
                str += to_string(child.second, level + 1);
            }
            return str;
        }

        /// Root of tree
        quadtree_node *root_;

        /// Number of elements in the tree
        /// We cache the size at each insertion/removal.
        /// Otherwise it would cost us O(n log(n)) to find out the size
        size_t size_{0};

        /// Number of dimensions of this rtree
        /// This is only needed when the dimension will be defined at runtime
        size_t dimensions_{0};

        /// Node allocator
        /// It's fundamental to allocate our nodes with an efficient allocator
        /// to have a tree that can compete with vectors
        /// This is a shared ptr because trees can also share the same
        /// allocator. The last one to use it should delete the allocator.
        /// This is an important component of archives, where it's
        /// unreasonable to create an allocator for every new front.
        std::shared_ptr<node_allocator_type> alloc_;

    };

    // MSVC hack (we cannot define it inside iterator_impl)
    template<class NUMBER_TYPE, size_t NUMBER_OF_DIMENSIONS, class ELEMENT_TYPE, template<typename> class ALLOCATOR>
    template <bool constness>
    const std::function<bool(const typename quad_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>::template iterator_impl<constness>::queue_element&,
                             const typename quad_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>::template iterator_impl<constness>::queue_element&)> quad_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>::iterator_impl<constness>::queue_comp = [](
            const typename quad_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>::template iterator_impl<constness>::queue_element& a,
            const typename quad_tree<NUMBER_TYPE, NUMBER_OF_DIMENSIONS, ELEMENT_TYPE, ALLOCATOR>::template iterator_impl<constness>::queue_element& b) -> bool {
        return std::get<2>(a) > std::get<2>(b);
    };

}

#endif //PARETO_FRONT_QUAD_TREE_H