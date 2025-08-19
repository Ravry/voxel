#pragma once
#include <thread>
#include <condition_variable>
#include "game/chunk_compound.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>
#include "core/log.h"
#include "engine/time.h"
#include "engine/camera.h"

namespace Voxel::Game {
    class ChunkManager {
    public:
        ChunkManager(glm::ivec3 position);
        ~ChunkManager();
        void update(glm::ivec3 position);
        void render_chunk_compounds(Plane* frustum, Shader& shader);

        static void worker_func();
        static int chunk_render_distance;
        static int num_chunks;
    private:
        void on_new_chunk_entered(glm::ivec3 chunk_space_position);
    private:
    };
}