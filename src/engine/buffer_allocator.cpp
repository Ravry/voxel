#include "engine/buffer_allocator.h"

namespace Voxel {
    BufferAllocator::BufferAllocator() {

        glGenVertexArrays(MAX_OBJECTS, vertex_array_objects.data());
        glGenBuffers(MAX_OBJECTS, vbo_ids.data());
        glGenBuffers(MAX_OBJECTS, ebo_ids.data());
        glGenBuffers(MAX_OBJECTS, ssbo_ids.data());

        for (size_t i {0}; i < MAX_OBJECTS; ++i) {
            freeSlots.push(i);
            glBindVertexArray(vertex_array_objects[i]);

            GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

            glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[i]);
            glBufferStorage(GL_ARRAY_BUFFER, sizeof(uint32_t) * 4000, nullptr, flags);
            vertex_buffer_objects[i] = (void*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(uint32_t) * 4000, flags);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_ids[i]);
            glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 15000, nullptr, flags);
            element_buffer_objects[i] = (void*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * 15000, flags);

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_ids[i]);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * 4096, nullptr, flags);
            shader_storage_buffer_objects[i] = (void*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int) * 4096, flags);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_ids[i]);

            glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(uint32_t), (void*)0);
            glEnableVertexAttribArray(0);
        }
    }

    void BufferAllocator::allocate_buffer(unsigned int& slot) {
        if (freeSlots.empty()) return;
        slot = freeSlots.front();
        freeSlots.pop();
    }

    void BufferAllocator::free_buffer(unsigned int slot) {
        freeSlots.push(slot);
    }
}
