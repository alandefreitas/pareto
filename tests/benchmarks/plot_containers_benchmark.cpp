#include <fstream>
#include <iomanip>
#include <matplot/matplot.h>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <filesystem>

int main(int argc, char **argv) {
    std::string filename = "containers_benchmark.json";
    if (argc == 2) {
        filename = argv[1];
    } else {
        std::cout << "No filename provided in the command line" << std::endl;
        std::cout << "Trying " << filename << " instead" << std::endl;
    }

    // Read file
    std::cout << "Reading " << filename << std::endl;
    clock_t start = clock();
    using json = nlohmann::json;
    std::ifstream fin(filename);
    if (!fin) {
        std::cout << "Cannot open file " << filename << std::endl;
    }
    json results;
    fin >> results;
    double end = clock();
    double cpu_time_used = ((float)(end - start)) / CLOCKS_PER_SEC;
    std::cout << "Time: " << cpu_time_used << " seconds" << std::endl;

    // Describe the experiment factors
    std::cout << "Set up factors " << filename << std::endl;
    std::vector<std::string> container_factors = {
        "implicit_tree", "quad_tree",  "boost_tree",
        "kd_tree",       "r_tree",     "r_star_tree", };

    std::vector<size_t> container_dimensions = {1, 2, 3, 5, 7, 9, 13};

    std::vector<double> double_container_dimensions;
    std::copy(container_dimensions.begin(), container_dimensions.end(),
              std::back_inserter(double_container_dimensions));

    std::vector<size_t> container_sizes = {50, 500, 5000};

    std::vector<std::string> container_operations = {
        "construct",          "insert",        "erase", "check_dominance",
        "query_intersection", "query_nearest", "igd",   "hypervolume"};

    size_t replicates = 30;

    // Helper function
    auto get_benchmark_name = [](std::string_view op, size_t m, size_t n,
                                 std::string_view container) {
        std::string r = std::string(op) + "<m=" + std::to_string(m) + "," +
                        std::string(container) + ">/" + std::to_string(n);
        if (op == "hypervolume") {
            r += "/10000";
        }
        r += "/iterations:1/threads:8";
        return r;
    };

    // Helper function
    auto mean = [](const std::vector<double> &v) {
        return std::accumulate(v.begin(), v.end(), 0.) / v.size();
    };

    // Helper function
    auto percentile = [](const std::vector<double> &v, int perc) {
        std::vector<double> v2 = v;
        unsigned long pos = v2.size() * perc / 100;
        std::nth_element(v2.begin(), v2.begin() + pos, v2.end());
        return v2[pos];
    };

    // Helper function
    auto remove_underline = [](const std::string &str, char new_char = ' ',
                               bool capitalize = true) -> std::string {
        if (str == "igd") {
            return "IGD";
        }
        std::string result = str;
        std::replace(result.begin(), result.end(), '_', new_char);
        if (capitalize) {
            result[0] = std::toupper(result[0]);
        }
        return result;
    };

    // Helper function
    auto container_color = [](const std::string &str) {
        if (str == "implicit_tree") {
            return matplot::default_color(1);
        } else if (str == "kd_tree") {
            return matplot::default_color(0);
        } else if (str == "quad_tree") {
            return matplot::default_color(2);
        } else if (str == "r_tree") {
            return matplot::default_color(4);
        } else if (str == "r_star_tree") {
            return matplot::default_color(3);
        } else if (str == "boost_tree") {
            return matplot::default_color(6);
        } else {
            return matplot::default_color(5);
        }
    };

    auto container_marker = [](const std::string &str) {
        if (str == "implicit_tree") {
            return matplot::line_spec::marker_style::square;
        } else if (str == "kd_tree") {
            return matplot::line_spec::marker_style::asterisk;
        } else if (str == "quad_tree") {
            return matplot::line_spec::marker_style::circle;
        } else if (str == "r_tree") {
            return matplot::line_spec::marker_style::diamond;
        } else if (str == "r_star_tree") {
            return matplot::line_spec::marker_style::plus_sign;
        } else if (str == "boost_tree") {
            return matplot::line_spec::marker_style::cross;
        } else {
            return matplot::line_spec::marker_style::none;
        }
    };

    auto container_display_name = [](const std::string &str) -> std::string {
        if (str == "implicit_tree") {
            return "Linear List";
        } else if (str == "kd_tree") {
            return "Kd-tree";
        } else if (str == "quad_tree") {
            return "Quadtree";
        } else if (str == "r_tree") {
            return "R-tree";
        } else if (str == "r_star_tree") {
            return "R*-tree";
        } else if (str == "boost_tree") {
            return "R-tree (Boost)";
        } else {
            return str;
        }
    };

    std::vector<std::string> container_display_names;
    std::transform(container_factors.begin(), container_factors.end(), std::back_inserter(container_display_names), container_display_name);

    // Iterate combinations of factors
    matplot::gcf()->quiet_mode(true);
    matplot::gcf()->width(matplot::gcf()->width() * 2);
    for (const std::string &op : container_operations) {
        std::cout << "Plotting results for " << op << std::endl;
        for (const size_t &n : container_sizes) {
            matplot::gca()->clear();
            matplot::gca()->y_axis().scale(matplot::axis_type::axis_scale::log);
            matplot::hold(false);
            matplot::title(remove_underline(op) + " (n = " + std::to_string(n) + ")");
            matplot::xlabel("m");
            matplot::ylabel("Time (ns)");
            matplot::xticks(double_container_dimensions);
            std::map<size_t, double> m_max;
            std::map<size_t, double> m_min;
            for (const std::string &container : container_factors) {
                std::vector<double> averages;
                std::vector<double> y_neg_error;
                std::vector<double> y_pos_error;
                for (const size_t &m : container_dimensions) {
                    std::string benchmark_name =
                        get_benchmark_name(op, m, n, container);
                    std::vector<double> replicate_times;
                    json &benchmarks = results["benchmarks"];
                    for (const auto &benchmark : benchmarks) {
                        if (benchmark["name"] == benchmark_name) {
                            auto replicate_time =
                                benchmark["real_time"].get<double>();
                            replicate_times.emplace_back(replicate_time);
                        }
                    }
                    if (!replicate_times.empty()) {
                        double avg = percentile(replicate_times, 50);
                        double y_pos = percentile(replicate_times, 75) - avg;
                        double y_neg = avg - percentile(replicate_times, 25);
                        std::cout << benchmark_name << ": " << avg << " +"
                                  << y_pos << " -" << y_neg << std::endl;
                        averages.emplace_back(avg);
                        y_pos_error.emplace_back(y_pos);
                        y_neg_error.emplace_back(y_neg);
                        if ((m_max.count(m) && averages.back() > m_max[m]) || !m_max.count(m)) {
                            m_max[m] = averages.back();
                        }
                        if ((m_min.count(m) && averages.back() < m_min[m]) || !m_min.count(m)) {
                            m_min[m] = averages.back();
                        }
                    }
                }
                if (!averages.empty()) {
                    matplot::gca()
                        ->errorbar(double_container_dimensions, averages,
                                   y_neg_error, y_pos_error, {}, {})
                        ->filled_curve(true)
                        .marker(container_marker(container))
                        .color(container_color(container));
                    matplot::hold(true);
                }
            }
            // Show min values
            std::vector<double> m_min_pos;
            std::vector<std::string> m_min_label;
            for (const auto &[k, v] : m_min) {
                m_min_pos.emplace_back(v);
                m_min_label.emplace_back(matplot::num2str(round(v)));
            }
            matplot::gca()->text(double_container_dimensions.back() - 0.1, m_min_pos.back(), m_min_label.back())
                ->alignment(matplot::labels::alignment::right);
            double_container_dimensions.pop_back();
            m_min_pos.pop_back();
            m_min_label.pop_back();
            std::transform(double_container_dimensions.begin(), double_container_dimensions.end(), double_container_dimensions.begin(), [](double x) { return x + 0.1; });
            matplot::gca()->text(double_container_dimensions, m_min_pos, m_min_label)
                ->alignment(matplot::labels::alignment::left);
            std::transform(double_container_dimensions.begin(), double_container_dimensions.end(), double_container_dimensions.begin(), [](double x) { return x - 0.1; });
            double_container_dimensions.push_back(container_dimensions.back());

            // Show max values
            std::vector<double> m_max_pos;
            std::vector<std::string> m_max_label;
            for (const auto &[k, v] : m_max) {
                m_max_pos.emplace_back(v);
                m_max_label.emplace_back(matplot::num2str(round(v)));
            }
            matplot::gca()->text(double_container_dimensions.back()-0.1, m_max_pos.back(), m_max_label.back())
                ->alignment(matplot::labels::alignment::right);
            double_container_dimensions.pop_back();
            m_max_pos.pop_back();
            m_max_label.pop_back();
            std::transform(double_container_dimensions.begin(), double_container_dimensions.end(), double_container_dimensions.begin(), [](double x) { return x + 0.1; });
            matplot::gca()->text(double_container_dimensions, m_max_pos, m_max_label)
                ->alignment(matplot::labels::alignment::left);
            std::transform(double_container_dimensions.begin(), double_container_dimensions.end(), double_container_dimensions.begin(), [](double x) { return x - 0.1; });
            double_container_dimensions.push_back(container_dimensions.back());

            auto lgd = matplot::legend(container_display_names);
            lgd->location(matplot::legend::general_alignment::topleft);
            lgd->num_rows(3);
            lgd->title("Data structure");

            matplot::gcf()->draw();

            std::string file_stem = "plots/" + op + "_n_" + std::to_string(n);
            matplot::save(file_stem + ".svg");

            // The next command requires inkscape
            // Inkspace generates better eps images with transparency
            auto p = std::filesystem::current_path();
            std::string cmd = "inkscape \"" + p.string() + "/" + file_stem + ".svg\" -o \"" + p.string() + "/"  + file_stem + ".eps\" --export-ignore-filters --export-ps-level=3";
            std::cout << "inkscape cmd: " << cmd << std::endl;
            system(cmd.c_str());
        }
    }
}