#include "noise.h"

namespace Voxel {
    const int SIZE = sizeof(uint16_t) * 8;
    uint8_t noise_map[SIZE][SIZE][SIZE] = {};

    Noise::Noise() {
        std::srand(0);

        for (int y = 0; y < SIZE; y++)
        {
            for (int z = 0; z < SIZE; z++)
            {
                for (int x = 0; x < SIZE; x++)
                {
                    noise_map[x][y][z] = rand() % 2;
                    // noise_map[x][y][z] = 1;
                }
            }
        }
    }

    int Noise::fetch(uint8_t x, uint8_t y, uint8_t z) {
        return noise_map[x][y][z];
    }
}