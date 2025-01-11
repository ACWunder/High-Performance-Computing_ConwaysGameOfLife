#include "CLI.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>


void CLI::run() {
    std::vector<std::pair<int, int>> sizes = {{10, 10}, {20, 20}, {100, 100}, {1000, 1000}, {10000, 10000}};
    std::vector<long long> times;
    int num_runs = 5; 

    for (auto& size : sizes) {
        int height = size.first;
        int width = size.second;
        long long total_time = 0;

        for (int run = 0; run < num_runs; ++run) {
            Grid world(height, width);
            world.setPrintEnabled(false);

            if (height > 5 && width > 5) {
                world.addBeacon(5, 5);
            }

            auto start_time = std::chrono::high_resolution_clock::now();
            world.evolve(); 
            auto end_time = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            total_time += duration.count();
        }

        long long average_time = total_time / num_runs;
        times.push_back(average_time);

        std::cout << "Average time for grid size (" << height << ", " << width << "): " << average_time << " ms\n";
    }

    std::ofstream outfile("times.txt");
    for (size_t i = 0; i < sizes.size(); ++i) {
        outfile << sizes[i].first << " " << sizes[i].second << " " << times[i] << "\n";
    }
    outfile.close();
}
