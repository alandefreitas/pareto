#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <iostream>
#include <random>
#include <chrono>
#include <pareto_front.h>

namespace py = pybind11;

// Random integers
unsigned randi();

// Random doubles
double randn();

template<class point_type>
using point_number = std::decay_t<decltype(point_type().template get<0>())>;


template<size_t idx = 0, class point_type, typename FUNCTION_T>
std::enable_if_t<idx < boost::geometry::traits::dimension<point_type>(), void>
visit_point(const point_type &k, FUNCTION_T fn) {
    fn(k.template get<idx>());
    visit_point<idx + 1>(k, fn);
}

template<size_t idx, class point_type, typename FUNCTION_T>
std::enable_if_t<idx == boost::geometry::traits::dimension<point_type>(), void>
visit_point(const point_type &k, FUNCTION_T fn) {}

template<size_t idx = 0, class point_type, typename FUNCTION_T>
std::enable_if_t<idx < boost::geometry::traits::dimension<point_type>(), void>
visit_point(point_type &k, FUNCTION_T fn) {
    point_number<point_type> tmp = k.template get<idx>();
    fn(tmp);
    k.template set<idx>(tmp);
    visit_point<idx + 1>(k, fn);
}

template<size_t idx, class point_type, typename FUNCTION_T>
std::enable_if_t<idx == boost::geometry::traits::dimension<point_type>(), void>
visit_point(point_type &k, FUNCTION_T fn) {}

template<class point_type>
std::vector<point_number<point_type>> point_to_vector(const point_type &v) {
    std::vector<point_number<point_type>> result;
    visit_point(v, [&](point_number<point_type> p) {
        result.emplace_back(p);
    });
    return result;
}


template<class point_type>
point_type vector_to_point(const std::vector<point_number<point_type>> &v) {
    point_type result;
    int i = 0;
    visit_point(result, [&](point_number<point_type> &p) {
        p = v[i];
        i++;
    });
    return result;
}

PYBIND11_MODULE(pyfront, mymodule) {
    using namespace py::literals; // for _a literal to define arguments
    using namespace pareto_front; // for _a literal to define arguments
    mymodule.doc() = "example module to export code";

    // random functions for tests
    mymodule.def("randn", &randn, "Get a double from a normal distribution");
    mymodule.def("randi", &randi, "Get an integer from a uniform distribution");

    // types
    using pareto_front_t = pareto_front<double, 2, unsigned>;
    using number_t = pareto_front_t::number_type;
    using point_t = pareto_front_t::point_type;
    using key_t = pareto_front_t::key_type;
    using mapped_t = pareto_front_t::mapped_type;
    using value_t = pareto_front_t::value_type;

    // point object
    py::class_<pareto_front_t::point_type> p(mymodule, "point2d");
    p.def(py::init<>());
    p.def(py::init<number_t const &>());
    p.def(py::init<number_t const &, number_t const &>());
    p.def(py::init<number_t const &, number_t const &, number_t const &>());
    p.def(py::init([](const std::vector<number_t> &s) {
        return new pareto_front_t::point_type(vector_to_point<point_t>(s));
    }));
    p.def_property("x",
                   [](const pareto_front_t::point_type &a) {
                       return a.get<0>();
                   },
                   [](pareto_front_t::point_type &a, double v) {
                       a.set<0>(v);
                   }
    );
    p.def_property("y",
                   [](const pareto_front_t::point_type &a) {
                       return a.get<1>();
                   },
                   [](pareto_front_t::point_type &a, double v) {
                       a.set<1>(v);
                   }
    );
    p.def("__repr__",
          [](const pareto_front_t::point_type &a) {
              std::stringstream ss;
              ss << "(" << a.get<0>() << ", " << a.get<1>() << ")";
              return ss.str();
          }
    );

    // pareto_front object
    py::class_<pareto_front_t> pf(mymodule, "front2d");
    pf.def(py::init<>());
    pf.def(py::init([](const std::vector<point_t> &v) {
        std::vector<value_t> v2;
        v2.reserve(v.size());
        for (const point_t &item : v) {
            v2.emplace_back(std::make_pair(item, mapped_t()));
        }
        return new pareto_front_t(v2);
    }));
    pf.def(py::init([](const std::vector<value_t> &v) {
        return new pareto_front_t(v);
    }));
    pf.def(py::init([](const std::vector<std::vector<number_t>> &v) {
        std::vector<value_t> v2;
        v2.reserve(v.size());
        for (const std::vector<number_t> &item : v) {
            v2.emplace_back(std::make_pair(vector_to_point<point_t>(item), mapped_t()));
        }
        return new pareto_front_t(v2);
    }));
    pf.def(py::init([](const std::vector<std::pair<std::vector<number_t>, mapped_t>> &v) {
        std::vector<value_t> v2;
        v2.reserve(v.size());
        for (const auto &[key, value] : v) {
            v2.emplace_back(std::make_pair(vector_to_point<point_t>(key), value));
        }
        return new pareto_front_t(v2);
    }));
    pf.def("empty", &pareto_front_t::empty);
    pf.def("size", &pareto_front_t::size);
    pf.def("emplace", [](pareto_front_t &a, const point_t &p) {
        a.emplace(p, mapped_t());
    });
    pf.def("emplace", [](pareto_front_t &a, const std::vector<number_t> &p) {
        a.emplace(vector_to_point<point_t>(p), mapped_t());
    });
    pf.def("emplace", [](pareto_front_t &a, const point_t &p, const mapped_t &v) {
        a.emplace(p, v);
    });
    pf.def("emplace", [](pareto_front_t &a, const std::vector<number_t> &p, const mapped_t &v) {
        a.emplace(vector_to_point<point_t>(p), v);
    });
    pf.def("emplace", [](pareto_front_t &a, const value_t &v) {
        a.emplace(v);
    });
    pf.def("emplace", [](pareto_front_t &a, const std::pair<std::vector<number_t>, mapped_t> &v) {
        a.emplace(vector_to_point<point_t>(v.first), v.second);
    });
    pf.def("emplace", [](pareto_front_t &a, const std::vector<std::pair<std::vector<number_t>, mapped_t>> &vs) {
        for (const auto &v : vs) {
            a.emplace(vector_to_point<point_t>(v.first), v.second);
        }
    });
    pf.def("emplace", [](pareto_front_t &a, py::iterator &iter) {
        while (iter != py::iterator::sentinel()) {
            a.emplace(iter->template cast<value_t>());
            ++iter;
        }
    });
    pf.def("insert", [](pareto_front_t &a, const point_t &p) {
        a.emplace(p, mapped_t());
    });
    pf.def("insert", [](pareto_front_t &a, const std::vector<number_t> &p) {
        a.emplace(vector_to_point<point_t>(p), mapped_t());
    });
    pf.def("insert", [](pareto_front_t &a, const point_t &p, const mapped_t &v) {
        a.emplace(p, v);
    });
    pf.def("insert", [](pareto_front_t &a, const std::vector<number_t> &p, const mapped_t &v) {
        a.emplace(vector_to_point<point_t>(p), v);
    });
    pf.def("insert", [](pareto_front_t &a, const value_t &v) {
        a.emplace(v);
    });
    pf.def("insert", [](pareto_front_t &a, const std::pair<std::vector<number_t>, mapped_t> &v) {
        a.emplace(vector_to_point<point_t>(v.first), v.second);
    });
    pf.def("insert", [](pareto_front_t &a, const std::vector<std::pair<std::vector<number_t>, mapped_t>> &vs) {
        for (const auto &v : vs) {
            a.emplace(vector_to_point<point_t>(v.first), v.second);
        }
    });
    pf.def("insert", [](pareto_front_t &a, py::iterator &iter) {
        std::vector<value_t> vs;
        while (iter != py::iterator::sentinel()) {
            vs.emplace_back(iter->template cast<value_t>());
            ++iter;
        }
        a.emplace(vs.begin(), vs.end());
    });
    pf.def("erase", [](pareto_front_t &a, const value_t &p) {
        a.erase(p);
    });
    pf.def("erase", [](pareto_front_t &a, const point_t &p) {
        a.erase(p);
    });
    pf.def("erase", [](pareto_front_t &a, const std::vector<number_t> &p) {
        a.erase(vector_to_point<point_t>(p));
    });
    pf.def("erase", [](pareto_front_t &a, const point_t &p, const mapped_t &v) {
        a.erase(std::make_pair(p, v));
    });
    pf.def("erase", [](pareto_front_t &a, const std::vector<number_t> &p, const mapped_t &v) {
        a.erase(std::make_pair(vector_to_point<point_t>(p), v));
    });
    pf.def("erase", [](pareto_front_t &a, const std::pair<std::vector<number_t>, mapped_t> &v) {
        a.erase(std::make_pair(vector_to_point<point_t>(v.first), v.second));
    });
    pf.def("erase", [](pareto_front_t &a, const std::vector<std::pair<std::vector<number_t>, mapped_t>> &vs) {
        for (const auto &v : vs) {
            a.erase(std::make_pair(vector_to_point<point_t>(v.first), v.second));
        }
    });
    pf.def("erase", [](pareto_front_t &a, py::iterator &iter) {
        while (iter != py::iterator::sentinel()) {
            a.erase(iter->template cast<value_t>());
            ++iter;
        }
    });
    pf.def("clear", &pareto_front_t::clear);
    pf.def("merge", [](pareto_front_t &a, pareto_front_t &b) {
        a.merge(b);
    });
    pf.def("swap", [](pareto_front_t &a, pareto_front_t &b) {
        a.swap(b);
    });
    pf.def("swap", [](pareto_front_t &a, pareto_front_t &b) {
        a.swap(b);
    });
    pf.def("find_intersection", [](const pareto_front_t &s, point_t min_corner, point_t max_corner) {
               return py::make_iterator(s.find_intersection(min_corner, max_corner), s.end());
           },
           py::keep_alive<0, 1>());
    pf.def("find_intersection",
           [](const pareto_front_t &s, std::vector<number_t> min_corner, std::vector<number_t> max_corner) {
               return py::make_iterator(
                       s.find_intersection(vector_to_point<point_t>(min_corner), vector_to_point<point_t>(max_corner)),
                       s.end());
           },
           py::keep_alive<0, 1>());
    pf.def("get_intersection", [](const pareto_front_t &s, point_t min_corner, point_t max_corner) {
        return std::vector<value_t>(s.find_intersection(min_corner, max_corner), s.end());
    });
    pf.def("get_intersection",
           [](const pareto_front_t &s, std::vector<number_t> min_corner, std::vector<number_t> max_corner) {
               return std::vector<value_t>(
                       s.find_intersection(vector_to_point<point_t>(min_corner), vector_to_point<point_t>(max_corner)),
                       s.end());
           });
    pf.def("find_nearest", [](const pareto_front_t &s, point_t p) {
               return py::make_iterator(s.find_nearest(p), s.end());
           },
           py::keep_alive<0, 1>());
    pf.def("find_nearest", [](const pareto_front_t &s, std::vector<number_t> p) {
               return py::make_iterator(s.find_nearest(vector_to_point<point_t>(p)), s.end());
           },
           py::keep_alive<0, 1>());
    pf.def("find_nearest", [](const pareto_front_t &s, point_t p, size_t k) {
               return py::make_iterator(s.find_nearest(p, k), s.end());
           },
           py::keep_alive<0, 1>());
    pf.def("find_nearest", [](const pareto_front_t &s, std::vector<number_t> p, size_t k) {
               return py::make_iterator(s.find_nearest(vector_to_point<point_t>(p), k), s.end());
           },
           py::keep_alive<0, 1>());
    pf.def("get_nearest", [](const pareto_front_t &s, point_t p) {
        return std::vector<value_t>(s.find_nearest(p), s.end());
    });
    pf.def("get_nearest", [](const pareto_front_t &s, std::vector<number_t> p) {
        return std::vector<value_t>(s.find_nearest(vector_to_point<point_t>(p)), s.end());
    });
    pf.def("get_nearest", [](const pareto_front_t &s, point_t p, size_t k) {
        return std::vector<value_t>(s.find_nearest(p, k), s.end());
    });
    pf.def("get_nearest", [](const pareto_front_t &s, std::vector<number_t> p, size_t k) {
        return std::vector<value_t>(s.find_nearest(vector_to_point<point_t>(p), k), s.end());
    });
    pf.def("hypervolume", [](const pareto_front_t &s) { return s.hypervolume(); });
    pf.def("hypervolume", [](const pareto_front_t &s, point_t reference) { return s.hypervolume(reference); });
    pf.def("hypervolume", [](const pareto_front_t &s, std::vector<number_t> reference) {
        return s.hypervolume(vector_to_point<point_t>(reference));
    });
    pf.def("dominates", [](const pareto_front_t &s, point_t reference) { return s.dominates(reference); });
    pf.def("dominates", [](const pareto_front_t &s, std::vector<number_t> reference) {
        return s.dominates(vector_to_point<point_t>(reference));
    });
    pf.def("ideal", &pareto_front_t::ideal);
    pf.def("nadir", &pareto_front_t::nadir);
    pf.def("worst", &pareto_front_t::worst);
    pf.def("find", [](const pareto_front_t &s, point_t p) {
               return py::make_iterator(s.find(p), s.end());
           },
           py::keep_alive<0, 1>());
    pf.def("find", [](const pareto_front_t &s, std::vector<number_t> p) {
               return py::make_iterator(s.find(vector_to_point<point_t>(p)), s.end());
           },
           py::keep_alive<0, 1>());
    pf.def("get", [](const pareto_front_t &s, std::vector<number_t> p) {
        auto it = s.find(vector_to_point<point_t>(p));
        if (it != s.end()) {
            return *it;
        } else {
            throw std::invalid_argument("Element is not is the pareto front");
        }
    });
    pf.def("contains", [](const pareto_front_t &s, point_t p) {
        return s.find(p) != s.end();
    });
    pf.def("contains", [](const pareto_front_t &s, std::vector<number_t> p) {
        return s.find(vector_to_point<point_t>(p)) != s.end();
    });
    pf.def("__iter__", [](const pareto_front_t &s) { return py::make_iterator(s.begin(), s.end()); },
           py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */);
    pf.def("__len__", &pareto_front_t::size);
    pf.def("__getitem__", [](const pareto_front_t &s, point_t p) {
        auto it = s.find(p);
        if (it != s.end()) {
            return *it;
        } else {
            throw std::invalid_argument("Element is not is the pareto front");
        }
    });
    pf.def("__getitem__", [](const pareto_front_t &s, std::vector<number_t> p) {
        auto it = s.find(vector_to_point<point_t>(p));
        if (it != s.end()) {
            return *it;
        } else {
            throw std::invalid_argument("Element is not is the pareto front");
        }
    });
    pf.def("__setitem__", [](pareto_front_t &s, point_t p, mapped_t v) {
        s.erase(p);
        s.insert(p,v);
    });
    pf.def("__setitem__", [](pareto_front_t &s, std::vector<number_t> p, mapped_t v) {
        s.erase(vector_to_point<point_t>(p));
        s.insert(p,v);
    });
    pf.def("__contains__", [](const pareto_front_t &s, point_t p) { return s.find(p) != s.end(); });
    pf.def("__repr__",
           [](const pareto_front_t &a) {
               std::stringstream ss;
               ss << "< Pareto front - " << a.size() << " points >";
               return ss.str();
           }
    );
}

std::mt19937 &generator() {
    static std::mt19937 g(
            (std::random_device()()) | std::chrono::high_resolution_clock::now().time_since_epoch().count());
    return g;
}

unsigned randi() {
    static std::uniform_int_distribution<unsigned> ud(0, 40);
    return ud(generator());;
}

double randn() {
    static std::normal_distribution nd;
    return nd(generator());
}