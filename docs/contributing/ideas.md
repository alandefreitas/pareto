---
layout: default
title: Ideas
nav_order: 1
has_children: false
parent: Contributing
has_toc: false
---
# Ideas

<img src="https://www.nicepng.com/png/detail/388-3881755_we-want-you-developer-clipart-youtube-uncle-sam.png" width="40%">.

If you're looking for an interesting project to contribute, look no more. Here are some cool features that might improve this project and help humanity:

* Spatial Containers
    * Features
        * Heteregenous keys
        * `max_dist` predicate
        * `spatial_set`
    * Performance
        * Make `erase` not invalidate iterators
        * Make use hints
        * Making runtime dimensions competitive with compile-time dimensions
        * Avoid unnecessary copies in general
        * Keep vectors sorted when $m < 3$
    * Code quality
        * Remove redundancies in container code
        * Avoid copies in insertion and removal algorithms 
        * Implement `comp_` for predicates and unit tests
        * Deprecate `find_*` in favour of `find(predicate_list)` only
        * Implement node handles `node_type`
* Front Container
    * More indicators
    * Special dominance relationships like $\epsilon$-dominance and cone-$\epsilon$-dominance
* Archives Container
    * Improve performance of archive queries
    * Let archive iterators move logically to next fronts
* Python bindings
    * Replicate Matplot++ examples with Matplotlib
    * Integrate scikit-build <sup>[ [1](https://scikit-build.readthedocs.io/en/latest/installation.html), [2](https://github.com/pybind/cmake_example), [3](https://github.com/pybind/scikit_build_example) ]</sup>
* Examples
    * Adjust the Matplot++ examples to receive `axes_handle` as parameter
    * More interesting examples

Contributions in which we are not interested:

* "I don't like this optional feature so I removed/deprecated it"
* "I removed this feature to support older versions of C++" but have not provided an equivalent alternative
* "I removed this feature so I don't have to update CMake" but have not provided an equivalent alternative
* "I'm creating this high-cost promise that we'll support ________ forever" but I'm not sticking around
* In doubt, please open a [discussion](https://github.com/alandefreitas/pareto/discussions) first



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
