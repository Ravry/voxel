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

    namespace Layers {
        static constexpr ObjectLayer NON_MOVING = 0;
        static constexpr ObjectLayer MOVING = 1;
        static constexpr ObjectLayer NUM_LAYERS = 2;
    }

    class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
    {
    public:
        virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
                case Layers::NON_MOVING:
                    return inObject2 == Layers::MOVING;
                case Layers::MOVING:
                    return true;
                default:
                    JPH_ASSERT(false);
                    return false;
            }
        }
    };

    namespace BroadPhaseLayers
    {
        static constexpr BroadPhaseLayer NON_MOVING(0);
        static constexpr BroadPhaseLayer MOVING(1);
        static constexpr uint NUM_LAYERS(2);
    };

    class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
        {
            mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        }

        virtual uint GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
        {
            JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
            return mObjectToBroadPhase[inLayer];
        }

        #if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
        {
            switch ((BroadPhaseLayer::Type)inLayer)
            {
                case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
                case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
                default:													JPH_ASSERT(false); return "INVALID";
            }
        }
        #endif

    private:
        BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
    {
    public:
        virtual bool				ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
                case Layers::NON_MOVING:
                    return inLayer2 == BroadPhaseLayers::MOVING;
                case Layers::MOVING:
                    return true;
                default:
                    JPH_ASSERT(false);
                    return false;
            }
        }
    };

    constexpr uint cMaxBodies = { 1024 };
    constexpr uint cNumBodyMutexes = { 0 };
    constexpr uint cMaxBodyPairs = { 1024 };
    constexpr uint cMaxContactConstraints { 1024 };
    constexpr float cDeltaTime { 1.0f / 60.0f };

    class PhysicsManager {
    public:
        static PhysicsManager& get_instance() {
            static PhysicsManager instance;
            return instance;
        }

        PhysicsManager();
        ~PhysicsManager();
        bool update(glm::vec3& position, glm::vec3& prev_position, float& lerp_t);

    private:
        struct implementation;
        std::unique_ptr<implementation> m_implementation;
    };
}