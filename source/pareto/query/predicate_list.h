//
// Created by Alan Freitas on 12/2/20.
//

#ifndef PARETO_PREDICATE_LIST_H
#define PARETO_PREDICATE_LIST_H

#include <variant>
#include <vector>

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
        using number_type = NUMBER_T;
        static constexpr size_t number_of_compile_dimensions = DimensionCount;
        using query_box_type = query_box<NUMBER_T, DimensionCount>;
        using point_type = point<NUMBER_T, DimensionCount>;
        using distance_type = typename point_type::distance_type;
        using key_type = point_type;
        using mapped_type = ELEMENT_TYPE;
        using value_type = std::pair<key_type, mapped_type>;
        using predicate_variant_type = predicate_variant<NUMBER_T, DimensionCount, ELEMENT_TYPE>;
        using vector_type = std::vector<predicate_variant_type>;

    public /* constructors */:
        /// \brief Construct an empty predicate list
        predicate_list() : predicates_({}) {}

        /// \brief Construct from a list of predicates
        explicit predicate_list(const std::vector<predicate_variant_type> &predicates) : predicates_(predicates) {
            compress();
        }

        /// \brief Construct from a list of predicates
        predicate_list(std::initializer_list<predicate_variant_type> predicates) :
            predicates_(predicates.begin(), predicates.end()) {
            compress();
        }

        /// \brief Construct from iterators
        template <class Iterator>
        predicate_list(Iterator predicates_begin, Iterator predicates_end) :
                predicates_(predicates_begin, predicates_end) {
            compress();
        }

        /// \brief Copy constructor from another predicate_list predicate
        predicate_list(const predicate_list &rhs) : predicates_(rhs.predicates_) {}

        predicate_list &operator=(const predicate_list &rhs) {
            predicates_ = rhs.predicates_;
            return *this;
        }

        /// \brief Move constructor from another predicate_list predicate
        predicate_list(predicate_list &&rhs) noexcept: predicates_(std::move(rhs.predicates_)) {}

        predicate_list &operator=(predicate_list &&rhs) noexcept {
            predicates_ = std::move(rhs.predicates_);
            return *this;
        }

        /// \brief Construct from a predicate_variant_type predicate
        explicit predicate_list(const predicate_variant_type &predicate) : predicates_({predicate}) {}

        /// \brief Construct from a intersects predicate
        explicit predicate_list(const intersects <number_type, number_of_compile_dimensions> &predicate)
                : predicates_({predicate_variant_type(predicate)}) {}

        /// \brief Construct from a disjoint predicate
        explicit predicate_list(const disjoint <number_type, number_of_compile_dimensions> &predicate)
                : predicates_({predicate_variant_type(predicate)}) {}

        /// \brief Construct from a within predicate
        explicit predicate_list(const within <number_type, number_of_compile_dimensions> &predicate)
                : predicates_({predicate_variant_type(predicate)}) {}

        /// \brief Construct from a nearest predicate
        explicit predicate_list(const nearest <number_type, number_of_compile_dimensions> &predicate)
                : predicates_({predicate_variant_type(predicate)}) {}

        /// \brief Construct from a satisfies predicate
        explicit predicate_list(const satisfies <number_type, number_of_compile_dimensions, mapped_type> &predicate)
                : predicates_({predicate_variant_type(predicate)}) {}

    public /* vector functions */:
        typename vector_type::iterator begin() {
            return predicates_.begin();
        }

        typename vector_type::iterator end() {
            return predicates_.end();
        }

        typename vector_type::const_iterator begin() const {
            return predicates_.begin();
        }

        typename vector_type::const_iterator end() const {
            return predicates_.end();
        }

        typename vector_type::size_type size() const {
            return predicates_.size();
        }

        typename vector_type::const_reference operator[](size_t idx) const {
            return predicates_[idx];
        }

        typename vector_type::reference operator[](size_t idx) {
            return predicates_[idx];
        }

        void clear() {
            return predicates_.clear();
        }

    public:
        /// \brief Check if predicate is of type intersects
        [[nodiscard]] bool contains_intersects() const {
            return std::any_of(predicates_.begin(), predicates_.end(), [](const auto& p) {
                return p.is_intersects();
            });
        }

        /// \brief Check if predicate is of type disjoint
        [[nodiscard]] bool contains_disjoint() const {
            return std::any_of(predicates_.begin(), predicates_.end(), [](const auto& p) {
                return p.is_disjoint();
            });
        }

        /// \brief Check if predicate is of type within
        [[nodiscard]] bool contains_within() const {
            return std::any_of(predicates_.begin(), predicates_.end(), [](const auto& p) {
                return p.is_within();
            });
        }

        /// \brief Check if predicate is of type nearest
        [[nodiscard]] bool contains_nearest() const {
            return std::any_of(predicates_.begin(), predicates_.end(), [](const auto& p) {
                return p.is_nearest();
            });
        }

        /// \brief Check if predicate is of type satisfies
        [[nodiscard]] bool contains_satisfies() const {
            return std::any_of(predicates_.begin(), predicates_.end(), [](const auto& p) {
                return p.is_satisfies();
            });
        }

                /// \brief Check if predicate is of type intersects
        [[nodiscard]] bool is_all_intersects() const {
            return std::all_of(predicates_.begin(), predicates_.end(), [](const auto& p) {
                return p.is_intersects();
            });
        }

        /// \brief Check if predicate is of type disjoint
        [[nodiscard]] bool is_all_disjoint() const {
            return std::all_of(predicates_.begin(), predicates_.end(), [](const auto& p) {
                return p.is_disjoint();
            });
        }

        /// \brief Check if predicate is of type within
        [[nodiscard]] bool is_all_within() const {
            return std::all_of(predicates_.begin(), predicates_.end(), [](const auto& p) {
                return p.is_within();
            });
        }

        /// \brief Check if predicate is of type nearest
        [[nodiscard]] bool is_all_nearest() const {
            return std::all_of(predicates_.begin(), predicates_.end(), [](const auto& p) {
                return p.is_nearest();
            });
        }

        /// \brief Check if predicate is of type satisfies
        [[nodiscard]] bool is_all_satisfies() const {
            return std::all_of(predicates_.begin(), predicates_.end(), [](const auto& p) {
                return p.is_satisfies();
            });
        }

        /// \brief Get a predicate of type intersects if the list contains any
        const intersects<number_type, number_of_compile_dimensions>* get_intersects() const {
            for (const auto& p : predicates_) {
                if (p.is_intersects()) {
                    return &p.as_intersects();
                }
            }
            return nullptr;
        }

        /// \brief Get a predicate of type disjoint if the list contains any
        const disjoint <number_type, number_of_compile_dimensions>* get_disjoint() const {
            for (const auto& p : predicates_) {
                if (p.is_disjoint()) {
                    return &p.as_disjoint();
                }
            }
            return nullptr;
        }

        /// \brief Get a predicate of type within if the list contains any
        const within <number_type, number_of_compile_dimensions>* get_within() const {
            for (const auto& p : predicates_) {
                if (p.is_within()) {
                    return &p.as_within();
                }
            }
            return nullptr;
        }

        /// \brief Get a predicate of type nearest if the list contains any
        const nearest <number_type, number_of_compile_dimensions>* get_nearest() const {
            for (const auto& p : predicates_) {
                if (p.is_nearest()) {
                    return &p.as_nearest();
                }
            }
            return nullptr;
        }

        /// \brief Get a predicate of type satisfies if the list contains any
        const satisfies <number_type, number_of_compile_dimensions, mapped_type>* get_satisfies() const {
            for (const auto& p : predicates_) {
                if (p.is_satisfies()) {
                    return &p.as_satisfies();
                }
            }
            return nullptr;
        }

        /// \brief Get predicate of type intersects if the list contains any
        intersects <number_type, number_of_compile_dimensions> *get_intersects() {
            for (auto& p : predicates_) {
                if (p.is_intersects()) {
                    return &p.as_intersects();
                }
            }
            return nullptr;
        }

        /// \brief Get predicate of type disjoint if the list contains any
        disjoint <number_type, number_of_compile_dimensions> *get_disjoint() {
            for (auto& p : predicates_) {
                if (p.is_disjoint()) {
                    return &p.as_disjoint();
                }
            }
            return nullptr;
        }

        /// \brief Get predicate of type within if the list contains any
        within <number_type, number_of_compile_dimensions> *get_within() {
            for (auto& p : predicates_) {
                if (p.is_within()) {
                    return &p.as_within();
                }
            }
            return nullptr;
        }

        /// \brief Get predicate of type nearest if the list contains any
        nearest <number_type, number_of_compile_dimensions> *get_nearest() {
            for (auto& p : predicates_) {
                if (p.is_nearest()) {
                    return &p.as_nearest();
                }
            }
            return nullptr;
        }

        /// \brief Get predicate of type satisfies if the list contains any
        satisfies <number_type, number_of_compile_dimensions, mapped_type> *get_satisfies() {
            for (auto& p : predicates_) {
                if (p.is_satisfies()) {
                    return &p.as_satisfies();
                }
            }
            return nullptr;
        }

        /// \brief Does the box pass the predicate list
        /// It passes the list if it passes all predicates there
        bool pass_predicate(const query_box_type &rhs) const {
            return std::all_of(predicates_.begin(), predicates_.end(), [&rhs](const auto& p) {
                return p.pass_predicate(rhs);
            });
        }

        /// \brief Can a child in this box pass the predicate list
        /// It might pass the list if it might pass all predicates there
        bool might_pass_predicate(const query_box_type &rhs) const {
            return std::all_of(predicates_.begin(), predicates_.end(), [&rhs](const auto& p) {
                return p.might_pass_predicate(rhs);
            });
        }

        /// \brief Does the point pass the predicate list
        /// It passes the list if it passes all predicates there
        bool pass_predicate(const point_type &rhs) const {
            return std::all_of(predicates_.begin(), predicates_.end(), [&rhs](const auto& p) {
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
                return p.might_pass_predicate(rhs);
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
        /// \param total_volume Total volume of the data structure we are manipulating
        void sort(number_type total_volume) {
            // Handle the most trivial cases
            // there is nothing to sort if there are less than 2 elements
            if (predicates_.size() < 2) {
                return;
            }

            // if it's a query box and a predicate, just swap or not
            if (predicates_.size() == 2) {
                if (predicates_[1].is_more_restrictive(predicates_[0], total_volume)) {
                    std::swap(predicates_[0], predicates_[1]);
                }
                return;
            }

            // For the general case
            // Sort predicates by how restrictive they are
            std::sort(predicates_.begin(), predicates_.end(), [&total_volume](const auto &a, const auto &b) {
                return a.is_more_restrictive(b, total_volume);
            });
        }

        /// \brief This function merges predicates with that same meaning
        void compress() {
            for (size_t i = 0; i < predicates_.size() - 1; ++i) {
                for (size_t j = i + 1; j < predicates_.size(); ++i) {
                    predicate_variant_type& p = predicates_[i];
                    predicate_variant_type& q = predicates_[j];
                    if (p.is_intersects()) {
                        if (q.is_intersects()) {
                            const query_box_type& pq = p.as_intersects().data();
                            const query_box_type& qq = q.as_intersects().data();
                            const bool q_is_already_in_p = pq.contains(qq);
                            if (q_is_already_in_p) {
                                p = q;
                                predicates_.erase(predicates_.begin() + j);
                                --i;
                            } else if (qq.contains(pq)) {
                                predicates_.erase(predicates_.begin() + j);
                                --i;
                            }
                        } else if (q.is_disjoint()) {
                            const query_box_type& pq = p.as_intersects().data();
                            const query_box_type& qq = q.as_disjoint().data();
                            const bool disjoint_not_queried_anyway = !pq.contains(qq);
                            if (disjoint_not_queried_anyway) {
                                predicates_.erase(predicates_.begin() + j);
                                --i;
                            } else {
                                const bool impossible_query = qq.contains(pq);
                                if (impossible_query) {
                                    predicates_ = {disjoint<number_type,DimensionCount>(point_type(qq.dimensions(), -std::numeric_limits<number_type>::max()), point_type(qq.dimensions(), std::numeric_limits<number_type>::max()))};
                                    return;
                                }
                            }
                        }
                    } else if (p.is_nearest()) {
                        if (q.is_nearest()) {
                            const nearest<number_type,DimensionCount>& pn = p.as_nearest();
                            const nearest<number_type,DimensionCount>& qn = q.as_nearest();
                            const size_t& pk = pn.k();
                            const size_t& qk = qn.k();
                            p.as_nearest().k(std::min(pk,qk));
                            query_box_type new_box;
                            if (pn.has_reference_point()) {
                                new_box.stretch(pn.reference_point());
                            } else {
                                new_box.stretch(pn.reference_box());
                            }
                            if (qn.has_reference_point()) {
                                new_box.stretch(qn.reference_point());
                            } else {
                                new_box.stretch(qn.reference_box());
                            }
                            p.as_nearest().reference_box(new_box);
                            predicates_.erase(predicates_.begin() + j);
                            --i;
                        }
                    }
                }
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
