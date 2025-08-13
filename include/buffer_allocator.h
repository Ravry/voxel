#pragma once
#include <memory>
#include <array>
#include <queue>
#include "buffer.h"

namespace Voxel {

    class BufferAllocator {
    public:
        BufferAllocator();
        void allocate_buffer(unsigned int& slot);

        static constexpr unsigned int MAX_OBJECTS = 20000;
        std::array<GLuint, MAX_OBJECTS> vertex_array_objects;
        std::array<void*, MAX_OBJECTS> vertex_buffer_objects;
        std::array<void*, MAX_OBJECTS> element_buffer_objects;
        std::queue<unsigned int> freeSlots;
    };
}