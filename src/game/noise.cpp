#include "noise.h"

namespace Voxel {

    Noise::Noise() {
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    }

    float Noise::fetch(float x, float y, float z) {
        // return 1;
        return (x < 16 && z < 16 && x >= 0 && z >= 0) ? 1 : 0;
        // return (noise.GetNoise(x, z) + 1) / 2;
    }
}