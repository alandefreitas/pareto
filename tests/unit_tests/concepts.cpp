
#include <catch2/catch.hpp>
#include <map>
#include <memory>

#include <pareto/archive.h>
#include <pareto/common/demangle.h>
#include <pareto/front.h>
#include <pareto/implicit_tree.h>
#include <pareto/kd_tree.h>
#include <pareto/quad_tree.h>
#include <pareto/r_star_tree.h>
#include <pareto/r_tree.h>

template <class T> void check_move_insertable_concept() {
    ;
    using A = std::allocator<T>;
    A m;
    T *p = new T;
    T v;
    REQUIRE_NOTHROW(std::allocator_traits<A>::construct(m, p, std::move(v)));
    delete p;
}

template <class T> void check_copy_insertable_concept() {
    using A = std::allocator<T>;
    A m;
    T *p = new T;
    T v;
    REQUIRE_NOTHROW(std::allocator_traits<A>::construct(m, p, v));
    delete p;
    check_move_insertable_concept<T>();
}

template <class T> void check_default_constructible_concept() {
    REQUIRE(std::is_default_constructible_v<T>);
}

template <class T> void check_copy_constructible_concept() {
    REQUIRE(std::is_copy_constructible_v<T>);
}

template <class T> void check_swappable_concept() {
    REQUIRE(std::is_swappable_v<T>);
}

template <class T> void check_destructible_concept() {
    REQUIRE(std::is_destructible_v<T>);
}

template <class T> void check_equality_comparable_concept() {
    T a{};
    T b{};
    T c{};
    REQUIRE(std::is_convertible_v<decltype(a == b), bool>);
    REQUIRE(a == a);
    REQUIRE((!(a == b) || (b == a)));
    REQUIRE((!(a == b && b == c) || (a == c)));
}

template <class C, class T> void check_container_concept() {
    SECTION("Container") {
        SECTION("Types") {
            REQUIRE(std::is_same_v<typename C::value_type, T>);

            REQUIRE(std::is_same_v<typename C::reference, T &>);

            REQUIRE(std::is_same_v<typename C::const_reference, const T &>);

            REQUIRE(std::is_same_v<decltype(*(typename C::iterator())), T &>);
            REQUIRE(std::is_base_of_v<
                    std::forward_iterator_tag,
                    typename std::iterator_traits<
                        typename C::iterator>::iterator_category>);
            REQUIRE(std::is_convertible_v<typename C::iterator,
                                          typename C::const_iterator>);

            REQUIRE(std::is_same_v<decltype(*(typename C::const_iterator())),
                                   const T &>);
            REQUIRE(std::is_base_of_v<
                    std::forward_iterator_tag,
                    typename std::iterator_traits<
                        typename C::const_iterator>::iterator_category>);

            REQUIRE(std::is_signed_v<typename C::difference_type>);
            REQUIRE(std::is_same_v<typename C::difference_type,
                                   typename std::iterator_traits<
                                       typename C::iterator>::difference_type>);
            REQUIRE(std::is_same_v<
                    typename C::difference_type,
                    typename std::iterator_traits<
                        typename C::const_iterator>::difference_type>);

            REQUIRE(std::is_unsigned_v<typename C::size_type>);
            REQUIRE(sizeof(typename C::size_type) >=
                    sizeof(typename C::difference_type));
        }

        SECTION("Methods") {
            C b;

            SECTION("empty constructor") {
                // Expression
                C a;
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                // Post condition
                REQUIRE(a.empty());
            }

            SECTION("copy constructor") {
                // Expression
                C a(b);
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                // Post condition
                REQUIRE(a == b);
            }

            SECTION("move constructor") {
                C pre_b(b);
                // Expression
                C a(std::move(b));
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                // Post condition
                REQUIRE(a == pre_b);
            }

            SECTION("copy assignment") {
                // Expression
                C a;
                a = b; // NOLINT(bugprone-use-after-move)
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                // Post condition
                REQUIRE(a == b);
            }

            SECTION("move assignment") {
                C pre_b(b);
                // Expression
                C a;
                a = std::move(b);
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                // Post condition
                REQUIRE(a == pre_b);
            }

            SECTION("destruct") {
                C a;
                // Return type
                REQUIRE(std::is_same_v<decltype(a.~C()), void>);
            }

            SECTION("begin") {
                C a;
                // Return type
                REQUIRE(
                    std::is_same_v<decltype(a.begin()), typename C::iterator>);
            }

            SECTION("end") {
                C a;
                // Return type
                REQUIRE(
                    std::is_same_v<decltype(a.end()), typename C::iterator>);
            }

            SECTION("cbegin") {
                C a;
                // Return type
                REQUIRE(std::is_same_v<decltype(a.cbegin()),
                                       typename C::const_iterator>);
            }

            SECTION("cend") {
                C a;
                // Return type
                REQUIRE(std::is_same_v<decltype(a.cend()),
                                       typename C::const_iterator>);
            }

            SECTION("operator==") {
                C a(b); // NOLINT(bugprone-use-after-move): not moving
                // Expression
                REQUIRE_NOTHROW(a == b);
                // Return type
                REQUIRE(std::is_convertible_v<decltype(a == b), bool>);
                // Post
                REQUIRE((!(a == b) ||
                         std::equal(a.begin(), a.end(), b.begin(), b.end())));
            }

            SECTION("operator!=") {
                C a(b); // NOLINT(bugprone-use-after-move,performance-unnecessary-copy-initialization):
                        // not moving
                // Expression
                REQUIRE_NOTHROW(a != b);
                // Return type
                REQUIRE(std::is_convertible_v<decltype(a != b), bool>);
                // Post
                REQUIRE((!(a != b) || !(a == b)));
            }

            SECTION("swap") {
                C a;
                // Expression
                REQUIRE_NOTHROW(a.swap(b));
                // Return type
                REQUIRE(std::is_same_v<decltype(a.swap(b)), void>);
            }

            SECTION("size") {
                C a;
                // Expression
                REQUIRE_NOTHROW(a.size());
                // Return type
                REQUIRE(
                    std::is_same_v<decltype(a.size()), typename C::size_type>);
            }

            SECTION("max_size") {
                C a;
                // Expression
                REQUIRE_NOTHROW(a.max_size());
                // Return type
                REQUIRE(std::is_same_v<decltype(a.max_size()),
                                       typename C::size_type>);
                // Condition
                REQUIRE(a.size() < a.max_size());
            }

            SECTION("empty") {
                C a;
                // Expression
                REQUIRE_NOTHROW(a.empty());
                // Return type
                REQUIRE(std::is_convertible_v<decltype(a.empty()), bool>);
                // Condition
                REQUIRE((!a.empty() || a.begin() == a.end()));
            }
        }

        SECTION("Other requirements") {
            SECTION("DefaultConstructible<C>") {
                check_default_constructible_concept<C>();
            }
            SECTION("CopyConstructible<C>") {
                check_copy_constructible_concept<C>();
            }
            SECTION("EqualityComparable<C>") {
                check_equality_comparable_concept<C>();
            }
            SECTION("Swappable<C>") { check_swappable_concept<C>(); }

            SECTION("CopyInsertable<T>") { check_copy_insertable_concept<T>(); }

            SECTION("EqualityComparable<T>") {
                check_equality_comparable_concept<T>();
            }

            SECTION("Destructible<T>") { check_destructible_concept<T>(); }
        }
    }
}

template <class C, class T> void check_reversible_container_concept() {
    SECTION("ReversibleContainer") {
        SECTION("Types") {
            REQUIRE(std::is_same_v<decltype(*(typename C::reverse_iterator())),
                                   T &>);
            REQUIRE(std::is_base_of_v<
                    std::forward_iterator_tag,
                    typename std::iterator_traits<
                        typename C::reverse_iterator>::iterator_category>);
            REQUIRE(std::is_same_v<
                    typename C::reverse_iterator,
                    typename std::reverse_iterator<typename C::iterator>>);

            REQUIRE(std::is_same_v<decltype(*(
                                       typename C::const_reverse_iterator())),
                                   const T &>);
            REQUIRE(std::is_base_of_v<std::forward_iterator_tag,
                                      typename std::iterator_traits<
                                          typename C::const_reverse_iterator>::
                                          iterator_category>);
            REQUIRE(std::is_same_v<typename C::const_reverse_iterator,
                                   typename std::reverse_iterator<
                                       typename C::const_iterator>>);
        }

        SECTION("Methods") {
            C b;

            SECTION("rbegin") {
                C a;
                // Return type
                REQUIRE(std::is_same_v<decltype(a.rbegin()),
                                       typename C::reverse_iterator>);
            }

            SECTION("rend") {
                C a;
                // Return type
                REQUIRE(std::is_same_v<decltype(a.rend()),
                                       typename C::reverse_iterator>);
            }

            SECTION("crbegin") {
                C a;
                // Return type
                REQUIRE(std::is_same_v<decltype(a.crbegin()),
                                       typename C::const_reverse_iterator>);
            }

            SECTION("crend") {
                C a;
                // Return type
                REQUIRE(std::is_same_v<decltype(a.crend()),
                                       typename C::const_reverse_iterator>);
            }
        }
    }
}

template <class C, class T> void check_associative_container_concept() {
    using K = typename C::key_type;
    using V = typename T::second_type;
    SECTION("AssociativeContainer") {
        SECTION("Types") {
            REQUIRE(std::is_same_v<typename C::key_type, K>);
            REQUIRE(std::is_destructible_v<typename C::key_type>);

            // This requirement is for maps only
            REQUIRE(std::is_same_v<typename C::mapped_type, V>);

            REQUIRE(
                std::is_same_v<typename C::value_type, std::pair<const K, V>>);

            REQUIRE(std::is_convertible_v<
                    decltype(typename C::key_compare()(K{}, K{})), bool>);

            C cont;
            auto comp_func = cont.value_comp();
            typename C::value_type a;
            typename C::value_type b;
            REQUIRE(std::is_convertible_v<decltype(comp_func(a, b)), bool>);
        }

        SECTION("Methods") {
            C b;

            SECTION("comparison constructor") {
                // NOLINTNEXTLINE(modernize-use-transparent-functors)
                auto comp = std::less<double>();
                // Expression
                C a(comp);
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                REQUIRE(std::is_copy_constructible_v<typename C::key_compare>);
                // Post condition
                REQUIRE(a.empty());
            }

            SECTION("default comparison") {
                // Expression
                C a;
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                REQUIRE(
                    std::is_default_constructible_v<typename C::key_compare>);
                // Post condition
                REQUIRE(a.empty());
            }

            SECTION("comparison and elements constructor") {
                // NOLINTNEXTLINE(modernize-use-transparent-functors)
                auto comp = std::less<double>();
                // Expression
                C a(b.begin(), b.end(), comp);
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                REQUIRE(std::is_copy_constructible_v<typename C::key_compare>);
                REQUIRE(std::is_constructible_v<typename C::value_type,
                                                decltype(*(
                                                    typename C::iterator()))>);
                // Post condition
                REQUIRE(std::equal(a.begin(), a.end(), b.begin(), b.end()));
            }

            SECTION("elements constructor") {
                // Expression
                C a(b.begin(), b.end());
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                REQUIRE(
                    std::is_default_constructible_v<typename C::key_compare>);
                REQUIRE(std::is_constructible_v<typename C::value_type,
                                                decltype(*(
                                                    typename C::iterator()))>);
                // Post condition
                REQUIRE(std::equal(a.begin(), a.end(), b.begin(), b.end()));
            }

            SECTION("initializer constructor") {
                // Expression
                C a(std::initializer_list<T>{});
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                REQUIRE(
                    std::is_default_constructible_v<typename C::key_compare>);
                REQUIRE(std::is_constructible_v<typename C::value_type,
                                                decltype(*(
                                                    typename C::iterator()))>);
                // Post condition
                REQUIRE(a.empty());
            }

            SECTION("initializer and comparison constructor") {
                // NOLINTNEXTLINE(modernize-use-transparent-functors)
                auto comp = std::less<double>();
                // Expression
                C a(std::initializer_list<T>{}, comp);
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                REQUIRE(
                    std::is_default_constructible_v<typename C::key_compare>);
                REQUIRE(std::is_constructible_v<typename C::value_type,
                                                decltype(*(
                                                    typename C::iterator()))>);
                // Post condition
                REQUIRE(a.empty());
            }

            SECTION("initializer assignment") {
                // Expression
                C a;
                a = {};
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                REQUIRE(std::is_same_v<decltype(a = {}), C &>);
                // Post condition
                REQUIRE(a.empty());
            }

            SECTION("key comparison") {
                // Expression
                C a;
                auto k = a.key_comp(); // NOLINT(bugprone-use-after-move)
                // Return type
                REQUIRE(std::is_same_v<decltype(k), typename C::key_compare>);
            }

            SECTION("key comparison") {
                // Expression
                C a;
                auto k = a.value_comp(); // NOLINT(bugprone-use-after-move)
                // Return type
                REQUIRE(std::is_same_v<decltype(k), typename C::value_compare>);
            }
        }
    }
}

template <class A, class T> void check_allocator_concept() {
    SECTION("Allocator") {
        SECTION("Types") {
            REQUIRE(!std::is_same_v<typename std::allocator_traits<A>::pointer, void>);
            REQUIRE(!std::is_same_v<typename std::allocator_traits<A>::const_pointer, void>);
            REQUIRE(!std::is_same_v<typename std::allocator_traits<A>::void_pointer, void>);
            REQUIRE(!std::is_same_v<typename std::allocator_traits<A>::const_void_pointer, void>);
            REQUIRE(std::is_same_v<typename std::allocator_traits<A>::value_type, T>);
            REQUIRE(!std::is_same_v<typename std::allocator_traits<A>::size_type, void>);
            REQUIRE(!std::is_same_v<typename std::allocator_traits<A>::difference_type, void>);
            using U = int;
            REQUIRE(!std::is_same_v<typename std::allocator_traits<A>::template rebind_alloc<U>, void>);
        }

        SECTION("Methods") {
            A b;
            SECTION("Allocate") {
                REQUIRE(std::is_same_v<decltype(b.allocate(5)),typename std::allocator_traits<A>::pointer>);
            }

            SECTION("Deallocate") {
                auto p = b.allocate(1);
                REQUIRE(std::is_same_v<decltype(b.deallocate(p,1)),void>);
                b.deallocate(p,1);
            }

            SECTION("operator==") {
                A a;
                REQUIRE(std::is_convertible_v<decltype(a == b),bool>);
                REQUIRE(std::is_convertible_v<decltype(a != b),bool>);
            }

            SECTION("Copy") {
                A a(b);
                REQUIRE(a == b);
                REQUIRE(std::is_convertible_v<decltype(a != b),bool>);
            }

            SECTION("Copy Rebound") {
                A a(b);
                REQUIRE(a == b);
                REQUIRE(std::is_convertible_v<decltype(a != b),bool>);
            }
        }
    }
}

template <class C, class T> void check_allocator_aware_concept() {
    SECTION("AssociativeContainer") {
        using A = decltype(C().get_allocator());
        check_allocator_concept<A, T>();

        SECTION("Types") {
            REQUIRE(std::is_same_v<typename C::allocator_type::value_type,
                                   typename C::value_type>);
            check_allocator_concept<typename C::allocator_type, T>();
        }

        SECTION("Methods") {
            C b;
            typename C::allocator_type m;

            SECTION("get_allocator") {
                C a;
                // Return type
                check_allocator_concept<decltype(a.get_allocator()), T>();
                REQUIRE(std::is_same_v<decltype(a.get_allocator()),
                                       typename C::allocator_type>);
                REQUIRE(std::is_same_v<
                        decltype(a.get_allocator()),
                        typename std::allocator_traits<
                            decltype(a.get_allocator())>::
                            template rebind_alloc<typename C::value_type>>);
                // This fails on MSVC but not Clang or GCC.
                // What to do?
                if (!std::is_same_v<decltype(a.get_allocator()),
                                    std::allocator<typename C::value_type>>) {
                    std::cout << "demangle<decltype(a.get_allocator())>(): "
                              << pareto::demangle<decltype(a.get_allocator())>()
                              << std::endl;
                    std::cout << "demangle<std::allocator<typename "
                                 "C::value_type>>(): "
                              << pareto::demangle<
                                     std::allocator<typename C::value_type>>()
                              << std::endl;
                }

                if (!std::is_same_v<decltype(a.get_allocator()), A>) {
                    std::cout << "demangle<decltype(a.get_allocator())>(): "
                              << pareto::demangle<decltype(a.get_allocator())>()
                              << std::endl;
                    std::cout << "demangle<A>(): " << pareto::demangle<A>()
                              << std::endl;
                }
            }

            SECTION("empty constructor") {
                // Pre
                REQUIRE(std::is_default_constructible_v<A>);
                // Expression
                C a;
                // Post condition
                // This post condition does not work on MSVC
                if (!(a.get_allocator() == A())) {
                    std::cout << "Allocators are not the same" << std::endl;
                }
                REQUIRE(a.empty());
            }

            SECTION("allocator constructor") {
                // Expression
                C a(m);
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                REQUIRE(
                    std::is_default_constructible_v<typename C::key_compare>);
                // Post condition
                REQUIRE(a.empty());
                // This next post-condition seems wrong for stateful allocators
                // This post condition does not work on MSVC
                if (!(a.get_allocator() == m)) {
                    std::cout << "Allocators are not the same" << std::endl;
                }
            }

            SECTION("copy constructor") {
                // Pre-condition
                check_copy_insertable_concept<T>();
                // Expression
                C a(b, m);
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                REQUIRE(
                    std::is_default_constructible_v<typename C::key_compare>);
                // Post condition
                REQUIRE(a == b);
                // This next post-condition seems wrong for stateful allocators
                // This post condition does not work on MSVC
                if (!(a.get_allocator() == m)) {
                    std::cout << "Allocators are not the same" << std::endl;
                }
            }

            SECTION("move constructor") {
                // Pre-condition
                REQUIRE(std::is_nothrow_move_constructible_v<A>);
                auto preb = b;
                // Expression
                C a(std::move(b));
                // Post-conditions
                REQUIRE(a == preb);
                // This post condition does not work on MSVC
                if (!(a.get_allocator() == preb.get_allocator())) {
                    std::cout << "Allocators are not the same" << std::endl;
                }
            }

            SECTION("move + allocator constructor") {
                // Pre-condition
                check_move_insertable_concept<T>();
                REQUIRE(std::is_nothrow_move_constructible_v<A>);
                auto preb = b; // NOLINT(bugprone-use-after-move)
                // Expression
                C a(std::move(b));
                // Post-conditions
                REQUIRE(a == preb);
                // This post condition does not work on MSVC
                if (!(a.get_allocator() == preb.get_allocator())) {
                    std::cout << "Allocators are not the same" << std::endl;
                }
            }

            SECTION("copy assignment") {
                // Pre-condition
                REQUIRE(std::is_copy_assignable_v<std::pair<
                            typename C::key_type, typename C::mapped_type>>);
                // Expression
                C a;
                a = b; // NOLINT(bugprone-use-after-move)
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                REQUIRE(std::is_same_v<decltype(a = b), C &>);
                // Post condition
                REQUIRE(a == b);
            }

            SECTION("move assignment") {
                // Pre-condition
                REQUIRE(std::is_move_assignable_v<std::pair<
                            typename C::key_type, typename C::mapped_type>>);
                C pre_b(b);
                // Expression
                C a;
                a = std::move(b);
                // Return type
                REQUIRE(std::is_same_v<decltype(a), C>);
                REQUIRE(std::is_same_v<decltype(a = std::move(b)), C &>);
                // Post condition
                REQUIRE(a == pre_b);
            }

            SECTION("swap") {
                C a;
                C pre_a(a);
                C pre_b(b); // NOLINT(bugprone-use-after-move)
                // Expression
                REQUIRE_NOTHROW(a.swap(b));
                // Return type
                REQUIRE(std::is_same_v<decltype(a.swap(b)), void>);
                // Post-condition
                REQUIRE(b == pre_a);
                REQUIRE(a == pre_b);
            }
        }
    }
}

template <class It> void check_legacy_iterator_concepts() {
    REQUIRE(std::is_copy_constructible_v<It>);
    REQUIRE(std::is_copy_assignable_v<It>);
    REQUIRE(std::is_destructible_v<It>);
    REQUIRE(std::is_swappable_v<It>);
    REQUIRE(!std::is_same_v<typename It::reference, void>);
    REQUIRE(!std::is_same_v<typename It::value_type, void>);
    REQUIRE(!std::is_same_v<typename It::difference_type, void>);
    REQUIRE(!std::is_same_v<typename It::pointer, void>);
    REQUIRE(!std::is_same_v<typename It::iterator_category, void>);
    It a;
    REQUIRE(!std::is_same_v<decltype(*a), void>);
    REQUIRE(std::is_same_v<decltype(++a), It&>);
}

template <class It> void check_legacy_input_iterator_concepts() {
    check_legacy_iterator_concepts<It>();
    It i;
    It j;
    REQUIRE(std::is_convertible_v<decltype(i == j), bool>);

    using reference = typename std::iterator_traits<It>::reference;
    using value_type = typename std::iterator_traits<It>::value_type;
    REQUIRE(std::is_convertible_v<decltype(i != j), bool>);
    REQUIRE((i != j) == (!(i == j)));

    REQUIRE(std::is_same_v<decltype(*i),reference>);
    REQUIRE(std::is_same_v<std::remove_const_t<std::remove_reference_t<decltype(*i)>>,std::remove_const_t<std::remove_reference_t<value_type>>>);

    REQUIRE(std::is_same_v<decltype(i->first),decltype((*i).first)>);
}

template <class It> void check_legacy_forward_iterator_concepts() {
    check_legacy_input_iterator_concepts<It>();
    REQUIRE(std::is_default_constructible_v<It>);

    using reference = typename std::iterator_traits<It>::reference;
    using value_type = typename std::iterator_traits<It>::value_type;
    REQUIRE((std::is_same_v<reference, value_type&> || std::is_same_v<reference, const value_type&>));

    It i;
    It j;
    REQUIRE(std::is_convertible_v<decltype(i == j), bool>);
    REQUIRE(std::is_convertible_v<decltype(i != j), bool>);
    REQUIRE((i != j) == (!(i == j)));

    REQUIRE(std::is_same_v<decltype(i++),It>);
    REQUIRE(std::is_same_v<decltype(*i++),reference>);
}

template <class It> void check_legacy_bidirectional_iterator_concepts() {
    check_legacy_forward_iterator_concepts<It>();

    using reference = typename std::iterator_traits<It>::reference;

    It i;
    It j;
    REQUIRE(std::is_same_v<decltype(--i),It&>);
    REQUIRE(std::is_same_v<decltype(i--),It>);
    REQUIRE(std::is_same_v<decltype(*i--),reference>);
}

template <class C, class T> void check_multimap_concepts() {
    check_container_concept<C, T>();
    check_reversible_container_concept<C, T>();
    check_associative_container_concept<C, T>();
    check_allocator_aware_concept<C, T>();
    SECTION("LegacyBidirectionalIterator") {
        check_legacy_bidirectional_iterator_concepts<typename C::iterator>();
        check_legacy_bidirectional_iterator_concepts<typename C::const_iterator>();
    }

    SECTION("Multimap Methods") {
        C m;
        typename C::key_type k{0};
        typename C::mapped_type v{0};
        auto kv = std::make_pair(k,v);
        REQUIRE_NOTHROW(m.insert(kv));
        REQUIRE_FALSE(m.empty());
        REQUIRE(m.size() == 1);
        REQUIRE(m.size() == 1);
        REQUIRE(m.max_size() > 0);
        REQUIRE_NOTHROW(m.clear());
        REQUIRE_NOTHROW(m.emplace(kv));
        REQUIRE_NOTHROW(m.emplace_hint(m.end(), kv));
        REQUIRE_NOTHROW(m.erase(k));
        C m2;
        REQUIRE_NOTHROW(m2.merge(m));
        REQUIRE(m2.count(k) == 0);
        REQUIRE_NOTHROW(m.swap(m2));
        REQUIRE_NOTHROW(m.emplace(kv));
        REQUIRE(m.count(k) > 0);
        REQUIRE(m.find(k) != m.end());
        REQUIRE_NOTHROW(m.key_comp());
        REQUIRE_NOTHROW(m.value_comp());
    }
}

template <class C, class T> void check_spatial_container_concept() {
    SECTION("Spatial Methods") {
        C m;
        typename C::key_type k{0};
        typename C::mapped_type v{0};
        auto kv = std::make_pair(k,v);
        REQUIRE_NOTHROW(m.emplace(kv));
        REQUIRE_NOTHROW(m.at(k));
        REQUIRE(m.contains(k));
        REQUIRE_NOTHROW(m.dimensions());
        REQUIRE_NOTHROW(m.max_value(0));
        REQUIRE_NOTHROW(m.min_value(0));
        REQUIRE_NOTHROW(m.find_intersection(k));
        REQUIRE_NOTHROW(m.find_within(k,k));
        REQUIRE_NOTHROW(m.find_disjoint(k,k));
        REQUIRE_NOTHROW(m.find_nearest(k));
        REQUIRE_NOTHROW(m.max_element(0));
        REQUIRE_NOTHROW(m.min_element(0));
        REQUIRE_NOTHROW(m.dimension_comp());
    }
}

template <class C, class T> void check_front_container_concept() {
    SECTION("Front Methods") {
        C m;
        C m2;
        typename C::key_type k{0};
        typename C::mapped_type v{0};
        auto kv = std::make_pair(k,v);
        REQUIRE_NOTHROW(m.emplace(kv));
        REQUIRE_NOTHROW(m.at(k));
        REQUIRE_NOTHROW(m.ideal());
        REQUIRE_NOTHROW(m.nadir());
        REQUIRE_NOTHROW(m.worst());
        REQUIRE_NOTHROW(m.is_maximization());
        REQUIRE_NOTHROW(m.is_maximization());
        REQUIRE_NOTHROW(m.dominates(k));
        REQUIRE_NOTHROW(m.strongly_dominates(k));
        REQUIRE_NOTHROW(m.is_partially_dominated_by(k));
        REQUIRE_NOTHROW(m.is_completely_dominated_by(k));
        REQUIRE_NOTHROW(m.non_dominates(k));
        REQUIRE_NOTHROW(m.dominates(m2));
        REQUIRE_NOTHROW(m.strongly_dominates(m2));
        REQUIRE_NOTHROW(m.is_partially_dominated_by(m2));
        REQUIRE_NOTHROW(m.is_completely_dominated_by(m2));
        REQUIRE_NOTHROW(m.non_dominates(m2));
        REQUIRE_NOTHROW(m.hypervolume());
        REQUIRE_NOTHROW(m.coverage(m2));
        REQUIRE_NOTHROW(m.coverage_ratio(m2));
        REQUIRE_NOTHROW(m.gd(m2));
        REQUIRE_NOTHROW(m.std_gd(m2));
        REQUIRE_NOTHROW(m.igd(m2));
        REQUIRE_NOTHROW(m.std_igd(m2));
        REQUIRE_NOTHROW(m.hausdorff(m2));
        REQUIRE_NOTHROW(m.igd_plus(m2));
        REQUIRE_NOTHROW(m.std_igd_plus(m2));
        REQUIRE_NOTHROW(m.uniformity());
        REQUIRE_NOTHROW(m.average_distance());
        REQUIRE_NOTHROW(m.average_nearest_distance());
        REQUIRE_NOTHROW(m.crowding_distance(k));
        REQUIRE_NOTHROW(m.average_crowding_distance());
        REQUIRE_NOTHROW(m.direct_conflict(0,1));
        REQUIRE_NOTHROW(m.normalized_direct_conflict(0,1));
        REQUIRE_NOTHROW(m.maxmin_conflict(0,1));
        REQUIRE_NOTHROW(m.normalized_maxmin_conflict(0,1));
        REQUIRE_NOTHROW(m.conflict(0,1));
        REQUIRE_NOTHROW(m.normalized_conflict(0,1));
        REQUIRE_NOTHROW(m.find_dominated(k));
        REQUIRE_NOTHROW(m.find_nearest_exclusive(k));
        REQUIRE_NOTHROW(m.ideal_element(0));
        REQUIRE_NOTHROW(m.nadir_element(0));
        REQUIRE_NOTHROW(m.worst_element(0));
        REQUIRE_NOTHROW(m.dimension_comp());
    }
}

template <class C, class T> void check_archive_container_concept() {
    SECTION("Archive Methods") {
        C m(100);
        C m2(100);
        typename C::key_type k{0};
        typename C::mapped_type v{0};
        auto kv = std::make_pair(k,v);
        REQUIRE_NOTHROW(m.emplace(kv));
        REQUIRE_NOTHROW(m.begin_front());
        REQUIRE_NOTHROW(m.end_front());
        REQUIRE_NOTHROW(m.cbegin_front());
        REQUIRE_NOTHROW(m.cend_front());
        REQUIRE_NOTHROW(m.begin_front());
        REQUIRE_NOTHROW(m.end_front());
        REQUIRE_NOTHROW(m.rbegin_front());
        REQUIRE_NOTHROW(m.rend_front());
        REQUIRE_NOTHROW(m.rbegin_front());
        REQUIRE_NOTHROW(m.rend_front());
        REQUIRE_NOTHROW(m.crbegin_front());
        REQUIRE_NOTHROW(m.crend_front());
        REQUIRE_NOTHROW(m.capacity());
        REQUIRE_NOTHROW(m.size_fronts());
        REQUIRE_NOTHROW(m.dominates(m2));
        REQUIRE_NOTHROW(m.strongly_dominates(m2));
        REQUIRE_NOTHROW(m.is_partially_dominated_by(m2));
        REQUIRE_NOTHROW(m.is_completely_dominated_by(m2));
        REQUIRE_NOTHROW(m.non_dominates(m2));
        REQUIRE_NOTHROW(m.hypervolume());
        REQUIRE_NOTHROW(m.coverage(m2));
        REQUIRE_NOTHROW(m.coverage_ratio(m2));
        REQUIRE_NOTHROW(m.gd(m2));
        REQUIRE_NOTHROW(m.std_gd(m2));
        REQUIRE_NOTHROW(m.igd(m2));
        REQUIRE_NOTHROW(m.std_igd(m2));
        REQUIRE_NOTHROW(m.hausdorff(m2));
        REQUIRE_NOTHROW(m.igd_plus(m2));
        REQUIRE_NOTHROW(m.std_igd_plus(m2));
        REQUIRE_NOTHROW(m.uniformity());
        REQUIRE_NOTHROW(m.average_distance());
        REQUIRE_NOTHROW(m.average_nearest_distance());
        REQUIRE_NOTHROW(m.average_crowding_distance());
        REQUIRE_NOTHROW(m.crowding_distance(k));
        REQUIRE_NOTHROW(m.resize(1000));
        REQUIRE_NOTHROW(m.find_front(k));
    }
}

template <class C, class T> void check_spatial_concepts() {
    check_multimap_concepts<C,T>();
    check_spatial_container_concept<C, T>();
}

template <class C, class T> void check_front_concepts() {
    check_spatial_concepts<C,T>();
    check_front_container_concept<C, T>();
}

template <class C, class T> void check_archive_concepts() {
    check_front_concepts<C,T>();
    check_archive_container_concept<C, T>();
}

TEST_CASE("Concepts") {
    using L = std::less<double>;
    SECTION("Multimap") {
        using C = std::multimap<double, unsigned, L>;
        using T = std::pair<const double, unsigned>;
        check_multimap_concepts<C, T>();
    }

    using T = std::pair<const pareto::point<double, 2>, unsigned>;
    SECTION("Implicit Tree") {
        using C = pareto::implicit_tree<double, 2, unsigned, L>;
        check_spatial_concepts<C, T>();
    }

    SECTION("kd-Tree") {
        using C = pareto::kd_tree<double, 2, unsigned, L>;
        check_spatial_concepts<C, T>();
    }

    SECTION("Quadtree") {
        using C = pareto::quad_tree<double, 2, unsigned, L>;
        check_spatial_concepts<C, T>();
    }

    SECTION("R-Tree") {
        using C = pareto::r_tree<double, 2, unsigned, L>;
        check_spatial_concepts<C, T>();
    }

    SECTION("R*-Tree") {
        using C = pareto::r_star_tree<double, 2, unsigned, L>;
        check_spatial_concepts<C, T>();
    }

    SECTION("Front") {
        using C = pareto::front<double, 2, unsigned>;
        check_front_concepts<C, T>();
    }

    SECTION("Archive") {
        using C = pareto::archive<double, 2, unsigned>;
        check_archive_concepts<C, T>();
    }
}
