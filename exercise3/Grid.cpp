#include "Grid.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string.h>
#include <random>
#include <ctime>
#include "opencl.hpp"
#include "utilities.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// This is the path to the OpenCL kernel file
const std::string kernel_path = "/home/users8/acgl/s3859682/Documents/abschluss"; 


Grid::Grid() : height(0), width(0), printEnabled(true) {}

Grid::Grid(int h, int w) : height(h), width(w), currentGeneration(h, std::vector<bool>(w, false)), nextGeneration(h, std::vector<bool>(w, false)), printEnabled(true) {}

std::pair<int, int> Grid::to2D(int p) const {
    return { p / width, p % width };
}

void Grid::initializePattern(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Attempting to open file at: " << filename << std::endl;
        std::cerr << "Error opening file!" << std::endl;
        std::cerr << "Error code: " << strerror(errno) << std::endl;
        return;
    }

    file >> height >> width;
    currentGeneration.resize(height, std::vector<bool>(width, false));
    nextGeneration.resize(height, std::vector<bool>(width, false));

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int cell;
            file >> cell;
            currentGeneration[i][j] = (cell == 1);
        }
    }

    file.close();
}

void Grid::print() const {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            std::cout << (currentGeneration[i][j] ? 'O' : '.');
        }
        std::cout << std::endl;
    }
}

long long Grid::run(int generations, int delay_ms) {
    auto start_time = std::chrono::high_resolution_clock::now();

    addGlider(25, 25);
    addToad(100, 100);
    addBeacon(125, 125);
    addRPentomino(150, 150);
    

    for (int step = 0; step < generations; ++step) {
        if (printEnabled) {
            clearScreen();
            std::cout << "Generation " << step + 1 << ":\n";
            print();
        }
        evolve();
        if (is_stable()) {
            std::cout << "\nStable configuration detected at generation " << step + 1 << ".\n";
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "\nTotal calculation time for " << generations << " generations: " << duration.count() << " ms\n";

    return duration.count();
}

bool Grid::load(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return false;
    }

    file >> height >> width;
    currentGeneration.resize(height, std::vector<bool>(width, false));
    nextGeneration.resize(height, std::vector<bool>(width, false));

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int cell;
            file >> cell;
            currentGeneration[i][j] = (cell == 1);
        }
    }

    file.close();
    return true;
}

bool Grid::save(const std::string &filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return false;
    }

    file << height << " " << width << "\n";
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            file << (currentGeneration[i][j] ? 1 : 0) << " ";
        }
        file << "\n";
    }

    file.close();
    return true;
}

void Grid::setSize(int h, int w) {
    height = h;
    width = w;
    currentGeneration.resize(height, std::vector<bool>(width, false));
    nextGeneration.resize(height, std::vector<bool>(width, false));
}

int Grid::getHeight() const { return height; }
int Grid::getWidth() const { return width; }

void Grid::setCell(int x, int y, bool state) {
    if (x >= 0 && x < height && y >= 0 && y < width) {
        currentGeneration[x][y] = state;
    } else {
        std::cerr << "Error: Coordinates out of bounds.\n";
    }
}

void Grid::setCell(int p, bool state) {
    auto coords = to2D(p);
    int x = coords.first;
    int y = coords.second;
    if (x >= 0 && x < height && y >= 0 && y < width) {
        currentGeneration[x][y] = state;
    } else {
        std::cerr << "Error: Index out of bounds.\n";
    }
}

bool Grid::getCell(int x, int y) const {
    if (x >= 0 && x < height && y >= 0 && y < width) {
        return currentGeneration[x][y];
    } else {
        std::cerr << "Error: Coordinates out of bounds.\n";
        return false;
    }
}

bool Grid::getCell(int p) const {
    auto coords = to2D(p);
    int x = coords.first;
    int y = coords.second;
    if (x >= 0 && x < height && y >= 0 && y < width) {
        return currentGeneration[x][y];
    } else {
        std::cerr << "Error: Index out of bounds.\n";
        return false;
    }
}

void Grid::addGlider(int x, int y) {
    std::vector<std::pair<int, int>> glider{{0, 1}, {1, 2}, {2, 0}, {2, 1}, {2, 2}};
    for (auto& p : glider) {
        setCell(x + p.first, y + p.second, true);
    }
}

void Grid::addToad(int x, int y) {
    std::vector<std::pair<int, int>> toad{{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 1}, {1, 2}};
    for (auto& p : toad) {
        setCell(x + p.first, y + p.second, true);
    }
}

void Grid::addBeacon(int x, int y) {
    std::vector<std::pair<int, int>> beacon{{0, 0}, {0, 1}, {1, 0}, {1, 1}, {2, 2}, {2, 3}, {3, 2}, {3, 3}};
    for (auto& p : beacon) {
        setCell(x + p.first, y + p.second, true);
    }
}

void Grid::addRPentomino(int x, int y) {
    std::vector<std::pair<int, int>> rPentomino{{0, 1}, {0, 2}, {1, 0}, {1, 1}, {2, 1}};
    for (auto& p : rPentomino) {
        setCell(x + p.first, y + p.second, true);
    }
}

void Grid::randomize() {
    // Seed the random number generator
    static std::mt19937 rng(std::time(nullptr));
    std::uniform_int_distribution<int> patternDist(0, 3); // 4 patterns: 0 to 3
    std::uniform_int_distribution<int> xDist(0, height - 1);
    std::uniform_int_distribution<int> yDist(0, width - 1);

    // Randomly select a pattern
    int pattern = patternDist(rng);

    // Generate random coordinates ensuring the pattern fits within the grid
    int x = xDist(rng);
    int y = yDist(rng);

    // Adjust the coordinates if necessary to ensure the pattern fits within the grid
    switch(pattern) {
        case 0: // Glider
            if (x + 2 >= height) x = height - 3;
            if (y + 2 >= width) y = width - 3;
            addGlider(x, y);
            break;
        case 1: // Toad
            if (x + 1 >= height) x = height - 2;
            if (y + 3 >= width) y = width - 4;
            addToad(x, y);
            break;
        case 2: // Beacon
            if (x + 3 >= height) x = height - 4;
            if (y + 3 >= width) y = width - 4;
            addBeacon(x, y);
            break;
        case 3: // RPentomino
            if (x + 2 >= height) x = height - 3;
            if (y + 2 >= width) y = width - 3;
            addRPentomino(x, y);
            break;
    }
}

void Grid::setPrintEnabled(bool enabled) {
    printEnabled = enabled;
}

int Grid::countLiveNeighbors(int x, int y) const {
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx != 0 || dy != 0) {
                int nx = (x + dx + height) % height;
                int ny = (y + dy + width) % width;
                count += currentGeneration[nx][ny] ? 1 : 0;
            }
        }
    }
    return count;
}


void Grid::evolve() {
    cl::Context context(CL_DEVICE_TYPE_DEFAULT);
    std::string kernel_code = util::loadProgram(kernel_path);
    cl::Program::Sources sources;
    sources.push_back({kernel_code.c_str(), kernel_code.length()});

    cl::Program program(context, sources);

    auto devices = context.getInfo<CL_CONTEXT_DEVICES>();
    if (program.build(devices) != CL_SUCCESS) {
        for (const auto& device : devices) {
            std::cerr << "Error building the kernel for device: "
                      << device.getInfo<CL_DEVICE_NAME>() << std::endl;
            std::cerr << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        }
        return;
    }

    cl::Kernel kernel(program, "evolve");

    cl::Buffer buffer_current(context, CL_MEM_READ_WRITE, height * width * sizeof(char));
    cl::Buffer buffer_next(context, CL_MEM_READ_WRITE, height * width * sizeof(char));
    cl::Buffer buffer_debug(context, CL_MEM_READ_WRITE, height * width * 6 * sizeof(char));

    cl::CommandQueue queue(context);

    std::vector<char> flat_current(height * width);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            flat_current[i * width + j] = currentGeneration[i][j] ? '0' : '.';
        }
    }


    cl_int err;
    err = queue.enqueueWriteBuffer(buffer_current, CL_TRUE, 0, height * width * sizeof(char), flat_current.data());
    if (err != CL_SUCCESS) {
        std::cerr << "Error writing buffer_current: " << err << std::endl;
        return;
    }

    kernel.setArg(0, buffer_current);
    kernel.setArg(1, buffer_next);
    kernel.setArg(2, height);
    kernel.setArg(3, width);
    kernel.setArg(4, buffer_debug);

    cl::NDRange global(height, width);
    err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    if (err != CL_SUCCESS) {
        std::cerr << "Error enqueueing kernel: " << err << std::endl;
        return;
    }

    queue.finish();

    std::vector<char> flat_next(height * width);
    std::vector<char> debug_info(height * width * 6);
    err = queue.enqueueReadBuffer(buffer_next, CL_TRUE, 0, height * width * sizeof(char), flat_next.data());
    if (err != CL_SUCCESS) {
        std::cerr << "Error reading buffer_next: " << err << std::endl;
        return;
    }

    err = queue.enqueueReadBuffer(buffer_debug, CL_TRUE, 0, height * width * 6 * sizeof(char), debug_info.data());
    if (err != CL_SUCCESS) {
        std::cerr << "Error reading buffer_debug: " << err << std::endl;
        return;
    }

    /*
    std::cout << "Debug Information (after reading from buffer):\n";
    for (int i = 0; i < height * width; ++i) {
        std::cout << debug_info[i * 6] << debug_info[i * 6 + 1] << debug_info[i * 6 + 2]
                  << debug_info[i * 6 + 3] << debug_info[i * 6 + 4] << debug_info[i * 6 + 5] << "\n";
    }
    */
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            nextGeneration[i][j] = (flat_next[i * width + j] == '0');
        }
    }

    currentGeneration.swap(nextGeneration);
}

bool Grid::is_stable() {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (nextGeneration[i][j] != currentGeneration[i][j]) {
                return false;
            }
        }
    }
    return true;
}
void Grid::clearScreen() const {
    #ifdef _WIN32
        system("cls");
    #else
        int result = system("clear");
        (void)result;
    #endif
}
