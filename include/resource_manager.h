#pragma once
#include <memory>
#include <unordered_map>
#include "shader.h"
#include "texture.h"

namespace Voxel::ResourceManager {
    template<typename T>
    inline std::unordered_map<std::string, std::unique_ptr<T>>& get_storage() {
        static std::unordered_map<std::string, std::unique_ptr<T>> storage;
        return storage;
    }

    template<typename T, typename... Args>
    inline T& create_resource(const std::string& name, Args&&... args) {
        auto resource = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *resource;
        get_storage<T>()[name] = std::move(resource);
        return ref;
    }

    template<typename T>
    inline T& get_resource(const std::string& name) {
        auto& storage = get_storage<T>();
        auto it = storage.find(name);
        if (it == storage.end()) {
            throw std::runtime_error("Resource not found: " + name);
        }
        return *it->second;
    }
}
