//
// Created by Alan Freitas on 12/3/20.
//

#ifndef PARETO_METAPROGRAMMING_H
#define PARETO_METAPROGRAMMING_H

#include <cstddef>

namespace pareto {

    /// \brief Get size of a pack
    template <typename... Targs>
    constexpr size_t pack_size(const Targs &...) {
        return sizeof...(Targs);
    }

    /// \brief Copy values of a pack of size 1 to the point components
    template<typename Iterator, typename value_type, typename... Targs>
    inline void copy_pack(Iterator begin, const value_type &k) {
        *begin = k;
    }

    /// \brief Copy values of a pack of size 2+ to the point components
    template<typename Iterator, typename value_type, typename... Targs>
    inline void copy_pack(Iterator begin, const value_type &k, const Targs &... ks) {
        *begin = k;
        ++begin;
        copy_pack(begin, ks...);
    }

    /// \brief Resize if vector, not resize if array
    template <typename T>
    void maybe_resize(T& v, size_t n);

    /// \brief Resize if vector, not resize if array
    template <typename number_t>
    void maybe_resize(std::vector<number_t>& v, size_t n) {
        v.resize(n);
    }

    /// \brief Resize if vector, not resize if array
    template <typename number_t, size_t compile_dimensions>
    void maybe_resize(std::array<number_t,compile_dimensions>& v [[maybe_unused]], size_t n [[maybe_unused]]) {}

    /// \brief Push back if vector, not push back if array
    template <typename T, typename T2>
    void maybe_push_back(T& v, const T2& n);

    /// \brief Push back if vector, not push back if array
    template <typename number_t>
    void maybe_push_back(std::vector<number_t>& v, const number_t& n) {
        v.push_back(n);
    }

    /// \brief Push back if vector, not push back if array
    template <typename number_t, size_t compile_dimensions>
    void maybe_push_back(std::array<number_t,compile_dimensions>& v [[maybe_unused]], const number_t& n [[maybe_unused]]) {}

    /// \brief Push back (move back) if vector, not push back if array
    template <typename T, typename T2>
    void maybe_push_back(T& v, T2&& n);

    /// \brief Push back (move back) if vector, not push back if array
    template <typename number_t>
    void maybe_push_back(std::vector<number_t>& v, number_t&& n) {
        v.push_back(n);
    }

    /// \brief Push back (move back) if vector, not push back if array
    template <typename number_t, size_t compile_dimensions>
    void maybe_push_back(std::array<number_t,compile_dimensions>& v [[maybe_unused]], number_t&& n [[maybe_unused]]) {}

    /// \brief Clear if vector, not clear if array
    template <typename T>
    void maybe_clear(T& v);

    /// \brief Clear if vector, not clear if array
    template <typename number_t>
    void maybe_clear(std::vector<number_t>& v) {
        v.clear();
    }

    /// \brief Clear if vector, not clear if array
    template <typename number_t, size_t compile_dimensions>
    void maybe_clear(std::array<number_t,compile_dimensions>& v [[maybe_unused]]) {}

    template <class T1, class T2>
    std::pair<std::add_const_t<T1>, T2>& protect_pair_key(std::pair<T1, T2>& r) {
        std::pair<T1, T2> *p = &r;
        using protected_type = std::pair<std::add_const_t<T1>, T2>;
        auto *cp = reinterpret_cast<protected_type *>(p);
        protected_type &cr = *cp;
        return cr;
    }

    template <class T1, class T2>
    const std::pair<std::add_const_t<T1>, T2>& protect_pair_key(const std::pair<T1, T2>& r) {
        const std::pair<T1, T2> *p = &r;
        using protected_type = const std::pair<std::add_const_t<T1>, T2>;
        auto *cp = reinterpret_cast<protected_type *>(p);
        protected_type &cr = *cp;
        return cr;
    }


    template <class T1, class T2>
    std::pair<std::remove_const_t<T1>, T2>& unprotect_pair_key(std::pair<T1, T2>& r) {
        std::pair<T1, T2> *p = &r;
        using unprotected_type = std::pair<std::remove_const_t<T1>, T2>;
        auto *cp = reinterpret_cast<unprotected_type *>(p);
        unprotected_type &cr = *cp;
        return cr;
    }

    template <class T1, class T2>
    const std::pair<std::remove_const_t<T1>, T2>& unprotect_pair_key(const std::pair<T1, T2>& r) {
        const std::pair<T1, T2> *p = &r;
        using unprotected_type = const std::pair<std::remove_const_t<T1>, T2>;
        auto *cp = reinterpret_cast<unprotected_type *>(p);
        unprotected_type &cr = *cp;
        return cr;
    }

    template <class T1>
    std::remove_const_t<T1>& unconst_reference(T1& r) {
        return const_cast<std::remove_const_t<T1>&>(r);
    }

    template <class T1>
    std::remove_const_t<T1>& unconst_reference(const T1& r) {
        return const_cast<std::remove_const_t<T1>&>(r);
    }

}

#endif //PARETO_METAPROGRAMMING_H
