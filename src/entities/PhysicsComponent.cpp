/**
 * @file PhysicsComponent.cpp
 * @brief VoxelCraft ECS - Physics Component Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "PhysicsComponent.hpp"
#include "Entity.hpp"
#include "TransformComponent.hpp"
#include "Logger.hpp"

namespace VoxelCraft {

    // Forward declarations for physics classes
    class RigidBody {
    public:
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 linearVelocity;
        glm::vec3 angularVelocity;
        float mass;
        bool isSleeping;
        bool isKinematic;
        bool gravityEnabled;

        RigidBody()
            : position(0.0f)
            , rotation(1.0f, 0.0f, 0.0f, 0.0f)
            , linearVelocity(0.0f)
            , angularVelocity(0.0f)
            , mass(1.0f)
            , isSleeping(false)
            , isKinematic(false)
            , gravityEnabled(true)
        {}
    };

    class Collider {
    public:
        enum class Type {
            Box,
            Sphere,
            Capsule,
            Mesh,
            Compound
        };

        Type type;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
        bool isTrigger;

        Collider(Type colliderType = Type::Box)
            : type(colliderType)
            , position(0.0f)
            , rotation(1.0f, 0.0f, 0.0f, 0.0f)
            , scale(1.0f)
            , isTrigger(false)
        {}
    };

    PhysicsComponent::PhysicsComponent(Entity* owner)
        : Component(owner, "PhysicsComponent")
        , m_bodyType(PhysicsBodyType::Dynamic)
        , m_properties()
        , m_rigidBody(nullptr)
        , m_isEnabled(true)
        , m_gravityEnabled(true)
        , m_isKinematic(false)
    {
        VOXELCRAFT_TRACE("PhysicsComponent created for entity '{}'",
                        owner ? owner->GetName() : "null");
    }

    PhysicsComponent::PhysicsComponent(Entity* owner, PhysicsBodyType bodyType)
        : Component(owner, "PhysicsComponent")
        , m_bodyType(bodyType)
        , m_properties()
        , m_rigidBody(nullptr)
        , m_isEnabled(true)
        , m_gravityEnabled(true)
        , m_isKinematic(bodyType == PhysicsBodyType::Kinematic)
    {
        VOXELCRAFT_TRACE("PhysicsComponent created for entity '{}' with body type {}",
                        owner ? owner->GetName() : "null", static_cast<int>(bodyType));
    }

    PhysicsComponent::PhysicsComponent(Entity* owner,
                                     PhysicsBodyType bodyType,
                                     const PhysicsProperties& properties)
        : Component(owner, "PhysicsComponent")
        , m_bodyType(bodyType)
        , m_properties(properties)
        , m_rigidBody(nullptr)
        , m_isEnabled(true)
        , m_gravityEnabled(true)
        , m_isKinematic(bodyType == PhysicsBodyType::Kinematic)
    {
        VOXELCRAFT_TRACE("PhysicsComponent created for entity '{}' with full properties",
                        owner ? owner->GetName() : "null");
    }

    PhysicsComponent::~PhysicsComponent() {
        VOXELCRAFT_TRACE("PhysicsComponent destroyed for entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");

        DestroyRigidBody();
    }

    bool PhysicsComponent::Initialize() {
        VOXELCRAFT_TRACE("PhysicsComponent initialized for entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");

        CreateRigidBody();
        return m_rigidBody != nullptr;
    }

    void PhysicsComponent::Update(double deltaTime) {
        if (!m_isEnabled || !m_rigidBody || m_isKinematic) {
            return;
        }

        // Sync transform component with physics body
        SyncTransform();

        // Handle physics simulation
        // In a real implementation, this would integrate with a physics engine
        // For now, we'll do basic integration

        if (!m_rigidBody->isSleeping) {
            // Apply gravity if enabled
            if (m_gravityEnabled && !m_isKinematic) {
                m_rigidBody->linearVelocity.y -= 9.81f * deltaTime;
            }

            // Apply damping
            m_rigidBody->linearVelocity *= (1.0f - m_properties.linearDamping * deltaTime);
            m_rigidBody->angularVelocity *= (1.0f - m_properties.angularDamping * deltaTime);

            // Update position and rotation
            m_rigidBody->position += m_rigidBody->linearVelocity * static_cast<float>(deltaTime);

            // Simple angular integration (simplified)
            if (glm::length(m_rigidBody->angularVelocity) > 0.001f) {
                glm::quat deltaRotation = glm::quat(
                    0.0f,
                    m_rigidBody->angularVelocity.x * deltaTime * 0.5f,
                    m_rigidBody->angularVelocity.y * deltaTime * 0.5f,
                    m_rigidBody->angularVelocity.z * deltaTime * 0.5f
                );
                m_rigidBody->rotation = glm::normalize(m_rigidBody->rotation * deltaRotation);
            }

            // Check for sleep condition
            if (glm::length(m_rigidBody->linearVelocity) < 0.01f &&
                glm::length(m_rigidBody->angularVelocity) < 0.01f) {
                m_rigidBody->isSleeping = true;
            }
        }
    }

    void PhysicsComponent::Render() {
        // Render debug information for physics bodies
        if (!m_isEnabled || !m_rigidBody) {
            return;
        }

        // In a real implementation, this would render:
        // - Collision shapes
        // - Velocity vectors
        // - Center of mass
        // - Contact points

        VOXELCRAFT_TRACE("Physics debug render for entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");
    }

    void PhysicsComponent::SetBodyType(PhysicsBodyType bodyType) {
        if (m_bodyType != bodyType) {
            m_bodyType = bodyType;
            m_isKinematic = (bodyType == PhysicsBodyType::Kinematic);

            // Recreate rigid body with new type
            DestroyRigidBody();
            CreateRigidBody();

            VOXELCRAFT_INFO("Physics body type changed to {} for entity '{}'",
                           static_cast<int>(bodyType),
                           GetOwner() ? GetOwner()->GetName() : "null");
        }
    }

    void PhysicsComponent::SetProperties(const PhysicsProperties& properties) {
        m_properties = properties;
        UpdatePhysicsProperties();
    }

    void PhysicsComponent::SetMass(float mass) {
        if (mass > 0.0f && m_properties.mass != mass) {
            m_properties.mass = mass;
            UpdatePhysicsProperties();
        }
    }

    void PhysicsComponent::SetLinearVelocity(const glm::vec3& velocity) {
        if (m_rigidBody) {
            m_rigidBody->linearVelocity = velocity;
            m_rigidBody->isSleeping = false;
            WakeUp();
        }
    }

    glm::vec3 PhysicsComponent::GetLinearVelocity() const {
        return m_rigidBody ? m_rigidBody->linearVelocity : glm::vec3(0.0f);
    }

    void PhysicsComponent::SetAngularVelocity(const glm::vec3& velocity) {
        if (m_rigidBody) {
            m_rigidBody->angularVelocity = velocity;
            m_rigidBody->isSleeping = false;
            WakeUp();
        }
    }

    glm::vec3 PhysicsComponent::GetAngularVelocity() const {
        return m_rigidBody ? m_rigidBody->angularVelocity : glm::vec3(0.0f);
    }

    void PhysicsComponent::ApplyForce(const glm::vec3& force, const glm::vec3& relativePosition) {
        if (m_rigidBody && m_isEnabled && !m_isKinematic) {
            // F = ma, so a = F/m
            glm::vec3 acceleration = force / m_properties.mass;
            m_rigidBody->linearVelocity += acceleration * 0.016f; // Assuming 60fps

            // Apply torque if relative position is not zero
            if (glm::length(relativePosition) > 0.001f) {
                glm::vec3 torque = glm::cross(relativePosition, force);
                ApplyTorque(torque);
            }

            m_rigidBody->isSleeping = false;
            WakeUp();
        }
    }

    void PhysicsComponent::ApplyImpulse(const glm::vec3& impulse, const glm::vec3& relativePosition) {
        if (m_rigidBody && m_isEnabled && !m_isKinematic) {
            // Δv = impulse/m
            glm::vec3 deltaVelocity = impulse / m_properties.mass;
            m_rigidBody->linearVelocity += deltaVelocity;

            // Apply angular impulse if relative position is not zero
            if (glm::length(relativePosition) > 0.001f) {
                glm::vec3 angularImpulse = glm::cross(relativePosition, impulse);
                // For simplicity, apply as angular velocity change
                // In a real physics engine, this would be more complex
                m_rigidBody->angularVelocity += angularImpulse / m_properties.mass;
            }

            m_rigidBody->isSleeping = false;
            WakeUp();
        }
    }

    void PhysicsComponent::ApplyTorque(const glm::vec3& torque) {
        if (m_rigidBody && m_isEnabled && !m_isKinematic) {
            // Simple torque application
            // In a real physics engine, this would involve inertia tensor
            m_rigidBody->angularVelocity += torque * 0.016f / m_properties.mass;
            m_rigidBody->isSleeping = false;
            WakeUp();
        }
    }

    void PhysicsComponent::ClearForces() {
        if (m_rigidBody) {
            m_rigidBody->linearVelocity = glm::vec3(0.0f);
            m_rigidBody->angularVelocity = glm::vec3(0.0f);
            m_rigidBody->isSleeping = false;
        }
    }

    bool PhysicsComponent::AddCollider(std::shared_ptr<Collider> collider) {
        if (!collider) {
            return false;
        }

        m_colliders.push_back(collider);
        VOXELCRAFT_TRACE("Added collider to physics component of entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");
        return true;
    }

    bool PhysicsComponent::RemoveCollider(std::shared_ptr<Collider> collider) {
        auto it = std::find(m_colliders.begin(), m_colliders.end(), collider);
        if (it != m_colliders.end()) {
            m_colliders.erase(it);
            VOXELCRAFT_TRACE("Removed collider from physics component of entity '{}'",
                            GetOwner() ? GetOwner()->GetName() : "null");
            return true;
        }
        return false;
    }

    std::vector<std::shared_ptr<Collider>> PhysicsComponent::GetColliders() const {
        return m_colliders;
    }

    void PhysicsComponent::ClearColliders() {
        m_colliders.clear();
        VOXELCRAFT_TRACE("Cleared all colliders from physics component of entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");
    }

    void PhysicsComponent::SetEnabled(bool enabled) {
        if (m_isEnabled != enabled) {
            m_isEnabled = enabled;

            if (m_rigidBody) {
                // In a real implementation, this would enable/disable the rigid body
                VOXELCRAFT_INFO("Physics component {} for entity '{}'",
                              enabled ? "enabled" : "disabled",
                              GetOwner() ? GetOwner()->GetName() : "null");
            }
        }
    }

    void PhysicsComponent::SetGravityEnabled(bool gravityEnabled) {
        if (m_gravityEnabled != gravityEnabled) {
            m_gravityEnabled = gravityEnabled;

            if (m_rigidBody) {
                m_rigidBody->gravityEnabled = gravityEnabled;
                VOXELCRAFT_INFO("Gravity {} for physics component of entity '{}'",
                              gravityEnabled ? "enabled" : "disabled",
                              GetOwner() ? GetOwner()->GetName() : "null");
            }
        }
    }

    void PhysicsComponent::SetKinematic(bool kinematic) {
        if (m_isKinematic != kinematic) {
            m_isKinematic = kinematic;

            if (m_rigidBody) {
                m_rigidBody->isKinematic = kinematic;
                VOXELCRAFT_INFO("Physics component set to {} for entity '{}'",
                              kinematic ? "kinematic" : "dynamic",
                              GetOwner() ? GetOwner()->GetName() : "null");
            }
        }
    }

    void PhysicsComponent::SetCollisionEnterCallback(std::function<void(PhysicsComponent*)> callback) {
        m_onCollisionEnter = callback;
    }

    void PhysicsComponent::SetCollisionStayCallback(std::function<void(PhysicsComponent*)> callback) {
        m_onCollisionStay = callback;
    }

    void PhysicsComponent::SetCollisionExitCallback(std::function<void(PhysicsComponent*)> callback) {
        m_onCollisionExit = callback;
    }

    void PhysicsComponent::SetTriggerEnterCallback(std::function<void(PhysicsComponent*)> callback) {
        m_onTriggerEnter = callback;
    }

    void PhysicsComponent::SetTriggerExitCallback(std::function<void(PhysicsComponent*)> callback) {
        m_onTriggerExit = callback;
    }

    void PhysicsComponent::SyncTransform() {
        if (!m_rigidBody) {
            return;
        }

        // Get transform component
        auto transform = GetOwner()->GetComponent<TransformComponent>();
        if (!transform) {
            return;
        }

        // Sync from transform to rigid body
        m_rigidBody->position = transform->GetPosition();
        m_rigidBody->rotation = transform->GetRotation();

        // Or sync from rigid body to transform (depending on simulation mode)
        // For dynamic bodies, we typically sync from rigid body to transform
        if (m_bodyType == PhysicsBodyType::Dynamic && !m_isKinematic) {
            transform->SetPosition(m_rigidBody->position);
            transform->SetRotation(m_rigidBody->rotation);
        }
    }

    bool PhysicsComponent::IsSleeping() const {
        return m_rigidBody ? m_rigidBody->isSleeping : true;
    }

    void PhysicsComponent::WakeUp() {
        if (m_rigidBody) {
            m_rigidBody->isSleeping = false;
        }
    }

    void PhysicsComponent::Sleep() {
        if (m_rigidBody) {
            m_rigidBody->isSleeping = true;
            m_rigidBody->linearVelocity = glm::vec3(0.0f);
            m_rigidBody->angularVelocity = glm::vec3(0.0f);
        }
    }

    void PhysicsComponent::CreateRigidBody() {
        if (m_rigidBody) {
            return;
        }

        m_rigidBody = std::make_unique<RigidBody>();
        m_rigidBody->mass = m_properties.mass;
        m_rigidBody->isKinematic = m_isKinematic;
        m_rigidBody->gravityEnabled = m_gravityEnabled;

        // Sync initial transform
        SyncTransform();

        VOXELCRAFT_INFO("Created rigid body for physics component of entity '{}'",
                      GetOwner() ? GetOwner()->GetName() : "null");
    }

    void PhysicsComponent::DestroyRigidBody() {
        if (m_rigidBody) {
            VOXELCRAFT_INFO("Destroyed rigid body for physics component of entity '{}'",
                          GetOwner() ? GetOwner()->GetName() : "null");
            m_rigidBody.reset();
        }
    }

    void PhysicsComponent::UpdatePhysicsProperties() {
        if (m_rigidBody) {
            m_rigidBody->mass = m_properties.mass;
        }
    }

} // namespace VoxelCraft
