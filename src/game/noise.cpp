#include "noise.h"

namespace Voxel {
    inline float normalize_noise(float noise) {
        return (noise + 1.f) * .5f;
    }

    Noise::Noise() {
        biome_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

        terrain_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        terrain_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        terrain_noise.SetFractalOctaves(5);
        terrain_noise.SetFractalLacunarity(2.f);
    }

    float Noise::fetch_heightmap(float x, float z) {
        // return 1;
        // return (x < 16 && z < 16 && x >= 0 && z >= 0) ? 1 : 0;

        float biomeValue = normalize_noise(biome_noise.GetNoise(x * 0.1f, z * 0.1f));
        biomeValue -= .4f;
        float amplitude = biomeValue * 128.0f;
        const float baseHeight = 4.0f * 16.0f;
        float terrainValue = normalize_noise(terrain_noise.GetNoise(x, z));
        return baseHeight + terrainValue * amplitude;
        // return y > 0 ? 0 : 1;
    }

    float Noise::fetch_cave(float x, float y, float z) {
        return normalize_noise(terrain_noise.GetNoise(x, y, z));
    }


}