#include "chunk.h"

namespace Voxel::Game {
    static std::map<int, std::shared_ptr<Chunk>> chunks;
    static std::unique_ptr<SSBO> ssbo;

    std::shared_ptr<Chunk> Chunk::create(Noise& noise, glm::vec3 position) {
        std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(noise, position);
        chunks[position.x + (position.y * SIZE) + (position.z * SIZE * SIZE)] = chunk;
        return chunk;
    }

    Chunk::Chunk(Noise& noise, glm::vec3 position) : position(position) {
        if (!ssbo.get()) {
            ssbo = std::make_unique<SSBO>();
        }

        for (uint16_t y = 0; y < SIZE; y++) {
            for (uint16_t z = 0; z < SIZE; z++)
            {
                for (uint16_t x = 0; x < SIZE; x++)
                {
                    uint16_t& row1 = voxels[z + (y * SIZE)];
                    uint16_t& row2 = voxels[x + (y * SIZE) + (SIZE * SIZE)];
                    uint16_t& row3 = voxels[x + (z * SIZE) + ((SIZE * SIZE) * 2)];

                    uint16_t noise_value = noise.fetch(x, y, z);

                    data[x + (y * SIZE) + (z * SIZE * SIZE)] = rand() % BlockType::MaxValue;

                    row1 |= noise_value << x;
                    row2 |= noise_value << z;
                    row3 |= noise_value << y;
                }
            }
        }
    }

    void Chunk::build_mesh() {
        if (built) return;

        Chunk* neighbor_chunks[6] {};

        int chunk_left_index = (position.x - SIZE) + (position.y * SIZE) + (position.z * SIZE * SIZE);
        if (chunks.find(chunk_left_index) != chunks.end()) neighbor_chunks[0] = chunks[chunk_left_index].get();
        else return;

        int chunk_right_index = (position.x + SIZE) + (position.y * SIZE) + (position.z * SIZE * SIZE);
        if (chunks.find(chunk_right_index) != chunks.end()) neighbor_chunks[1] = chunks[chunk_right_index].get();
        else return;

        int chunk_front_index = position.x + (position.y * SIZE) + ((position.z - SIZE) * SIZE * SIZE);
        if (chunks.find(chunk_front_index) != chunks.end()) neighbor_chunks[2] = chunks[chunk_front_index].get();
        else return;

        int chunk_back_index = position.x + (position.y * SIZE) + ((position.z + SIZE) * SIZE * SIZE);
        if (chunks.find(chunk_back_index) != chunks.end()) neighbor_chunks[3] = chunks[chunk_back_index].get();
        else return;

        // int chunk_bottom_index = position.x + ((position.y - SIZE) * SIZE) + (position.z * SIZE * SIZE);
        // if (chunks.find(chunk_bottom_index) != chunks.end()) neighbor_chunks[4] = chunks[chunk_bottom_index].get();
        // else return;
        //
        // int chunk_top_index = position.x + ((position.y + SIZE) * SIZE) + (position.z * SIZE * SIZE);
        // if (chunks.find(chunk_top_index) != chunks.end()) neighbor_chunks[5] = chunks[chunk_top_index].get();
        // else return;

        mesh = std::make_shared<Mesh>(voxels, neighbor_chunks, SIZE);
        instance = std::make_shared<Instance3D>(mesh.get(), glm::vec3(0, 0, 0), position /*+ glm::vec3((position.x/16) * 2, (position.y/16) * 2,  (position.z / 16) * 2)*/);
        built = true;
    }


    void Chunk::render() {
        if (!built) return;

        Shader* shader = Shader::get_shader("greedy").get();
        shader->use();
        ssbo->bind();
        ssbo->data(0, data, sizeof(data));
        instance->render(*shader);
        ssbo->unbind();
    }
}
