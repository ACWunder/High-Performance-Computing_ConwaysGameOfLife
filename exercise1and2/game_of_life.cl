__kernel void evolve(__global char* current, __global char* next, int height, int width, __global char* debug) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    int count = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx != 0 || dy != 0) {
                int nx = (x + dx + height) % height;
                int ny = (y + dy + width) % width;
                count += (current[nx * width + ny] == '0');
            }
        }
    }

    int index = x * width + y;
    int cell_state = (current[index] == '0');
    next[index] = (cell_state ? (count == 2 || count == 3) : (count == 3)) ? '0' : '.';

    // Write debug information
    debug[index * 6] = 'C';
    debug[index * 6 + 1] = (char)('0' + x / 10);
    debug[index * 6 + 2] = (char)('0' + x % 10);
    debug[index * 6 + 3] = (char)('0' + y / 10);
    debug[index * 6 + 4] = (char)('0' + y % 10);
    debug[index * 6 + 5] = next[index];
}
