#include <iostream>
#include <random>
#include <chrono>
#include <pareto_front.h>

int main() {
    std::mt19937 g((std::random_device()()) | std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::normal_distribution nd;
    auto randn = [&]() { return nd(g); };
    std::uniform_int_distribution<unsigned> ud(0, 40);
    auto randi = [&]() { return ud(g); };

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