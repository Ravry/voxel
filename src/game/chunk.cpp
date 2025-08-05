#include "chunk.h"

namespace Voxel::Game {
    Chunk::Chunk(Noise& noise, glm::vec3 position) : position(position) {
        uint16_t voxels[SIZE * SIZE * 3] = {};

        for (uint16_t y = 0; y < SIZE; y++)
        {
            for (uint16_t z = 0; z < SIZE; z++)
            {
                for (uint16_t x = 0; x < SIZE; x++)
                {
                    uint16_t& row1 = voxels[z + (y * SIZE)];
                    uint16_t& row2 = voxels[x + (y * SIZE) + (SIZE * SIZE)];
                    uint16_t& row3 = voxels[x + (z * SIZE) + ((SIZE * SIZE) * 2)];

                    uint16_t noise_value = noise.fetch(x, y, z);
                    //if (noise_value)
                        // instance = std::make_unique<Instance3D>()(&cube_mesh, glm::vec3((float)noise_value), glm::vec3(x + .5, y + .5, z + .5), glm::vec3(.2f));

                    data[x + (y * SIZE) + (z * SIZE * SIZE)] = rand() % BlockType::MaxValue;

                    row1 |= noise_value << x;
                    row2 |= noise_value << z;
                    row3 |= noise_value << y;
                }
            }
        }

        mesh = std::make_shared<Mesh>(voxels, SIZE);
        instance = std::make_shared<Instance3D>(mesh.get(), glm::vec3(0, 0, 0), position);
    }

    void Chunk::render(Shader& shader, SSBO& ssbo) {
        ssbo.bind();
        ssbo.data(0, data, sizeof(data));
        instance->render(shader);
        ssbo.unbind();
    }
}
