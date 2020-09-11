#include "binding_n_dimensional.h"

/// Max number of dimensions for the front
/// Anything beyond 10 (default) dimensions will have the dimension determined at runtime
constexpr size_t max_num_dimensions = MAX_NUM_DIMENSIONS_PYTHON;
constexpr bool boost_rtree_is_deprecated = true;

/// Create bindings for front with n = 1 dimensions
/// i.e., create bindings for pareto_front<double, n, object>
///       create bindings for archive<double, n, object>
template<size_t n = max_num_dimensions, class module_t>
std::enable_if_t<0 == n, void>
binding_for_all_dimensions(module_t &m) {
    using namespace pareto;
    // Always create bindings for vector and kd trees
    binding_for_N_dimensional<n, vector_tree_tag>(m,true);
    binding_for_N_dimensional<n, kd_tree_tag>(m,false);

    // Maybe create bindings for other data structures
#ifdef BUILD_BINDING_FOR_ALL_STRUCTURES
    binding_for_N_dimensional<n, quad_tree_tag>(m, false);
    // boost trees do not work with 0 dimensions
    binding_for_N_dimensional<n, r_tree_tag>(m, false);
    binding_for_N_dimensional<n, r_star_tree_tag>(m, false);
#endif

    // Create alias for the default data structure
    // The default data structure is front0d instead of front0dDATASTRUCTURE
    std::string default_class_name = "front" + std::to_string(n) + "d";
    std::string registered_name = "front" + std::to_string(n) + "d";
    if constexpr (n == 1) {
        registered_name += tag_to_string<vector_tree_tag>();
    } else {
        registered_name += tag_to_string<kd_tree_tag>();
    }
    m.attr(default_class_name.c_str()) = m.attr(registered_name.c_str());
}

/// Create bindings for front with n > 1 dimensions
/// Create bindings for pareto_front<double, n, py_object>,
/// pareto_front<double, n-1, py_object>,
/// pareto_front<double, n-2, py_object>,
/// ...
/// pareto_front<double, 0, py_object>
template<size_t n = max_num_dimensions, class module_t>
std::enable_if_t<0 < n, void>
binding_for_all_dimensions(module_t &m) {
    using namespace pareto;
    // Always create bindings for vector and kd trees
    binding_for_N_dimensional<n, vector_tree_tag>(m,true);
    binding_for_N_dimensional<n, kd_tree_tag>(m,false);

    // Maybe create bindings for other data structures
#ifdef BUILD_BINDING_FOR_ALL_STRUCTURES
    binding_for_N_dimensional<n, quad_tree_tag>(m,false);
    if constexpr (!boost_rtree_is_deprecated) {
        binding_for_N_dimensional<n, boost_tree_tag>(m,false);
    }
    binding_for_N_dimensional<n, r_tree_tag>(m,false);
    binding_for_N_dimensional<n, r_star_tree_tag>(m,false);
#endif

    // Create aliases for the default data structure
    // The default data structure is frontNd instead of frontNdDATASTRUCTURE
    std::string default_class_name = "front" + std::to_string(n) + "d";
    std::string registered_name = "front" + std::to_string(n) + "d";
    if constexpr (n == 1) {
        registered_name += tag_to_string<vector_tree_tag>();
    } else {
        registered_name += tag_to_string<kd_tree_tag>();
    }
    m.attr(default_class_name.c_str()) = m.attr(registered_name.c_str());

    // Create bindings for lower dimensions n-1, n-2, ...
    binding_for_all_dimensions<n - 1>(m);
}

/// Return a py object with a front with n = 0 dimensions
/// py::object with pareto_front<double, 0, py_object>
template<size_t n = max_num_dimensions,
         template<typename, size_t, typename, typename> class FRONT_OR_ARCHIVE,
         class... Args>
std::enable_if_t<0 == n, py::object>
cast_for_dimension(std::string tag, size_t d, Args &&... args) {
    using namespace pareto;
    if (d == n) {
        if (tag.empty() || tag == "default") {
            py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, kd_tree_tag>(args...));
            return obj;
        } else if (tag == "list") {
            py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, vector_tree_tag>(args...));
            return obj;
        } else if (tag == "kdtree") {
            py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, kd_tree_tag>(args...));
            return obj;
#ifdef BUILD_BINDING_FOR_ALL_STRUCTURES
        } else if (tag == "quadtree") {
            py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, quad_tree_tag>(args...));
            return obj;
        } else if (tag == "boostrtree") {
            if constexpr (boost_rtree_is_deprecated) {
                // Boost rtrees are very inefficient for some operations
                // They also rely on an operator== that is deprecated for py::object
                // The only reason we implemented it is for benchmarks
                std::cerr << "The tag 'boostrtree' is deprecated. Using 'rtree' instead." << std::endl;
                py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, r_tree_tag>(args...));
                return obj;
            } else {
                py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, boost_tree_tag>(args...));
                return obj;
            }
        } else if (tag == "rtree") {
            py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, r_tree_tag>(args...));
            return obj;
        } else if (tag == "rstartree") {
            py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, r_star_tree_tag>(args...));
            return obj;
#endif
        }
#ifdef BUILD_BINDING_FOR_ALL_STRUCTURES
        throw std::invalid_argument("Invalid data structure tag. Valid tags are '', 'default', 'list', 'quadtree', 'kdtree', 'boostrtree', 'rtree', or 'rstartree'");
#else
        throw std::invalid_argument("Invalid data structure tag. Valid tags are '', 'default', 'list', or 'kdtree'");
#endif
    } else {
        // Create a front/archive with dimension defined at runtime
        auto runtime_front = FRONT_OR_ARCHIVE<double, 0, py::object, kd_tree_tag>(args...);
        // set the runtime dimension
        runtime_front.dimensions(d);
        // create python object
        py::object obj = py::cast(runtime_front);
        return obj;
    }
}

/// Create a py object with a front with n > 0 dimensions
/// py::object with pareto_front<double, n, py_object>
template<size_t n = max_num_dimensions,
         template<typename, size_t, typename, typename> class FRONT_OR_ARCHIVE,
         class... Args>
std::enable_if_t<0 < n, py::object>
cast_for_dimension(std::string tag, size_t d, Args &&... args) {
    using namespace pareto;
    if (d == n) {
        if (tag.empty() || tag == "default") {
            if constexpr (n == 1) {
                py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, vector_tree_tag>(args...));
                return obj;
            } else {
                py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, kd_tree_tag>(args...));
                return obj;
            }
        } else if (tag == "list") {
            py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, vector_tree_tag>(args...));
            return obj;
        } else if (tag == "kdtree") {
            py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, kd_tree_tag>(args...));
            return obj;
#ifdef BUILD_BINDING_FOR_ALL_STRUCTURES
        } else if (tag == "quadtree") {
            py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, quad_tree_tag>(args...));
            return obj;
        } else if (tag == "boostrtree") {
            if constexpr (boost_rtree_is_deprecated) {
                // Boost rtrees are VERY inefficient for some operations
                // The only reason we implemented it is for benchmarks
                std::cerr << "The tag 'boostrtree' is deprecated. Using 'rtree' instead." << std::endl;
                py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, r_tree_tag>(args...));
                return obj;
            } else {
                py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, boost_tree_tag>(args...));
                return obj;
            }
        } else if (tag == "rtree") {
            py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, r_tree_tag>(args...));
            return obj;
        } else if (tag == "rstartree") {
            py::object obj = py::cast(FRONT_OR_ARCHIVE<double, n, py::object, r_star_tree_tag>(args...));
            return obj;
#endif
        }
#ifdef BUILD_BINDING_FOR_ALL_STRUCTURES
        throw std::invalid_argument("Invalid data structure tag. Valid tags are '', 'default', 'list', 'quadtree', 'kdtree', 'boostrtree', 'rtree', or 'rstartree'");
#else
        throw std::invalid_argument("Invalid data structure tag. Valid tags are '', 'default', 'list', or 'kdtree'");
#endif
    } else {
        return cast_for_dimension<n-1, FRONT_OR_ARCHIVE>(tag, d, args...);
    }
}

/// Create modules
PYBIND11_MODULE(pyfront, m) {
    using namespace py::literals;
    using namespace pareto;
    m.doc() = "A container to maintain and query multi-dimensional Pareto fronts and archives efficiently";

    /// Create bindings for n, n-1, n-2, n-3, ..., 3, 2, 1 dimensions
    binding_for_all_dimensions<max_num_dimensions>(m);

    /// Create function that returns a py object front with n dimensions
    m.def("front", [](size_t dimensions) {
        return cast_for_dimension<max_num_dimensions, front>("", dimensions);
    });
    m.def("front", [](size_t dimensions, bool b){
        return cast_for_dimension<max_num_dimensions, front>("", dimensions, b);
    });
    m.def("front", [](std::initializer_list<bool> b){
        return cast_for_dimension<max_num_dimensions, front>("", b.size(), b);
    });
    m.def("front", [](const std::vector<uint8_t>& v){
        return cast_for_dimension<max_num_dimensions, front>("", v.size(), v);
    });
    m.def("front", [](const std::vector<std::string>& str){
        std::vector<uint8_t> v(str.size());
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == "minimization" || str[i] == "min") {
                v[i] = 0x01;
            } else if (str[i] == "maximization" || str[i] == "max") {
                v[i] = 0x00;
            } else {
                throw std::invalid_argument("Invalid tag. The valid keywords are 'minimization', 'min', 'maximization', or 'max'");
            }
        }
        return cast_for_dimension<max_num_dimensions, front>("", v.size(), v);
    });

    /// Function that returns a py object front with n dimensions
    /// The user can choose the data structure with a tag
    m.def("front", [](std::string tag, size_t dimensions) {
        return cast_for_dimension<max_num_dimensions, front>(tag, dimensions);
    });
    m.def("front", [](std::string tag, size_t dimensions, bool b){
        return cast_for_dimension<max_num_dimensions, front>(tag, dimensions, b);
    });
    m.def("front", [](std::string tag, std::initializer_list<bool> b){
        return cast_for_dimension<max_num_dimensions, front>(tag, b.size(), b);
    });
    m.def("front", [](std::string tag, const std::vector<uint8_t>& v){
        return cast_for_dimension<max_num_dimensions, front>(tag, v.size(), v);
    });
    m.def("front", [](std::string tag, const std::vector<std::string>& str){
        std::vector<uint8_t> v(str.size());
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == "minimization" || str[i] == "min") {
                v[i] = 0x01;
            } else if (str[i] == "maximization" || str[i] == "max") {
                v[i] = 0x00;
            } else {
                throw std::invalid_argument("Invalid tag. The valid keywords are 'minimization', 'min', 'maximization', or 'max'");
            }
        }
        return cast_for_dimension<max_num_dimensions, front>(tag, v.size(), v);
    });

    /// Function that returns a py object archive with n dimensions
    m.def("archive", [](size_t dimensions) {
        return cast_for_dimension<max_num_dimensions, archive>("", dimensions);
    });
    m.def("archive", [](size_t dimensions, size_t max_size) {
        return cast_for_dimension<max_num_dimensions, archive>("", dimensions, max_size);
    });
    m.def("archive", [](size_t dimensions, bool b, size_t max_size) {
        return cast_for_dimension<max_num_dimensions, archive>("", dimensions, max_size, b);
    });
    m.def("archive", [](std::initializer_list<bool> b, size_t max_size) {
        return cast_for_dimension<max_num_dimensions, archive>("", b.size(), max_size, b);
    });
    m.def("archive", [](const std::vector<uint8_t>& v, size_t max_size) {
        return cast_for_dimension<max_num_dimensions, archive>("", v.size(), max_size, v);
    });
    m.def("archive", [](const std::vector<std::string>& str, size_t max_size) {
        std::vector<uint8_t> v(str.size());
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == "minimization" || str[i] == "min") {
                v[i] = 0x01;
            } else if (str[i] == "maximization" || str[i] == "max") {
                v[i] = 0x00;
            } else {
                throw std::invalid_argument("Invalid tag. The valid keywords are 'minimization', 'min', 'maximization', or 'max'");
            }
        }
        return cast_for_dimension<max_num_dimensions, archive>("", v.size(), max_size, v);
    });

    /// Function that returns a py object archive with n dimensions
    /// The user can choose the data structure with a tag
    m.def("archive", [](std::string tag, size_t dimensions, size_t max_size) {
        return cast_for_dimension<max_num_dimensions, archive>(tag, dimensions, max_size);
    });
    m.def("archive", [](std::string tag, size_t dimensions) {
        return cast_for_dimension<max_num_dimensions, archive>(tag, dimensions);
    });
    m.def("archive", [](std::string tag, size_t dimensions, bool b, size_t max_size) {
        return cast_for_dimension<max_num_dimensions, archive>(tag, dimensions, max_size, b);
    });

    m.def("archive", [](std::string tag, std::initializer_list<bool> b, size_t max_size) {
        return cast_for_dimension<max_num_dimensions, archive>(tag, b.size(), max_size, b);
    });

    m.def("archive", [](std::string tag, const std::vector<uint8_t>& v, size_t max_size) {
        return cast_for_dimension<max_num_dimensions, archive>(tag, v.size(), max_size, v);
    });
    m.def("archive", [](std::string tag, const std::vector<std::string>& str, size_t max_size) {
        std::vector<uint8_t> v(str.size());
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == "minimization" || str[i] == "min") {
                v[i] = 0x01;
            } else if (str[i] == "maximization" || str[i] == "max") {
                v[i] = 0x00;
            } else {
                throw std::invalid_argument("Invalid tag. The valid keywords are 'minimization', 'min', 'maximization', or 'max'");
            }
        }
        return cast_for_dimension<max_num_dimensions, archive>(tag, v.size(), max_size, v);
    });
}