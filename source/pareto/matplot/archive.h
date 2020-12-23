//
// Created by Alan Freitas on 12/4/20.
//

#ifndef PARETO_MATPLOT_ARCHIVE_H
#define PARETO_MATPLOT_ARCHIVE_H

#include <pareto/matplot/front.h>

namespace pareto {
    template <typename ARCHIVE_TYPE>
    void plot_archive(const ARCHIVE_TYPE &ar, size_t front_idx = 0) {
        if (ar.empty()) {
            return;
        }
        bool p = matplot::gca()->hold();
        bool q = matplot::gcf()->quiet_mode();
        matplot::gcf()->quiet_mode(true);
        if (ar.dimensions() == 1) {
            std::vector<double> x;
            std::vector<double> y;
            for (const auto &[k, v] : ar) {
                x.emplace_back(k[0]);
                y.emplace_back(0.);
            }

            // Plot direction arrow
            if (!p) {
                matplot::gca()->clear();
            }
            if (x.empty()) {
                if (ar.is_minimization(0)) {
                    matplot::gca()->arrow(1, 0, 0, 0);
                } else {
                    matplot::gca()->arrow(0, 0, 1, 0);
                }
            } else if (x.size() == 1) {
                if (ar.is_minimization(0)) {
                    matplot::gca()->arrow(x[0] + 1., 0, x[0], 0);
                } else {
                    matplot::gca()->arrow(x[0] - 1, 0, x[0], 0);
                }
            } else {
                if (ar.is_minimization(0)) {
                    matplot::gca()->arrow(matplot::max(x), 0, matplot::min(x),
                                          0);
                } else {
                    matplot::gca()->arrow(matplot::min(x), 0, matplot::max(x),
                                          0);
                }
            }
            matplot::hold(true);

            // Plot histogram
            if (x.size() > 10) {
                auto h = matplot::gca()->hist(x);
            }

            // Plot points
            matplot::scatter(x, y);

            // Label axes
            if (ar.is_minimization(0)) {
                matplot::xlabel("\\min f_1");
            } else {
                matplot::xlabel("\\max f_1");
            }
            matplot::title("Archive size " + matplot::num2str(ar.size()) +
                           " in " + matplot::num2str(ar.size_fronts()) + " fronts");
        } else if (ar.dimensions() == 2) {
            // Calculate reference point for rectangles and lines
            auto ideal_point = ar.ideal();
            auto worst_point = ar.worst();
            double max_x = std::max(ideal_point[0], worst_point[0]);
            double min_x = std::min(ideal_point[0], worst_point[0]);
            double max_y = std::max(ideal_point[1], worst_point[1]);
            double min_y = std::min(ideal_point[1], worst_point[1]);
            double x_range = max_x - min_x;
            double y_range = max_y - min_y;
            if (ar.size() != 1) {
                worst_point[0] +=
                    ar.is_minimization(0) ? 0.2 * x_range : -0.2 * x_range;
                worst_point[1] +=
                    ar.is_minimization(1) ? 0.2 * y_range : -0.2 * y_range;
            } else {
                worst_point[0] += ar.is_minimization(0) ? +1 : -1;
                worst_point[1] += ar.is_minimization(1) ? +1 : -1;
            }

            // Plot direction arrow
            if (!p) {
                matplot::gca()->clear();
            }
            if (!ar.empty()) {
                matplot::gca()->arrow(worst_point[0], worst_point[1],
                                      ideal_point[0], ideal_point[1]);
            }
            matplot::hold(true);

            // Plot all fronts
            size_t i = 0;
            for (auto pf_it = ar.begin_front(); pf_it != ar.end_front();
                 ++pf_it) {
                if (i < front_idx) {
                    plot_front(*pf_it, false, ar.size() < 10, worst_point,
                               "--");
                } else {
                    plot_front(*pf_it, false, ar.size() < 10, worst_point, "-");
                }
                ++i;
                matplot::hold(true);
            }

            // Label axes
            if (ar.is_minimization(0)) {
                matplot::xlabel("\\min f_1");
            } else {
                matplot::xlabel("\\max f_1");
            }
            if (ar.is_minimization(1)) {
                matplot::ylabel("\\min f_2");
            } else {
                matplot::ylabel("\\max f_2");
            }
            matplot::title("Archive size " + matplot::num2str(ar.size()) +
                           " in " + matplot::num2str(ar.size_fronts()) + " fronts");

            matplot::hold(false);
        } else {
            size_t m = ar.dimensions();
            std::vector<std::vector<double>> X(m);
            std::vector<double> C;
            double front_index = 1.;
            for (auto pf_it = ar.begin_front(); pf_it != ar.end_front();
                 ++pf_it) {
                for (const auto &[k, v] : *pf_it) {
                    for (size_t i = 0; i < m; ++i) {
                        X[i].emplace_back(k[i]);
                    }
                    C.emplace_back(front_index);
                    ;
                }
                ++front_index;
            }

            // Reverse order to plot first fronts last
            for (auto &xs : X) {
                std::reverse(xs.begin(), xs.end());
            }
            std::reverse(C.begin(), C.end());

            if (X[0].size() > 1) {
                std::vector<double> x;
                for (const auto &xs : X) {
                    x.emplace_back(xs.front());
                }
                matplot::gca()->plot(x);
            } else {
                matplot::parallelplot(X, C);
            }
            matplot::gca()->x_axis().tick_values({1, 2, 3});
            matplot::gca()->x_axis().ticklabels({"f_1", "f_2", "f_3"});
            matplot::xlabel("");
            matplot::ylabel("");
        }
        matplot::gca()->hold(p);
        matplot::gcf()->quiet_mode(q);
        matplot::gcf()->draw();
    }
} // namespace pareto

#endif // PARETO_MATPLOT_ARCHIVE_H
