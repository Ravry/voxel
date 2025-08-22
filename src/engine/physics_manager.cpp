#include "engine/physics_manager.h"

#include "Jolt/Physics/Body/BodyLockMulti.h"
#include <stack>

namespace {
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
}

namespace Voxel::Physics {
    struct PhysicsManager::implementation {
        BasicBroadPhaseLayerInterface broad_phase_layer_interface;
        BasicObjectVsBroadPhaseLayerFilter object_vs_broadphase_layer_filter;
        BasicObjectLayerVsObjectLayerFilter object_vs_object_layer_filter;
        PhysicsSystem physics_system;
        TempAllocatorImpl temp_allocator{10 * 1024 * 1024};
        JobSystemThreadPool job_system{ cMaxPhysicsJobs, cMaxPhysicsBarriers, static_cast<int>(thread::hardware_concurrency() - 1)};
        BodyInterface* body_interface_ptr;
        BodyID sphere_id;
    };

    void trace_implementation(const char *inFMT, ...) {
        va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);
        plog_warn("{}", buffer);
    }

    bool assert_implementation(const char *inExpression, const char *inMessage, const char *inFile, uint inLine) {
        plog_error("{}:{}: ({}) {}", inFile, inLine, inExpression, (inMessage != nullptr) ? inMessage : "");
        return true;
    }

    static std::unordered_map<unsigned int, Body*> bodies_map;

    PhysicsManager::PhysicsManager() {
        static bool once {false};
        if (!once) {
            once = true;
            RegisterDefaultAllocator();
            static std::unique_ptr<Factory> factory = std::make_unique<Factory>();
            Factory::sInstance = factory.get();
            Trace = trace_implementation;
            JPH_IF_ENABLE_ASSERTS(AssertFailed = assert_implementation);
            RegisterTypes();
        }

        m_implementation = std::make_unique<implementation>();

	    m_implementation->physics_system.Init(
	        cMaxBodies,
	        cNumBodyMutexes,
	        cMaxBodyPairs,
	        cMaxContactConstraints,
	        m_implementation->broad_phase_layer_interface,
	        m_implementation->object_vs_broadphase_layer_filter,
	        m_implementation->object_vs_object_layer_filter
	    );

        m_implementation->body_interface_ptr = &m_implementation->physics_system.GetBodyInterface();

        BodyCreationSettings sphere_settings(new SphereShape(0.5f), RVec3(20.0_r, 100.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, PhysicsLayers::MOVING);
        m_implementation->sphere_id = m_implementation->body_interface_ptr->CreateAndAddBody(sphere_settings, EActivation::Activate);
        m_implementation->body_interface_ptr->SetLinearVelocity(m_implementation->sphere_id, Vec3(0.0f, -1.0f, 0.0f));

        m_implementation->physics_system.OptimizeBroadPhase();
    }

    PhysicsManager::~PhysicsManager() {
        auto& body_interface = m_implementation->physics_system.GetBodyInterface();
        for (auto& [_, body] : bodies_map) {
            if (!body) return;
            body_interface.RemoveBody(body->GetID());
            body_interface.DestroyBody(body->GetID());
        }
        bodies_map.clear();
        UnregisterTypes();
    }


    void PhysicsManager::add_body(BodyCreationSettings& settings, unsigned int& slot) {
        static unsigned int c_slot = 0;
        slot = c_slot++;
        auto& body_interface = m_implementation->physics_system.GetBodyInterface();
        BodyCreationSettings _settings(
            new SphereShape(.5f),
            Vec3(0, 0, 0),
            Quat::sIdentity(),
            EMotionType::Static,
            PhysicsLayers::NON_MOVING
        );
        Body* body = body_interface.CreateBody(settings);
        body_interface.AddBody(body->GetID(), EActivation::DontActivate);
        bodies_map[slot] = body;
    }

    void PhysicsManager::remove_body(unsigned int slot) {
        auto body = bodies_map[slot];
        if (!body) return;
        // if (body->GetID().IsInvalid()) return;
        auto& body_interface = m_implementation->physics_system.GetBodyInterface();
        body_interface.RemoveBody(body->GetID());
        body_interface.DestroyBody(body->GetID());
        bodies_map.erase(slot);
    }

    bool PhysicsManager::update(float& lerp_t) {
        static float accumulator {0.f};
        accumulator += Time::delta_time;

        while (accumulator >= cDeltaTime) {
            for (auto& sub : physics_subscribers) sub();
            m_implementation->physics_system.Update(cDeltaTime, cCollisionSteps, &m_implementation->temp_allocator, &m_implementation->job_system);
            accumulator -= cDeltaTime;
        }

        lerp_t = accumulator / cDeltaTime;
        return true;
    }
}