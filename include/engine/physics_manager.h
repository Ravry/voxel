#pragma once
#include <cstdarg>
#include <queue>
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

namespace Voxel::Physics {
    class PhysicsManager {
    public:
        static PhysicsManager& get_instance() {
            static PhysicsManager instance;
            return instance;
        }

        PhysicsManager();
        ~PhysicsManager();

        void add_body(JPH::BodyID& body_id);
        bool update(glm::vec3& position, glm::vec3& prev_position, float& lerp_t);

    private:
        struct implementation;
        std::unique_ptr<implementation> m_implementation;
    };
}