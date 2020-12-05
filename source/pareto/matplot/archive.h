//
// Created by Alan Freitas on 12/4/20.
//

#ifndef PARETO_MATPLOT_ARCHIVE_H
#define PARETO_MATPLOT_ARCHIVE_H

#include <pareto/matplot/front.h>

namespace pareto {
    template <typename ARCHIVE_TYPE>
    void plot_archive(const ARCHIVE_TYPE &ar) {
        bool p = matplot::gca()->hold();
        if (ar.dimensions() == 1) {
            return;
        } else if (ar.dimensions() == 2) {
            // Use same reference point for rectangles and lines
            auto ideal_point = ar.ideal();
            auto worst_point = ar.worst();
            double max_x = std::max(ideal_point[0], worst_point[0]);
            double min_x = std::min(ideal_point[0], worst_point[0]);
            double max_y = std::max(ideal_point[1], worst_point[1]);
            double min_y = std::min(ideal_point[1], worst_point[1]);
            double x_range = max_x - min_x;
            double y_range = max_y - min_y;
            worst_point[0] += ar.is_minimization(0) ? 0.2 * x_range : - 0.2 * x_range;
            worst_point[1] += ar.is_minimization(1) ? 0.2 * y_range : - 0.2 * y_range;
            for (auto pf_it = ar.begin_front(); pf_it != ar.end_front(); ++pf_it) {
                plot_front(*pf_it, false, false, worst_point);
                matplot::hold(true);
            }
            matplot::hold(false);
        } else {
            auto ax = matplot::gca();
            std::vector<std::vector<double>> X(3);
            std::vector<double> C;
            double front_index = 1.;
            for (auto pf_it = ar.begin_front(); pf_it != ar.end_front(); ++pf_it) {
                plot_front(*pf_it);
                for (const auto &[k, v] : *pf_it) {
                    X[0].emplace_back(k[0]);
                    X[1].emplace_back(k[1]);
                    X[2].emplace_back(k[2]);
                    C.emplace_back(front_index);
                }
                ++front_index;
            }
            std::reverse(X[0].begin(), X[0].end());
            std::reverse(X[1].begin(), X[1].end());
            std::reverse(X[2].begin(), X[2].end());
            std::reverse(C.begin(), C.end());
            matplot::parallelplot(X,C);
            ax->x_axis().tick_values({1, 2, 3});
            ax->x_axis().ticklabels({"f_1", "f_2", "f_3"});
            matplot::xlabel("");
            matplot::ylabel("");
        }
        matplot::gca()->hold(p);
    }
}

#endif //PARETO_MATPLOT_ARCHIVE_H
