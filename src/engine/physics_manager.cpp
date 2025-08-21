#include "engine/physics_manager.h"

namespace {
    namespace PhysicsLayers {
        static constexpr ObjectLayer NON_MOVING = 0;
        static constexpr ObjectLayer MOVING = 1;
        static constexpr ObjectLayer NUM_LAYERS = 2;
    }

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
    };

    bool assert_implementation(const char *inExpression, const char *inMessage, const char *inFile, uint inLine) {
        plog_error("{}:{}: ({}) {}", inFile, inLine, inExpression, (inMessage != nullptr) ? inMessage : "");
        return true;
    };

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
        UnregisterTypes();
    }

    void PhysicsManager::add_body(JPH::BodyID& body_id) {

    }

    bool PhysicsManager::update(glm::vec3& position, glm::vec3& prev_position, float& lerp_t) {
        static bool activate_physics {false};
        static float accumulator {0.f};

        if (Input::is_key_pressed(GLFW_KEY_F)) activate_physics = true;

        accumulator += Time::delta_time;

        while (accumulator >= cDeltaTime) {
            if (!activate_physics) {
                accumulator = 0.f;
                return false;
            }

            prev_position = position;

            if (m_implementation->body_interface_ptr->IsActive(m_implementation->sphere_id)) {
                RVec3 _position = m_implementation->body_interface_ptr->GetCenterOfMassPosition(m_implementation->sphere_id);
                position.x = _position.GetX();
                position.y = _position.GetY() - 0.5f;
                position.z = _position.GetZ();
            } else {
                m_implementation->body_interface_ptr->SetPosition(m_implementation->sphere_id, Vec3(25, 100.f, 0), EActivation::Activate);
            }

            const int cCollisionSteps = 1;
            m_implementation->physics_system.Update(
                cDeltaTime,
                cCollisionSteps,
                &m_implementation->temp_allocator,
                &m_implementation->job_system
            );

            accumulator -= cDeltaTime;
        }

        lerp_t = accumulator / cDeltaTime;

        return true;
    }
}