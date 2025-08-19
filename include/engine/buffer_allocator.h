#pragma once
#include <memory>
#include <array>
#include <queue>
#include <iostream>
#include <ostream>
#include "engine/buffer.h"

namespace Voxel {

    class BufferAllocator {
    public:
        BufferAllocator();
        void allocate_buffer(unsigned int& slot);
        void free_buffer(unsigned int slot);

        static constexpr unsigned int MAX_OBJECTS = 3000;

        std::array<GLuint, MAX_OBJECTS> vbo_ids;
        std::array<GLuint, MAX_OBJECTS> ebo_ids;
        std::array<GLuint, MAX_OBJECTS> ssbo_ids;

        std::array<GLuint, MAX_OBJECTS> vertex_array_objects;
        std::array<void*, MAX_OBJECTS> vertex_buffer_objects;
        std::array<void*, MAX_OBJECTS> element_buffer_objects;
        std::array<void*, MAX_OBJECTS> shader_storage_buffer_objects;
        std::queue<unsigned int> freeSlots;
    };
}