# Pareto Front Container (C++ / Python)

This is container / data structure to maintain and query multi-dimensional pareto fronts with its most expensive operations in O(n log n) time. Whenever a point dominates the other, the dominated point is automatically removed.

Besides the usual container functions, it includes the functions:

* `dominates`: check if a point is dominated by the front
* `find_intersection`: find points inside a hyperbox
* `find_nearest`: find `k` nearest point to a point 
* `hypervolume`: calculate the hypervolume
* `ideal`: ideal point
* `merge`: merge pareto fronts
* `nadir`: nadir point
* `worst`: worst point

# Example

Python:

```python
import pyfront
from pyfront import randn, randi

pf = pyfront.front2d([[[0.2, 0.5], 7], [[-0.5, 2.5], 8]])

# Insert some more points
for i in range(0, 40):
    pf.insert([randn(), randn()], randi())

# Show pareto front
print(pf)
for [point, value] in pf:
    print('point:', point, ': value:', value, end=', ')
print('\n')

# Get some properties
print('size:', pf.size())
print('hypervolume:', pf.hypervolume())
print('ideal:', pf.ideal())
print('nadir:', pf.nadir())
print('worst:', pf.worst())
print('')

# Check if the set dominates a point
print('pf.dominates([5.5, 5.5]):', pf.dominates([5.5, 5.5]))
print('pf.dominates([-5.5, -5.5]):', pf.dominates([-5.5, -5.5]))
print('')

# Show points in the box {0,0} to {5,5}
for [point, value] in pf.find_intersection([0, 0], [5, 5]):
    print(point,":",value, end=', ')
print('\n')

# Show point nearest to {2.5,2.5}
for point_and_value in pf.find_nearest([2.5, 2.5]):
    print(point_and_value)
print('')

# Show 5 points nearest to {2.5,2.5}
print(pf.get_nearest([2.5, 2.5], 5))
print('')

# Remove point closest to 0.0
print(pf.get_nearest([0.0,0.0]))
pf.erase(pf.find_nearest([0.0,0.0]))

# Remove the next closest
it = pf.find_nearest([0.0,0.0])
pf.erase(it)

# Show final closest
print(pf.get_nearest([0.0,0.0]))
```

C++:

```cpp
#include <iostream>
#include <random>
#include <chrono>
#include <pareto_front.h>

// Random integers
unsigned randi();
// Random doubles
double randn();

int main() {
    using namespace pareto_front;

    // Create a 2 dimensional pareto front with two elements
    // The the "objective values" are double
    // The mapped element associated with a point is unsigned
    using pareto_front_t = pareto_front<double, 2, unsigned>;
    pareto_front_t pf = {{{0.2,  0.5}, 7},
                         {{-0.5, 2.5}, 8}};

    // Insert some more points
    for (size_t i = 0; i < 40; ++i) {
        // The easy way...
        pf.insert({{randn(), randn()},randi()});

        // Or the long way...
        pareto_front_t::key_type k = {randn(), randn()};
        pareto_front_t::mapped_type v = randi();
        auto p = std::make_pair(k, v);
        pf.insert(p);
    }

    // Show pareto front
    for (const auto &[k, v] : pf) {
        std::cout << "[" << k.get<0>() << ", " << k.get<1>() << "] -> " << v << std::endl;
    }
    std::cout << std::endl;

    // Get some properties
    std::cout << "pf.size(): " << pf.size() << std::endl;
    std::cout << "pf.hypervolume(): " << pf.hypervolume() << std::endl;

    auto p_ideal = pf.ideal();
    std::cout << "pf.ideal(): [" << p_ideal.get<0>() << ", " << p_ideal.get<1>() << "]" << std::endl;

    auto p_nadir = pf.nadir();
    std::cout << "pf.nadir(): [" << p_nadir.get<0>() << ", " << p_nadir.get<1>() << "]" << std::endl;

    auto p_worst = pf.worst();
    std::cout << "pf.worst(): [" << p_worst.get<0>() << ", " << p_worst.get<1>() << "]" << std::endl;

    // Show points in the box {0,0} to {5,5}
    for (auto it = pf.find_intersection({0,0},{5,5}); it != pf.end(); ++it) {
        std::cout << "[" << it->first.get<0>() << ", " << it->first.get<1>() << "] -> " << it->second << std::endl;
    }
    std::cout << std::endl;

    // Show point nearest to {2.5,2.5}
    auto it = pf.find_nearest({2.5,2.5});
    if (it != pf.end()) {
        std::cout << "[" << it->first.get<0>() << ", " << it->first.get<1>() << "] -> " << it->second << std::endl;
        std::cout << std::endl;
    }

    // Show 5 points nearest to {2.5,2.5}
    for (auto it = pf.find_nearest({2.5,2.5},5); it != pf.end(); ++it) {
        std::cout << "[" << it->first.get<0>() << ", " << it->first.get<1>() << "] -> " << it->second << std::endl;
    }
    std::cout << std::endl;

    // Remove point closest to 0.0
    it = pf.find_nearest({0.0,0.0});
    std::cout << "Removing closest to 0.0: [" << it->first.get<0>() << ", " << it->first.get<1>() << "] -> " << it->second << std::endl;
    pf.erase(it);

    // Remove the next closest
    it = pf.find_nearest({0.0,0.0});
    std::cout << "Removing closest to 0.0: [" << it->first.get<0>() << ", " << it->first.get<1>() << "] -> " << it->second << std::endl;
    pf.erase(it);

    // Show final closest
    it = pf.find_nearest({0.0,0.0});
    std::cout << "Closest is now: [" << it->first.get<0>() << ", " << it->first.get<1>() << "] -> " << it->second << std::endl;

    return 0;
}

std::mt19937& generator() {
    static std::mt19937 g((std::random_device()()) | std::chrono::high_resolution_clock::now().time_since_epoch().count());
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
```

## Installing

### Python

Check if you have [Cmake](http://cmake.org) 3.14+ installed:

```bash
cmake -version
```

Install or update [Cmake](http://cmake.org) 3.14+ if needed, and compile it:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

This should get you a library `pyfront.*` in the `bindings` directory. Put that library in your `PYTHONPATH` and you can now import it with:

```python
import pyfront
``` 

### C++

Download the whole project and add the subdirectory to your Cmake project:

```cmake
add_subdirectory(pareto_front)
# ...
target_link_libraries(my_target PUBLIC pareto_front)
```

Or install CPM.cmake and then:

```cmake
CPMAddPackage(
    NAME pareto_front
    GITHUB_REPOSITORY alandefreitas/pareto_front
)
# ...
target_link_libraries(my_target PUBLIC pareto_front)
```

If you're not using Cmake, your project needs to include the headers from the boost library and the headers in the `sources` directory. In any other system where you already have access to [boost](http://boost.org), just use the header file in the sources directory. 


# Dependencies

This code depends on the boost geometry package. This is a header only package. The cmake script will automatically download and link it.

# To-do

- [x] Algorithms in log-linear time
- [x] Python bindings
- [ ] More indicators