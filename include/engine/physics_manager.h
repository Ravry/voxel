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

    class BasicObjectLayerVsObjectLayerFilter : public ObjectLayerPairFilter
    {
    public:
        virtual bool ShouldCollide(ObjectLayer object_a, ObjectLayer object_b) const override {
            switch (object_a)
            {
                case PhysicsLayers::NON_MOVING:
                    return object_b == PhysicsLayers::MOVING;
                case PhysicsLayers::MOVING:
                    return true;
                default:
                    plog_error("invalid phase");
                    return false;
            }
        }
    };

    namespace BroadPhaseLayers {
        static constexpr BroadPhaseLayer NON_MOVING(0);
        static constexpr BroadPhaseLayer MOVING(1);
        static constexpr uint NUM_LAYERS(2);
    };

    class BasicBroadPhaseLayerInterface final : public BroadPhaseLayerInterface
    {
    public:
        BasicBroadPhaseLayerInterface() {
            mObjectToBroadPhase[PhysicsLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[PhysicsLayers::MOVING] = BroadPhaseLayers::MOVING;
        }

        virtual uint GetNumBroadPhaseLayers() const override {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override {
            JPH_ASSERT(inLayer < PhysicsLayers::NUM_LAYERS);
            return mObjectToBroadPhase[inLayer];
        }

        #if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override {
            switch ((BroadPhaseLayer::Type)inLayer) {
                case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
                    return "NON_MOVING";
                case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
                    return "MOVING";
                default:
                    return "INVALID";
            }
        }
        #endif

    private:
        BroadPhaseLayer mObjectToBroadPhase[PhysicsLayers::NUM_LAYERS];
    };

    class BasicObjectVsBroadPhaseLayerFilter : public ObjectVsBroadPhaseLayerFilter
    {
    public:
        virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override {
            switch (inLayer1) {
                case PhysicsLayers::NON_MOVING:
                    return inLayer2 == BroadPhaseLayers::MOVING;
                case PhysicsLayers::MOVING:
                    return true;
                default:
                    return false;
            }
        }
    };

    constexpr uint cMaxBodies = { 1024 };
    constexpr uint cNumBodyMutexes = { 0 };
    constexpr uint cMaxBodyPairs = { 1024 };
    constexpr uint cMaxContactConstraints { 1024 };
    constexpr float cDeltaTime { 1.0f / 60.0f };
    constexpr int cCollisionSteps = 1;

    class PhysicsManager {
    public:
        static PhysicsManager& get_instance() {
            static PhysicsManager instance;
            return instance;
        }

        PhysicsManager();
        ~PhysicsManager();

        Body* add_body(BodyCreationSettings& settings, unsigned int& slot);
        void remove_body(unsigned int slot);
        bool update();

        std::vector<std::function<void()>> physics_subscribers;

        struct implementation {
            BasicBroadPhaseLayerInterface broad_phase_layer_interface;
            BasicObjectVsBroadPhaseLayerFilter object_vs_broadphase_layer_filter;
            BasicObjectLayerVsObjectLayerFilter object_vs_object_layer_filter;
            PhysicsSystem physics_system;
            TempAllocatorImpl temp_allocator{10 * 1024 * 1024};
            JobSystemThreadPool job_system{ cMaxPhysicsJobs, cMaxPhysicsBarriers, static_cast<int>(thread::hardware_concurrency() - 1)};
            BodyInterface* body_interface_ptr;
            BodyID sphere_id;
        };
        std::unique_ptr<implementation> m_implementation;
        float lerp_t {0.f};
    };
}