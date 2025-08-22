#include "game/chunk_manager.h"

namespace Voxel::Game {
    static int64_t chunk_position_to_key(int x, int z) {
        return ((int64_t)x << 32) | (uint32_t)z;
    }
    
    static glm::vec3 chunk_key_to_position(int64_t key) {
        return glm::vec3(key >> 32, 0, (int32_t)key);
    }

    static glm::ivec3 world_space_to_chunk_space(glm::ivec3 position_world_space) {
        return glm::ivec3(
            (position_world_space.x / SIZE) * SIZE,
            0,
            (position_world_space.z / SIZE) * SIZE
        );
    }

    static std::thread worker_thread;
    static std::atomic<bool> worker_should_exit {false};
    static std::condition_variable worker_cv;

    static std::unordered_map<int64_t, std::unique_ptr<ChunkCompound>> chunks_cached;

    static std::unordered_map<int64_t, ChunkCompound*> chunks_render;
    static std::mutex chunks_render_mutex;

    static bool position_updated {false};
    static glm::ivec3 player_current_chunk_position {0};
    static std::mutex player_position_mutex;

    static Noise noise;

    int ChunkManager::chunk_render_distance {8};

    void ChunkManager::worker_func() {
        const int render_distance_squared = chunk_render_distance * chunk_render_distance;
        
        while (!worker_should_exit) {
            glm::vec3 _position;
            std::unique_lock<std::mutex> lock(player_position_mutex);
            worker_cv.wait(lock, [] { return position_updated || worker_should_exit; });
            position_updated = false;
            _position = player_current_chunk_position;
            lock.unlock();

            std::queue<int64_t> chunks_requested;
            for (int x = -chunk_render_distance; x <= chunk_render_distance; x++) {
                for (int z = -chunk_render_distance; z <= chunk_render_distance; z++) {
                    if (x * x + z * z <= render_distance_squared) {
                        chunks_requested.push(
                            chunk_position_to_key(_position.x + x * SIZE, _position.z + z * SIZE)
                        );
                    }
                }
            }

            std::unordered_map<int64_t, ChunkCompound*> _chunks_new;
            {
                while (!chunks_requested.empty()) {
                    auto chunk_key = chunks_requested.front();
                    chunks_requested.pop();

                    if (!chunks_cached.contains(chunk_key)) {
                        chunks_cached[chunk_key] = std::make_unique<ChunkCompound>(noise, chunk_key_to_position(chunk_key));
                    }
                    _chunks_new[chunk_key] = chunks_cached[chunk_key].get();
                }
            }

            for (auto& [_, chunk] : _chunks_new) {
                chunk->build_chunk_meshes();
            }

            {
                std::lock_guard<std::mutex> lock_render(chunks_render_mutex);
                for (auto it = chunks_render.begin(); it != chunks_render.end(); ) {
                    if (!_chunks_new.contains(it->first)) {
                        it->second->unload();
                        it = chunks_render.erase(it);
                    } else {
                        ++it;
                    }
                }

                for (auto& [chunk_key, chunk] : _chunks_new) {
                    chunks_render[chunk_key] = chunk;
                }
            }

            num_chunks = chunks_cached.size();
        }
    }

    ChunkManager::ChunkManager(glm::ivec3 position) {
        worker_thread = std::thread(worker_func);
        on_new_chunk_entered(position);
    }

    ChunkManager::~ChunkManager() {
        worker_should_exit = true;
        worker_cv.notify_one();
        worker_thread.join();
    }

    void ChunkManager::update(glm::ivec3 position_world_space) {
        glm::ivec3 position_chunk_space = world_space_to_chunk_space(position_world_space);
        static glm::ivec3 position_current = position_chunk_space;
        if (position_current != position_chunk_space) {
            position_current = position_chunk_space;
            on_new_chunk_entered(position_chunk_space);
        }
    }

    void ChunkManager::render_chunk_compounds(Plane* frustum, Shader& shader) {
        std::lock_guard<std::mutex> lock_render(chunks_render_mutex);
        for (auto& [_, chunk] : chunks_render) {
            if (!is_box_in_frustum(frustum, chunk->position, chunk->position + glm::vec3(SIZE, SIZE * NUM_CHUNKS_PER_COMPOUND, SIZE)))
                continue;

            chunk->render(frustum, shader);
        }
    }

    void ChunkManager::on_new_chunk_entered(glm::ivec3 position_chunk_space) {
        {
            std::lock_guard<std::mutex> lock_position(player_position_mutex);
            position_updated = true;
            player_current_chunk_position = position_chunk_space;
        }

        worker_cv.notify_one();
    }

    int ChunkManager::num_chunks {0};
}