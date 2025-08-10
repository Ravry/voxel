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

    std::shared_ptr<Chunk> Chunk::create(int* height_map, std::vector<glm::ivec2>& tree_positions, Noise& noise, glm::ivec3 position) {
        std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(height_map, tree_positions, noise, position);
        chunks[ChunkPos {position.x, position.y, position.z}] = chunk;
        return chunk;
    }

    Chunk::Chunk(int* height_map, std::vector<glm::ivec2>& tree_positions, Noise& noise, glm::ivec3 position) : position(position) {
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

                    int noise_value = height_map[x + (z * SIZE)];

                    int world_space_position_y = position.y + y;
                    BlockType block = BlockType::Air;
                    if (world_space_position_y == 0)
                        block = BlockType::Bedrock;
                    else if (world_space_position_y == noise_value) {
                        if (world_space_position_y > 128)
                            block = BlockType::Snow;
                        else if (world_space_position_y > 96)
                            block = BlockType::Stone;
                        else
                            block = BlockType::Grass;
                    }
                    else if (world_space_position_y < noise_value) {
                        block = BlockType::Dirt;
                    }

                    data[x + (y * SIZE) + (z * SIZE * SIZE)] = block;

                    noise_value = block == BlockType::Air ? 0 : 1;
                    row1 |= noise_value << x;
                    row2 |= noise_value << z;
                    row3 |= noise_value << y;
                }
            }
        }

        const int tree_height = 4;
        for (auto& tree_position : tree_positions) {
            int ground_y = height_map[tree_position.x + (tree_position.y * SIZE)];

            if (ground_y > 96)
                break;

            for (int y = 0; y < SIZE; y++) {
                int y_position_world_space = y + position.y;

                if (y_position_world_space > ground_y) {
                    int current_tree_height = (y_position_world_space - ground_y);
                    if (current_tree_height < tree_height) {
                        data[tree_position.x + (y * SIZE) + (tree_position.y * SIZE * SIZE)] = BlockType::Wood;
                        uint16_t& row1 = voxels[tree_position.y + (y * SIZE)];
                        uint16_t& row2 = voxels[tree_position.x  + (y * SIZE) + (SIZE * SIZE)];
                        uint16_t& row3 = voxels[tree_position.x  + (tree_position.y * SIZE) + ((SIZE * SIZE) * 2)];
                        row1 |= 1 << tree_position.x ;
                        row2 |= 1 << tree_position.y;
                        row3 |= 1 << y;
                    }
                    else if (current_tree_height < tree_height + 4) {
                        for (int z = tree_position.y - 2; z < tree_position.y + 3; z++) {
                            for (int x = tree_position.x - 2; x < tree_position.x + 3; x++)
                            {
                                int relX = x - tree_position.x;
                                int relZ = z - tree_position.y;
                                if (current_tree_height - 4 != 1 && current_tree_height - 4 != 2) {
                                    if (relX <= -2 || relX >= 2 || relZ <= -2 || relZ >= 2)
                                        continue;
                                }

                                data[x + (y * SIZE) + (z * SIZE * SIZE)] = BlockType::Leafs;
                                uint16_t& row1 = voxels[z + (y * SIZE)];
                                uint16_t& row2 = voxels[x  + (y * SIZE) + (SIZE * SIZE)];
                                uint16_t& row3 = voxels[x  + (z * SIZE) + ((SIZE * SIZE) * 2)];
                                row1 |= 1 << x;
                                row2 |= 1 << z;
                                row3 |= 1 << y;
                            }
                        }
                    }
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

        Shader& shader = ResourceManager::get_resource<Shader>("greedy");
        shader.use();
        ssbo->bind();
        ssbo->data(0, data, sizeof(data));
        instance->render(shader);
        ssbo->unbind();
    }
}
