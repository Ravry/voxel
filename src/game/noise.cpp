#include "noise.h"

namespace Voxel {

    Noise::Noise() {
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    }

    float Noise::fetch(float x, float y, float z) {
        return (noise.GetNoise(x, z) + 1) / 2;
    }
}