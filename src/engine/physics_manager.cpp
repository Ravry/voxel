#include "engine/physics_manager.h"

#include "Jolt/Physics/Body/BodyLockMulti.h"
#include <stack>

namespace Voxel::Physics {
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

    Body* PhysicsManager::add_body(BodyCreationSettings& settings, unsigned int& slot) {
        static unsigned int c_slot = 0;
        slot = c_slot++;
        auto& body_interface = m_implementation->physics_system.GetBodyInterface();
        Body* body = body_interface.CreateBody(settings);
        body_interface.AddBody(body->GetID(), EActivation::DontActivate);
        bodies_map[slot] = body;
        return body;
    }

    void PhysicsManager::remove_body(unsigned int slot) {
        auto body = bodies_map[slot];
        if (!body) return;
        if (body->GetID().IsInvalid()) return;
        auto& body_interface = m_implementation->physics_system.GetBodyInterface();
        body_interface.RemoveBody(body->GetID());
        body_interface.DestroyBody(body->GetID());
        bodies_map.erase(slot);
    }

    bool PhysicsManager::update() {
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