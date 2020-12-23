#include <chrono>
#include <iostream>
#include <map>
#include <pareto/spatial_map.h>

int main() {
    using std::cout;
    using std::endl;
    using namespace pareto;
    // Constructors
    spatial_map<double, 3, unsigned> m;

    // Element access
    m(-2.57664, -1.52034, 0.600798) = 17;
    m(-2.14255, -0.518684, -2.92346) = 32;
    m(-1.63295, 0.912108, -2.12953) = 36;
    m(-0.653036, 0.927688, -0.813932) = 13;
    m(-0.508188, 0.871096, -2.25287) = 32;
    m(-2.55905, -0.271349, 0.898137) = 6;
    m(-2.31613, -0.219302, 0) = 8;
    m(-0.639149, 1.89515, 0.858653) = 10;
    m(-0.401531, 2.30172, 0.58125) = 39;
    m(0.0728106, 1.91877, 0.399664) = 25;
    m(-1.09756, 1.33135, 0.569513) = 20;
    m(-0.894115, 1.01387, 0.462008) = 11;
    m(-1.45049, 1.35763, 0.606019) = 17;
    m(0.152711, 1.99514, -0.112665) = 13;
    m(-2.3912, 0.395611, 2.78224) = 11;
    m(-0.00292544, 1.29632, -0.578346) = 20;
    m(0.157424, 2.30954, -1.23614) = 6;
    m(0.453686, 1.02632, -2.24833) = 30;
    m(0.693712, 1.12267, -1.37375) = 12;
    m(1.49101, 3.24052, 0.724771) = 24;

    std::cout << "Element access: " << m(1.49101, 3.24052, 0.724771) << std::endl;

    // Iterators
    std::cout << "Iterators:" << std::endl;
    for (const auto& [point, value]: m) {
        cout << point << " -> " << value << endl;
    }

    std::cout << "Reversed Iterators:" << std::endl;
    for (auto it = m.rbegin(); it != m.rend(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }

    // Capacity and Reference Points
    if (!m.empty()) {
        cout << "Map is not empty" << endl;
    } else {
        cout << "Map is empty" << endl;
    }
    cout << m.size() << " elements in the spatial map" << endl;
    cout << m.dimensions() << " dimensions" << endl;
    for (size_t i = 0; i < m.dimensions(); ++i) {
        cout << "Min value in dimension " << i << ": " << m.min_value(i) << endl;
        cout << "Max value in dimension " << i << ": " << m.max_value(i) << endl;
    }

    // Modifiers
    m.insert({{1.49101, 3.24052, 0.724771}, 24});
    m.erase({1.49101, 3.24052, 0.724771});

    // Lookup and queries
    std::cout << "Lookup and queries" << std::endl;
    for (auto it = m.find_intersection({-10,-10,-10}, {-2.3912, 0.395611, 2.78224}); it != m.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = m.find_within({-10,-10,-10}, {-2.3912, 0.395611, 2.78224}); it != m.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = m.find_disjoint({-10,-10,-10}, {+0.71, +1.19, +0.98}); it != m.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = m.find_nearest({-2.3912, 0.395611, 2.78224}, 2); it != m.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    auto it = m.find_nearest({2.5, 2.5, 2.5});
    cout << it->first << " -> " << it->second << endl;

    // Observers
    auto fn = m.dimension_comp();
    if (fn(2.,3.)) {
        std::cout << "2 is less than 3" << std::endl;
    } else {
        std::cout << "2 is not less than 3" << std::endl;
    }

    // Relational operators
    spatial_map<double, 3, unsigned> m2(m);
    if (m == m2) {
        std::cout << "The containers have the same elements" << std::endl;
    } else {
        if (m.size() != m2.size()) {
            std::cout << "The containers do not have the same elements" << std::endl;
        } else {
            std::cout << "The containers might not have the same elements"
                      << std::endl;
        }
    }

    spatial_map<double, 3, unsigned> m3(m.begin(), m.end());
    if (m == m3) {
        std::cout << "The containers have the same elements" << std::endl;
    } else {
        if (m.size() != m3.size()) {
            std::cout << "The containers do not have the same elements" << std::endl;
        } else {
            std::cout << "The containers might not have the same elements"
                      << std::endl;
        }
    }

    return 0;
}
