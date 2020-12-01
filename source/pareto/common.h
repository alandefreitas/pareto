//
// Created by Alan Freitas on 2020-06-04.
//

#ifndef PARETO_FRONT_COMMON_H
#define PARETO_FRONT_COMMON_H

#include <vector>
#include <array>

namespace pareto {

    template <typename T>
    void maybe_resize(T& v, size_t n);

    template <typename number_t>
    void maybe_resize(std::vector<number_t>& v, size_t n) {
        v.resize(n);
    }

    template <typename number_t, size_t compile_dimensions>
    void maybe_resize(std::array<number_t,compile_dimensions>& v [[maybe_unused]], size_t n [[maybe_unused]]) {}

    template <typename T, typename T2>
    void maybe_push_back(T& v, const T2& n);

    template <typename number_t>
    void maybe_push_back(std::vector<number_t>& v, const number_t& n) {
        v.push_back(n);
    }

    template <typename number_t, size_t compile_dimensions>
    void maybe_push_back(std::array<number_t,compile_dimensions>& v [[maybe_unused]], const number_t& n [[maybe_unused]]) {}

    template <typename T, typename T2>
    void maybe_push_back(T& v, T2&& n);

    template <typename number_t>
    void maybe_push_back(std::vector<number_t>& v, number_t&& n) {
        v.push_back(n);
    }

    template <typename number_t, size_t compile_dimensions>
    void maybe_push_back(std::array<number_t,compile_dimensions>& v [[maybe_unused]], number_t&& n [[maybe_unused]]) {}

    template <typename T>
    void maybe_clear(T& v);

    template <typename number_t>
    void maybe_clear(std::vector<number_t>& v) {
        v.clear();
    }

    template <typename number_t, size_t compile_dimensions>
    void maybe_clear(std::array<number_t,compile_dimensions>& v [[maybe_unused]]) {}

    inline std::vector<uint8_t> init_list_to_vector(std::initializer_list<bool> is_minimization) {
        std::vector<uint8_t> v(is_minimization.begin(), is_minimization.end());
        return v;
    }

    template <class MAPPED_TYPE>
    bool mapped_type_custom_equality_operator(const MAPPED_TYPE& m1, const MAPPED_TYPE& m2) {
        return m1 == m2;
    }

}

#endif //PARETO_FRONT_COMMON_H
