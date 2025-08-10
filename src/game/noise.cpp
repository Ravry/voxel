#include "noise.h"

namespace Voxel {

    Noise::Noise() {
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFractalOctaves(5);
        noise.SetFractalLacunarity(2.f);
    }

    float Noise::fetch_heightmap(float x, float z) {
        // return 1;
        // return (x < 16 && z < 16 && x >= 0 && z >= 0) ? 1 : 0;
        return (noise.GetNoise(x, z) + 1) / 2;
        // return y > 0 ? 0 : 1;
    }
}