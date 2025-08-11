#include "chunk.h"
#include "gizmo.h"

namespace Voxel::Game {
    static std::unique_ptr<VAO> vao_box_gizmo;

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

    std::shared_ptr<Chunk> Chunk::create(int* height_map, Noise& noise, glm::ivec3 position) {
        std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(height_map, noise, position);
        chunks[ChunkPos {position.x, position.y, position.z}] = chunk;
        return chunk;
    }

    Chunk::Chunk(int* height_map, Noise& noise, glm::ivec3 position) : position(position) {
        auto set_block = [&](int x, int y, int z, BlockType block) {
            data[x + (y * SIZE) + (z * SIZE * SIZE)] = block;
            int16_t bit {0};
            if (block != BlockType::Air) {
                bit = 1;
                is_empty = false;
            }
            uint16_t& row1 = voxels[z + (y * SIZE)] |= bit << x;
            uint16_t& row2 = voxels[x  + (y * SIZE) + (SIZE * SIZE)] |= bit << z;
            uint16_t& row3 = voxels[x  + (z * SIZE) + ((SIZE * SIZE) * 2)] |= bit << y;
        };

        for (uint16_t y = 0; y < SIZE; y++) {
            for (uint16_t z = 0; z < SIZE; z++)
            {
                for (uint16_t x = 0; x < SIZE; x++)
                {
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

                    set_block(x, y, z, block);
                }
            }
        }
    }

    void Chunk::build_mesh(int* height_map, std::vector<glm::ivec2>& tree_positions) {
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

        auto set_block = [&](int x, int y, int z, BlockType block) {
            data[x + (y * SIZE) + (z * SIZE * SIZE)] = block;
            int16_t bit {0};
            if (block != BlockType::Air) {
                bit = 1;
                is_empty = false;
            }
            uint16_t& row1 = voxels[z + (y * SIZE)] |= bit << x;
            uint16_t& row2 = voxels[x  + (y * SIZE) + (SIZE * SIZE)] |= bit << z;
            uint16_t& row3 = voxels[x  + (z * SIZE) + ((SIZE * SIZE) * 2)] |= bit << y;
        };

        const int tree_height = 2 + rand() % 3;
        const int leafs_height = 4;
        for (auto& tree_position : tree_positions) {
            int ground_y = height_map[tree_position.x + (tree_position.y * SIZE)];
            int chunk_y = (ground_y / 16) * 16;
            if (ground_y > 96 || position.y != chunk_y)
                break;

            int ground_y_chunk_space = (ground_y % 16) + 1;

            for (int h = 0; h < tree_height; h++) {
                int y = ground_y_chunk_space + h;
                if (y >= 16)
                    break;
                set_block(tree_position.x, y, tree_position.y, BlockType::Wood);
            }

            for (int y = ground_y_chunk_space + tree_height; y < ground_y_chunk_space + tree_height + leafs_height; y++) {
                if (y >= 16)
                    break;

                for (int z = tree_position.y -1; z < tree_position.y + 2; z++) {
                    for (int x = tree_position.x - 1; x < tree_position.x + 2; x++)
                    {
                        set_block(x, y, z, BlockType::Leafs);
                    }
                }
            }
        }

        mesh = std::make_shared<Mesh>(voxels, neighbor_chunks, SIZE);
        built = true;
    }


    void Chunk::render() {
        if (!vao_box_gizmo.get()) {
            vao_box_gizmo = std::make_unique<VAO>();
            Gizmo::setup_line_box_gizmo(*vao_box_gizmo.get());
        }

        if (!built) return;

        if (!instance.get() && mesh->triangles > 0) {
            instance = std::make_unique<Instance3D>(mesh.get(), glm::vec3(1.f), position);
            ssbo = std::make_unique<SSBO>();
            ssbo->bind();
            ssbo->data(0, data, sizeof(unsigned int) * 16 * 16 * 16);
            ssbo->unbind();
        }
        else if (instance.get()) {
            Gizmo::render_line_box_gizmo(*vao_box_gizmo.get(), position, glm::vec3(16.f));
            ssbo->bind();
            instance->render(ResourceManager::get_resource<Shader>("greedy"));
            ssbo->unbind();
        }
    }
}
