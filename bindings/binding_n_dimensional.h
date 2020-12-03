#ifndef PARETO_FRONT_BINDING_N_DIMENSIONAL
#define PARETO_FRONT_BINDING_N_DIMENSIONAL

#include "python_instantiations.h"
#include <string>
#include <chrono>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <random>

namespace py = pybind11;

template <class TAG>
std::string tag_to_string() {
    using namespace pareto;
    if constexpr (std::is_same_v<TAG,vector_tree_tag>) {
        return "list";
    } else if constexpr (std::is_same_v<TAG,quad_tree_tag>) {
        return "quadtree";
    } else if constexpr (std::is_same_v<TAG,kd_tree_tag>) {
        return "kdtree";
    } else if constexpr (std::is_same_v<TAG,boost_tree_tag>) {
        return "boostrtree";
    } else if constexpr (std::is_same_v<TAG,r_tree_tag>) {
        return "rtree";
    } else if constexpr (std::is_same_v<TAG,r_star_tree_tag>) {
        return "rstartree";
    }
    // unreachable code:
    // throw std::invalid_argument("Invalid data structure tag");
}

// tag_to_string() should explicitly instantiated in another .cpp
// file somewhere
extern template std::string tag_to_string<pareto::vector_tree_tag>();
extern template std::string tag_to_string<pareto::quad_tree_tag>();
extern template std::string tag_to_string<pareto::kd_tree_tag>();
extern template std::string tag_to_string<pareto::boost_tree_tag>();
extern template std::string tag_to_string<pareto::r_tree_tag>();
extern template std::string tag_to_string<pareto::r_star_tree_tag>();

/// Create bindings for a pareto front with N dimensions
/// When defining default = true, we create a special "front"
/// class name with no extra qualifiers. This will be the default
/// front for that dimension.
template<size_t N, class TAG>
void binding_for_N_dimensional(py::module &m, bool define_point_class = false) {
    using namespace py::literals;
    using namespace pareto;

    // Types for convenience
    using pareto_front_type = front<double, N, py::object, TAG>;
    using pareto_archive_type = archive<double, N, py::object, TAG>;
    using number_type = typename pareto_front_type::number_type;
    using point_type = typename pareto_front_type::point_type;
    // using key_type = typename pareto_front_type::key_type;
    using mapped_type = typename pareto_front_type::mapped_type;
    using value_type = typename pareto_front_type::value_type;

    // Point object
    // only define it once for each dimension, all trees of same
    // dimension use the same point type
    if (define_point_class) {
        std::string class_name;
        class_name = "point" + std::to_string(N) + "d";
        py::class_<point_type> p(m, class_name.c_str());

        // default constructor
        p.def(py::init<>());

        if constexpr (N == 0) {
            // constructor with size and number
            p.def(py::init<size_t>());
            p.def(py::init([](size_t dim, const number_type value) {
              return new point_type(dim, value);
            }));
        } else {
            // constructor with number in all dimensions
            p.def(py::init([](const number_type value) {
              return new point_type(N, value);
            }));
        }

        // constructor from initializer list
        p.def(py::init<std::initializer_list<number_type>>());

        // copy/move constructors
        p.def(py::init<const point_type &>());

        // zero dimension point constructor (dimension not defined at compile time)
        if constexpr (N != 0) {
            p.def(py::init([](const point<number_type, 0> &p2) {
              return new point_type(p2);
            }));
        }

        // range constructor
        p.def(py::init([](const std::vector<number_type> &v) {
          return new point_type(v);
        }));

        // convenience property to get the first dimension (x)
        p.def_property("x",
                       [](const point_type &a) {
                         return a[0];
                       },
                       [](point_type &a, const number_type v) {
                         a[0] = v;
                       }
        );

        if constexpr (N > 1) {
            // convenience property to get the second dimension (y)
            p.def_property("y",
                           [](const point_type &a) {
                             return a[1];
                           },
                           [](point_type &a, const number_type v) {
                             a[1] = v;
                           }
            );
        }

        if constexpr (N > 2) {
            // convenience property to get the third dimension (z)
            p.def_property("z",
                           [](const point_type &a) {
                             return a[2];
                           },
                           [](point_type &a, const number_type v) {
                             a[2] = v;
                           }
            );
        }
        // get number of dimensions
        p.def("dimensions", &point_type::dimensions);

        // check dominance
        p.def("dominates",
              [](const point_type &a, const point_type &b) {
                return a.dominates(b);
              });

        p.def("dominates",
              [](const point_type &a, const point_type &b, bool is_minimization) {
                return a.dominates(b, is_minimization);
              });

        p.def("dominates",
              [](const point_type &a, const point_type &b, const std::vector<bool> &is_minimization) {
                return a.dominates(b, is_minimization);
              });

        p.def("dominates",
              [](const point_type &a, const point_type &b, const std::vector<uint8_t> &is_minimization) {
                return a.dominates(b, is_minimization);
              });

        p.def("dominates",
              [](const point_type &a, const point_type &b, const std::vector<int> &is_minimization) {
                return a.dominates(b, is_minimization);
              });

        // check strong dominance
        p.def("strongly_dominates",
              [](const point_type &a, const point_type &b) {
                return a.strongly_dominates(b);
              });

        p.def("strongly_dominates",
              [](const point_type &a, const point_type &b, bool is_minimization) {
                return a.strongly_dominates(b, is_minimization);
              });

        p.def("strongly_dominates",
              [](const point_type &a, const point_type &b, const std::vector<bool> &is_minimization) {
                return a.strongly_dominates(b, is_minimization);
              });

        p.def("strongly_dominates",
              [](const point_type &a, const point_type &b, const std::vector<uint8_t> &is_minimization) {
                return a.strongly_dominates(b, is_minimization);
              });

        p.def("strongly_dominates",
              [](const point_type &a, const point_type &b, const std::vector<int> &is_minimization) {
                return a.strongly_dominates(b, is_minimization);
              });

        // check strong dominance
        p.def("non_dominates",
              [](const point_type &a, const point_type &b) {
                return a.non_dominates(b);
              });

        p.def("non_dominates",
              [](const point_type &a, const point_type &b, bool is_minimization) {
                return a.non_dominates(b, is_minimization);
              });

        p.def("non_dominates",
              [](const point_type &a, const point_type &b, const std::vector<bool> &is_minimization) {
                return a.non_dominates(b, is_minimization);
              });

        p.def("non_dominates",
              [](const point_type &a, const point_type &b, const std::vector<uint8_t> &is_minimization) {
                return a.non_dominates(b, is_minimization);
              });

        p.def("non_dominates",
              [](const point_type &a, const point_type &b, const std::vector<int> &is_minimization) {
                return a.non_dominates(b, is_minimization);
              });

        // distance
        p.def("distance",
              [](const point_type &a, const point_type &b) {
                return a.distance(b);
              });

        // distance
        p.def("quadrant", &point_type::quadrant);

        // operators
        p.def(py::self < py::self);
        p.def(py::self > py::self);
        p.def(py::self <= py::self);
        p.def(py::self >= py::self);
        p.def(py::self == py::self);
        p.def(py::self != py::self);
        p.def(py::self + py::self);
        p.def(py::self += py::self);
        p.def(py::self - py::self);
        p.def(py::self -= py::self);
        p.def(py::self * py::self);
        p.def(py::self *= py::self);
        p.def(py::self / py::self);
        p.def(py::self /= py::self);
        p.def(py::self + number_type());
        p.def(py::self += number_type());
        p.def(py::self - number_type());
        p.def(py::self -= number_type());
        p.def(py::self * number_type());
        p.def(py::self *= number_type());
        p.def(py::self / number_type());
        p.def(py::self /= number_type());

        // operator[]
        p.def("__getitem__",
              [](const point_type &a, size_t n) {
                return a[n];
              }
        );

        p.def("__setitem__", [](point_type& p, size_t n, number_type v) {
          std::cout << "setting point at position " << n << " with value " << v << std::endl;
          p[n] = v;
          std::cout << "p[n] = " << p[n] << std::endl;
        });

        // iterators
        p.def("__iter__", [](const point_type &s) {
          return py::make_iterator(s.begin(), s.end());
        }, py::keep_alive<0, 1>());

        p.def("__iter__", [](point_type &s) {
          return py::make_iterator(s.begin(), s.end());
        }, py::keep_alive<0, 1>());

        // print the point
        p.def("__repr__",
              [](const point_type &a) {
                std::stringstream ss;
                ss << a;
                return ss.str();
              }
        );
    }

    ///////////////////////////
    /// pareto object ///
    ///////////////////////////
    std::string class_name2 = "front" + std::to_string(N) + "d" + tag_to_string<TAG>();
    py::class_<pareto_front_type> pf(m, class_name2.c_str());

    // Default constructor
    pf.def(py::init<>());

    // Define optimization direction
    pf.def(py::init<bool>());
    pf.def(py::init<std::initializer_list<bool>>());
    pf.def(py::init<const std::vector<uint8_t>&>());
    pf.def(py::init<const std::array<uint8_t, N>&>());

    // Bulk insert initial values
    pf.def(py::init<std::initializer_list<value_type>>());
    pf.def(py::init<std::initializer_list<value_type>,bool>());
    pf.def(py::init<std::initializer_list<value_type>,std::vector<uint8_t>>());
    pf.def(py::init<std::initializer_list<value_type>,std::initializer_list<bool>>());
    pf.def(py::init<const std::vector<value_type>&>());
    pf.def(py::init<const std::vector<value_type>&,bool>());
    pf.def(py::init<const std::vector<value_type>&,std::vector<uint8_t>>());
    pf.def(py::init<const std::vector<value_type>&,std::initializer_list<bool>>());
    pf.def(py::init([](py::iterator &iter) {
      auto p = new pareto_front_type();
      while (iter != py::iterator::sentinel()) {
          p->emplace(iter->template cast<value_type>());
          ++iter;
      }
      return p;
    }));

    pf.def(py::init([](py::iterator &iter, bool is_minimization) {
      auto p = new pareto_front_type(is_minimization);
      while (iter != py::iterator::sentinel()) {
          p->emplace(iter->template cast<value_type>());
          ++iter;
      }
      return p;
    }));

    pf.def(py::init([](py::iterator &iter, const std::vector<uint8_t>& is_minimization) {
      auto p = new pareto_front_type(is_minimization);
      while (iter != py::iterator::sentinel()) {
          p->emplace(iter->template cast<value_type>());
          ++iter;
      }
      return p;
    }));

    pf.def(py::init([](py::iterator &iter, std::initializer_list<bool> is_minimization) {
      auto p = new pareto_front_type(is_minimization);
      while (iter != py::iterator::sentinel()) {
          p->emplace(iter->template cast<value_type>());
          ++iter;
      }
      return p;
    }));

    // Copy/move constructors
    pf.def(py::init<const pareto_front_type &>());

    // Helper functions to construct appropriate point types in python
    pf.def("point", [](const pareto_front_type &s) {
        return point_type(s.dimensions());
    });

    // the used parameter needs to be here to generate the proper binding in the object: pf.point(...)
    pf.def("point", [](const pareto_front_type &, const std::vector<double> &v) {
        return point_type(v.begin(), v.end());
    });

    // the used parameter needs to be here to generate the proper binding in the object: pf.point(...)
    pf.def("point", [](const pareto_front_type &, std::initializer_list<double> v) {
        return point_type(v);
    });

    // the used parameter needs to be here to generate the proper binding in the object: pf.point(...)
    pf.def("point", [](const pareto_front_type &, const point_type &p) {
        return point_type(p);
    });

    if constexpr (N != 0) {
        // the used parameter needs to be here to generate the proper binding in the object: pf.point(...)
        pf.def("point", [](const pareto_front_type &, const point<number_type, 0> &p2) {
            return point_type(p2);
        });
    }

    // Iterators
    pf.def("__iter__", [](const pareto_front_type &s) {
      return py::make_iterator(s.begin(), s.end());
    }, py::keep_alive<0, 1>());

    pf.def("__iter__", [](pareto_front_type &s) {
      return py::make_iterator(s.begin(), s.end());
    }, py::keep_alive<0, 1>());

    pf.def("__reversed__", [](const pareto_front_type &s) {
      return py::make_iterator(s.rbegin(), s.rend());
    }, py::keep_alive<0, 1>());

    pf.def("__reversed__", [](pareto_front_type &s) {
      return py::make_iterator(s.rbegin(), s.rend());
    }, py::keep_alive<0, 1>());

    // Non-modifying functions
    pf.def("empty", &pareto_front_type::empty);

    pf.def("size", &pareto_front_type::size);
    pf.def("__len__", &pareto_front_type::size);

    pf.def("dimensions", [](const pareto_front_type& a) {
      return a.dimensions();
    });

    if constexpr (N == 0) {
        pf.def("dimensions", [](pareto_front_type &a, size_t s) {
          a.dimensions(s);
        });
    }

    pf.def("is_minimization", [](const pareto_front_type& a) {
      return a.is_minimization();
    });

    pf.def("is_minimization", [](const pareto_front_type& a, size_t index) {
      return a.is_minimization(index);
    });

    pf.def("is_maximization", [](const pareto_front_type& a) {
      return a.is_maximization();
    });

    pf.def("is_maximization", [](const pareto_front_type& a, size_t index) {
      return a.is_maximization(index);
    });

    // Reference points
    pf.def("ideal", [](const pareto_front_type& a, size_t index) {
      return a.ideal(index);
    });

    pf.def("ideal", [](const pareto_front_type& a) {
      return a.ideal();
    });

    pf.def("dimension_ideal", [](const pareto_front_type &s, size_t d) {
      auto it = s.dimension_ideal(d);
      if (it != s.end()) {
          return *it;
      } else {
          return value_type();
      }
    });

    pf.def("nadir", [](const pareto_front_type& a, size_t index) {
      return a.nadir(index);
    });

    pf.def("nadir", [](const pareto_front_type& a) {
      return a.nadir();
    });

    pf.def("dimension_nadir", [](const pareto_front_type &s, size_t d) {
      auto it = s.dimension_nadir(d);
      if (it != s.end()) {
          return *it;
      } else {
          return value_type();
      }
    });

    pf.def("worst", [](const pareto_front_type& a, size_t index) {
      return a.worst(index);
    });

    pf.def("worst", [](const pareto_front_type& a) {
      return a.worst();
    });

    pf.def("dimension_worst", [](const pareto_front_type &s, size_t d) {
      auto it = s.dimension_worst(d);
      if (it != s.end()) {
          return *it;
      } else {
          return value_type();
      }
    });

    // Search
    pf.def("find", [](const pareto_front_type &s, point_type p) { return py::make_iterator(s.find(p), s.end());}, py::keep_alive<0, 1>());

    pf.def("find", [](pareto_front_type &s, point_type p) { return py::make_iterator(s.find(p), s.end());}, py::keep_alive<0, 1>());

    pf.def("contains", [](const pareto_front_type &s, point_type p) { return s.find(p) != s.end(); });

    pf.def("contains", [](pareto_front_type &s, point_type p) { return s.find(p) != s.end(); });

    pf.def("__contains__", [](const pareto_front_type &s, point_type p) { return s.find(p) != s.end(); });

    pf.def("__contains__", [](pareto_front_type &s, point_type p) { return s.find(p) != s.end(); });

    pf.def("get", [](const pareto_front_type &s, point_type p) {
      auto it = s.find(p);
      if (it != s.end()){
          return *it;
      } else {
          throw std::invalid_argument("Element is not is the pareto front");
      }});

    pf.def("__getitem__", [](const pareto_front_type &s, point_type p) {
      auto it = s.find(p);
      if (it != s.end()) {
          return it->second;
      } else {
          throw std::invalid_argument("Element is not is the pareto front");
      }
    });

    pf.def("__getitem__", [](const pareto_front_type &s, std::initializer_list<number_type> p) {
      auto it = s.find(point_type(p));
      if (it != s.end()) {
          return it->second;
      } else {
          throw std::invalid_argument("Element is not is the pareto front");
      }
    });

    pf.def("__getitem__", [](const pareto_front_type &s, const std::vector<number_type>& v) {
      auto it = s.find(point_type(v.begin(), v.end()));
      if (it != s.end()) {
          return it->second;
      } else {
          throw std::invalid_argument("Element is not is the pareto front");
      }
    });

//    pf.def("__getitem__", [](const pareto_front_type &s, const py::object& o) {
//        std::cout << "getting item?" << o << std::endl;
//    });

    // Operators
    pf.def(py::self == py::self);
    pf.def(py::self != py::self);

    // Insert item dictionary
    pf.def("insert", [](pareto_front_type &a, const value_type &v) {
      a.insert(v);
    });

    pf.def("insert", [](pareto_front_type &a, const point_type &p, const mapped_type m) {
      a.insert(std::make_pair(p,m));
    });

    pf.def("emplace", [](pareto_front_type &a, const point_type &p, const mapped_type m) {
      a.emplace(p,m);
    });

    pf.def("insert", [](pareto_front_type &a, const std::vector<number_type> &p, const mapped_type m) {
      a.insert(std::make_pair(point_type(p.begin(), p.end()),m));
    });

    pf.def("insert", [](pareto_front_type &a, const std::vector<value_type> &v) {
      a.insert(v.begin(), v.end());
    });

    pf.def("insert", [](pareto_front_type &a, std::initializer_list<value_type> v) {
      a.insert(v);
    });

    pf.def("insert", [](pareto_front_type &a, const std::vector<std::pair<std::vector<number_type>, mapped_type>> &v) {
        for (const auto& [k,m]: v) {
            a.insert(std::make_pair(point_type(k.begin(), k.end()),m));
        }
    });

    pf.def("insert", [](pareto_front_type &a, py::iterator &iter) {
      while (iter != py::iterator::sentinel()) {
          a.emplace(iter->template cast<value_type>());
          ++iter;
      }
    });

    pf.def("__setitem__", [](pareto_front_type &s, point_type p, mapped_type v) {
      s.erase(p);
      s.insert(std::make_pair(p, v));
    });

    pf.def("__setitem__", [](pareto_front_type &s, std::vector<number_type> p, mapped_type v) {
      s.erase(point_type(p));
      s.insert(std::make_pair(point_type(p), v));
    });

    // Erase item
    pf.def("erase", [](pareto_front_type &a, const value_type &p) {
      a.erase(p.first);
    });

    pf.def("erase", [](pareto_front_type &a, const point_type &p) {
      a.erase(p);
    });

    pf.def("erase", [](pareto_front_type &a, py::iterator &iter) {
      while (iter != py::iterator::sentinel()) {
          a.erase(iter->template cast<point_type>());
          ++iter;
      }
    });

    pf.def("__delitem__", [](pareto_front_type &s, point_type p) {
      s.erase(p);
    });

    pf.def("__delitem__", [](pareto_front_type &s, std::vector<number_type> p) {
      s.erase(point_type(p));
    });

    pf.def("clear", &pareto_front_type::clear);

    // Merge fronts
    pf.def("merge", [](pareto_front_type &a, pareto_front_type &b) {
      a.merge(b);
    });

    // Swap fronts
    pf.def("swap", [](pareto_front_type &a, pareto_front_type &b) {
      a.swap(b);
    });

    // Queries
    pf.def("find_intersection", [](const pareto_front_type &s, const point_type& min_corner, const point_type& max_corner) {
      return py::make_iterator(s.find_intersection(min_corner, max_corner), s.end());
    }, py::keep_alive<0, 1>());

    pf.def("get_intersection", [](const pareto_front_type &s, point_type min_corner, point_type max_corner) {
      return std::vector<value_type>(s.find_intersection(min_corner, max_corner), s.end());
    });

    pf.def("find_within", [](const pareto_front_type &s, const point_type& min_corner, const point_type& max_corner) {
      return py::make_iterator(s.find_within(min_corner, max_corner), s.end());
    }, py::keep_alive<0, 1>());

    pf.def("get_within", [](const pareto_front_type &s, point_type min_corner, point_type max_corner) {
      return std::vector<value_type>(s.find_within(min_corner, max_corner), s.end());
    });

    pf.def("find_disjoint", [](const pareto_front_type &s, const point_type& min_corner, const point_type& max_corner) {
      return py::make_iterator(s.find_disjoint(min_corner, max_corner), s.end());
    }, py::keep_alive<0, 1>());

    pf.def("get_disjoint", [](const pareto_front_type &s, point_type min_corner, point_type max_corner) {
      return std::vector<value_type>(s.find_disjoint(min_corner, max_corner), s.end());
    });

    pf.def("find_nearest", [](const pareto_front_type &s, const point_type& p) {
      return py::make_iterator(s.find_nearest(p), s.end());
    }, py::keep_alive<0, 1>());

    pf.def("get_nearest", [](const pareto_front_type &s, point_type p) {
      return *s.find_nearest(p);
    });

    pf.def("find_nearest_exclusive", [](const pareto_front_type &s, const point_type& p) {
      return py::make_iterator(s.find_nearest_exclusive(p), s.end());
    }, py::keep_alive<0, 1>());

    pf.def("get_nearest_exclusive", [](const pareto_front_type &s, point_type p) {
      return std::vector<value_type>(s.find_nearest_exclusive(p), s.end());
    });

    pf.def("find_nearest", [](const pareto_front_type &s, point_type p, size_t k) {
      return py::make_iterator(s.find_nearest(p, k), s.end());
    }, py::keep_alive<0, 1>());

    pf.def("get_nearest", [](const pareto_front_type &s, point_type p, size_t k) {
      return std::vector<value_type>(s.find_nearest(p, k), s.end());
    });

    // Compare fronts
    pf.def("dominates", [](const pareto_front_type &s, const point_type& reference) { return s.dominates(reference); });
    pf.def("dominates", [](const pareto_front_type &s, const pareto_front_type& reference) { return s.dominates(reference); });

    pf.def("strongly_dominates", [](const pareto_front_type &s, const point_type& reference) { return s.strongly_dominates(reference); });
    pf.def("strongly_dominates", [](const pareto_front_type &s, const pareto_front_type& reference) { return s.strongly_dominates(reference); });

    pf.def("non_dominates", [](const pareto_front_type &s, const point_type& reference) { return s.non_dominates(reference); });
    pf.def("non_dominates", [](const pareto_front_type &s, const pareto_front_type& reference) { return s.non_dominates(reference); });

    pf.def("is_partially_dominated_by", [](const pareto_front_type &s, const point_type& reference) { return s.is_partially_dominated_by(reference); });
    pf.def("is_partially_dominated_by", [](const pareto_front_type &s, const pareto_front_type& reference) { return s.is_partially_dominated_by(reference); });

    pf.def("is_completely_dominated_by", [](const pareto_front_type &s, const point_type& reference) { return s.is_completely_dominated_by(reference); });
    pf.def("is_completely_dominated_by", [](const pareto_front_type &s, const pareto_front_type& reference) { return s.is_completely_dominated_by(reference); });

    // Indicators
    pf.def("hypervolume", [](const pareto_front_type &s) { return s.hypervolume(); });
    pf.def("hypervolume", [](const pareto_front_type &s, const point_type& reference) { return s.hypervolume(reference); });
    pf.def("hypervolume", [](const pareto_front_type &s, const point_type& reference, size_t sample_size) { return s.hypervolume(reference, sample_size); });

    pf.def("coverage", &pareto_front_type::coverage);
    pf.def("coverage_ratio", &pareto_front_type::coverage_ratio);

    pf.def("gd", &pareto_front_type::gd);
    pf.def("std_gd", &pareto_front_type::std_gd);
    pf.def("igd", &pareto_front_type::igd);
    pf.def("std_igd", &pareto_front_type::std_igd);
    pf.def("hausdorff", &pareto_front_type::hausdorff);
    pf.def("igd_plus", &pareto_front_type::igd_plus);
    pf.def("std_igd_plus", &pareto_front_type::std_igd_plus);

    pf.def("uniformity", &pareto_front_type::uniformity);
    pf.def("average_distance", &pareto_front_type::average_distance);
    pf.def("average_nearest_distance", &pareto_front_type::average_nearest_distance);
    pf.def("crowding_distance",  py::overload_cast<const point_type&>(&pareto_front_type::crowding_distance));
    pf.def("average_crowding_distance", &pareto_front_type::average_crowding_distance);

    pf.def("direct_conflict", &pareto_front_type::direct_conflict);
    pf.def("normalized_direct_conflict", &pareto_front_type::normalized_direct_conflict);
    pf.def("direct_conflict", &pareto_front_type::direct_conflict);
    pf.def("normalized_direct_conflict", &pareto_front_type::normalized_direct_conflict);
    pf.def("maxmin_conflict", &pareto_front_type::maxmin_conflict);
    pf.def("normalized_maxmin_conflict", &pareto_front_type::normalized_maxmin_conflict);
    pf.def("conflict", &pareto_front_type::conflict);
    pf.def("normalized_conflict", &pareto_front_type::normalized_conflict);

    pf.def("__repr__",
           [](const pareto_front_type &a) {
             std::stringstream ss;
             ss << a;
             return ss.str();
           }
    );

    /////////////////////////////
    /// pareto archive object ///
    /////////////////////////////
    std::string class_name3;
    class_name3 = "archive" + std::to_string(N) + "d" + tag_to_string<TAG>();
    py::class_<pareto_archive_type> ar(m, class_name3.c_str());

    // Default constructor
    ar.def(py::init<>());

    // Define archive capacity
    ar.def(py::init<size_t>());

    // Define optimization direction
    ar.def(py::init<size_t, bool>());
    ar.def(py::init<size_t, std::initializer_list<bool>>());
    ar.def(py::init<size_t, const std::vector<uint8_t>&>());
    ar.def(py::init<size_t, const std::array<uint8_t, N>&>());

    // Bulk insert initial values
    ar.def(py::init<size_t, std::initializer_list<value_type>>());
    ar.def(py::init<size_t, std::initializer_list<value_type>,bool>());
    ar.def(py::init<size_t, std::initializer_list<value_type>,std::vector<uint8_t>>());
    ar.def(py::init<size_t, std::initializer_list<value_type>,std::initializer_list<bool>>());
    ar.def(py::init<size_t, const std::vector<value_type>&>());
    ar.def(py::init<size_t, const std::vector<value_type>&,bool>());
    ar.def(py::init<size_t, const std::vector<value_type>&,std::vector<uint8_t>>());
    ar.def(py::init<size_t, const std::vector<value_type>&,std::initializer_list<bool>>());
    ar.def(py::init([](size_t max_size, py::iterator &iter) {
      auto p = new pareto_archive_type(max_size);
      while (iter != py::iterator::sentinel()) {
          p->emplace(iter->template cast<value_type>());
          ++iter;
      }
      return p;
    }));

    ar.def(py::init([](size_t max_size, py::iterator &iter, bool is_minimization) {
      auto p = new pareto_archive_type(max_size, is_minimization);
      while (iter != py::iterator::sentinel()) {
          p->emplace(iter->template cast<value_type>());
          ++iter;
      }
      return p;
    }));

    ar.def(py::init([](size_t max_size, py::iterator &iter, const std::vector<uint8_t>& is_minimization) {
      auto p = new pareto_archive_type(max_size, is_minimization);
      while (iter != py::iterator::sentinel()) {
          p->emplace(iter->template cast<value_type>());
          ++iter;
      }
      return p;
    }));

    ar.def(py::init([](size_t max_size, py::iterator &iter, std::initializer_list<bool> is_minimization) {
      auto p = new pareto_archive_type(max_size, is_minimization);
      while (iter != py::iterator::sentinel()) {
          p->emplace(iter->template cast<value_type>());
          ++iter;
      }
      return p;
    }));

    // Copy/move constructors
    ar.def(py::init<const pareto_archive_type &>());

    // Helper functions to construct appropriate point types in python
    ar.def("point", [](const pareto_archive_type &s) {
        return point_type(s.dimensions());
    });

    // the used parameter needs to be here to generate the proper binding in the object: ar.point(...)
    ar.def("point", [](const pareto_archive_type &, const std::vector<double> &v) {
        return point_type(v.begin(), v.end());
    });

    // the used parameter needs to be here to generate the proper binding in the object: ar.point(...)
    ar.def("point", [](const pareto_archive_type &, std::initializer_list<double> v) {
        return point_type(v);
    });

    // the used parameter needs to be here to generate the proper binding in the object: ar.point(...)
    ar.def("point", [](const pareto_archive_type &, const point_type &p) {
        return point_type(p);
    });

    if constexpr (N != 0) {
        // the used parameter needs to be here to generate the proper binding in the object: ar.point(...)
        ar.def("point", [](const pareto_archive_type &, const point<number_type, 0> &p2) {
            return point_type(p2);
        });
    }

    // Iterators
    ar.def("__iter__", [](const pareto_archive_type &s) {
      return py::make_iterator(s.begin(), s.end());
    }, py::keep_alive<0, 1>());

    ar.def("__iter__", [](pareto_archive_type &s) {
      return py::make_iterator(s.begin(), s.end());
    }, py::keep_alive<0, 1>());

    ar.def("fronts", [](const pareto_archive_type &s) {
      return py::make_iterator(s.begin_front(), s.end_front());
    }, py::keep_alive<0, 1>());

    ar.def("fronts", [](pareto_archive_type &s) {
      return py::make_iterator(s.begin_front(), s.end_front());
    }, py::keep_alive<0, 1>());

    ar.def("__reversed__", [](const pareto_archive_type &s) {
      return py::make_iterator(s.rbegin(), s.rend());
    }, py::keep_alive<0, 1>());

    ar.def("__reversed__", [](pareto_archive_type &s) {
      return py::make_iterator(s.rbegin(), s.rend());
    }, py::keep_alive<0, 1>());

    // Non-modifying functions
    ar.def("empty", &pareto_archive_type::empty);

    ar.def("size", &pareto_archive_type::size);
    ar.def("__len__", &pareto_archive_type::size);

    ar.def("dimensions", [](const pareto_archive_type& a) {
      return a.dimensions();
    });
    ar.def("dimensions", [](pareto_archive_type& a, size_t s) {
      a.dimensions(s);
    });

    ar.def("is_minimization", [](const pareto_archive_type& a) {
      return a.is_minimization();
    });

    ar.def("is_minimization", [](const pareto_archive_type& a, size_t index) {
      return a.is_minimization(index);
    });

    ar.def("is_maximization", [](const pareto_archive_type& a) {
      return a.is_maximization();
    });

    ar.def("is_maximization", [](const pareto_archive_type& a, size_t index) {
      return a.is_maximization(index);
    });

    // Reference points
    ar.def("ideal", [](const pareto_archive_type& a, size_t index) {
      return a.ideal(index);
    });

    ar.def("ideal", [](const pareto_archive_type& a) {
      return a.ideal();
    });

    ar.def("dimension_ideal", [](const pareto_archive_type &s, size_t d) {
      auto it = s.dimension_ideal(d);
      if (it != s.end()) {
          return *it;
      } else {
          return value_type();
      }
    });

    ar.def("nadir", [](const pareto_archive_type& a, size_t index) {
      return a.nadir(index);
    });

    ar.def("nadir", [](const pareto_archive_type& a) {
      return a.nadir();
    });

    ar.def("dimension_nadir", [](const pareto_archive_type &s, size_t d) {
      auto it = s.dimension_nadir(d);
      if (it != s.end()) {
          return *it;
      } else {
          return value_type();
      }
    });

    ar.def("worst", [](const pareto_archive_type& a, size_t index) {
      return a.worst(index);
    });

    ar.def("worst", [](const pareto_archive_type& a) {
      return a.worst();
    });

    ar.def("dimension_worst", [](const pareto_archive_type &s, size_t d) {
      auto it = s.dimension_worst(d);
      if (it != s.end()) {
          return *it;
      } else {
          return value_type();
      }
    });

    // Search
    ar.def("find", [](const pareto_archive_type &s, point_type p) { return py::make_iterator(s.find(p), s.end());}, py::keep_alive<0, 1>());

    ar.def("find", [](pareto_archive_type &s, point_type p) { return py::make_iterator(s.find(p), s.end());}, py::keep_alive<0, 1>());

    ar.def("contains", [](const pareto_archive_type &s, point_type p) { return s.find(p) != s.end(); });

    ar.def("contains", [](pareto_archive_type &s, point_type p) { return s.find(p) != s.end(); });

    ar.def("__contains__", [](const pareto_archive_type &s, point_type p) { return s.find(p) != s.end(); });

    ar.def("__contains__", [](pareto_archive_type &s, point_type p) { return s.find(p) != s.end(); });

    ar.def("get", [](const pareto_archive_type &s, point_type p) {
      auto it = s.find(p);
      if (it != s.end()){
          return *it;
      } else {
          throw std::invalid_argument("Element is not is the pareto archive");
      }});

    ar.def("__getitem__", [](const pareto_archive_type &s, point_type p) {
      auto it = s.find(p);
      if (it != s.end()) {
          return it->second;
      } else {
          throw std::invalid_argument("Element is not is the pareto archive");
      }
    });

    ar.def("__getitem__", [](const pareto_archive_type &s, std::initializer_list<number_type> p) {
      auto it = s.find(point_type(p));
      if (it != s.end()) {
          return it->second;
      } else {
          throw std::invalid_argument("Element is not is the pareto archive");
      }
    });

    ar.def("__getitem__", [](const pareto_archive_type &s, const std::vector<number_type>& v) {
      auto it = s.find(point_type(v.begin(), v.end()));
      if (it != s.end()) {
          return it->second;
      } else {
          throw std::invalid_argument("Element is not is the pareto archive");
      }
    });

//    ar.def("__getitem__", [](const pareto_archive_type &s, const py::object& o) {
//        std::cout << "getting item?" << o << std::endl;
//    });

    // Operators
    ar.def(py::self == py::self);
    ar.def(py::self != py::self);

    // Insert item dictionary
    ar.def("insert", [](pareto_archive_type &a, const value_type &v) {
      a.insert(v);
    });

    ar.def("insert", [](pareto_archive_type &a, const point_type &p, const mapped_type m) {
      a.insert(std::make_pair(p,m));
    });

    ar.def("emplace", [](pareto_archive_type &a, const point_type &p, const mapped_type m) {
      a.emplace(p,m);
    });

    ar.def("insert", [](pareto_archive_type &a, const std::vector<number_type> &p, const mapped_type m) {
      a.insert(std::make_pair(point_type(p.begin(), p.end()),m));
    });

    ar.def("insert", [](pareto_archive_type &a, const std::vector<value_type> &v) {
      a.insert(v.begin(), v.end());
    });

    ar.def("insert", [](pareto_archive_type &a, std::initializer_list<value_type> v) {
      a.insert(v);
    });

    ar.def("insert", [](pareto_archive_type &a, const std::vector<std::pair<std::vector<number_type>, mapped_type>> &v) {
        for (const auto& [k,m]: v) {
            a.insert(std::make_pair(point_type(k.begin(), k.end()),m));
        }
    });

    ar.def("insert", [](pareto_archive_type &a, py::iterator &iter) {
      while (iter != py::iterator::sentinel()) {
          a.emplace(iter->template cast<value_type>());
          ++iter;
      }
    });

    ar.def("__setitem__", [](pareto_archive_type &s, point_type p, mapped_type v) {
      s.erase(p);
      s.insert(std::make_pair(p, v));
    });

    ar.def("__setitem__", [](pareto_archive_type &s, std::vector<number_type> p, mapped_type v) {
      s.erase(point_type(p));
      s.insert(std::make_pair(point_type(p), v));
    });

    // Erase item
    ar.def("erase", [](pareto_archive_type &a, const value_type &p) {
      a.erase(p.first);
    });

    ar.def("erase", [](pareto_archive_type &a, const point_type &p) {
      a.erase(p);
    });

    ar.def("erase", [](pareto_archive_type &a, py::iterator &iter) {
      while (iter != py::iterator::sentinel()) {
          a.erase(iter->template cast<point_type>());
          ++iter;
      }
    });

    ar.def("__delitem__", [](pareto_archive_type &s, point_type p) {
      s.erase(p);
    });

    ar.def("__delitem__", [](pareto_archive_type &s, std::vector<number_type> p) {
      s.erase(point_type(p));
    });

    ar.def("resize", &pareto_archive_type::resize);

    ar.def("clear", &pareto_archive_type::clear);

    // Merge archives
    ar.def("merge", [](pareto_archive_type &a, pareto_archive_type &b) {
      a.merge(b);
    });

    // Swap archives
    ar.def("swap", [](pareto_archive_type &a, pareto_archive_type &b) {
      a.swap(b);
    });

    // Queries
    ar.def("find_intersection", [](const pareto_archive_type &s, const point_type& min_corner, const point_type& max_corner) {
      return py::make_iterator(s.find_intersection(min_corner, max_corner), s.end());
    }, py::keep_alive<0, 1>());

    ar.def("get_intersection", [](const pareto_archive_type &s, point_type min_corner, point_type max_corner) {
      return std::vector<value_type>(s.find_intersection(min_corner, max_corner), s.end());
    });

    ar.def("find_within", [](const pareto_archive_type &s, const point_type& min_corner, const point_type& max_corner) {
      return py::make_iterator(s.find_within(min_corner, max_corner), s.end());
    }, py::keep_alive<0, 1>());

    ar.def("get_within", [](const pareto_archive_type &s, point_type min_corner, point_type max_corner) {
      return std::vector<value_type>(s.find_within(min_corner, max_corner), s.end());
    });

    ar.def("find_disjoint", [](const pareto_archive_type &s, const point_type& min_corner, const point_type& max_corner) {
      return py::make_iterator(s.find_disjoint(min_corner, max_corner), s.end());
    }, py::keep_alive<0, 1>());

    ar.def("get_disjoint", [](const pareto_archive_type &s, point_type min_corner, point_type max_corner) {
      return std::vector<value_type>(s.find_disjoint(min_corner, max_corner), s.end());
    });

    ar.def("find_nearest", [](const pareto_archive_type &s, const point_type& p) {
      return py::make_iterator(s.find_nearest(p), s.end());
    }, py::keep_alive<0, 1>());

    ar.def("get_nearest", [](const pareto_archive_type &s, point_type p) {
      return *s.find_nearest(p);
    });

    ar.def("find_nearest_exclusive", [](const pareto_archive_type &s, const point_type& p) {
      return py::make_iterator(s.find_nearest_exclusive(p), s.end());
    }, py::keep_alive<0, 1>());

    ar.def("get_nearest_exclusive", [](const pareto_archive_type &s, point_type p) {
      return std::vector<value_type>(s.find_nearest_exclusive(p), s.end());
    });

    ar.def("find_nearest", [](const pareto_archive_type &s, point_type p, size_t k) {
      return py::make_iterator(s.find_nearest(p, k), s.end());
    }, py::keep_alive<0, 1>());

    ar.def("get_nearest", [](const pareto_archive_type &s, point_type p, size_t k) {
      return std::vector<value_type>(s.find_nearest(p, k), s.end());
    });

    // Compare archives
    ar.def("dominates", [](const pareto_archive_type &s, const point_type& reference) { return s.dominates(reference); });
    ar.def("dominates", [](const pareto_archive_type &s, const pareto_archive_type& reference) { return s.dominates(reference); });

    ar.def("strongly_dominates", [](const pareto_archive_type &s, const point_type& reference) { return s.strongly_dominates(reference); });
    ar.def("strongly_dominates", [](const pareto_archive_type &s, const pareto_archive_type& reference) { return s.strongly_dominates(reference); });

    ar.def("non_dominates", [](const pareto_archive_type &s, const point_type& reference) { return s.non_dominates(reference); });
    ar.def("non_dominates", [](const pareto_archive_type &s, const pareto_archive_type& reference) { return s.non_dominates(reference); });

    ar.def("is_partially_dominated_by", [](const pareto_archive_type &s, const point_type& reference) { return s.is_partially_dominated_by(reference); });
    ar.def("is_partially_dominated_by", [](const pareto_archive_type &s, const pareto_archive_type& reference) { return s.is_partially_dominated_by(reference); });

    ar.def("is_completely_dominated_by", [](const pareto_archive_type &s, const point_type& reference) { return s.is_completely_dominated_by(reference); });
    ar.def("is_completely_dominated_by", [](const pareto_archive_type &s, const pareto_archive_type& reference) { return s.is_completely_dominated_by(reference); });

    // Indicators
    ar.def("hypervolume", [](const pareto_archive_type &s) { return s.hypervolume(); });
    ar.def("hypervolume", [](const pareto_archive_type &s, const point_type& reference) { return s.hypervolume(reference); });
    ar.def("hypervolume", [](const pareto_archive_type &s, const point_type& reference, size_t sample_size) { return s.hypervolume(reference, sample_size); });

    ar.def("coverage",  [](const pareto_archive_type &a, const pareto_front_type& b) {
      return a.coverage(b);
    });
    ar.def("coverage",  [](const pareto_archive_type &a, const pareto_archive_type& b) {
      return a.coverage(b);
    });

    ar.def("coverage_ratio",  [](const pareto_archive_type &a, const pareto_front_type& b) {
      return a.coverage_ratio(b);
    });
    ar.def("coverage_ratio",  [](const pareto_archive_type &a, const pareto_archive_type& b) {
      return a.coverage_ratio(b);
    });

    ar.def("gd",  [](const pareto_archive_type &a, const pareto_front_type& b) {
      return a.gd(b);
    });
    ar.def("gd",  [](const pareto_archive_type &a, const pareto_archive_type& b) {
      return a.gd(b);
    });

    ar.def("std_gd",  [](const pareto_archive_type &a, const pareto_front_type& b) {
      return a.std_gd(b);
    });
    ar.def("std_gd",  [](const pareto_archive_type &a, const pareto_archive_type& b) {
      return a.std_gd(b);
    });

    ar.def("igd",  [](const pareto_archive_type &a, const pareto_front_type& b) {
      return a.igd(b);
    });
    ar.def("igd",  [](const pareto_archive_type &a, const pareto_archive_type& b) {
      return a.igd(b);
    });

    ar.def("std_igd",  [](const pareto_archive_type &a, const pareto_front_type& b) {
      return a.std_igd(b);
    });
    ar.def("std_igd",  [](const pareto_archive_type &a, const pareto_archive_type& b) {
      return a.std_igd(b);
    });

    ar.def("hausdorff",  [](const pareto_archive_type &a, const pareto_front_type& b) {
      return a.hausdorff(b);
    });
    ar.def("hausdorff",  [](const pareto_archive_type &a, const pareto_archive_type& b) {
      return a.hausdorff(b);
    });

    ar.def("igd_plus",  [](const pareto_archive_type &a, const pareto_front_type& b) {
      return a.igd_plus(b);
    });
    ar.def("igd_plus",  [](const pareto_archive_type &a, const pareto_archive_type& b) {
      return a.igd_plus(b);
    });

    ar.def("std_igd_plus",  [](const pareto_archive_type &a, const pareto_front_type& b) {
      return a.std_igd_plus(b);
    });
    ar.def("std_igd_plus",  [](const pareto_archive_type &a, const pareto_archive_type& b) {
      return a.std_igd_plus(b);
    });

    ar.def("uniformity", &pareto_archive_type::uniformity);
    ar.def("average_distance", &pareto_archive_type::average_distance);
    ar.def("average_nearest_distance", &pareto_archive_type::average_nearest_distance);
    ar.def("crowding_distance",  py::overload_cast<const point_type&>(&pareto_archive_type::crowding_distance));
    ar.def("average_crowding_distance", &pareto_archive_type::average_crowding_distance);

    ar.def("direct_conflict", &pareto_archive_type::direct_conflict);
    ar.def("normalized_direct_conflict", &pareto_archive_type::normalized_direct_conflict);
    ar.def("direct_conflict", &pareto_archive_type::direct_conflict);
    ar.def("normalized_direct_conflict", &pareto_archive_type::normalized_direct_conflict);
    ar.def("maxmin_conflict", &pareto_archive_type::maxmin_conflict);
    ar.def("normalized_maxmin_conflict", &pareto_archive_type::normalized_maxmin_conflict);
    ar.def("conflict", &pareto_archive_type::conflict);
    ar.def("normalized_conflict", &pareto_archive_type::normalized_conflict);

    ar.def("__repr__",
           [](const pareto_archive_type &a) {
             std::stringstream ss;
             ss << a;
             return ss.str();
           }
    );
}

// This is an automatically generated file that will extern all these templates
// We have to do it automatically because we don't know how many templates we
// need to extern
// extern template void binding_for_N_dimensional<10,pareto::r_star_tree_tag>();
#include <binding_n_dimensional_extern_templates.h>

#endif