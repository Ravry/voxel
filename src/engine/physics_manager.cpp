#include "engine/physics_manager.h"


namespace Voxel::Physics {
    static void TraceImpl(const char *inFMT, ...)
    {
        va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);
        LOG("{}", buffer);
    }

    static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
    {
        LOG("{}:{}: ({}) {}", inFile, inLine, inExpression, (inMessage != nullptr) ? inMessage : "");
        return true;
    };

    static BodyID sphere_id;

    PhysicsManager::PhysicsManager() {
        RegisterDefaultAllocator();
        factory = std::make_unique<Factory>();
        Factory::sInstance = factory.get();
        Trace = TraceImpl;
        JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)
        RegisterTypes();
        temp_allocator = std::make_unique<TempAllocatorImpl>(10 * 1024 * 1024);
        job_system = std::make_unique<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
        const uint cMaxBodies = 1024;
        const uint cNumBodyMutexes = 1024;
        const uint cMaxBodyPairs = 1024;
        const uint cMaxContactConstraints = 1024;

        broad_phase_layer_interface = std::make_unique<BPLayerInterfaceImpl>();
        object_vs_broadphase_layer_filter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
        object_vs_object_layer_filter = std::make_unique<ObjectLayerPairFilterImpl>();

        physics_system = std::make_unique<PhysicsSystem>();
	    physics_system->Init(
	        cMaxBodies,
	        cNumBodyMutexes,
	        cMaxBodyPairs,
	        cMaxContactConstraints,
	        *broad_phase_layer_interface,
	        *object_vs_broadphase_layer_filter,
	        *object_vs_object_layer_filter
	    );

        body_activation_listener = std::make_unique<MyBodyActivationListener>();
        physics_system->SetBodyActivationListener(body_activation_listener.get());

        contact_listener = std::make_unique<MyContactListener>();
        physics_system->SetContactListener(contact_listener.get());

        BodyInterface& body_interface = physics_system->GetBodyInterface();
        body_interface_ptr = &body_interface;

        BodyCreationSettings sphere_settings(new SphereShape(0.5f), RVec3(20.0_r, 100.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
        sphere_id = body_interface.CreateAndAddBody(sphere_settings, EActivation::Activate);
        body_interface.SetLinearVelocity(sphere_id, Vec3(0.0f, -1.0f, 0.0f));

        physics_system->OptimizeBroadPhase();
    }

    bool PhysicsManager::update(glm::vec3& position, glm::vec3& prev_position, float& lerp_t) {
        static bool activate_physics {false};
        static float accumulator {0.f};

        if (Input::is_key_pressed(GLFW_KEY_F))
            activate_physics = true;

        accumulator += Time::delta_time;

        while (accumulator >= cDeltaTime) {
            if (!activate_physics) {
                accumulator = 0.f;
                return false;
            }

            prev_position = position;

            if (body_interface_ptr->IsActive(sphere_id)) {
                RVec3 _position = body_interface_ptr->GetCenterOfMassPosition(sphere_id);
                position.x = _position.GetX();
                position.y = _position.GetY() - 0.5f;
                position.z = _position.GetZ();
            } else {
                body_interface_ptr->SetPosition(sphere_id, Vec3(25, 100.f, 0), EActivation::Activate);
            }

            const int cCollisionSteps = 1;
            physics_system->Update(cDeltaTime, cCollisionSteps, temp_allocator.get(), job_system.get());

            accumulator -= cDeltaTime;
        }

        lerp_t = accumulator / cDeltaTime;

        return true;
    }

    std::vector<BodyID> body_ids;
    static std::mutex body_ids_mutex;

    //second-thread
    void PhysicsManager::add_body_from_shape(JPH::Ref<Shape>& shape, BodyID& id, RVec3 position) {
        BodyCreationSettings settings(shape, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
        Body* body = body_interface_ptr->CreateBody(settings);
        if (!body) return;
        id = body->GetID();
        {
            std::lock_guard<std::mutex> lock(body_ids_mutex);
            body_ids.push_back(id);
        }
    }

    std::vector<BodyID> body_ids_to_remove;
    static std::mutex body_ids_to_remove_mutex;

    //second_thread
    void PhysicsManager::remove_body(BodyID &id) {
        std::lock_guard<std::mutex> lock(body_ids_to_remove_mutex);
        body_ids_to_remove.push_back(id);
    }

    //main thread
    void PhysicsManager::commit_bodies() {
        {
            std::lock_guard<std::mutex> lock(body_ids_to_remove_mutex);
            body_interface_ptr->RemoveBodies(body_ids_to_remove.data(), body_ids_to_remove.size());
            body_interface_ptr->DestroyBodies(body_ids_to_remove.data(), body_ids_to_remove.size());
            body_ids_to_remove.clear();
        }

        {
            std::lock_guard<std::mutex> lock(body_ids_mutex);
            if (body_ids.empty()) return;

            BodyInterface::AddState add_state = body_interface_ptr->AddBodiesPrepare(
                body_ids.data(),
                static_cast<int>(body_ids.size())
            );

            body_interface_ptr->AddBodiesFinalize(
                body_ids.data(),
                body_ids.size(),
                add_state,
                EActivation::DontActivate
            );

            body_ids.clear();
        }

        physics_system->OptimizeBroadPhase();
    }


    PhysicsManager::~PhysicsManager() {
        UnregisterTypes();
    }
}
