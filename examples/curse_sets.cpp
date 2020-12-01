//
// Created by Alan Freitas on 11/30/20.
//

#include <iostream>
#include <vector>
#include <map>

int main() {
    std::map<float,std::vector<std::vector<float>>> X;
    const size_t resolution = 10;
    for (size_t i = 0; i <= resolution; ++i) {
        float p1 = static_cast<float>(i) * 1.f/resolution;
        for (size_t j = 0; i <= resolution; ++i) {
            float p2 = static_cast<float>(j) * 1.f/resolution;
            std::vector<float> objective_vector{p1,p2};
            float volume = p1 * p2;
            auto it = X.find(volume);
            if (it != X.end()) {
                it->second.emplace_back(objective_vector);
            } else {
                X[volume] = {objective_vector};
            }
        }
    }

    // Show results
    for (const auto &[volume, vectors] : X) {
        std::cout << volume << ": {";
        for (const auto &vector : vectors) {
            std::cout << "[";
            bool first_component = true;
            for (const auto &component : vector) {
                if (first_component) {
                    std::cout << component;
                    first_component = false;
                } else {
                    std::cout << "," << component;
                }
            }
            std::cout << "] ";
        }
        std::cout << "}" << std::endl;
    }

    return 0;
}