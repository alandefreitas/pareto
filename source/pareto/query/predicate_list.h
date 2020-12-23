//
// Created by Alan Freitas on 12/2/20.
//

#ifndef PARETO_PREDICATE_LIST_H
#define PARETO_PREDICATE_LIST_H

#include <variant>
#include <vector>
#include <optional>

#include <pareto/point.h>
#include <pareto/query/predicate_variant.h>

namespace pareto {

    /// \class List of predicates of any type
    /// This class stores a list of predicates that can be evaluated
    /// as a predicate_union or as a predicate_intersection.
    ///
    /// The main vector functions are exposed through this class
    /// to make it easy to access the predicates.
    ///
    /// Besides this basic functionality, this class offers:
    /// * a function check whether a list passes all predicates at once
    ///   (as union or intersection)
    /// * an implementation to sanitize the lists
    ///
    /// The main purpose of a class for predicate lists is to include
    /// functions to sanitize the lists. We sort, filter and compress
    /// predicates whenever new predicates are inserted.
    /// * Sort: most restrictive predicates should come first
    /// * Filter: only one nearest predicate is allowed
    /// * Compress: only one nearest predicate is allows
    /// This makes the list of predicates more efficient
    template<typename NUMBER_T, std::size_t DimensionCount, class ELEMENT_TYPE>
    class predicate_list {
        using dimension_type = NUMBER_T;
        static constexpr size_t number_of_compile_dimensions = DimensionCount;
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
        using key_type = point_type;
        using mapped_type = ELEMENT_TYPE;
        using value_type = std::pair<key_type, mapped_type>;
        using predicate_variant_type =
            predicate_variant<NUMBER_T, DimensionCount, ELEMENT_TYPE>;
        using intersects_type =
            intersects<dimension_type, number_of_compile_dimensions>;
        using disjoint_type =
            disjoint<dimension_type, number_of_compile_dimensions>;
        using within_type =
            within<dimension_type, number_of_compile_dimensions>;
        using nearest_type =
            nearest<dimension_type, number_of_compile_dimensions>;
        using satisfies_type =
            satisfies<dimension_type, number_of_compile_dimensions,
                      ELEMENT_TYPE>;
        using vector_type = std::vector<predicate_variant_type>;

      public /* constructors */:
        /// \brief Construct an empty predicate list
        predicate_list() : predicates_({}) {}

        /// \brief Construct from a list of predicates
        explicit predicate_list(
            const std::vector<predicate_variant_type> &predicates)
            : predicates_(predicates) {
            compress();
        }

        /// \brief Construct from a list of predicates
        predicate_list(std::initializer_list<predicate_variant_type> predicates) :
                predicates_(predicates.begin(), predicates.end()) {
            compress();
        }

        /// \brief Construct from iterators
        template<class Iterator>
        predicate_list(Iterator predicates_begin, Iterator predicates_end) :
                predicates_(predicates_begin, predicates_end) {
            compress();
        }

        /// \brief Copy constructor from another predicate_list predicate
        predicate_list(const predicate_list &rhs) : predicates_(rhs.predicates_) {}

        /// \brief Copy attribution operator
        predicate_list &operator=(const predicate_list &rhs) {
            predicates_ = rhs.predicates_;
            return *this;
        }

        /// \brief Move constructor from another predicate_list predicate
        predicate_list(predicate_list &&rhs) noexcept: predicates_(std::move(rhs.predicates_)) {}

        /// \brief Move attribution operator
        predicate_list &operator=(predicate_list &&rhs) noexcept {
            predicates_ = std::move(rhs.predicates_);
            return *this;
        }

        /// \brief Construct from a predicate_variant_type predicate
        explicit predicate_list(const predicate_variant_type &predicate) : predicates_({predicate}) {}

        /// \brief Construct from a intersects predicate
        explicit predicate_list(
            const intersects<dimension_type, number_of_compile_dimensions>
                &predicate)
            : predicates_({predicate_variant_type(predicate)}) {}

        /// \brief Construct from a disjoint predicate
        explicit predicate_list(
            const disjoint<dimension_type, number_of_compile_dimensions>
                &predicate)
            : predicates_({predicate_variant_type(predicate)}) {}

        /// \brief Construct from a within predicate
        explicit predicate_list(
            const within<dimension_type, number_of_compile_dimensions>
                &predicate)
            : predicates_({predicate_variant_type(predicate)}) {}

        /// \brief Construct from a nearest predicate
        explicit predicate_list(
            const nearest<dimension_type, number_of_compile_dimensions>
                &predicate)
            : predicates_({predicate_variant_type(predicate)}) {}

        /// \brief Construct from a satisfies predicate
        explicit predicate_list(
            const satisfies<dimension_type, number_of_compile_dimensions,
                            mapped_type> &predicate)
            : predicates_({predicate_variant_type(predicate)}) {}

      public /* vector functions */:
        /// \brief Get iterator to first predicate
        typename vector_type::iterator begin() {
            return predicates_.begin();
        }

        /// \brief Get iterator to past-the-end predicate
        typename vector_type::iterator end() {
            return predicates_.end();
        }

        /// \brief Get iterator to first predicate
        typename vector_type::const_iterator begin() const {
            return predicates_.begin();
        }

        /// \brief Get iterator to past-the-end predicate
        typename vector_type::const_iterator end() const {
            return predicates_.end();
        }

        /// \brief Get number of predicates
        typename vector_type::size_type size() const {
            return predicates_.size();
        }

        /// \brief Get const reference to i-th predicate
        typename vector_type::const_reference operator[](size_t idx) const {
            return predicates_[idx];
        }

        /// \brief Get reference to i-th predicate
        typename vector_type::reference operator[](size_t idx) {
            return predicates_[idx];
        }

        /// \brief Clear all predicates from list
        void clear() {
            return predicates_.clear();
        }

    public:
        /// \brief Check if predicate is of type intersects
        [[nodiscard]] bool contains_intersects() const {
            return std::any_of(predicates_.begin(), predicates_.end(), [](const auto &p) {
                return p.is_intersects();
            });
        }

        /// \brief Check if predicate is of type disjoint
        [[nodiscard]] bool contains_disjoint() const {
            return std::any_of(predicates_.begin(), predicates_.end(), [](const auto &p) {
                return p.is_disjoint();
            });
        }

        /// \brief Check if predicate is of type within
        [[nodiscard]] bool contains_within() const {
            return std::any_of(predicates_.begin(), predicates_.end(), [](const auto &p) {
                return p.is_within();
            });
        }

        /// \brief Check if predicate is of type nearest
        [[nodiscard]] bool contains_nearest() const {
            return std::any_of(predicates_.begin(), predicates_.end(), [](const auto &p) {
                return p.is_nearest();
            });
        }

        /// \brief Check if predicate is of type satisfies
        [[nodiscard]] bool contains_satisfies() const {
            return std::any_of(predicates_.begin(), predicates_.end(), [](const auto &p) {
                return p.is_satisfies();
            });
        }

        /// \brief Check if predicate is of type intersects
        [[nodiscard]] bool is_all_intersects() const {
            return std::all_of(predicates_.begin(), predicates_.end(), [](const auto &p) {
                return p.is_intersects();
            });
        }

        /// \brief Check if predicate is of type disjoint
        [[nodiscard]] bool is_all_disjoint() const {
            return std::all_of(predicates_.begin(), predicates_.end(), [](const auto &p) {
                return p.is_disjoint();
            });
        }

        /// \brief Check if predicate is of type within
        [[nodiscard]] bool is_all_within() const {
            return std::all_of(predicates_.begin(), predicates_.end(), [](const auto &p) {
                return p.is_within();
            });
        }

        /// \brief Check if predicate is of type nearest
        [[nodiscard]] bool is_all_nearest() const {
            return std::all_of(predicates_.begin(), predicates_.end(), [](const auto &p) {
                return p.is_nearest();
            });
        }

        /// \brief Check if predicate is of type satisfies
        [[nodiscard]] bool is_all_satisfies() const {
            return std::all_of(predicates_.begin(), predicates_.end(), [](const auto &p) {
                return p.is_satisfies();
            });
        }

        /// \brief Get a predicate of type intersects if the list contains any
        const intersects<dimension_type, number_of_compile_dimensions> *
        get_intersects() const {
            for (const auto &p : predicates_) {
                if (p.is_intersects()) {
                    return &p.as_intersects();
                }
            }
            return nullptr;
        }

        /// \brief Get a predicate of type disjoint if the list contains any
        const disjoint<dimension_type, number_of_compile_dimensions> *
        get_disjoint() const {
            for (const auto &p : predicates_) {
                if (p.is_disjoint()) {
                    return &p.as_disjoint();
                }
            }
            return nullptr;
        }

        /// \brief Get a predicate of type within if the list contains any
        const within<dimension_type, number_of_compile_dimensions> *
        get_within() const {
            for (const auto &p : predicates_) {
                if (p.is_within()) {
                    return &p.as_within();
                }
            }
            return nullptr;
        }

        /// \brief Get a predicate of type nearest if the list contains any
        const nearest<dimension_type, number_of_compile_dimensions> *
        get_nearest() const {
            for (const auto &p : predicates_) {
                if (p.is_nearest()) {
                    return &p.as_nearest();
                }
            }
            return nullptr;
        }

        /// \brief Get a predicate of type satisfies if the list contains any
        const satisfies<dimension_type, number_of_compile_dimensions,
                        mapped_type> *
        get_satisfies() const {
            for (const auto &p : predicates_) {
                if (p.is_satisfies()) {
                    return &p.as_satisfies();
                }
            }
            return nullptr;
        }

        /// \brief Get predicate of type intersects if the list contains any
        intersects<dimension_type, number_of_compile_dimensions> *
        get_intersects() {
            for (auto &p : predicates_) {
                if (p.is_intersects()) {
                    return &p.as_intersects();
                }
            }
            return nullptr;
        }

        /// \brief Get predicate of type disjoint if the list contains any
        disjoint<dimension_type, number_of_compile_dimensions> *get_disjoint() {
            for (auto &p : predicates_) {
                if (p.is_disjoint()) {
                    return &p.as_disjoint();
                }
            }
            return nullptr;
        }

        /// \brief Get predicate of type within if the list contains any
        within<dimension_type, number_of_compile_dimensions> *get_within() {
            for (auto &p : predicates_) {
                if (p.is_within()) {
                    return &p.as_within();
                }
            }
            return nullptr;
        }

        /// \brief Get predicate of type nearest if the list contains any
        nearest<dimension_type, number_of_compile_dimensions> *get_nearest() {
            for (auto &p : predicates_) {
                if (p.is_nearest()) {
                    return &p.as_nearest();
                }
            }
            return nullptr;
        }

        /// \brief Get predicate of type satisfies if the list contains any
        satisfies<dimension_type, number_of_compile_dimensions, mapped_type> *
        get_satisfies() {
            for (auto &p : predicates_) {
                if (p.is_satisfies()) {
                    return &p.as_satisfies();
                }
            }
            return nullptr;
        }

        /// \brief Does the box pass the predicate list
        /// It passes the list if it passes all predicates there
        bool pass_predicate(const query_box_type &rhs) const {
            return std::all_of(predicates_.begin(), predicates_.end(), [&rhs](const auto &p) {
                return p.pass_predicate(rhs);
            });
        }

        /// \brief Can a child in this box pass the predicate list
        /// It might pass the list if it might pass all predicates there
        bool might_pass_predicate(const query_box_type &rhs) const {
            return std::all_of(predicates_.begin(), predicates_.end(), [&rhs](const auto &p) {
                return p.might_pass_predicate(rhs);
            });
        }

        /// \brief Does the point pass the predicate list
        /// It passes the list if it passes all predicates there
        bool pass_predicate(const point_type &rhs) const {
            return std::all_of(predicates_.begin(), predicates_.end(), [&rhs](const auto &p) {
                return p.pass_predicate(rhs);
            });
        }

        /// \brief Can a child in this point pass the predicate list
        /// It might pass the list if it might pass all predicates there
        bool might_pass_predicate(const point_type &rhs) const {
            return std::all_of(predicates_.begin(), predicates_.end(), [&rhs](const auto &p) {
                return p.might_pass_predicate(rhs);
            });
        }

        /// \brief Does the value pass the predicate list
        /// It passes the list if it passes all predicates there
        bool pass_predicate(const value_type &rhs) const {
            return std::all_of(predicates_.begin(), predicates_.end(), [&rhs](const auto &p) {
                return p.pass_predicate(rhs);
            });
        }

        /// \brief Can a child in this value pass the predicate list
        /// It might pass the list if it might pass all predicates there
        bool might_pass_predicate(const value_type &rhs) const {
            return std::all_of(predicates_.begin(), predicates_.end(), [&rhs](const auto &p) {
                return p.might_pass_predicate(rhs);
            });
        }

        /// \brief Sort predicates by how restrictive they are
        /// Sorting puts the predicates in its most efficient order
        /// \param total_volume Total volume of the data structure we are
        /// manipulating
        void sort(dimension_type total_volume) {
            const bool nothing_to_sort = predicates_.size() < 2;
            if (nothing_to_sort) {
                return;
            }

            const bool only_two_predicates = predicates_.size() == 2;
            if (only_two_predicates) {
                if (predicates_[1].is_more_restrictive(predicates_[0],
                                                       total_volume)) {
                    std::swap(predicates_[0], predicates_[1]);
                }
                return;
            }

            std::sort(predicates_.begin(), predicates_.end(), [&total_volume](const auto &a, const auto &b) {
                return a.is_more_restrictive(b, total_volume);
            });
        }

        /// \brief This function merges predicates with that same meaning
        void compress() {
            if (predicates_.size() < 2) {
                return;
            }
            for (size_t i = 0; i < predicates_.size() - 1; ++i) {
                for (size_t j = i + 1; j < predicates_.size(); ++j) {
                    auto compressed_predicate =
                        compress(predicates_[i], predicates_[j]);
                    if (compressed_predicate) {
                        predicates_[i] = *compressed_predicate;
                        predicates_.erase(predicates_.begin() + j);
                        --j;
                    }
                }
            }
        }

        /// \brief Attempt to compress a pair of predicate variants
        /// This function goes though all valid kinds of compression.
        /// If the two variants are good candidates for compression, we convert the predicates
        /// to their underlying type and call the corresponding function.
        /// If the pair cannot be compressed, we return std::nullopt so the
        /// calling function knows we could not compress anything.
        /// Within/intersects/disjoint predicate can be compressed by throwing away
        /// redundant predicates.
        /// Nearest predicates can be compressed to their lowest number of nearest
        /// points.
        /// The only predicate that can never be compressed in a satisfies predicate
        std::optional<predicate_variant_type>
        compress(const predicate_variant_type &a, const predicate_variant_type &b) {
            if (a.is_intersects()) {
                if (b.is_intersects()) {
                    return compress(a.as_intersects(), b.as_intersects());
                } else if (b.is_within()) {
                    return compress(a.as_intersects(), b.as_within());
                } else if (b.is_disjoint()) {
                    return compress(a.as_intersects(), b.as_disjoint());
                }
            } else if (a.is_within()) {
                if (b.is_intersects()) {
                    return compress(a.as_within(), b.as_intersects());
                } else if (b.is_within()) {
                    return compress(a.as_within(), b.as_within());
                } else if (b.is_disjoint()) {
                    return compress(a.as_within(), b.as_disjoint());
                }
            } else if (a.is_disjoint()) {
                if (b.is_intersects()) {
                    return compress(a.as_disjoint(), b.as_intersects());
                } else if (b.is_within()) {
                    return compress(a.as_disjoint(), b.as_within());
                } else if (b.is_disjoint()) {
                    return compress(a.as_disjoint(), b.as_disjoint());
                }
            } else if (a.is_nearest()) {
                if (b.is_nearest()) {
                    return compress(a.as_nearest(), b.as_nearest());
                }
            }
            return std::nullopt;
        }

        /// \brief Compress a pair of predicates <intersects, intersects>
        /// If you predicate contains the other, we can remove the predicate
        /// with the largest hyperbox
        std::optional<predicate_variant_type> compress(const intersects_type &a, const intersects_type &b) {
            const query_box_type &pq = a.data();
            const query_box_type &qq = b.data();
            const bool q_is_already_in_p = pq.contains(qq);
            if (q_is_already_in_p) {
                return a;
            } else if (qq.contains(pq)) {
                return b;
            }
            return std::nullopt;
        }

        /// \brief Compress a pair of predicates <intersects, within>
        /// If you predicate contains the other, we can remove the predicate
        /// with the largest hyperbox.
        /// We have to take care of the borders though.
        /// Intersects contains within: retain within
        /// Within contains intersects:
        /// * Intersects is on the borders / same hyperbox: retain within
        /// * Intersects does not touch the borders / both hyperbox points different: retain intersects
        /// * Intersects touches one of the borders / only one hyperbox point is equal: cannot compress
        std::optional<predicate_variant_type> compress(const intersects_type &a, const within_type &b) {
            const query_box_type &aq = a.data();
            const query_box_type &bq = b.data();
            if (aq.contains(bq)) {
                return b;
            } else if (bq.contains(aq)) {
                bool touch_min_border = aq.min() == bq.min();
                bool touch_max_border = aq.max() == bq.max();
                if (touch_min_border && touch_max_border) {
                    return b;
                } else if (!touch_min_border && !touch_max_border) {
                    return a;
                }
            }
            return std::nullopt;
        }

        /// \brief Compress a pair of predicates <within, intersects>
        /// \copydetails std::optional<predicate_variant_type> compress(const intersects_type& a, const within_type & b)
        std::optional<predicate_variant_type> compress(const within_type &a, const intersects_type &b) {
            return compress(b, a);
        }

        /// \brief Compress a pair of predicates <intersects, disjoint>
        /// If the disjoint and intersects don't overlap, we can just throw the disjoint away
        /// If the intersects is inside the disjoint, the query is impossible (return any impossible query)
        std::optional<predicate_variant_type> compress(const intersects_type &a, const disjoint_type &b) {
            const query_box_type &aq = a.data();
            const query_box_type &bq = b.data();
            if (!aq.overlap(bq)) {
                return a;
            } else if (bq.contains(aq)) {
                auto impossible_box = bq;
                impossible_box.stretch_to_infinity();
                return disjoint_type(impossible_box);
            }
            return std::nullopt;
        }

        /// \brief Compress a pair of predicates <intersects, disjoint>
        /// If the disjoint and intersects don't overlap, we can just throw the disjoint away
        /// If the intersects is inside the disjoint, the query is impossible (return any impossible query)
        std::optional<predicate_variant_type> compress(const disjoint_type &a, const intersects_type &b) {
            return compress(b, a);
        }

        /// \brief Compress a pair of predicates <within, within>
        /// If you predicate contains the other, we can remove the predicate
        /// with the largest hyperbox.
        /// This is akin to compressing <intersects, intersects> pairs
        std::optional<predicate_variant_type> compress(const within_type &a, const within_type &b) {
            const query_box_type &pq = a.data();
            const query_box_type &qq = b.data();
            const bool q_is_already_in_p = pq.contains(qq);
            if (q_is_already_in_p) {
                return a;
            } else if (qq.contains(pq)) {
                return b;
            }
            return std::nullopt;
        }

        /// \brief Compress a pair of predicates <within, disjoint>
        /// If the disjoint and within don't overlap, we can just throw the disjoint away
        /// If the within is inside the disjoint, the query is impossible (return any impossible query)
        std::optional<predicate_variant_type> compress(const within_type &a, const disjoint_type &b) {
            const query_box_type &aq = a.data();
            const query_box_type &bq = b.data();
            if (!aq.overlap(bq)) {
                return a;
            } else if (bq.contains(aq)) {
                auto max_number = std::numeric_limits<dimension_type>::max();
                return disjoint_type(point_type(bq.dimensions(), -max_number), point_type(bq.dimensions(), max_number));
            }
            return std::nullopt;
        }

        /// \brief Compress a pair of predicates <within, disjoint>
        /// If the disjoint and within don't overlap, we can just throw the disjoint away
        /// If the within is inside the disjoint, the query is impossible (return any impossible query)
        std::optional<predicate_variant_type> compress(const disjoint_type &a, const within_type &b) {
            return compress(b, a);
        }

        /// \brief Compress a pair of predicates <nearest, nearest>
        /// Because a nearest type predicate changes how the query algorithm works,
        /// only one nearest predicate is allowed per predicate list.
        /// If there are two of these, instead of throwing an error, we
        /// create a new nearest predicate with the lowest k between them and,
        /// if the points are different, we look for an intermediary point.
        /// In the future, we can look at some other alternatives:
        /// * Keeping a list of reference points so that we would consider the
        ///   distance to the closest point in the list of reference points
        /// * Creating one stack per nearest predicate and updating all stacks
        ///   per point visited. This would considerably change how things work now.
        std::optional<predicate_variant_type> compress(const nearest_type &a, const nearest_type &b) {
            const size_t new_k = std::min(a.k(), b.k());
            if (a.has_reference_point() && b.has_reference_point()) {
                query_box_type q(a.reference_point(), b.reference_point());
                return nearest(q.center(), new_k);
            } else {
                query_box_type qa = a.has_reference_box() ? a.reference_box() : query_box_type(a.reference_point(),
                                                                                               a.has_reference_point());
                query_box_type qb = b.has_reference_box() ? b.reference_box() : query_box_type(b.reference_point(),
                                                                                               b.has_reference_point());
                qa.stretch(qb);
                return nearest(qa, new_k);
            }
        }

        /// \brief Check if two predicate lists have the same predicates
        bool operator==(const predicate_list &rhs) const {
            return predicates_ == rhs.predicates_;
        }

        /// \brief Check if two predicates are of different types or have different parameters
        bool operator!=(const predicate_list &rhs) const {
            return predicates_ != rhs.predicates_;
        }

    private:
        /// \brief List of predicates
        vector_type predicates_;

    };
}

#endif //PARETO_PREDICATE_LIST_H
