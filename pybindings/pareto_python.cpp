#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
namespace py = pybind11;

#include <pareto/archive.h>
#include <pareto/common/demangle.h>
#include <pareto/front.h>
#include <pareto/implicit_tree.h>
#include <pareto/kd_tree.h>
#include <pareto/quad_tree.h>
#include <pareto/r_tree.h>
#include <pareto/r_star_tree.h>
#include <pareto/spatial_map.h>

/// \class Defines a custom equality operator for py::object
/// The operator== is deprecated on pybind, so we can't use
/// it inside the tree containers for comparisons
namespace pareto {
    template <>
    bool mapped_type_custom_equality_operator<pybind11::object>(
        const pybind11::object &m1, const pybind11::object &m2) {
        return m1.is(m2);
    }
} // namespace pareto

/// Define the standard type we'll use in the bindings
using dimension_type = double;
using mapped_type = py::object;
using point_type = pareto::point<dimension_type, 0>;
using value_type = std::pair<const pareto::point<dimension_type, 0>, mapped_type>;

template <class def_t> void bind_point_concept(def_t &p) {
    // Constructors
    p.def(py::init<>());
    p.def(py::init<size_t>());
    p.def(py::init<std::initializer_list<dimension_type>>());
    p.def(py::init([](size_t dim, const dimension_type value) {
        return new point_type(dim, value);
    }));
    p.def(py::init<const point_type &>());
    p.def(py::init([](const std::vector<dimension_type> &v) {
        return new point_type(v.begin(), v.end());
    }));

    // Assignment
    p.def("assign",
          py::overload_cast<const point_type &>(&point_type::operator=));

    // Non-modifiers
    p.def("dimensions", &point_type::dimensions);
    p.def("size", &point_type::size);
    p.def("quadrant", &point_type::size);

    // Modifiers
    p.def("clear", &point_type::clear);
    p.def("push_back",
          py::overload_cast<dimension_type &&>(&point_type::push_back));
    p.def("push_back",
          py::overload_cast<const dimension_type &>(&point_type::push_back));

    // Dominance relationships
    p.def("dominates", [](const point_type &a, const point_type &b) {
        return a.dominates(b);
    });
    p.def("dominates",
          [](const point_type &a, const point_type &b, bool is_minimization) {
              return a.dominates(b, is_minimization);
          });
    p.def("dominates", [](const point_type &a, const point_type &b,
                          const std::vector<bool> &is_minimization) {
        return a.dominates(b, is_minimization);
    });
    p.def("dominates", [](const point_type &a, const point_type &b,
                          const std::vector<uint8_t> &is_minimization) {
        return a.dominates(b, is_minimization);
    });
    p.def("dominates", [](const point_type &a, const point_type &b,
                          const std::vector<int> &is_minimization) {
        return a.dominates(b, is_minimization);
    });

    // Check strong dominance
    p.def("strongly_dominates", [](const point_type &a, const point_type &b) {
        return a.strongly_dominates(b);
    });
    p.def("strongly_dominates",
          [](const point_type &a, const point_type &b, bool is_minimization) {
              return a.strongly_dominates(b, is_minimization);
          });
    p.def("strongly_dominates", [](const point_type &a, const point_type &b,
                                   const std::vector<bool> &is_minimization) {
        return a.strongly_dominates(b, is_minimization);
    });
    p.def("strongly_dominates",
          [](const point_type &a, const point_type &b,
             const std::vector<uint8_t> &is_minimization) {
              return a.strongly_dominates(b, is_minimization);
          });
    p.def("strongly_dominates", [](const point_type &a, const point_type &b,
                                   const std::vector<int> &is_minimization) {
        return a.strongly_dominates(b, is_minimization);
    });

    // Check strong dominance
    p.def("non_dominates", [](const point_type &a, const point_type &b) {
        return a.non_dominates(b);
    });
    p.def("non_dominates",
          [](const point_type &a, const point_type &b, bool is_minimization) {
              return a.non_dominates(b, is_minimization);
          });
    p.def("non_dominates", [](const point_type &a, const point_type &b,
                              const std::vector<bool> &is_minimization) {
        return a.non_dominates(b, is_minimization);
    });
    p.def("non_dominates", [](const point_type &a, const point_type &b,
                              const std::vector<uint8_t> &is_minimization) {
        return a.non_dominates(b, is_minimization);
    });
    p.def("non_dominates", [](const point_type &a, const point_type &b,
                              const std::vector<int> &is_minimization) {
        return a.non_dominates(b, is_minimization);
    });

    // Distance
    p.def("distance", [](const point_type &a, const point_type &b) {
        return a.distance(b);
    });

    // Values
    p.def("values", [](const point_type &a) { return a.values(); });

    // Quadrant
    p.def("quadrant", [](const point_type &a, const point_type &b) {
        return a.quadrant(b, std::less<>());
    });

    // Iterators
    p.def(
        "__iter__",
        [](const point_type &s) {
            return py::make_iterator(s.begin(), s.end());
        },
        py::keep_alive<0, 1>());

    p.def(
        "__iter__",
        [](point_type &s) { return py::make_iterator(s.begin(), s.end()); },
        py::keep_alive<0, 1>());

    // Operators
    // See
    // https://docs.python.org/3/reference/datamodel.html#special-method-names
    p.def("__eq__",
          [](const point_type &a, const point_type &b) { return a == b; });
    p.def("__ne__",
          [](const point_type &a, const point_type &b) { return a != b; });
    p.def("__add__",
          [](const point_type &a, const point_type &b) { return a + b; });
    p.def("__sub__",
          [](const point_type &a, const point_type &b) { return a - b; });
    p.def("__mul__",
          [](const point_type &a, const point_type &b) { return a * b; });
    p.def("__truediv__",
          [](const point_type &a, const point_type &b) { return a / b; });
    p.def("__iadd__", [](point_type &a, const point_type &b) { a += b; });
    p.def("__isub__", [](point_type &a, const point_type &b) { a -= b; });
    p.def("__imul__", [](point_type &a, const point_type &b) { a *= b; });
    p.def("__itruediv__", [](point_type &a, const point_type &b) { a /= b; });
    p.def("__add__",
          [](const point_type &a, const dimension_type &b) { return a + b; });
    p.def("__sub__",
          [](const point_type &a, const dimension_type &b) { return a - b; });
    p.def("__mul__",
          [](const point_type &a, const dimension_type &b) { return a * b; });
    p.def("__truediv__",
          [](const point_type &a, const dimension_type &b) { return a / b; });
    p.def("__iadd__", [](point_type &a, const dimension_type &b) { a += b; });
    p.def("__isub__", [](point_type &a, const dimension_type &b) { a -= b; });
    p.def("__imul__", [](point_type &a, const dimension_type &b) { a *= b; });
    p.def("__itruediv__", [](point_type &a, const dimension_type &b) { a /= b; });

    // operator[]
    p.def("__getitem__", [](const point_type &a, size_t n) { return a[n]; });
    p.def("__setitem__",
          [](point_type &p, size_t n, dimension_type v) { p[n] = v; });

    // Print the point
    p.def("__repr__", [](const point_type &a) {
        std::stringstream ss;
        ss << a;
        return ss.str();
    });
}

template <class C, class def_t> void bind_spatial_container_concept(def_t &p) {
    // Constructors
    p.def(py::init<>());
    p.def(py::init<const C &>());
    p.def(py::init([](const std::vector<value_type> &v) {
        return new C(v.begin(), v.end());
    }));
    p.def(py::init([](std::initializer_list<value_type> il) {
        return new C(il.begin(), il.end());
    }));

    // Assignment
    p.def("assign", py::overload_cast<const C &>(&C::operator=));
    p.def("assign", [](C &lhs, const std::vector<value_type> &il) {
        // this is an alternative to operator=(std::initializer_list)
        lhs.clear();
        lhs.insert(il.begin(), il.end());
    });

    // Element access
    p.def("at", py::overload_cast<const point_type &>(&C::at));
    p.def("at", py::overload_cast<const point_type &>(&C::at, py::const_));
    p.def("__getitem__", py::overload_cast<const point_type &>(&C::operator[]));
    p.def("__getitem__", [](C &lhs, std::vector<dimension_type> k) {
        return lhs[point_type(k.begin(), k.end())];
    });

    p.def("__setitem__", [](C &lhs, const point_type &k, const mapped_type &v) {
        lhs[k] = v;
    });
    p.def("__setitem__", [](C &lhs, std::vector<dimension_type> k, mapped_type v) {
        lhs[point_type(k.begin(), k.end())] = v;
    });

    // Iterators
    p.def(
        "__iter__",
        [](const C &s) { return py::make_iterator(s.cbegin(), s.cend()); },
        py::keep_alive<0, 1>());
    p.def(
        "__iter__", [](C &s) { return py::make_iterator(s.begin(), s.end()); },
        py::keep_alive<0, 1>());

    // Reverse iterators
    p.def(
        "__reversed__",
        [](const C &s) { return py::make_iterator(s.crbegin(), s.crend()); },
        py::keep_alive<0, 1>());
    p.def(
        "__reversed__",
        [](C &s) { return py::make_iterator(s.rbegin(), s.rend()); },
        py::keep_alive<0, 1>());

    // Non-modifiers / Multimap concept
    p.def("empty", &C::empty);
    p.def("size", &C::size);
    p.def("__len__", &C::size);
    p.def("max_size", &C::max_size);

    // Non-modifiers / Spatial concept
    p.def("dimensions", [](const C &lhs) { return lhs.dimensions(); });
    p.def("max_value", &C::max_value);
    p.def("min_value", &C::min_value);

    // Modifiers / Multimap concept
    p.def("swap", &C::swap);
    p.def("clear", &C::clear);
    p.def("insert", [](C &lhs, const value_type &v) { lhs.insert(v); });
    p.def("insert", [](C &lhs, const std::vector<value_type> &v) {
        lhs.insert(v.begin(), v.end());
    });
    p.def("emplace", [](C &lhs, const value_type &v) { lhs.emplace(v); });
    p.def("erase", [](C &lhs, const point_type &k) { return lhs.erase(k); });
    p.def("erase", [](C &lhs, const std::vector<dimension_type> &k) {
        return lhs.erase(point_type(k.begin(), k.end()));
    });
    p.def("__delitem__", [](C &lhs, const point_type &k) { return lhs.erase(k); });
    p.def("__delitem__", [](C &lhs, std::vector<dimension_type> k) {
      return lhs.erase(point_type(k.begin(), k.end()));
    });
    p.def("merge", [](C &lhs, C &rhs) { return lhs.merge(rhs); });

    // Lookup / Multimap concept
    p.def("count",
          [](const C &lhs, const point_type &k) { return lhs.count(k); });
    p.def(
        "find",
        [](const C &s, const point_type &k) {
            return py::make_iterator(s.find(k), s.cend());
        },
        py::keep_alive<0, 1>());
    p.def(
        "find",
        [](C &s, const point_type &k) {
            return py::make_iterator(s.find(k), s.end());
        },
        py::keep_alive<0, 1>());
    p.def("contains",
          [](const C &lhs, const point_type &k) { return lhs.contains(k); });
    p.def("__contains__",
          [](const C &lhs, const point_type &k) { return lhs.contains(k); });
    p.def("__contains__",
          [](const C &lhs, const std::vector<dimension_type> &k) { return lhs.contains(point_type(k.begin(), k.end())); });

    // Lookup / Spatial concept
    p.def(
        "find_intersection",
        [](const C &s, const point_type &k) {
            return py::make_iterator(s.find_intersection(k), s.cend());
        },
        py::keep_alive<0, 1>());
    p.def(
        "find_intersection",
        [](const C &s, const point_type &lb, const point_type &ub) {
            return py::make_iterator(s.find_intersection(lb, ub), s.cend());
        },
        py::keep_alive<0, 1>());
    p.def(
        "find_within",
        [](const C &s, const point_type &lb, const point_type &ub) {
            return py::make_iterator(s.find_within(lb, ub), s.cend());
        },
        py::keep_alive<0, 1>());
    p.def(
        "find_disjoint",
        [](const C &s, const point_type &lb, const point_type &ub) {
            return py::make_iterator(s.find_disjoint(lb, ub), s.cend());
        },
        py::keep_alive<0, 1>());
    p.def(
        "find_nearest",
        [](const C &s, const point_type &p) {
            return py::make_iterator(s.find_nearest(p), s.cend());
        },
        py::keep_alive<0, 1>());
    p.def(
        "find_nearest",
        [](const C &s, const point_type &p, size_t k) {
            return py::make_iterator(s.find_nearest(p, k), s.cend());
        },
        py::keep_alive<0, 1>());
    p.def("max_element", [](const C &lhs, size_t dimension) {
        return *lhs.max_element(dimension);
    });
    p.def("min_element", [](const C &lhs, size_t dimension) {
        return *lhs.min_element(dimension);
    });

    // Print the point
    p.def("__repr__", [](const C &a) {
        std::string class_name = pareto::demangle<C>();
        auto it = std::find(class_name.begin(), class_name.end(), '<');
        return std::string(class_name.begin(), it) + " (size " + std::to_string(a.size()) + ")";
    });

    // Operators
    p.def("__eq__", [](const C &a, const C &b) { return a == b; });
    p.def("__ne__", [](const C &a, const C &b) { return a != b; });
}

std::vector<uint8_t> to_directions(const std::vector<std::string> &tags) {
    std::vector<uint8_t> v(tags.size());
    for (size_t i = 0; i < tags.size(); ++i) {
        if (tags[i] == "minimization" || tags[i] == "min") {
            v[i] = 0x01;
        } else if (tags[i] == "maximization" || tags[i] == "max") {
            v[i] = 0x00;
        } else {
            throw std::invalid_argument(
                "Invalid tag. The valid keywords are 'minimization', "
                "'min', 'maximization', or 'max'");
        }
    }
    return v;
}

template <class C, class def_t> void bind_front_container_concept(def_t &p) {
    // Constructors
    p.def(py::init([](const std::vector<std::string> &d) {
        std::vector<uint8_t> m = to_directions(d);
        return new C({}, m.begin(), m.end());
    }));
    p.def(py::init([](const std::vector<value_type> &v, const std::vector<std::string> &d) {
        std::vector<uint8_t> m = to_directions(d);
        return new C(v.begin(), v.end(), m.begin(), m.end());
    }));
    p.def(py::init([](const std::vector<value_type> &v, const std::vector<uint8_t> &m) {
        return new C(v.begin(), v.end(), m.begin(), m.end());
    }));
    p.def(py::init([](const std::vector<value_type> &v, const std::vector<bool> &m) {
        return new C(v.begin(), v.end(), m.begin(), m.end());
    }));

    // Non-modifiers / Reference points / Front concept
    p.def("ideal", [](const C &lhs) { return lhs.ideal(); });
    p.def("ideal", [](const C &lhs, size_t dimension) { return lhs.ideal(dimension); });
    p.def("nadir", [](const C &lhs) { return lhs.nadir(); });
    p.def("nadir", [](const C &lhs, size_t dimension) { return lhs.nadir(dimension); });
    p.def("worst", [](const C &lhs) { return lhs.worst(); });
    p.def("worst", [](const C &lhs, size_t dimension) { return lhs.worst(dimension); });
    p.def("is_minimization", [](const C &lhs) { return lhs.is_minimization(); });
    p.def("is_minimization", [](const C &lhs, size_t dimension) { return lhs.is_minimization(dimension); });
    p.def("is_maximization", [](const C &lhs) { return lhs.is_maximization(); });
    p.def("is_maximization", [](const C &lhs, size_t dimension) { return lhs.is_maximization(dimension); });

    // Non-modifiers / Dominance / Front concept
    p.def("dominates", [](const C &lhs, const point_type& p) { return lhs.dominates(p); });
    p.def("strongly_dominates", [](const C &lhs, const point_type& p) { return lhs.strongly_dominates(p); });
    p.def("is_partially_dominated_by", [](const C &lhs, const point_type& p) { return lhs.is_partially_dominated_by(p); });
    p.def("is_completely_dominated_by", [](const C &lhs, const point_type& p) { return lhs.is_completely_dominated_by(p); });
    p.def("non_dominates", [](const C &lhs, const point_type& p) { return lhs.non_dominates(p); });
    p.def("dominates", [](const C &lhs, const C& P2) { return lhs.dominates(P2); });
    p.def("strongly_dominates", [](const C &lhs, const C& P2) { return lhs.strongly_dominates(P2); });
    p.def("is_partially_dominated_by", [](const C &lhs, const C& P2) { return lhs.is_partially_dominated_by(P2); });
    p.def("is_completely_dominated_by", [](const C &lhs, const C& P2) { return lhs.is_completely_dominated_by(P2); });
    p.def("non_dominates", [](const C &lhs, const C& P2) { return lhs.non_dominates(P2); });

    // Non-modifiers / Indicators / Front concept
    p.def("hypervolume", [](const C &lhs) { return lhs.hypervolume(); });
    p.def("hypervolume", [](const C &lhs, const point_type& p) { return lhs.hypervolume(p); });
    p.def("hypervolume", [](const C &lhs, size_t sample_size) { return lhs.hypervolume(sample_size); });
    p.def("hypervolume", [](const C &lhs, size_t sample_size, const point_type& p) { return lhs.hypervolume(sample_size, p); });
    p.def("coverage", [](const C &lhs, const C& rhs) { return lhs.coverage(rhs); });
    p.def("coverage_ratio", [](const C &lhs, const C& rhs) { return lhs.coverage_ratio(rhs); });
    p.def("gd", [](const C &lhs, const C& rhs) { return lhs.gd(rhs); });
    p.def("std_gd", [](const C &lhs, const C& rhs) { return lhs.std_gd(rhs); });
    p.def("igd", [](const C &lhs, const C& rhs) { return lhs.igd(rhs); });
    p.def("std_igd", [](const C &lhs, const C& rhs) { return lhs.std_igd(rhs); });
    p.def("hausdorff", [](const C &lhs, const C& rhs) { return lhs.hausdorff(rhs); });
    p.def("igd_plus", [](const C &lhs, const C& rhs) { return lhs.igd_plus(rhs); });
    p.def("std_igd_plus", [](const C &lhs, const C& rhs) { return lhs.std_igd_plus(rhs); });
    p.def("uniformity", [](const C &lhs) { return lhs.uniformity(); });
    p.def("average_distance", [](const C &lhs) { return lhs.average_distance(); });
    p.def("average_nearest_distance", [](const C &lhs, size_t k) { return lhs.average_nearest_distance(k); });
    p.def("crowding_distance", [](const C &lhs, const point_type& k) { return lhs.crowding_distance(k); });
    p.def("average_crowding_distance", [](const C &lhs) { return lhs.average_crowding_distance(); });
    p.def("direct_conflict", [](const C &lhs, size_t a, size_t b) { return lhs.direct_conflict(a,b); });
    p.def("normalized_direct_conflict", [](const C &lhs, size_t a, size_t b) { return lhs.normalized_direct_conflict(a,b); });
    p.def("maxmin_conflict", [](const C &lhs, size_t a, size_t b) { return lhs.maxmin_conflict(a,b); });
    p.def("normalized_maxmin_conflict", [](const C &lhs, size_t a, size_t b) { return lhs.normalized_maxmin_conflict(a,b); });
    p.def("conflict", [](const C &lhs, size_t a, size_t b) { return lhs.conflict(a,b); });
    p.def("normalized_conflict", [](const C &lhs, size_t a, size_t b) { return lhs.normalized_conflict(a,b); });

    // Lookup / Pareto concept
    p.def(
        "find_dominated",
        [](const C &s, const point_type &k) {
            return py::make_iterator(s.find_dominated(k), s.cend());
        },
        py::keep_alive<0, 1>());
    p.def(
        "find_nearest_exclusive",
        [](const C &s, const point_type& k) {
            return py::make_iterator(s.find_nearest_exclusive(k), s.cend());
        },
        py::keep_alive<0, 1>());
    p.def("ideal_element", [](const C &lhs, size_t dimension) {
        return *lhs.ideal_element(dimension);
    });
    p.def("nadir_element", [](const C &lhs, size_t dimension) {
        return *lhs.nadir_element(dimension);
    });
    p.def("worst_element", [](const C &lhs, size_t dimension) {
        return *lhs.worst_element(dimension);
    });

    // Operators
    // https://docs.python.org/3/reference/datamodel.html#special-method-names
    p.def("__lt__", [](const C &a, const C &b) { return a < b; });
    p.def("__le__", [](const C &a, const C &b) { return a <= b; });
    p.def("__gt__", [](const C &a, const C &b) { return a > b; });
    p.def("__ge__", [](const C &a, const C &b) { return a >= b; });
}

template <class C, class def_t> void bind_archive_container_concept(def_t &p) {
    // Constructors (one extra constructor for each with the capacity)
    p.def(py::init<size_t>());
    p.def(py::init([](size_t max_capacity, const std::vector<std::string> &d) {
        std::vector<uint8_t> m = to_directions(d);
        return new C(max_capacity, {}, m.begin(), m.end());
    }));
    p.def(py::init([](size_t max_capacity, const std::vector<value_type> &v) {
        return new C(max_capacity, v.begin(), v.end());
    }));
    p.def(py::init([](size_t max_capacity, std::initializer_list<value_type> il) {
        return new C(max_capacity, il.begin(), il.end());
    }));
    p.def(py::init([](size_t max_capacity, const std::vector<value_type> &v, const std::vector<std::string> &d) {
        std::vector<uint8_t> m = to_directions(d);
        return new C(max_capacity, v.begin(), v.end(), m.begin(), m.end());
    }));
    p.def(py::init([](size_t max_capacity, const std::vector<value_type> &v, const std::vector<uint8_t> &m) {
        return new C(max_capacity, v.begin(), v.end(), m.begin(), m.end());
    }));
    p.def(py::init([](size_t max_capacity, const std::vector<value_type> &v, const std::vector<bool> &m) {
        return new C(max_capacity, v.begin(), v.end(), m.begin(), m.end());
    }));

    // Iterators
    p.def(
        "fronts",
        [](const C &s) {
          return py::make_iterator(s.begin_front(), s.end_front());
        },
        py::keep_alive<0, 1>());

    p.def(
        "fronts",
        [](C &s) { return py::make_iterator(s.begin_front(), s.end_front()); },
        py::keep_alive<0, 1>());
    p.def(
        "reverse_fronts",
        [](const C &s) {
          return py::make_iterator(s.rbegin_front(), s.rend_front());
        },
        py::keep_alive<0, 1>());

    p.def(
        "reverse_fronts",
        [](C &s) { return py::make_iterator(s.rbegin_front(), s.rend_front()); },
        py::keep_alive<0, 1>());

    // Non-modifiers / Reference points / Archive concept
    p.def("capacity", [](const C &lhs) { return lhs.capacity(); });
    p.def("size_fronts", [](const C &lhs) { return lhs.size_fronts(); });

    // Non-modifiers / Dominance / Archive concept
    using front_type = typename C::front_type;
    p.def("dominates", [](const C &lhs, const front_type& P2) { return lhs.dominates(P2); });
    p.def("strongly_dominates", [](const C &lhs, const front_type& P2) { return lhs.strongly_dominates(P2); });
    p.def("is_partially_dominated_by", [](const C &lhs, const front_type& P2) { return lhs.is_partially_dominated_by(P2); });
    p.def("is_completely_dominated_by", [](const C &lhs, const front_type& P2) { return lhs.is_completely_dominated_by(P2); });
    p.def("non_dominates", [](const C &lhs, const front_type& P2) { return lhs.non_dominates(P2); });

    // Modifiers / Archive concepts
    p.def("resize", [](C &lhs, size_t new_size) { return lhs.resize(new_size); });

    // Lookup / Archive concept
    p.def("find_front", [](const C &lhs, const point_type& k) {
        return *lhs.find_front(k);
    });
}

/// Create modules
PYBIND11_MODULE(pareto, m) {
    using namespace py::literals;
    using namespace pareto;
    m.doc() =
        "A library for spatial containers, Pareto fronts, and Pareto archives";

    /*
     * Bind point type
     */
    py::class_<point_type> p(m, "point");
    bind_point_concept(p);

    /*
     * Bind spatial types
     */
    using type2 = implicit_tree<dimension_type, 0, mapped_type>;
    py::class_<type2> sm2(m, "implicit_tree");
    bind_spatial_container_concept<type2>(sm2);

    using type3 = kd_tree<dimension_type, 0, mapped_type>;
    py::class_<type3> sm3(m, "kd_tree");
    bind_spatial_container_concept<type3>(sm3);

    using type4 = quad_tree<dimension_type, 0, mapped_type>;
    py::class_<type4> sm4(m, "quad_tree");
    bind_spatial_container_concept<type4>(sm4);

    using type5 = r_tree<dimension_type, 0, mapped_type>;
    py::class_<type5> sm5(m, "r_tree");
    bind_spatial_container_concept<type5>(sm5);

    using type6 = r_star_tree<dimension_type, 0, mapped_type>;
    py::class_<type6> sm6(m, "r_star_tree");
    bind_spatial_container_concept<type6>(sm6);

    m.attr("spatial_map") = m.attr("r_tree");

    /*
     * Bind front types
     */
    using ftype2 = front<dimension_type, 0, mapped_type,
                         implicit_tree<dimension_type, 0, mapped_type>>;
    py::class_<ftype2> fsm2(m, "implicit_front");
    bind_spatial_container_concept<ftype2>(fsm2);
    bind_front_container_concept<ftype2>(fsm2);

    using ftype3 = front<dimension_type, 0, mapped_type,
                         kd_tree<dimension_type, 0, mapped_type>>;
    py::class_<ftype3> fsm3(m, "kd_front");
    bind_spatial_container_concept<ftype3>(fsm3);
    bind_front_container_concept<ftype3>(fsm3);

    using ftype4 = front<dimension_type, 0, mapped_type,
                         quad_tree<dimension_type, 0, mapped_type>>;
    py::class_<ftype4> fsm4(m, "quad_front");
    bind_spatial_container_concept<ftype4>(fsm4);
    bind_front_container_concept<ftype4>(fsm4);

    using ftype5 =
        front<dimension_type, 0, mapped_type, r_tree<dimension_type, 0, mapped_type>>;
    py::class_<ftype5> fsm5(m, "r_front");
    bind_spatial_container_concept<ftype5>(fsm5);
    bind_front_container_concept<ftype5>(fsm5);

    using ftype6 = front<dimension_type, 0, mapped_type,
                         r_star_tree<dimension_type, 0, mapped_type>>;
    py::class_<ftype6> fsm6(m, "r_star_front");
    bind_spatial_container_concept<ftype6>(fsm6);
    bind_front_container_concept<ftype6>(fsm6);

    m.attr("front") = m.attr("r_front");

    /*
     * Bind archive types
     */
    using atype2 = archive<dimension_type, 0, mapped_type,
                         implicit_tree<dimension_type, 0, mapped_type>>;
    py::class_<atype2> asm2(m, "implicit_archive");
    bind_spatial_container_concept<atype2>(asm2);
    bind_front_container_concept<atype2>(asm2);
    bind_archive_container_concept<atype2>(asm2);

    using atype3 = archive<dimension_type, 0, mapped_type,
                         kd_tree<dimension_type, 0, mapped_type>>;
    py::class_<atype3> asm3(m, "kd_archive");
    bind_spatial_container_concept<atype3>(asm3);
    bind_front_container_concept<atype3>(asm3);
    bind_archive_container_concept<atype3>(asm3);

    using atype4 = archive<dimension_type, 0, mapped_type,
                         quad_tree<dimension_type, 0, mapped_type>>;
    py::class_<atype4> asm4(m, "quad_archive");
    bind_spatial_container_concept<atype4>(asm4);
    bind_front_container_concept<atype4>(asm4);
    bind_archive_container_concept<atype4>(asm4);

    using atype5 =
        archive<dimension_type, 0, mapped_type, r_tree<dimension_type, 0, mapped_type>>;
    py::class_<atype5> asm5(m, "r_archive");
    bind_spatial_container_concept<atype5>(asm5);
    bind_front_container_concept<atype5>(asm5);
    bind_archive_container_concept<atype5>(asm5);

    using atype6 = archive<dimension_type, 0, mapped_type,
                         r_star_tree<dimension_type, 0, mapped_type>>;
    py::class_<atype6> asm6(m, "r_star_archive");
    bind_spatial_container_concept<atype6>(asm6);
    bind_front_container_concept<atype6>(asm6);
    bind_archive_container_concept<atype6>(asm6);

    m.attr("archive") = m.attr("r_archive");
}
