#include <chrono>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <pareto/archive.h>
#include <pareto/matplot/front.h>
#include <pareto/matplot/archive.h>

int main() {
    using namespace pareto;
    // Generate a 2d front
    front<double, 2, unsigned> pf2d({true, true});
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
    archive<double, 2, unsigned> ar2d(100, {true, true});
    for (size_t i = 0; i < f1.size(); ++i) {
        ar2d(f1[i], f2[i]) = static_cast<unsigned>(i);
    }

    // Plot a single 2d archive
    matplot::hold(false);
    plot_archive(ar2d);
    matplot::save("archive2d.svg");
    matplot::show();

    // Generate a max/max front
    front<double, 2, unsigned> pf2d_max({false, false});
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
    front<double, 3, unsigned> pf3d({true, true, true});
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
        front<double, 2, unsigned> pftm({i < 2, i % 2 == 0});
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
    archive<double, 3, unsigned> ar3d(100, {true, true, true});
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