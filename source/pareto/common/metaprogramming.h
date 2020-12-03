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



}

#endif //PARETO_METAPROGRAMMING_H
