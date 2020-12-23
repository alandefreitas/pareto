//
// Created by Alan Freitas on 12/4/20.
//

#ifndef PARETO_MATPLOT_FRONT_H
#define PARETO_MATPLOT_FRONT_H

#include <matplot/matplot.h>

namespace pareto {
    template<typename FRONT_TYPE>
    void plot_front(const FRONT_TYPE &pf,
                    bool draw_rect = true,
                    bool draw_text = false,
                    const std::optional<typename FRONT_TYPE::key_type> &ref = std::nullopt,
                    std::string_view line_spec = "-") {
        if (pf.empty()) {
            return;
        }
        bool p = matplot::gca()->hold();
        bool q = matplot::gcf()->quiet_mode();
        matplot::gcf()->quiet_mode(true);
        if (pf.dimensions() == 1) {
            std::vector<double> x;
            std::vector<double> y;
            for (const auto &[k, v] : pf) {
                x.emplace_back(k[0]);
                y.emplace_back(0.);
            }
            matplot::scatter(x,y);
            matplot::xlabel("f_1");
        } else if (pf.dimensions() == 2) {
            std::vector<std::pair<double, double>> xs;
            std::vector<double> x;
            std::vector<double> y;
            for (const auto &[k, v] : pf) {
                xs.emplace_back(k[0], k[1]);
            }
            std::sort(xs.begin(), xs.end());
            for (const auto &[first, second] : xs) {
                x.emplace_back(first);
                y.emplace_back(second);
            }

            // Reference point for rectangles and lines
            auto reference_point = ref ? *ref : pf.nadir();
            if (!ref) {
                double x_range = x.back() - x.front();
                double y_range = std::abs(y.front() - y.back());
                if (pf.is_minimization(0)) {
                    reference_point[0] += 0.2 * x_range;
                } else {
                    reference_point[0] -= 0.2 * x_range;
                }
                if (pf.is_minimization(1)) {
                    reference_point[1] += 0.2 * y_range;
                } else {
                    reference_point[1] -= 0.2 * y_range;
                }
            }

            if (draw_rect) {
                // Background rectangles
                for (size_t i = 0; i < x.size(); ++i) {
                    double x1 = std::min(reference_point[0], x[i]);
                    double y1 = std::min(reference_point[1], y[i]);
                    double x2 = std::max(reference_point[0], x[i]);
                    double y2 = std::max(reference_point[1], y[i]);
                    matplot::rectangle(x1, y1, x2 - x1, y2 - y1)
                            ->fill(true)
                            .color(matplot::color_array{0.7f, 0.7f, 0.7f, 0.7f});
                    matplot::hold(true);
                }
            }

            // Stairs
            std::vector<double> x_stairs;
            std::vector<double> y_stairs;
            x_stairs.emplace_back(pf.is_minimization(0) ? x[0] : reference_point[0]);
            y_stairs.emplace_back(pf.is_minimization(0) ? reference_point[1] : y[0]);
            for (size_t i = 0; i < x.size(); ++i) {
                x_stairs.emplace_back(x[i]);
                y_stairs.emplace_back(y[i]);
                if (i != x.size() - 1) {
                    x_stairs.emplace_back(pf.is_minimization(0) ? x[i + 1] : x[i]);
                    y_stairs.emplace_back(pf.is_minimization(0) ? y[i] : y[i + 1]);
                }
            }
            x_stairs.emplace_back(pf.is_minimization(0) ? reference_point[0] : x.back());
            y_stairs.emplace_back(pf.is_minimization(0) ? y.back() : reference_point[1]);
            matplot::plot(x_stairs,y_stairs,line_spec);
            matplot::hold(true);

            // Scatter plot
            matplot::scatter(x, y);

            // Texts
            if (draw_text) {
                std::vector<std::string> labels(x.size());
                for (size_t i = 0; i < x.size(); ++i) {
                    labels[i] = matplot::num2str(x[i]).substr(0, 4) + "," + matplot::num2str(y[i]).substr(0, 4);
                }
                if (pf.is_maximization(0)) {
                    matplot::text(x, y, labels)->alignment(matplot::labels::alignment::left);
                } else {
                    matplot::text(x, y, labels)->alignment(matplot::labels::alignment::right);
                }
            }

            // x/y labels
            matplot::xlabel("f_1");
            matplot::ylabel("f_2");
            matplot::hold(false);
        } else {
            size_t m = pf.dimensions();
            std::vector<std::vector<double>> X(m);
            for (const auto &[k, v] : pf) {
                for (size_t i = 0; i < m; ++i) {
                    X[i].emplace_back(k[i]);
                }
            }
            matplot::parallelplot(X, X[0]);
            std::vector<double> ts(m);
            std::iota(ts.begin(), ts.end(), 1.);
            matplot::gca()->x_axis().tick_values(ts);
            std::vector<std::string> tl(m);
            for (size_t i = 0; i < m; ++i) {
                tl[i] = "f_" + std::to_string(i + 1);
            }
            matplot::gca()->x_axis().ticklabels(tl);
            matplot::hold(false);
        }
        matplot::gca()->hold(p);
        matplot::gcf()->quiet_mode(q);
        matplot::gcf()->draw();
    }
}

#endif //PARETO_MATPLOT_FRONT_H
