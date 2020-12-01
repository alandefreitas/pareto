#include <chrono>
#include <iostream>
#include <map>
#include <matplot/matplot.h>
#include <numeric>
#include <pareto/archive.h>
#include <random>

void plot_front(const pareto::front<double, 2, unsigned> &pf, bool draw_rect = true);
void plot_front(const pareto::front<double, 3, unsigned> &pf);
void plot_archive(const pareto::archive<double, 2, unsigned> &pf);
void plot_archive(const pareto::archive<double, 3, unsigned> &pf);

int main() {
    // Generate a 2d front
    pareto::front<double, 2, unsigned> pf2d({true, true});
    auto f1 = matplot::randn(10000, 0., 1.);
    auto f2 = matplot::randn(10000, 0., 1.);
    for (size_t i = 0; i < f1.size(); ++i) {
        pf2d(f1[i], f2[i]) = static_cast<unsigned>(i);
    }

    // Plot a single 2d front
    matplot::hold(false);
    plot_front(pf2d);
    matplot::save("front2d_b.svg");
    matplot::show();

    // Generate a 2d archive
    pareto::archive<double, 2, unsigned> ar2d(100, {true, true});
    for (size_t i = 0; i < f1.size(); ++i) {
        ar2d(f1[i], f2[i]) = static_cast<unsigned>(i);
    }

    // Plot a single 2d archive
    matplot::hold(false);
    plot_archive(ar2d);
    matplot::save("archive2d.svg");
    matplot::show();

    // Generate a max/max front
    pareto::front<double, 2, unsigned> pf2d_max({false, false});
    for (size_t i = 0; i < f1.size(); ++i) {
        pf2d_max(f1[i], f2[i]) = static_cast<unsigned>(i);
    }

    // Plot single front
    matplot::cla();
    matplot::hold(false);
    plot_front(pf2d_max);
    matplot::save("front2d.svg");
    matplot::show();

    // Generate a 3d front
    pareto::front<double, 3, unsigned> pf3d({true, true, true});
    auto f3 = matplot::randn(10000, 0., 1.);
    for (size_t i = 0; i < f1.size(); ++i) {
        pf3d(f1[i], f2[i], f3[i]) = static_cast<unsigned>(i);
    }

    // Plot a single 3d front
    matplot::hold(false);
    plot_front(pf3d);
    matplot::xlabel("");
    matplot::ylabel("");
    matplot::save("front3d.svg");
    matplot::show();

    // Plot 4 fronts with subplots
    matplot::cla();
    for (size_t i = 0; i < 4; ++i) {
        matplot::subplot(2,2,i);
        pareto::front<double, 2, unsigned> pftm({i < 2, i % 2 == 0});
        for (size_t j = 0; j < f1.size(); ++j) {
            pftm(f1[j], f2[j]) = static_cast<unsigned>(j);
        }
        matplot::hold(false);
        plot_front(pftm);
        if (i < 2) {
            matplot::xlabel("Minimize f1");
        } else {
            matplot::xlabel("Maximize f1");
        }
        if (i % 2 == 0) {
            matplot::ylabel("Minimize f2");
        } else {
            matplot::ylabel("Maximize f2");
        }
    }
    matplot::save("front2d_directions.svg");
    matplot::show();
    matplot::hold(false);

    // Compare fronts and archives
    auto f = matplot::gcf();
    f->width(f->width() * 1.5);
    auto ax = matplot::subplot(2,2,0);
    ax->clear();
    matplot::gcf()->current_axes(ax);
    plot_front(pf2d);

    ax = matplot::subplot(2,2,1);
    ax->clear();
    matplot::gcf()->current_axes(ax);
    plot_front(pf3d);
    matplot::xlabel("");
    matplot::ylabel("");
    ax->x_axis().ticklabels({"f1", "f2", "f3"});

    ax = matplot::subplot(2,2,2);
    ax->clear();
    matplot::gcf()->current_axes(ax);
    plot_archive(ar2d);

    ax = matplot::subplot(2,2,3);
    ax->clear();
    matplot::gcf()->current_axes(ax);
    pareto::archive<double, 3, unsigned> ar3d(100, {true, true, true});
    for (size_t i = 0; i < f1.size(); ++i) {
        ar3d(f1[i], f2[i], f3[i]) = static_cast<unsigned>(i);
    }
    plot_archive(ar3d);
    matplot::xlabel("");
    matplot::ylabel("");
    ax->x_axis().ticklabels({"f1", "f2", "f3"});

    matplot::save("pareto_cover.svg");
    matplot::show();
    matplot::hold(false);

    return 0;
}

void plot_front(const pareto::front<double, 2, unsigned> &pf, bool draw_rect) {
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

    if (draw_rect) {
        auto reference_point = pf.nadir();
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
    if (pf.is_minimization(0)) {
        matplot::stairs(x, y)->stair_style(
            matplot::stair::stair_style::trace_x_first);
    } else {
        matplot::stairs(x, y)->stair_style(
            matplot::stair::stair_style::trace_y_first);
    }
    matplot::hold(true);

    // Scatter plot
    matplot::scatter(x, y);

    // Texts
    /*
    std::vector<std::string> labels(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        labels[i] = matplot::num2str(x[i]).substr(0,4) + "," + matplot::num2str(y[i]).substr(0,4);
    }
    if (pf.is_maximization(0)) {
        matplot::text(x,y,labels)->alignment(matplot::labels::alignment::left);
    } else {
        matplot::text(x,y,labels)->alignment(matplot::labels::alignment::right);
    }
    */

    // x/y labels
    matplot::xlabel("Objective 1");
    matplot::ylabel("Objective 2");

    matplot::hold(false);
}

void plot_front(const pareto::front<double, 3, unsigned> &pf) {
    std::vector<std::vector<double>> X(3);
    for (const auto &[k, v] : pf) {
        X[0].emplace_back(k[0]);
        X[1].emplace_back(k[1]);
        X[2].emplace_back(k[2]);
    }
    matplot::parallelplot(X,X[0]);
    matplot::gca()->x_axis().tick_values({1, 2, 3});
    matplot::gca()->x_axis().ticklabels({"f_1", "f_2", "f_3"});
    matplot::hold(false);
}

void plot_archive(const pareto::archive<double, 2, unsigned> &ar) {
    for (auto pf_it = ar.begin_front(); pf_it != ar.end_front(); ++pf_it) {
        plot_front(*pf_it, false);
        matplot::hold(true);
    }
    matplot::hold(false);
}

void plot_archive(const pareto::archive<double, 3, unsigned> &ar) {
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
    matplot::hold(false);
}

