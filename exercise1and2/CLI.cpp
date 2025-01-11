#include "CLI.h"
#include <iostream>


void CLI::run() {
    Grid world;

    std::cout << "Would you like to set up the world from a file or create a new empty one? (1 for file, 0 for empty): ";
    int choice;
    std::cin >> choice;

    if (choice == 1) {
        std::string filename;
        std::cout << "Enter the filename: ";
        std::cin >> filename;
        world.initializePattern(filename);
    } else {
        int height, width;
        std::cout << "Enter the height of the grid: ";
        std::cin >> height;
        std::cout << "Enter the width of the grid: ";
        std::cin >> width;
        world.setSize(height, width);
    }

    bool printEnabled;
    std::cout << "Enable printing after each generation? (1 for yes, 0 for no): ";
    std::cin >> printEnabled;
    world.setPrintEnabled(printEnabled);

    int delay_ms;
    std::cout << "Enter delay in milliseconds between generations: ";
    std::cin >> delay_ms;

    std::cout << "Running scalar version...\n";
    long long scalar_time = world.run(20, delay_ms);
    std::cout << "Scalar version time: " << scalar_time << " ms\n";

    std::cout << "Running OpenCL version...\n";
    long long opencl_time = world.run_with_opencl(20, delay_ms);
    std::cout << "OpenCL version time: " << opencl_time << " ms\n";

    // long long timeTaken = world.run(20, delay_ms);
    std::cout << "Time taken for evolution (scalar): " << scalar_time << " ms\n";

    std::cout << "Time taken for evolution (opencl): " << opencl_time << " ms\n";

    /* Testing other features
    world.save("world_state.txt");
    Grid loadedWorld;
    loadedWorld.load("world_state.txt");
    loadedWorld.run(10, delay_ms);
    */
}
