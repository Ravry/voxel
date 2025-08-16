#pragma once
#include <thread>
#include <condition_variable>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>
#include "log.h"
#include "chunk_compound.h"
#include "utils.h"
#include "camera.h"

namespace Voxel::Game {
    class ChunkManager {
    public:
        ChunkManager(glm::ivec3 position);
        ~ChunkManager();
        void update(glm::ivec3 position);
        void render_chunk_compounds(Camera& camera, bool frustum_cull);

        static void worker_func();
    private:
        void on_new_chunk_entered(glm::ivec3 chunk_space_position);
    private:
        int chunk_render_distance {8};
    };
}