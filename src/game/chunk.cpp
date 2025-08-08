#include "chunk.h"

namespace Voxel::Game {
    struct ChunkPos {
        int32_t x, y, z;

        bool operator<(const ChunkPos& other) const {
            if (x != other.x) return x < other.x;
            if (y != other.y) return y < other.y;
            return z < other.z;
        }

        bool operator==(const ChunkPos& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct ChunkPosHash {
        std::size_t operator()(const ChunkPos& pos) const {
            return std::hash<int32_t>()(pos.x) ^
                   (std::hash<int32_t>()(pos.y) << 1) ^
                   (std::hash<int32_t>()(pos.z) << 2);
        }
    };

    static std::unordered_map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosHash> chunks;
    static std::unique_ptr<SSBO> ssbo;

    std::shared_ptr<Chunk> Chunk::create(Noise& noise, glm::ivec3 position) {
        std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(noise, position);
        chunks[ChunkPos {position.x, position.y, position.z}] = chunk;
        return chunk;
    }

    Chunk::Chunk(Noise& noise, glm::ivec3 position) : position(position) {
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

                    int world_space_position_y = position.y + y;

                    uint16_t noise_value = (int)(noise.fetch(position.x + x, world_space_position_y, position.z + z) * 31);

                    BlockType block = BlockType::Air;
                    if (world_space_position_y > 0) {
                        if (world_space_position_y > 10) {
                            if (world_space_position_y > 25)
                                block = BlockType::Snow;
                            else
                                block = BlockType::Stone;
                        }
                        else {
                            if (world_space_position_y == noise_value) {
                                block = BlockType::Grass;
                            }
                            else
                                block = BlockType::Dirt;
                        }
                    }
                    else {
                        block = BlockType::Bedrock;
                    }

                    data[x + (y * SIZE) + (z * SIZE * SIZE)] = block;
                    noise_value = world_space_position_y <= noise_value ? 1 : 0;
                    row1 |= noise_value << x;
                    row2 |= noise_value << z;
                    row3 |= noise_value << y;
                }
            }
        }
    }

    void Chunk::build_mesh() {
        if (built) return;

        Chunk* neighbor_chunks[6] {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

        ChunkPos chunk_left_index {position.x - SIZE, position.y, position.z};
        if (chunks.find(chunk_left_index) != chunks.end()) neighbor_chunks[0] = chunks[chunk_left_index].get();
        else return;

        ChunkPos chunk_right_index { position.x + SIZE, position.y, position.z };
        if (chunks.find(chunk_right_index) != chunks.end()) neighbor_chunks[1] = chunks[chunk_right_index].get();
        else return;

        ChunkPos chunk_front_index { position.x, position.y, position.z - SIZE };
        if (chunks.find(chunk_front_index) != chunks.end()) neighbor_chunks[2] = chunks[chunk_front_index].get();
        else return;

        ChunkPos chunk_back_index { position.x, position.y, position.z + SIZE };
        if (chunks.find(chunk_back_index) != chunks.end()) neighbor_chunks[3] = chunks[chunk_back_index].get();
        else return;

        ChunkPos chunk_bottom_index { position.x, position.y - SIZE, position.z };
        if (chunks.find(chunk_bottom_index) != chunks.end()) neighbor_chunks[4] = chunks[chunk_bottom_index].get();

        ChunkPos chunk_top_index { position.x, position.y + SIZE, position.z };
        if (chunks.find(chunk_top_index) != chunks.end()) neighbor_chunks[5] = chunks[chunk_top_index].get();

        mesh = std::make_shared<Mesh>(voxels, neighbor_chunks, SIZE);
        instance = std::make_shared<Instance3D>(mesh.get(), glm::vec3(0, 0, 0), position);
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
