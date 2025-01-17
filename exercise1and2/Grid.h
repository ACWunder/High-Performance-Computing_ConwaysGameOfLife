#ifndef GRID_H
#define GRID_H

#include <vector>
#include <string>

class Grid {
    private:
        int height, width;
        std::vector<std::vector<bool>> currentGeneration;
        std::vector<std::vector<bool>> nextGeneration;
        bool printEnabled;

        int countLiveNeighbors(int x, int y) const;
        void evolve();
        void evolve_cpu();
        bool is_stable();
        void clearScreen() const;
        void print() const;
        void randomize();

    public:
        Grid();
        Grid(int h, int w);
        std::pair<int, int> to2D(int p) const;
        void initializePattern(const std::string &filename);
        long long run(int generations, int delay_ms);
        long long run_with_opencl(int generations, int delay_ms);
        bool load(const std::string &filename);
        bool save(const std::string &filename) const;
        void setSize(int h, int w);
        int getHeight() const;
        int getWidth() const;
        void setCell(int x, int y, bool state);
        void setCell(int p, bool state);
        bool getCell(int x, int y) const;
        bool getCell(int p) const;
        void addGlider(int x, int y);
        void addToad(int x, int y);
        void addBeacon(int x, int y);
        void addRPentomino(int x, int y);
        void setPrintEnabled(bool enabled);
};

#endif // GRID_H
