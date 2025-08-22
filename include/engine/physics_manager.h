#pragma once
#include <cstdarg>
#include <queue>
#include <functional>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <glm/glm.hpp>

#include "core/log.h"
#include "engine/input.h"

JPH_SUPPRESS_WARNINGS

using namespace JPH;
using namespace JPH::literals;

namespace {
    namespace PhysicsLayers {
        static constexpr ObjectLayer NON_MOVING = 0;
        static constexpr ObjectLayer MOVING = 1;
        static constexpr ObjectLayer NUM_LAYERS = 2;
    }
}

namespace Voxel::Physics {
    class PhysicsManager {
    public:
        static PhysicsManager& get_instance() {
            static PhysicsManager instance;
            return instance;
        }

        PhysicsManager();
        ~PhysicsManager();

        void add_body(BodyCreationSettings& settings, unsigned int& slot);
        void remove_body(unsigned int slot);
        bool update(float& lerp_t);

        std::vector<std::function<void()>> physics_subscribers;
    private:
        struct implementation;
        std::unique_ptr<implementation> m_implementation;
    };
}