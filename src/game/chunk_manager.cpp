#include "chunk_manager.h"

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
    static std::atomic<bool> worker_should_exit;
    static std::condition_variable worker_cv;

    static std::queue<int64_t> chunks_requested;
    static std::mutex chunks_requested_mutex;

    static std::unordered_map<int64_t, std::unique_ptr<ChunkCompound>> chunks_cached;

    static std::unordered_map<int64_t, ChunkCompound*> chunks_render;
    static std::mutex chunks_render_mutex;

    static Noise noise;

    void ChunkManager::worker_func() {
        while (!worker_should_exit) {
            std::unique_lock<std::mutex> lock(chunks_requested_mutex);
            worker_cv.wait(lock, [] { return !chunks_requested.empty() || worker_should_exit; });
            std::vector<int64_t> _chunk_requests;
            while (!chunks_requested.empty()) {
                _chunk_requests.push_back(chunks_requested.front());
                chunks_requested.pop();
            }
            lock.unlock();

            std::unordered_map<int64_t, ChunkCompound*> _chunks_new;
            {
                for (int64_t chunk_key : _chunk_requests) {
                    if (!chunks_cached.contains(chunk_key)) {
                        chunks_cached[chunk_key] = std::make_unique<ChunkCompound>(noise, chunk_key_to_position(chunk_key));
                    }
                    _chunks_new[chunk_key] = chunks_cached[chunk_key].get();
                }
            }

            for (auto& chunk : _chunks_new) {
                chunk.second->build_chunk_meshes();
            }

            {
                std::lock_guard<std::mutex> lock_render(chunks_render_mutex);
                for (auto& chunk : chunks_render) {
                    if (!_chunks_new.contains(chunk.first)) {
                        chunk.second->unload();
                        chunks_render.erase(chunk.first);
                    }
                }

                for (auto& chunk : _chunks_new) {
                    chunks_render[chunk.first] = chunk.second;
                }
            }
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
        // LOG("ChunkManager::~ChunkManager()");
    }

    void ChunkManager::update(glm::ivec3 position_world_space) {
        glm::ivec3 position_chunk_space = world_space_to_chunk_space(position_world_space);
        static glm::ivec3 position_current = position_chunk_space;
        if (position_current != position_chunk_space) {
            position_current = position_chunk_space;
            on_new_chunk_entered(position_chunk_space);
        }
    }

    void ChunkManager::render_chunk_compounds(Camera& camera, bool frustum_cull, Shader& shader) {
        std::lock_guard<std::mutex> lock_render(chunks_render_mutex);
        for (auto& chunk : chunks_render) {
            if (frustum_cull && !camera.is_box_in_frustum(camera.frustum, chunk.second->position, chunk.second->position + glm::vec3(16.f, 16.f * num_chunks_per_compound, 16.f)))
                continue;

            chunk.second->render(camera, frustum_cull, shader);
        }
    }

    void ChunkManager::on_new_chunk_entered(glm::ivec3 position_chunk_space) {
        const int render_distance_squared = chunk_render_distance * chunk_render_distance;
        {
            std::lock_guard<std::mutex> lock(chunks_requested_mutex);
            for (int x = -chunk_render_distance; x <= chunk_render_distance; x++) {
                for (int z = -chunk_render_distance; z <= chunk_render_distance; z++) {
                    if (x * x + z * z <= render_distance_squared) {
                        chunks_requested.push(chunk_position_to_key(position_chunk_space.x + x * SIZE, position_chunk_space.z + z * SIZE));
                    }
                }
            }
        }

        worker_cv.notify_one();
    }
}