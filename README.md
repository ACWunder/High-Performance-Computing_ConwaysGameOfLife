# High-Performance Computing: Conway's Game of Life

This project was part of the **High-Performance Computing Practical Course**, where I optimized **Conway's Game of Life** using advanced techniques like **OpenCL** to leverage parallel computing.

---

## 📖 About the Project

The aim of the project was to implement and optimize Conway's Game of Life on a toroidal grid, focusing on efficient memory usage, parallelization, and performance. The implementation involved:

- A scalar version for benchmarking.
- An optimized parallel version using OpenCL kernels for high-performance execution.
- A command-line interface for user interaction with features like grid manipulation, pattern addition, and evolution visualization.

---

## ✨ Features

- **OpenCL Integration**: Accelerated computation with custom kernels.
- **Benchmarking**: Comparison of scalar and parallel implementations across various grid sizes.
- **Command-Line Interface**: 
  - Load/save worlds.
  - Add patterns (e.g., Gliders, Beacons).
  - Run and visualize multiple generations.
- **Randomized World Creation**: Generate worlds with random patterns.

---

## 🚀 Performance Highlights

- Optimization techniques included memory-efficient data types, parallel task distribution, and minimizing race conditions.
- Benchmarked performance on grid sizes up to \(10,000 \times 10,000\) cells, achieving significant speed-ups in the parallel version.

---

This project demonstrates the power of high-performance computing techniques and their application to computationally intensive problems.
