#include "physics_manager.h"


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
        const uint cNumBodyMutexes = 0;
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

        BodyCreationSettings sphere_settings(new SphereShape(0.5f), RVec3(200.0_r, 100.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
        sphere_id = body_interface.CreateAndAddBody(sphere_settings, EActivation::Activate);
        body_interface.SetLinearVelocity(sphere_id, Vec3(0.0f, -1.0f, 0.0f));

        physics_system->OptimizeBroadPhase();
    }

    static int body_c {0};
    void PhysicsManager::add_body_from_shape(JPH::Ref<Shape>& shape, BodyID& id, RVec3 position) {
        if (!shape.GetPtr()) {
            LOG("ERROR: Null shape passed to add_body_from_shape");
            return;
        }

        BodyCreationSettings body_settings(shape, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
        id = body_interface_ptr->CreateAndAddBody(body_settings, EActivation::DontActivate);

        if (id.IsInvalid()) {
            // LOG("ERROR: Failed to create body");
        }
        else {
            body_c++;
        }

        LOG("body_c: {}", body_c);
    }

    void PhysicsManager::free_body(BodyID& id) {
        body_interface_ptr->RemoveBody(id);
        body_interface_ptr->DestroyBody(id);
    }


    bool PhysicsManager::update(glm::vec3& position, glm::vec3& prev_position, float& lerp_t) {
        static bool activate_physics {false};
        static float accumulator {0.f};

        if (Input::is_key_pressed(GLFW_KEY_F))
            activate_physics = true;

        accumulator += Time::Timer::delta_time;

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
                body_interface_ptr->SetPosition(sphere_id, Vec3(200, 100.f, 0), EActivation::Activate);
            }

            const int cCollisionSteps = 1;
            physics_system->Update(cDeltaTime, cCollisionSteps, temp_allocator.get(), job_system.get());

            accumulator -= cDeltaTime;
        }

        lerp_t = accumulator / cDeltaTime;

        return true;
    }


    PhysicsManager::~PhysicsManager() {
        free_body(sphere_id);
        UnregisterTypes();
    }
}
