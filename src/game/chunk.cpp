#include "chunk.h"
#include "buffer_allocator.h"
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

    static std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash> chunks;
    static std::unique_ptr<BufferAllocator> buffer_allocator;

    Chunk* Chunk::create(int* height_map, BlockType* block_types, std::vector<glm::ivec2>& tree_positions, Noise& noise, glm::ivec3 position) {
        std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(height_map, block_types, tree_positions, noise, position);
        Chunk* ptr = chunk.get();
        chunks[ChunkPos {position.x, position.y, position.z}] = std::move(chunk);
        return ptr;
    }

    void Chunk::generate_trees(Noise& noise, int* height_map, std::vector<glm::ivec2>& tree_positions) {
        const int stem_height = 2 + rand() % 3;
        const int leafs_height = 3 + rand() % 2;
        for (auto& tree_position : tree_positions) {
            int ground_y = height_map[tree_position.x + (tree_position.y * SIZE)];
            int chunk_y = (ground_y / SIZE) * SIZE;
            if (ground_y > 96 || position.y != chunk_y)
                break;

            int ground_y_chunk_space = (ground_y % SIZE) + 1;


            if (noise.fetch_cave(position.x + tree_position.x, position.y + ground_y_chunk_space, position.z + tree_position.y) >= .7f)
                continue;

            for (int h = 0; h < stem_height; h++) {
                int y = ground_y_chunk_space + h;
                block_types_ptr[tree_position.x + ((y%SIZE) * SIZE) + (tree_position.y * SIZE * SIZE) + ((y/SIZE) * SIZE * SIZE *SIZE)] = BlockType::Wood;
            }

            for (int y = ground_y_chunk_space + stem_height; y < ground_y_chunk_space + stem_height + leafs_height; y++) {
                for (int z = tree_position.y - 1; z < tree_position.y + 2; z++) {
                    for (int x = tree_position.x - 1; x < tree_position.x + 2; x++)
                    {
                        block_types_ptr[x + ((y%SIZE) * SIZE) + (z * SIZE * SIZE) + ((y/SIZE) * SIZE * SIZE *SIZE)] = BlockType::Leafs;
                    }
                }
            }
        }
    }

    void Chunk::set_block(int x, int y, int z, BlockType block) {
        block_types_ptr[x + (y * SIZE) + (z * SIZE * SIZE)] = block;
        int16_t bit {0};
        if (block != BlockType::Air) {
            bit = 1;
            is_empty = false;
        }
        uint16_t& row1 = voxels[z + (y * SIZE)] |= bit << x;
        uint16_t& row2 = voxels[x  + (y * SIZE) + (SIZE * SIZE)] |= bit << z;
        uint16_t& row3 = voxels[x  + (z * SIZE) + ((SIZE * SIZE) * 2)] |= bit << y;
    }

    Chunk::Chunk(int* height_map, BlockType* block_types, std::vector<glm::ivec2>& tree_positions, Noise& noise, glm::ivec3 position) : position(position), block_types_ptr(&block_types[(position.y * SIZE * SIZE)])
    {
        generate_trees(noise, height_map, tree_positions);

        for (uint16_t y = 0; y < SIZE; y++)
        {
            for (uint16_t z = 0; z < SIZE; z++)
            {
                for (uint16_t x = 0; x < SIZE; x++)
                {
                    int noise_value = height_map[x + (z * SIZE)];
                    int world_space_position_y = position.y + y;

                    BlockType block = block_types_ptr[x + (y * SIZE) + (z * SIZE * SIZE)];
                    if (block == BlockType::Air) {
                        if (world_space_position_y == 0) block = BlockType::Bedrock;
                        else if (world_space_position_y == noise_value) {
                            if (world_space_position_y > 128) block = BlockType::Snow;
                            else if (world_space_position_y > 96) block = BlockType::Stone;
                            else if (noise.fetch_cave(position.x + x, position.y + y, position.z + z) < .7f) block = BlockType::Grass;
                        }
                        else if (world_space_position_y < noise_value) {
                            if (noise.fetch_cave(position.x + x, position.y + y, position.z + z) < .7f) {
                                if (world_space_position_y > noise_value - 2) block = BlockType::Dirt;
                                else if (world_space_position_y < 20 && ((float)rand()/RAND_MAX) < .01 ) block = BlockType::Diamond;
                                else block = BlockType::Stone;
                            }

                        }
                    }

                    set_block(x, y, z, block);
                }
            }
        }
    }

    void Chunk::build_mesh() {
        if (built) return;

        Chunk* neighbor_chunks[6] {nullptr};

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

        mesh = std::make_unique<Mesh>(voxels, neighbor_chunks, SIZE);
        built = true;
    }

    void Chunk::render() {
        if (!vao_box_gizmo) {
            vao_box_gizmo = std::make_unique<VAO>();
            Gizmo::setup_line_box_gizmo(*vao_box_gizmo);
        }

        if (!built) return;

        if (!buffer_allocator) buffer_allocator = std::make_unique<BufferAllocator>();

        if (!allocated && mesh->triangles > 0) {
            buffer_allocator->allocate_buffer(slot);
            memcpy(buffer_allocator->vertex_buffer_objects[slot], mesh->vertices.data(), mesh->vertices.size() * sizeof(uint32_t));
            memcpy(buffer_allocator->element_buffer_objects[slot], mesh->indices.data(), mesh->indices.size() * sizeof(unsigned int));
            memcpy(buffer_allocator->shader_storage_buffer_objects[slot], block_types_ptr, 4096 * sizeof(unsigned int));
            allocated = true;
        }

        ResourceManager::get_resource<Shader>("greedy").use().set_uniform_mat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(position)));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_allocator->ssbo_ids[slot]);
        glBindVertexArray(buffer_allocator->vertex_array_objects[slot]);
        glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        Gizmo::render_line_box_gizmo(*vao_box_gizmo.get(), position, glm::vec3(16.f));
    }

    void Chunk::unload() {
        if (!allocated) return;

        allocated = false;
        buffer_allocator->free_buffer(slot);
    }
}