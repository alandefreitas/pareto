//
// Created by Alan Freitas on 2020-06-03.
//

#ifndef PARETO_FRONT_QUAD_TREE_H
#define PARETO_FRONT_QUAD_TREE_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <forward_list>
#include <functional>
#include <map>
#include <queue>
#include <sstream>
#include <vector>

#include <pareto/point.h>
#include <pareto/query/predicates.h>
#include <pareto/query/query_box.h>

namespace pareto {
    /// \class quad_tree
    /// Implementation of a Point Quad-Tree with an interface that matches the
    /// pareto front. Quad-trees are the classic benchmark for Pareto Fronts. If
    /// points have n dimensions, each node potencially has 2^n children. We
    /// determine the child index by comparing the points in each dimension.
    ///
    /// In our quad-containers variant, we keep the minimum bounding rectangle
    /// of all containers branches, so that we can more easily query the data.
    /// This is the only adaptation we include. Unlike an R-Tree, this is not a
    /// fundamental part of the data structure, it's only a way to have
    /// pre-processed data for queries. The query for which this is most useful
    /// is the max/min query, which would need to iterate many nodes without
    /// this information.
    ///
    /// This implementation used
    /// https://github.com/danshapero/quadtree
    /// as reference for correctness, but the design is completely different.
    /// \see https://en.wikipedia.org/wiki/Quadtree#Point_quadtree
    template <class K, size_t M, class T, typename C = std::less<K>,
              class A = default_allocator_type<std::pair<const point<K, M>, T>>>
    class quad_tree : container_with_pool {
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

      private /* Quadtree options */:
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
            /// \brief Construct empty root node
            quadtree_node() : quadtree_node(nullptr) {}

            /// \brief Construct child node
            explicit quadtree_node(quadtree_node *parent) : parent_(parent) {}

            /// \brief Construct child node with value
            quadtree_node(quadtree_node *parent, const value_type &value)
                : value_(value), parent_(parent),
                  bounds_(box_type(value.first)) {}

            /// \brief An internal node, contains other nodes
            [[nodiscard]] bool is_internal_node() const {
                return !children_.empty();
            }

            /// \brief A leaf, contains data only
            [[nodiscard]] bool is_leaf_node() const {
                return children_.empty();
            }

            /// \brief Value: a quad-containers node holds a value itself.
            unprotected_value_type value_;

            /// \brief Node children
            /// Unlike other data structures, we use a map instead of
            /// an array because a quad_tree node can potentially have
            /// 2^n children. We cannot waste space with an exponential
            /// number of children that don't exist on every node.
            using children_map = std::map<size_t, quadtree_node *>;
            children_map children_;

            /// \brief Parent node
            /// The quad-containers nodes only need values and branches. All
            /// other member elements are conveniences for faster queries.
            /// Keeping track of parent nodes is important for more efficient
            /// iterators. A link to the parent node allows us to eliminate
            /// stacks and easily start the iterator from any node. This makes
            /// the use of iterators with predicates much easier because we just
            /// have to move forward until a predicate is valid.
            quadtree_node *parent_;

            /// \brief Minimal Bounding Rectangle
            /// The bounds are useful to make queries faster
            /// We can eliminate branches by only checking their
            /// bounds.
            /// The bounds of a point quad-containers are implicitly defined
            /// by the points in the node and its children.
            box_type bounds_;
        };

        using node_allocator_type = typename std::allocator_traits<
            allocator_type>::template rebind_alloc<quadtree_node>;

      public /* iterators */:
        /// Iterator is not erase_impl safe. Erase elements will invalidate the
        /// iterators. Because iterator and const_iterator are almost the same,
        /// we define iterator as iterator<false> and const_iterator as
        /// iterator<true> \see
        /// https://stackoverflow.com/questions/2150192/how-to-avoid-code-duplication-implementing-const-and-non-const-iterators
        template <bool is_const = false> class iterator_impl {
          private /* Internal Types */:
            template <class TYPE, class CONST_TYPE>
            using const_toggle =
                std::conditional_t<!is_const, TYPE, CONST_TYPE>;

            template <class U>
            using maybe_add_const = const_toggle<U, std::add_const_t<U>>;

            using node_pointer =
                const_toggle<quadtree_node *, const quadtree_node *>;

            using tree_pointer = const_toggle<quad_tree *, const quad_tree *>;

            using predicate_variant_type =
                predicate_variant<dimension_type, number_of_compile_dimensions,
                                  mapped_type>;

            using predicate_list_type =
                predicate_list<dimension_type, number_of_compile_dimensions,
                               mapped_type>;

          public /* LegacyIterator Types */:
            using value_type = maybe_add_const<quad_tree::value_type>;
            using reference =
                const_toggle<quad_tree::reference, quad_tree::const_reference>;
            using difference_type = quad_tree::difference_type;
            using pointer =
                const_toggle<quad_tree::pointer, quad_tree::const_pointer>;
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
                : current_tree_(rhs.current_tree_),
                  current_node_(rhs.current_node_),
                  predicates_(rhs.predicates_),
                  nearest_points_iterated_(rhs.nearest_points_iterated_) {
                nearest_queue_.reserve(rhs.nearest_queue_.size());
                for (const auto &[a, b, c] : rhs.nearest_queue_) {
                    nearest_queue_.emplace_back(std::make_tuple(a, b, c));
                }
            }

            /// \brief Copy assignment
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            iterator_impl &operator=(const iterator_impl<rhs_is_const> &rhs) {
                current_tree_ = rhs.current_tree_;
                current_node_ = rhs.current_node_;
                predicates_ = rhs.predicates_;
                nearest_points_iterated_ = rhs.nearest_points_iterated_;
                nearest_queue_.reserve(rhs.nearest_queue_.size());
                for (const auto &[a, b, c] : rhs.nearest_queue_) {
                    nearest_queue_.emplace_back(std::make_tuple(a, b, c));
                }
                // advance_if_invalid();
                return *this;
            }

            /// \brief Destructor
            ~iterator_impl() = default;

          public /* LegacyForwardIterator Constructors */:
            /// \brief Default constructor
            /// Just a placeholder for future assignment
            /// This iterator does not belong to any "== space"
            iterator_impl() : iterator_impl(nullptr, nullptr) {}

          public /* ContainerConcept Constructors */:
            /// \brief Convert to const iterator
            // NOLINTNEXTLINE(google-explicit-constructor)
            operator iterator_impl<true>() {
                iterator_impl<true> r;
                r = *this;
                return r;
            }

          public /* SpatialContainer Concept Constructors */:
            /// \brief Move constructor
            /// Move constructors are important for spatial containers
            /// because iterators *might* contain a large list of predicates
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            // NOLINTNEXTLINE(google-explicit-constructor)
            iterator_impl(iterator_impl<rhs_is_const> &&rhs)
                : current_tree_(rhs.current_tree_),
                  current_node_(rhs.current_node_),
                  predicates_(std::move(rhs.predicates_)),
                  nearest_points_iterated_(rhs.nearest_points_iterated_) {
                nearest_queue_.reserve(rhs.nearest_queue_.size());
                for (const auto &[a, b, c] : rhs.nearest_queue_) {
                    nearest_queue_.emplace_back(std::make_tuple(a, b, c));
                }
                // advance_if_invalid();
            }

            /// \brief Move assignment
            template <bool rhs_is_const,
                      class = std::enable_if_t<is_const || !rhs_is_const>>
            iterator_impl &operator=(iterator_impl<rhs_is_const> &&rhs) {
                current_tree_ = rhs.current_tree_;
                current_node_ = rhs.current_node_;
                predicates_ = std::move(rhs.predicates_);
                nearest_points_iterated_ = rhs.nearest_points_iterated_;
                nearest_queue_ = rhs.nearest_queue_;
                return *this;
            }

          public /* Internal Constructors / Used by Container */:
            /// \brief Tree and root node
            /// When the current pointer == nullptr, we are at end().
            /// We need the tree_ so that a.end() != b.end().
            iterator_impl(tree_pointer tree_, node_pointer root_)
                : current_tree_(tree_), current_node_(root_), nearest_queue_{},
                  nearest_points_iterated_(0) {
                advance_if_invalid();
            }

            /// \brief Tree begin
            explicit iterator_impl(tree_pointer tree_)
                : iterator_impl(tree_, tree_->root_) {}

            /// \brief Iterator with predicate initializer list
            iterator_impl(
                tree_pointer tree_, node_pointer root_,
                std::initializer_list<predicate_variant_type> predicate_list)
                : iterator_impl(tree_, root_, predicate_list.begin(),
                                predicate_list.end()) {}

            /// \brief Iterator with predicate vector
            iterator_impl(tree_pointer tree_, node_pointer root_,
                          const predicate_list_type &predicate_list)
                : iterator_impl(tree_, root_, predicate_list.begin(),
                                predicate_list.end()) {}

            /// \brief Iterator with iterators to predicates
            template <class predicate_iterator_type>
            iterator_impl(tree_pointer tree_, node_pointer root_,
                          predicate_iterator_type predicate_begin,
                          predicate_iterator_type predicate_end)
                : current_tree_(tree_), current_node_(root_),
                  predicates_(predicate_begin, predicate_end), nearest_queue_{},
                  nearest_points_iterated_(0) {
                sort_predicates();
                initialize_nearest_algorithm();
                advance_if_invalid();
            }

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
                assert(is_not_null());
                constexpr bool is_protected = std::is_const_v<
                    typename decltype(current_node_->value_)::first_type>;
                if constexpr (is_protected) {
                    return current_node_->value_;
                } else {
                    return protect_pair_key(current_node_->value_);
                }
            }

            iterator_impl &operator++() {
                advance_to_next_valid();
                return *this;
            }

          public /* LegacyInputIterator */:
            pointer operator->() const { return &operator*(); }

          public /* LegacyForwardIterator */:
            /// \brief Equality operator
            /// The equality operator ignores the predicates
            /// It only matters if they point to the same element here
            template <bool rhs_is_const>
            bool operator==(const iterator_impl<rhs_is_const> &rhs) const {
                if (current_tree_ == nullptr && rhs.current_tree_ == nullptr) {
                    return true;
                } else if (current_tree_ == nullptr ||
                           rhs.current_tree_ == nullptr) {
                    return false;
                } else {
                    return (current_node_ == rhs.current_node_) &&
                           (current_tree_ == rhs.current_tree_);
                }
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
                advance_to_next_valid();
                return tmp;
            }

          public /* LegacyBidirectionalIterator */:
            /// \brief Decrement iterator
            iterator_impl &operator--() {
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

          private /* Internal functions */:
            /// \brief Advance to next element if current element is invalid
            void advance_if_invalid() {
                if (is_end()) {
                    return;
                }
                const bool need_to_iterate_to_nearest =
                    predicates_.get_nearest() != nullptr &&
                    nearest_points_iterated_ == 0;
                if (need_to_iterate_to_nearest ||
                    !predicates_.pass_predicate(current_node_->value_)) {
                    // advance if current is not valid
                    advance_to_next_valid(false);
                }
            }

            /// \brief Is iterator_impl at the end
            [[nodiscard]] bool is_end() const {
                // We represent the "end" by keeping a pointer to nullptr
                return current_node_ == nullptr || current_tree_ == nullptr;
            }

            /// \brief Is iterator_impl at the begin
            [[nodiscard]] bool is_begin() const {
                // if there's no object, this can be both begin and end
                if (current_tree_ == nullptr) {
                    return true;
                }
                // in the general case, begin is the root node (depth-first)
                return current_node_ == current_tree_->root_;
            }

            /// \brief Is iterator_impl pointing to valid data
            [[nodiscard]] bool is_not_null() const { return !is_end(); }

            /// \brief Initialize queue for the nearest element algorithm
            void initialize_nearest_algorithm() {
                auto nearest_predicate = predicates_.get_nearest();
                if (nearest_predicate == nullptr) {
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
                    iterator_impl it =
                        iterator_impl(current_tree_, root, predicates_);
                    iterator_impl end = iterator_impl(current_tree_, nullptr);
                    // Iterate until the end so we have the nearest points
                    // pre-processed
                    while (it != end) {
                        ++it;
                    }
                    // Copy the pre-processed results for nearest points
                    nearest_set_ = it.nearest_set_;
                    nearest_points_iterated_ = it.nearest_points_iterated_;
                }

                // Initialize nearest queue
                // https://dl.acm.org/doi/pdf/10.1145/320248.320255 (page 278)
                // The algorithm basically enqueues nodes by potential distance
                // in a priority queue If first in the queue is an internal
                // node, enqueue all its children. If first in the queue is a
                // leaf node, report the element as i-th nearest.

                // 1. Queue <- NewPriorityQueue()
                // This is already done when we create the iterator_impl with
                // the nearest_queue_ member variable.

                // 2. Enqueue(Queue, R-containers.RootNode, 0)
                // This line in the algorithm is just to ensure there's
                // something to dequeue at the first iteration. For this reason,
                // the distance doesn't matter. It's just easier to Enqueue all
                // branches of quad-containers.RootNode already, as would always
                // happen at the first iteration anyway. Because this is a
                // quad-containers, we also enqueue the node value.
                nearest_queue_.emplace_back(
                    current_node_, true,
                    nearest_predicate->distance(current_node_->value_.first));
                std::push_heap(nearest_queue_.begin(), nearest_queue_.end(),
                               queue_comp);
                for (auto &child : current_node_->children_) {
                    nearest_queue_.emplace_back(
                        child.second, false,
                        nearest_predicate->distance(current_node_->bounds_));
                    std::push_heap(nearest_queue_.begin(), nearest_queue_.end(),
                                   queue_comp);
                }

                // 3. while not IsEmpty(Queue) do
                // From this point the algorithm is implemented inside the
                // "advance_to_next_valid" function so that we can iteractively
                // return the nearest points.
            }

            /// \brief Advance to the next valid element through the nearest
            /// element algorithm
            void advance_to_next_valid_through_nearest() {
                // If we already iterated all nearest points the predicate asked
                // for
                auto nearest_predicate = predicates_.get_nearest();
                if (nearest_points_iterated_ >= nearest_predicate->k()) {
                    advance_to_end();
                    return;
                }

                // If we already iterated this nearest point in the past
                // we should have the pre-processed results
                while (nearest_points_iterated_ < nearest_set_.size()) {
                    bool is_value;
                    bool passed_predicate;
                    std::tie(current_node_, is_value, passed_predicate) =
                        nearest_set_[nearest_points_iterated_];
                    assert(is_value);
                    ++nearest_points_iterated_;
                    if (passed_predicate) {
                        return;
                    }
                }

                // We go to the real algorithm after dealing with the trivial
                // cases The first steps (1 and 2) were executed in
                // 'initialize_nearest_algorithm' The advance step starts the
                // loop that looks for more nearest elements
                // 3. while not IsEmpty(Queue) do
                while (!nearest_queue_.empty()) {
                    // 4. Element <- Dequeue(Queue)
                    auto [element_node, is_value, distance] =
                        nearest_queue_.front();
                    auto &element = element_node->value_;
                    std::pop_heap(nearest_queue_.begin(), nearest_queue_.end(),
                                  queue_comp);
                    nearest_queue_.pop_back();
                    // 5. if element is an object or its bounding rectangle
                    // The way we represent the containers, it's never a
                    // bounding rectangle because we only represent points in
                    // our containers. So we only test if it's an object (a
                    // value).
                    if (is_value) {
                        // 6. if Element is the bounding rectangle
                        //     7. That never happens in our containers
                        // 8. else
                        // 9.     Report Element
                        // in our version, we only report it if it also passes
                        // the other predicates
                        if (predicates_.pass_predicate(element)) {
                            ++nearest_points_iterated_;
                            current_node_ = element_node;
                            // put it in the pre-processed set of results
                            nearest_set_.emplace_back(current_node_, true,
                                                      true);
                            return;
                        } else {
                            // if it doesn't pass the predicates, we just count
                            // it as one more nearest point we would have
                            // reported
                            ++nearest_points_iterated_;
                            nearest_set_.emplace_back(current_node_, true,
                                                      false);
                            if (nearest_points_iterated_ >=
                                nearest_predicate->k()) {
                                advance_to_end();
                                return;
                            }
                        }
                    } else {
                        // 11. else if Element is a leaf node then
                        // 15. else /* Element is a non-leaf node*/
                        // These two conditions have blocks enqueuing all child
                        // elements. So they are equivalent to us because we use
                        // variants for the branches.
                        // 12. for each entry (Object, Rect) in leaf node
                        // Element do
                        // 16. for each entry (Node,   Rect) in      node
                        // Element do
                        nearest_queue_.emplace_back(
                            element_node, true,
                            nearest_predicate->distance(
                                element_node->value_.first));
                        std::push_heap(nearest_queue_.begin(),
                                       nearest_queue_.end(), queue_comp);
                        for (auto &child : element_node->children_) {
                            // 13. Enqueue(Queue, [Object], Dist(QueryObject,
                            // Rect))
                            // 17. Enqueue(Queue, Node,     Dist(QueryObject,
                            // Rect))
                            nearest_queue_.emplace_back(
                                child.second, false,
                                nearest_predicate->distance(
                                    child.second->bounds_));
                            std::push_heap(nearest_queue_.begin(),
                                           nearest_queue_.end(), queue_comp);
                        }
                    }
                }
                // Nothing else to report
                advance_to_end();
            }

            /// \brief Return to previous valid element with the nearest element
            /// algorithm When using this algorithm backwards, we calculate all
            /// nearest elements before iterating because it's the only way to
            /// get it backwards
            void return_to_previous_valid_through_nearest() {
                // If previous point is already iterated we get the results from
                // there
                while (nearest_points_iterated_ == 0 ||
                       nearest_points_iterated_ - 1 < nearest_set_.size()) {
                    // If everything is iterated
                    if (nearest_points_iterated_ == 0) {
                        return_to_begin();
                        return;
                    } else {
                        // if nearest_points_iterated_ - 1 is already
                        // pre-processed
                        bool is_value;
                        bool passed_predicate;
                        std::tie(current_node_, is_value, passed_predicate) =
                            nearest_set_[nearest_points_iterated_ - 1];
                        assert(is_value);
                        --nearest_points_iterated_;
                        if (passed_predicate) {
                            return;
                        }
                    }
                }

                // If we got here, we have a problem
                throw std::logic_error("We should have the pre-processed "
                                       "results for the nearest points");
            }

            /// \brief Advance to the end iterator
            void advance_to_end() { current_node_ = nullptr; }

            /// \brief Return to the begin iterator
            void return_to_begin() {
                if (current_tree_ != nullptr) {
                    current_node_ = current_tree_->root_;
                }
            }

            /// \brief Go to next element with depth first search (when there's
            /// no nearest predicate)
            void advance_to_next_valid_depth_first(
                bool first_time_in_this_branch = false) {
                // this should usually be the case at the start
                while (!is_end()) {
                    // return if first time
                    // if we haven't checked the current node yet
                    if (first_time_in_this_branch) {
                        if (predicates_.pass_predicate(current_node_->value_)) {
                            // found a valid value in current node
                            // point to it (already does) and return
                            return;
                        }
                        first_time_in_this_branch = false;
                    } else {
                        // if we already checked this node,
                        // go to first valid child if there are children, stop
                        // iterating
                        bool predicate_might_pass = false;
                        // if it has children, try to go to its first child
                        // that might pass the predicate
                        for (auto &child : current_node_->children_) {
                            // If node children might have nodes that pass all
                            // predicates
                            if (predicates_.might_pass_predicate(
                                    child.second->bounds_)) {
                                // Found a child that might pass predicates
                                // Point to it and continue looking until we
                                // find a value_type that actually passes the
                                // predicate
                                current_node_ = child.second;
                                predicate_might_pass = true;
                                first_time_in_this_branch = true;
                                break;
                            }
                        }

                        // go to right sibling, stop iterating
                        // No more branches that could pass the predicates to
                        // test Go to previous level / parent node In the parent
                        // node we look for the next sibling that might pass the
                        // predicates. If no siblings can pass the predicates,
                        // we go to the next parent again and look for more
                        // siblings. We do that until we reach the end.
                        if (!predicate_might_pass) {
                            // the function already goes to end() if no right
                            // sibling, stop iterating
                            go_to_sibling(true);
                            first_time_in_this_branch = true;
                        }
                    }
                }
            }

            /// \brief Go to previous valid element with backwards depth first
            /// search
            void return_to_previous_valid_depth_first(
                bool first_time_in_this_branch = false) {
                // this should usually be the case at the start
                while (!is_begin()) {
                    // return if first time
                    if (first_time_in_this_branch) {
                        if (predicates_.pass_predicate(current_node_->value_)) {
                            // found a valid value in current node
                            // point to it (already does) and return
                            return;
                        }
                        first_time_in_this_branch = false;
                    } else {
                        // go to left sibling and then to rightmost valid
                        // element, stop iterating if this is end(), left
                        // sibling will be the root node
                        auto previous_node = current_node_;
                        // this function already goes to parent if there are no
                        // left siblings, stop iterating
                        go_to_sibling(false);
                        // go_to_sibling goes to parent when there is no sibling
                        // go_to_sibling goes to root when it's end()
                        bool there_was_sibling =
                            previous_node == nullptr ||
                            previous_node->parent_ != current_node_;
                        if (there_was_sibling) {
                            // go to rightmost valid element in the sibling
                            // we will visit this node later when we go to the
                            // parent
                            bool children_might_pass_predicate = true;
                            while (children_might_pass_predicate) {
                                children_might_pass_predicate = false;
                                for (auto child_it =
                                         current_node_->children_.rbegin();
                                     child_it !=
                                     current_node_->children_.rend();
                                     ++child_it) {
                                    auto &child = *child_it;
                                    if (predicates_.might_pass_predicate(
                                            child.second->bounds_)) {
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

            /// \brief Find the next data element in the containers (For
            /// internal use only) If we have a nearest predicate, the way we
            /// iterate the containers is completely different. We need to use
            /// the nearest queue to decide which branches will be iterated
            /// first. We iterate even the branches that may fail other
            /// predicates because this is the only way for us to count how many
            /// nearest points we have visited.
            ///
            /// If there is no nearest predicate, we use a depth-first search in
            /// which we ignore branches that have no potential to pass the
            /// predicates. For small query boxes (as small as a branch), this
            /// leads to log(n) queries.
            void advance_to_next_valid(bool first_time_in_this_branch = false) {
                if (predicates_.get_nearest() != nullptr) {
                    advance_to_next_valid_through_nearest();
                } else {
                    advance_to_next_valid_depth_first(
                        first_time_in_this_branch);
                }
            }

            /// \brief Go to previous valid element (nearest search or depth
            /// first search)
            void
            return_to_previous_valid(bool first_time_in_this_branch = false) {
                if (predicates_.get_nearest() != nullptr) {
                    return_to_previous_valid_through_nearest();
                } else {
                    return_to_previous_valid_depth_first(
                        first_time_in_this_branch);
                }
            }

            /// \brief Go to the next sibling node
            void go_to_sibling(bool move_right = true) {
                // if we are at the "end" element and we are moving right
                if (!move_right && current_node_ == nullptr) {
                    // the root is considered the
                    current_node_ = current_tree_->root_;
                    return;
                }
                while (current_node_->parent_ != nullptr) {
                    // Move to parent and find index of current branch there
                    auto branch_address = current_node_;
                    current_node_ = current_node_->parent_;
                    size_t branch_index = current_node_->value_.first.quadrant(
                        branch_address->value_.first,
                        current_tree_->dimension_comp());
                    auto branch_it =
                        current_node_->children_.find(branch_index);
                    assert(branch_it != current_node_->children_.end());
                    // Look for next sibling that might pass the predicates
                    if (move_right) {
                        ++branch_it;
                        for (; branch_it != current_node_->children_.end();
                             ++branch_it) {
                            if (predicates_.might_pass_predicate(
                                    branch_it->second->bounds_)) {
                                current_node_ = branch_it->second;
                                return;
                            }
                        }
                    } else {
                        // if not at first child yet
                        while (branch_it != current_node_->children_.begin()) {
                            // go to previous child (which might also be the
                            // first child)
                            --branch_it;
                            // try to pass the predicate there
                            if (predicates_.might_pass_predicate(
                                    branch_it->second->bounds_)) {
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
                        *this = iterator_impl(
                            current_tree_, current_tree_->root_, predicates_);
                    }
                }
            }

            /// \brief Sort predicates in the predicate list
            void sort_predicates() {
                // for the more general case
                dimension_type volume_root = 0.;
                // if there is any disjoint predicate, we need the root volume
                if (predicates_.size() > 1 && predicates_.contains_disjoint()) {
                    // find root node
                    auto root = current_node_;
                    while (root->parent_ != nullptr) {
                        root = root->parent_;
                    }
                    // calculate volume of root minimum bounding rectangle
                    volume_root = root->bounds_.volume();
                }
                predicates_.sort(volume_root);
            }

            /// \brief Stack as we are doing iteration instead of recursion
            tree_pointer current_tree_;

            /// \brief Stack as we are doing iteration instead of recursion
            node_pointer current_node_;

            /// \brief Predicate constraining the search area
            predicate_list_type predicates_;

            // Pair with branch (node or object) and distance to the reference
            // point The branch is represented by the node and a boolean marking
            // if the pointer represents itself or its value
            // tuple<node, represents value, distance>
            using queue_element =
                std::tuple<node_pointer, bool,
                           typename point_type::distance_type>;

            // Function to compare queue_elements by their distance to the
            // reference point
            static const std::function<bool(const queue_element &,
                                            const queue_element &)>
                queue_comp;

            // Queue <- NewPriorityQueue()
            std::vector<queue_element> nearest_queue_;

            // Number of nearest points we have iterated so far
            size_t nearest_points_iterated_{0};

            // Set of nearest values we have already found
            // Some pointers represent the value of a node
            // Some pointers represent the node itself with its bounding box
            // tuple<node, represents value, passed predicate>
            std::vector<std::tuple<node_pointer, bool, bool>> nearest_set_;

          public:
            // Allow hiding of non-public functions while allowing manipulation
            // by logical owner
            friend class quad_tree;
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
        explicit quad_tree(const allocator_type &alloc =
                               placeholder_allocator<allocator_type>())
            : root_(nullptr), size_(0),
              dimensions_(number_of_compile_dimensions),
              alloc_(construct_allocator<node_allocator_type>(alloc)),
              comp_(dimension_compare()) {}

        /// \brief Copy constructor
        /// Copy constructors of AllocatorAwareContainers
        /// obtain their instances of the allocator by calling
        /// std::allocator_traits<allocator_type>::
        ///       select_on_container_copy_construction
        /// on the allocator of the container being copied.
        /// \param rhs
        quad_tree(const quad_tree &rhs)
            : size_(rhs.size_), dimensions_(rhs.dimensions_),
              alloc_(std::allocator_traits<node_allocator_type>::
                         select_on_container_copy_construction(rhs.alloc_)),
              comp_(rhs.comp_) {
            if (rhs.root_ != nullptr) {
                root_ = allocate_quadtree_node();
                copy_recursive(root_, rhs.root_);
            } else {
                root_ = nullptr;
            }
        }

        /// \brief Copy constructor data but use another allocator
        quad_tree(const quad_tree &rhs, const allocator_type &alloc)
            : size_(rhs.size_), dimensions_(rhs.dimensions_),
              alloc_(std::allocator_traits<node_allocator_type>::
                         select_on_container_copy_construction(alloc)),
              comp_(rhs.comp_) {
            if (rhs.root_ != nullptr) {
                root_ = allocate_quadtree_node();
                copy_recursive(root_, rhs.root_);
            } else {
                root_ = nullptr;
            }
        }

        /// \brief Move constructor
        /// Move constructors obtain their instances of allocators
        /// by move-constructing from the allocator belonging to
        /// the old container
        quad_tree(quad_tree &&rhs) noexcept
            : root_(std::move(rhs.root_)), size_(rhs.size_),
              dimensions_(rhs.dimensions_), alloc_(std::move(rhs.alloc_)),
              comp_(rhs.comp_) {
            rhs.root_ = nullptr;
        }

        /// \brief Move constructor data but use new allocator
        quad_tree(quad_tree &&rhs, const allocator_type &alloc) noexcept
            : root_(std::move(rhs.root_)), size_(rhs.size_),
              dimensions_(rhs.dimensions_),
              alloc_(std::allocator_traits<node_allocator_type>::
                         select_on_container_copy_construction(alloc)),
              comp_(rhs.comp_) {
            rhs.root_ = nullptr;
        }

        /// \brief Destructor
        virtual ~quad_tree() { remove_all_records(); }

      public /* Constructors: AssociativeContainer + AllocatorAwareContainer */:
        /// \brief Create container with custom comparison function
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        explicit quad_tree(const C &comp,
                           const allocator_type &alloc =
                               placeholder_allocator<allocator_type>())
            : quad_tree({}, comp, alloc) {}

        /// \brief Construct with iterators + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        quad_tree(InputIt first, InputIt last,
                  const C &comp = dimension_compare(),
                  const allocator_type &alloc =
                      placeholder_allocator<allocator_type>())
            : quad_tree(alloc) {
            comp_ = comp;
            std::vector<unprotected_value_type> v(first, last);
            std::sort(v.begin(), v.end(), [this](const auto &a, const auto &b) {
                return std::lexicographical_compare(
                    a.first.begin(), a.first.end(), b.first.begin(),
                    b.first.end(), comp_);
            });
            bulk_insert(v, root_);
        }

        /// \brief Construct with list + comparison
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        quad_tree(std::initializer_list<value_type> il,
                  const C &comp = dimension_compare(),
                  const allocator_type &alloc =
                      placeholder_allocator<allocator_type>())
            : quad_tree(il.begin(), il.end(), comp, alloc) {}

        /// \brief Construct with iterators
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        template <class InputIt>
        quad_tree(InputIt first, InputIt last, const allocator_type &alloc)
            : quad_tree(alloc) {
            std::vector<unprotected_value_type> v(first, last);
            std::sort(v.begin(), v.end(), [this](const auto &a, const auto &b) {
                return std::lexicographical_compare(
                    a.first.begin(), a.first.end(), b.first.begin(),
                    b.first.end(), comp_);
            });
            bulk_insert(v, root_);
        }

        /// \brief Construct with iterators
        /// Allocator aware containers reimplement all constructors with
        /// an extra allocator parameter
        quad_tree(std::initializer_list<value_type> il,
                  const allocator_type &alloc)
            : quad_tree(il.begin(), il.end(), alloc) {}

      public /* Assignment: Container + AllocatorAwareContainer */:
        /// \brief Copy assignment
        /// Copy-assignment will replace the allocator only if
        /// std::allocator_traits<allocator_type>::
        ///     propagate_on_container_copy_assignment::value is true
        /// \note if the allocators of the source and the target containers
        /// do not compare equal, copy assignment has to deallocate the
        /// target's memory using the old allocator and then allocate it
        /// using the new allocator before copying the elements
        quad_tree &operator=(const quad_tree &rhs) {
            if (&rhs == this) {
                return *this;
            }
            // deallocate the target's memory using the old allocator
            // in any case, because these records will not exist anymore
            remove_all_records();
            size_ = rhs.size_;
            dimensions_ = rhs.dimensions_;
            comp_ = rhs.comp_;
            const bool should_copy = std::allocator_traits<
                allocator_type>::propagate_on_container_copy_assignment::value;
            if constexpr (should_copy) {
                alloc_ = rhs.alloc_;
            }
            if (rhs.root_ != nullptr) {
                root_ = allocate_quadtree_node();
                copy_recursive(root_, rhs.root_);
            } else {
                root_ = nullptr;
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
        quad_tree &operator=(quad_tree &&rhs) noexcept {
            if (&rhs == this) {
                return *this;
            }
            // deallocate the target's memory using the old allocator
            // in any case, because these records will not exist anymore
            remove_all_records();
            size_ = rhs.size_;
            dimensions_ = rhs.dimensions_;
            comp_ = rhs.comp_;
            const bool should_move = std::allocator_traits<
                allocator_type>::propagate_on_container_move_assignment::value;
            if constexpr (should_move) {
                alloc_ = std::move(rhs.alloc_);
                root_ = rhs.root_;
            } else {
                const bool same_memory_resources = alloc_ == rhs.alloc_;
                if (same_memory_resources) {
                    root_ = rhs.root_;
                } else {
                    if (rhs.root_ != nullptr) {
                        root_ = allocate_quadtree_node();
                        copy_recursive(root_, rhs.root_);
                    } else {
                        root_ = nullptr;
                    }
                }
            }
            rhs.root_ = nullptr;
            return *this;
        }

      public /* Assignment: AssociativeContainer */:
        /// \brief Initializer list assignment
        quad_tree &operator=(std::initializer_list<value_type> il) noexcept {
            clear();
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
        /// \brief Get reference to element at a given position, and throw error
        /// if it does not exist
        mapped_type &at(const key_type &k) {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("quad_tree::at: key not found");
            }
        }

        /// \brief Get reference to element at a given position, and throw error
        /// if it does not exist
        const mapped_type &at(const key_type &k) const {
            auto it = find(k);
            if (it != end()) {
                return it->second;
            } else {
                throw std::out_of_range("quad_tree::at: key not found");
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
        const_iterator begin() const noexcept { return cbegin(); }

        /// \brief Get iterator to past-the-end element
        const_iterator end() const noexcept { return cend(); }

        /// \brief Get iterator to first element
        const_iterator cbegin() const noexcept {
            return const_iterator(this, root_);
        }

        /// \brief Get iterator to past-the-end element
        const_iterator cend() const noexcept {
            return const_iterator(this, nullptr);
        }

        /// \brief Get iterator to first element
        iterator begin() noexcept { return iterator(this, root_); }

        /// \brief Get iterator to past-the-end element
        iterator end() noexcept { return iterator(this, nullptr); }

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
        [[nodiscard]] bool empty() const noexcept { return root_ == nullptr; }

        /// \brief Get container size
        [[nodiscard]] size_type size() const noexcept { return size_; }

        /// \brief Get container max size
        /// The container is by no means guaranteed to be able
        /// to reach that size
        [[nodiscard]] size_type max_size() const noexcept {
            return std::numeric_limits<size_type>::max();
        }

      public /* Non-Modifying Functions / Capacity / Spatial Concept */:
        /// \brief Get container dimensions
        [[nodiscard]] size_t dimensions() const noexcept {
            if constexpr (number_of_compile_dimensions != 0) {
                return number_of_compile_dimensions;
            } else {
                return dimensions_;
            }
        }

        /// \brief Get maximum value in a given dimension
        dimension_type max_value(size_t dimension) const {
            return root_ ? root_->bounds_.max()[dimension]
                         : std::numeric_limits<dimension_type>::max();
        }

        /// \brief Get minimum value in a given dimension
        dimension_type min_value(size_t dimension) const {
            return root_ ? root_->bounds_.min()[dimension]
                         : std::numeric_limits<dimension_type>::min();
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
        void swap(quad_tree &rhs) noexcept {
            std::swap(root_, rhs.root_);
            std::swap(size_, rhs.size_);
            std::swap(dimensions_, rhs.dimensions_);
            std::swap(comp_, rhs.comp_);
            const bool should_swap = std::allocator_traits<
                allocator_type>::propagate_on_container_swap::value;
            if constexpr (should_swap) {
                std::swap(alloc_, rhs.alloc_);
            }
        }

      public /* Modifiers: Multimap Concept */:
        /// \brief erase all entries from containers
        void clear() {
            // Delete all existing nodes
            remove_all_records();
            root_ = nullptr;
            size_ = 0;
        }

        /// Insert entry
        /// \param v Pair value <point, value>
        /// \return Iterator to the new element
        /// \return True if insertion happened successfully
        iterator insert(const value_type &v) {
            quadtree_node *destination_node = insert_branch(v, root_);
            return iterator(this, destination_node);
        }

        iterator insert(value_type &&v) {
            quadtree_node *destination_node = insert_branch(v, root_);
            return iterator(this, destination_node);
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
        template <class Inputiterator>
        void insert(Inputiterator first, Inputiterator last) {
            while (first != last) {
                insert(*first);
                ++first;
            }
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

        /// \brief Erase element
        iterator erase(const_iterator position) {
            return erase(create_unconst_iterator(position));
        }

        /// Erase element
        iterator erase(iterator position) {
            iterator next_position = std::next(position);
            quadtree_node *node_to_remove = position.current_node_;
            if (node_to_remove == nullptr) {
                return end();
            }
            if (next_position != end()) {
                key_type next_key = next_position->first;
                erase_impl(node_to_remove);
                return find(next_key);
            } else {
                erase_impl(node_to_remove);
                return end();
            }
        }

        /// \brief Remove range of iterators from the front
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

        /// \brief Erase elements with given key
        size_type erase(const key_type &k) {
            iterator first = find_intersection(k);
            iterator last = end();
            auto s = static_cast<size_type>(std::distance(first, last));
            erase(first, last);
            return s;
        }

        /// \brief Splices nodes from another container
        void merge(quad_tree &source) noexcept {
            insert(source.begin(), source.end());
        }

      public /* Lookup / Multimap Concept */:
        /// \brief Returns the number of elements with key that compares
        /// equivalent to the specified argument.
        size_type count(const key_type &k) const {
            iterator it = (const_cast<quad_tree *>(this))->find_intersection(k);
            iterator end = (const_cast<quad_tree *>(this))->end();
            return static_cast<size_type>(std::distance(it, end));
        }

        /// \brief Returns the number of elements with key that compares
        /// equivalent to the specified argument.
        template <class L> size_type count(const L &k) const {
            iterator it = find_intersection(key_type{k}, key_type{k});
            return static_cast<size_type>(std::distance(it, end()));
        }

        /// \brief Finds an element with key equivalent to key
        iterator find(const key_type &k) {
            iterator it = find_intersection(k, k);
            it.predicates_.clear();
            return it;
        }

        /// \brief Finds an element with key equivalent to key
        const_iterator find(const key_type &k) const {
            const_iterator it = find_intersection(k, k);
            it.predicates_.clear();
            return it;
        }

        /// \brief Finds an element with key equivalent to key
        template <class L> iterator find(const L &x) {
            iterator it = find_intersection(key_type{x}, key_type{x});
            it.predicates_.clear();
            return it;
        }

        /// \brief Finds an element with key equivalent to key
        template <class L> const_iterator find(const L &x) const {
            const_iterator it = find_intersection(key_type{x}, key_type{x});
            it.predicates_.clear();
            return it;
        }

        /// \brief Finds an element with key equivalent to key
        bool contains(const key_type &k) const { return find(k) != end(); }

        /// \brief Finds an element with key equivalent to key
        template <class L> bool contains(const L &x) const {
            return find(x) != end();
        }

      public /* Query iterators */:
        /// \brief Get iterator to first element that passes the list of
        /// predicates
        const_iterator find(const predicate_list_type &ps) const noexcept {
            return root_ ? const_iterator(this, root_, ps) : end();
        }

        /// \brief Get iterator to first element that passes the list of
        /// predicates
        iterator find(const predicate_list_type &ps) noexcept {
            return root_ ? iterator(this, root_, ps) : end();
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
        iterator find_intersection(const point_type &lb, const point_type &ub) {
            point_type lb_ = lb;
            point_type ub_ = ub;
            normalize_corners(lb_, ub_);
            return iterator(this, root_, {intersects(lb_, ub_)});
        }

        /// \brief Find intersection between points and query box
        const_iterator find_intersection(const point_type &lb,
                                         const point_type &ub) const {
            point_type lb_ = lb;
            point_type ub_ = ub;
            normalize_corners(lb_, ub_);
            return const_iterator(this, root_, {intersects(lb_, ub_)});
        }

        /// \brief Find intersection between points and query box
        iterator find_intersection(const point_type &lb, const point_type &ub,
                                   std::function<bool(const value_type &)> fn) {
            point_type lb_ = lb;
            point_type ub_ = ub;
            normalize_corners(lb_, ub_);
            return iterator(
                this, root_,
                {intersects(lb_, ub_),
                 satisfies<dimension_type, number_of_compile_dimensions,
                           mapped_type>(fn)});
        }

        /// \brief Find intersection between points and query box
        const_iterator
        find_intersection(const point_type &lb, const point_type &ub,
                          std::function<bool(const value_type &)> fn) const {
            point_type lb_ = lb;
            point_type ub_ = ub;
            normalize_corners(lb_, ub_);
            return const_iterator(
                this, root_,
                {intersects(lb_, ub_),
                 satisfies<dimension_type, number_of_compile_dimensions,
                           mapped_type>(fn)});
        }

        /// \brief Get points inside a box (excluding borders)
        iterator find_within(const point_type &lb, const point_type &ub) {
            point_type lb_ = lb;
            point_type ub_ = ub;
            normalize_corners(lb_, ub_);
            return iterator(this, root_, {within(lb_, ub_)});
        }

        /// \brief Find points within a query box
        const_iterator find_within(const point_type &lb,
                                   const point_type &ub) const {
            point_type lb_ = lb;
            point_type ub_ = ub;
            normalize_corners(lb_, ub_);
            return const_iterator(this, root_, {within(lb_, ub_)});
        }

        /// \brief Get outside inside a box (excluding borders)
        iterator find_disjoint(const point_type &lb, const point_type &ub) {
            point_type lb_ = lb;
            point_type ub_ = ub;
            normalize_corners(lb_, ub_);
            return iterator(this, root_, {disjoint(lb_, ub_)});
        }

        /// \brief Find points outside a query box
        const_iterator find_disjoint(const point_type &lb,
                                     const point_type &ub) const {
            point_type lb_ = lb;
            point_type ub_ = ub;
            normalize_corners(lb_, ub_);
            return const_iterator(this, root_, {disjoint(lb_, ub_)});
        }

        /// Find the point closest to this point
        /// We adapted the algorithm a little so it would work for quad-trees
        /// instead of r-trees
        /// \see Hjaltason, Gísli R., and Hanan Samet. "Distance browsing in
        /// spatial databases." ACM Transactions on Database Systems (TODS) 24.2
        /// (1999): 265-318. \see
        /// https://dl.acm.org/doi/pdf/10.1145/320248.320255 \see
        /// https://stackoverflow.com/questions/45816632/nearest-neighbor-algorithm-in-r-containers
        iterator find_nearest(const point_type &p) {
            return iterator(this, root_, {nearest(p)});
        }

        /// \brief Find points closest to a reference point
        const_iterator find_nearest(const point_type &p) const {
            return const_iterator(this, root_, {nearest(p)});
        }

        /// \brief Get points closest to a reference point or box
        iterator find_nearest(const point_type &p, size_t k) {
            return iterator(this, root_, {nearest(p, k)});
        }

        /// \brief Find points closest to a reference point
        const_iterator find_nearest(const point_type &p, size_t k) const {
            return const_iterator(this, root_, {nearest(p, k)});
        }

        /// \brief Get points closest to a reference point or box
        iterator find_nearest(const box_type &b, size_t k) {
            return iterator(this, root_, {nearest(b, k)});
        }

        /// \brief Find points closest to a reference point
        const_iterator find_nearest(const box_type &b, size_t k) const {
            return const_iterator(this, root_, {nearest(b, k)});
        }

        /// \brief Get points closest to a reference point or box
        iterator find_nearest(const point_type &p, size_t k,
                              std::function<bool(const value_type &)> fn) {
            return iterator(
                this, root_,
                {nearest(p, k),
                 satisfies<dimension_type, number_of_compile_dimensions,
                           mapped_type>(fn)});
        }

        /// \brief Find points closest to a reference point
        const_iterator
        find_nearest(const point_type &p, size_t k,
                     std::function<bool(const value_type &)> fn) const {
            return const_iterator(
                this, root_,
                {nearest(p, k),
                 satisfies<dimension_type, number_of_compile_dimensions,
                           mapped_type>(fn)});
        }

        /// \brief Get iterator to element with maximum value in a given
        /// dimension
        iterator max_element(size_t dimension) {
            if (empty()) {
                return end();
            }
            return iterator(this, recursive_max_element(root_, dimension));
        }

        /// \brief Get iterator to element with maximum value in a given
        /// dimension
        const_iterator max_element(size_t dimension) const {
            if (empty()) {
                return end();
            }
            return const_iterator(this,
                                  recursive_max_element(root_, dimension));
        }

        /// \brief Get iterator to element with minimum value in a given
        /// dimension
        iterator min_element(size_t dimension) {
            if (empty()) {
                return end();
            }
            return iterator(this, recursive_min_element(root_, dimension));
        }

        /// \brief Get iterator to element with minimum value in a given
        /// dimension
        const_iterator min_element(size_t dimension) const {
            if (empty()) {
                return end();
            }
            return const_iterator(this,
                                  recursive_min_element(root_, dimension));
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

        /// \brief Returns the function object that compares numbers
        /// This is the comparison operator for a single dimension
        dimension_compare dimension_comp() const noexcept { return comp_; }

      protected:
        /// \brief Bulk insertion inserts the median before other elements
        template <class InputIterator>
        void bulk_insert(InputIterator l_begin, InputIterator l_end,
                         const value_type &v, InputIterator r_begin,
                         InputIterator r_end) {
            bulk_insert(l_begin, l_end, v, r_begin, r_end, root_);
        }

        /// \brief Bulk insertion inserts the median before other elements
        void bulk_insert(const std::vector<unprotected_value_type> &v,
                         quadtree_node *&node) {
            // bulk insert ranges {1, median - 1}, median, { median + 1, end()}
            if (!v.empty()) {
                if (v.size() == 1) {
                    insert_branch(v[0], node);
                } else {
                    size_t median_pos = v.size() / 2;
                    bulk_insert(v.begin(), v.begin() + median_pos,
                                v[median_pos], v.begin() + median_pos + 1,
                                v.end(), node);
                }
            }
        }

        /// \brief Bulk insertion inserts the median before other elements
        template <class InputIterator>
        void bulk_insert(InputIterator l_begin, InputIterator l_end,
                         const value_type &v, InputIterator r_begin,
                         InputIterator r_end, quadtree_node *&node) {
            insert_branch(v, node);
            size_t l_size = std::distance(l_begin, l_end);
            if (l_size != 0) {
                if (l_size == 1) {
                    insert_branch(*l_begin, node);
                } else {
                    size_t l_median_pos = l_size / 2;
                    bulk_insert(l_begin, l_begin + l_median_pos,
                                *(l_begin + l_median_pos),
                                l_begin + l_median_pos + 1, l_end, node);
                }
            }
            size_t r_size = std::distance(r_begin, r_end);
            if (r_size != 0) {
                if (r_size == 1) {
                    insert_branch(*r_begin, node);
                } else {
                    size_t r_median_pos = r_size / 2;
                    bulk_insert(r_begin, r_begin + r_median_pos,
                                *(r_begin + r_median_pos),
                                r_begin + r_median_pos + 1, r_end, node);
                }
            }
        }

        /// \brief Insert a value into a containers node
        /// insert_branch provides for splitting the root;
        /// insert_branch_recursive does the recursion.
        /// \param value Value to be inserted
        /// \param a_root Node where we should insert the value (usually root
        /// node) \param a_level Number of steps up from the leaf level to
        /// insert e.g. a data rectangle goes in at level = 0. \return return 1
        /// if root was split, 0 if it was not. \return Pointer to the node
        /// containing the value we inserted \return Index of the element we
        /// inserted in the node that contains it
        quadtree_node *insert_branch(const value_type &v,
                                     quadtree_node *&root_node) {
            if constexpr (number_of_compile_dimensions == 0) {
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

            /// \brief Find the region that would contain the point P.
            quadtree_node *current = root_node;
            size_t quadrant = current->value_.first.quadrant(v.first, comp_);
            auto quadrant_it = current->children_.find(quadrant);
            while (quadrant_it != current->children_.end()) {
                current = quadrant_it->second;
                quadrant = current->value_.first.quadrant(v.first, comp_);
                quadrant_it = current->children_.find(quadrant);
            }

            /// \brief Add point where you fall off the containers.
            /// The element would be in current->children_[quadrant]
            quadtree_node *new_node = allocate_quadtree_node(current, v);
            current->children_.emplace(quadrant, new_node);

            /// \brief Adjust the minimum bounds up to the root
            current->bounds_.stretch(v.first);
            while (current->parent_ != nullptr) {
                current = current->parent_;
                current->bounds_.stretch(v.first);
            }

            ++size_;
            return new_node;
        }

        void erase_impl(quadtree_node *node_to_remove) {
            // if this is empty
            if (node_to_remove == nullptr) {
                // we can't remove anything
                return;
            }

            // List of elements we need to reinsert in the containers after
            // removing the node
            std::vector<unprotected_value_type> reinsert_list;

            // copy and erase_impl all elements under node_to_remove to the
            // reinsert_list
            move_to_reinsert(node_to_remove, reinsert_list, false);

            // remove the root
            quadtree_node *current_node = node_to_remove->parent_;
            if (node_to_remove != root_) {
                // find it in the parent node and erase_impl it from the
                // children
                auto it = current_node->children_.find(
                    current_node->value_.first.quadrant(
                        node_to_remove->value_.first, comp_));
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

            // Reinsert all elements from the vector under the node
            auto comp = [this](const auto &a, const auto &b) {
                return std::lexicographical_compare(
                    a.first.begin(), a.first.end(), b.first.begin(),
                    b.first.end(), comp_);
            };
            std::sort(reinsert_list.begin(), reinsert_list.end(), comp);
            bulk_insert(reinsert_list,
                        current_node == nullptr ? root_ : current_node);

            // if current node was not root, recalculate bounds for parent nodes
            // up to the root
            while (current_node != nullptr) {
                current_node->bounds_ =
                    minimum_bounding_rectangle(current_node);
                current_node = current_node->parent_;
            }
        }

        /// \brief Find the smallest rectangle that includes all rectangles in
        /// branches of a node.
        box_type minimum_bounding_rectangle(quadtree_node *a_node) {
            assert(a_node);
            box_type rect = box_type(a_node->value_.first);
            for (auto &[quadrant, child] : a_node->children_) {
                rect.stretch(child->bounds_);
            }
            return rect;
        }

        /// \brief Copy contents of the whole containers
        void copy_recursive(quadtree_node *current,
                            quadtree_node *current_parent,
                            const quadtree_node *other) {
            current->value_ = other->value_;
            current->bounds_ = other->bounds_;
            current->parent_ = current_parent;
            for (const auto &[quadrant, child] : other->children_) {
                auto l_child = allocate_quadtree_node();
                current->children_.emplace(quadrant, l_child);
                copy_recursive(l_child, current, child);
            }
        }

        /// \brief Copy recursive for root nodes
        void copy_recursive(quadtree_node *current,
                            const quadtree_node *other) {
            copy_recursive(current, nullptr, other);
        }

        /// \brief Copy elements from the node to a list so that we can reinsert
        /// them again later This is the part of the algorithm to remove nodes
        /// from the quadtree It's a *very* inefficient operation
        void
        move_to_reinsert(quadtree_node *&node_to_remove,
                         std::vector<unprotected_value_type> &reinsert_list,
                         bool move_root) {
            for (auto &[quadrant, child_node] : node_to_remove->children_) {
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

        /// \brief Recursively find the max element
        quadtree_node *recursive_max_element(quadtree_node *parent_node,
                                             size_t dimension) const {
            if (parent_node->children_.empty()) {
                return parent_node;
            }
            if (parent_node->value_.first[dimension] >=
                parent_node->bounds_.second()[dimension]) {
                return parent_node;
            }
            // we could iterate only though the elements that have
            // quadrant[dimension] == 1 but both algorithms would be O(n) on
            // average and calculating these indexes would involve loops with
            // assignments and comparisons that would cost more than checking
            // the invalid elements
            auto max_it = std::max_element(
                parent_node->children_.begin(), parent_node->children_.end(),
                [&dimension](
                    const typename quadtree_node::children_map::value_type &a,
                    const typename quadtree_node::children_map::value_type &b) {
                    return a.second->bounds_.second()[dimension] <
                           b.second->bounds_.second()[dimension];
                });
            assert(max_it != parent_node->children_.end());
            if (parent_node->value_.first[dimension] >=
                max_it->second->bounds_.second()[dimension]) {
                return parent_node;
            } else {
                return recursive_max_element(max_it->second, dimension);
            }
        }

        /// \brief Recursively find the min element
        quadtree_node *recursive_min_element(quadtree_node *parent_node,
                                             size_t dimension) const {
            if (parent_node->children_.empty()) {
                return parent_node;
            }
            if (parent_node->value_.first[dimension] <=
                parent_node->bounds_.first()[dimension]) {
                return parent_node;
            }
            auto min_it = std::min_element(
                parent_node->children_.begin(), parent_node->children_.end(),
                [&dimension](
                    const typename quadtree_node::children_map::value_type &a,
                    const typename quadtree_node::children_map::value_type &b) {
                    return a.second->bounds_.first()[dimension] <
                           b.second->bounds_.first()[dimension];
                });
            assert(min_it != parent_node->children_.end());
            if (parent_node->value_.first[dimension] <=
                min_it->second->bounds_.first()[dimension]) {
                return parent_node;
            } else {
                return recursive_min_element(min_it->second, dimension);
            }
        }

        /// \brief Remove all points from the containers
        void remove_all_records() {
            if (root_ == nullptr) {
                return;
            }
            remove_all_records(root_);
        }

        /// \brief Remove all points from the containers
        void remove_all_records(quadtree_node *node) {
            assert(node);
            if (node->is_internal_node()) {
                for (auto &child : node->children_) {
                    assert(child.second != node->parent_);
                    assert(child.second->parent_ == node);
                    remove_all_records(child.second);
                }
            }
            deallocate_quadtree_node(node);
        }

        /// \brief Convert containers to string
        /// For debugging only
        [[nodiscard, maybe_unused]] std::string to_string() const {
            std::string str;
            // auto current = root_;
            str += to_string(root_, 0);
            return str;
        }

        /// \brief Convert containers to string
        /// For debugging only
        std::string to_string(const quadtree_node *current,
                              size_t level) const {
            std::string str(level * 2, ' ');
            std::stringstream ss;
            ss << current->value_.first;
            // ss << current->value_.first << ": " << current->value_.second;
            assert(current->bounds_.contains(current->value_.first));
            str += ss.str() + "\n";
            for (const auto &child : current->children_) {
                str += to_string(child.second, level + 1);
            }
            return str;
        }

      private /* Allocate nodes */:
        /// \brief Allocate a quadtree-node
        template <class... Args>
        quadtree_node *allocate_quadtree_node(Args &&...args) {
            auto p =
                std::allocator_traits<node_allocator_type>::allocate(alloc_, 1);
            std::allocator_traits<node_allocator_type>::construct(
                alloc_, p, std::forward<Args>(args)...);
            return p;
        }

        /// \brief Deallocate a quadtree-node
        void deallocate_quadtree_node(quadtree_node *p) {
            std::allocator_traits<node_allocator_type>::destroy(alloc_, p);
            std::allocator_traits<node_allocator_type>::deallocate(alloc_, p,
                                                                   1);
        }

      private /* members */:

        /// \brief Tree root node
        quadtree_node *root_;

        /// \brief Number of elements in the containers
        /// We cache the size at each insertion/removal.
        /// Otherwise it would cost us O(n log(n)) to find out the size
        size_t size_{0};

        /// \brief Number of dimensions of this rtree
        /// This is only needed when the dimension will be defined at runtime
        size_t dimensions_{0};

        /// \brief Node allocator
        /// It's fundamental to allocate our nodes with an efficient allocator
        /// to have a containers that can compete with vectors
        /// This is a shared ptr because trees can also share the same
        /// allocator. The last one to use it should delete the allocator.
        /// This is an important component of archives, where it's
        /// unreasonable to create an allocator for every new front.
        node_allocator_type alloc_;

        /// \brief Key comparison (single dimension)
        dimension_compare comp_{dimension_compare()};
    };

    // MSVC hack (we cannot define it inside iterator_impl)
    template <class N, size_t M, class E, class C, class A>
    template <bool constness>
    const std::function<
        bool(const typename quad_tree<N, M, E, C, A>::template iterator_impl<
                 constness>::queue_element &,
             const typename quad_tree<N, M, E, C, A>::template iterator_impl<
                 constness>::queue_element &)>
        quad_tree<N, M, E, C, A>::iterator_impl<constness>::queue_comp =
            [](const typename quad_tree<N, M, E, C, A>::template iterator_impl<
                   constness>::queue_element &a,
               const typename quad_tree<N, M, E, C, A>::template iterator_impl<
                   constness>::queue_element &b) -> bool {
        return std::get<2>(a) > std::get<2>(b);
    };

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
    bool operator==(const quad_tree<K, M, T, C, A> &lhs,
                    const quad_tree<K, M, T, C, A> &rhs) {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        return std::equal(
            lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
            [](const typename quad_tree<K, M, T, C, A>::value_type &a,
               const typename quad_tree<K, M, T, C, A>::value_type &b) {
                return a.first == b.first &&
                       mapped_type_custom_equality_operator(a.second, b.second);
            });
    }

    /// \brief Inequality operator
    template <class K, size_t M, class T, class C, class A>
    bool operator!=(const quad_tree<K, M, T, C, A> &lhs,
                    const quad_tree<K, M, T, C, A> &rhs) {
        return !(lhs == rhs);
    }

} // namespace pareto

#endif // PARETO_FRONT_QUAD_TREE_H