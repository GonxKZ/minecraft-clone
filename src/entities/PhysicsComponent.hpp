/**
 * @file PhysicsComponent.hpp
 * @brief VoxelCraft ECS - Physics Component
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * Physics component for entities that need physical simulation.
 */

#ifndef VOXELCRAFT_ENTITIES_PHYSICS_COMPONENT_HPP
#define VOXELCRAFT_ENTITIES_PHYSICS_COMPONENT_HPP

#include "Component.hpp"
#include <memory>

namespace VoxelCraft {

    class RigidBody;
    class Collider;

    /**
     * @enum PhysicsBodyType
     * @brief Type of physics body
     */
    enum class PhysicsBodyType {
        Static,         ///< Static body (no movement)
        Kinematic,      ///< Kinematic body (scripted movement)
        Dynamic         ///< Dynamic body (full physics simulation)
    };

    /**
     * @enum CollisionLayer
     * @brief Collision layers for filtering
     */
    enum class CollisionLayer {
        Default = 1 << 0,       ///< Default collision layer
        Player = 1 << 1,        ///< Player collision layer
        Enemy = 1 << 2,         ///< Enemy collision layer
        Terrain = 1 << 3,       ///< Terrain collision layer
        Projectile = 1 << 4,    ///< Projectile collision layer
        Trigger = 1 << 5,       ///< Trigger collision layer
        UI = 1 << 6,            ///< UI collision layer
        All = 0xFFFFFFFF        ///< All collision layers
    };

    /**
     * @struct PhysicsProperties
     * @brief Physical properties of the body
     */
    struct PhysicsProperties {
        float mass;                     ///< Mass of the body (kg)
        float linearDamping;            ///< Linear damping (0-1)
        float angularDamping;           ///< Angular damping (0-1)
        float restitution;              ///< Restitution (bounciness, 0-1)
        float friction;                 ///< Friction coefficient
        float rollingFriction;          ///< Rolling friction coefficient
        bool isTrigger;                 ///< Is this a trigger volume?
        uint32_t collisionLayer;        ///< Collision layer mask
        uint32_t collisionMask;         ///< Collision mask (what it can collide with)

        PhysicsProperties()
            : mass(1.0f)
            , linearDamping(0.1f)
            , angularDamping(0.1f)
            , restitution(0.5f)
            , friction(0.5f)
            , rollingFriction(0.1f)
            , isTrigger(false)
            , collisionLayer(static_cast<uint32_t>(CollisionLayer::Default))
            , collisionMask(0xFFFFFFFF)
        {}
    };

    /**
     * @class PhysicsComponent
     * @brief Component for entities that need physical simulation
     *
     * The PhysicsComponent integrates the entity with the physics system,
     * providing collision detection, rigid body dynamics, and physical interaction.
     */
    class PhysicsComponent : public Component {
    public:
        /**
         * @brief Constructor
         * @param owner Entity that owns this component
         */
        PhysicsComponent(Entity* owner);

        /**
         * @brief Constructor with body type
         * @param owner Entity that owns this component
         * @param bodyType Type of physics body
         */
        PhysicsComponent(Entity* owner, PhysicsBodyType bodyType);

        /**
         * @brief Constructor with full properties
         * @param owner Entity that owns this component
         * @param bodyType Type of physics body
         * @param properties Physics properties
         */
        PhysicsComponent(Entity* owner,
                        PhysicsBodyType bodyType,
                        const PhysicsProperties& properties);

        /**
         * @brief Destructor
         */
        ~PhysicsComponent() override;

        /**
         * @brief Get component type name
         * @return Component type name
         */
        std::string GetTypeName() const override { return "PhysicsComponent"; }

        /**
         * @brief Initialize the component
         * @return true if successful, false otherwise
         */
        bool Initialize() override;

        /**
         * @brief Update the component
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime) override;

        /**
         * @brief Render debug information
         */
        void Render() override;

        // Body type operations

        /**
         * @brief Set body type
         * @param bodyType New body type
         */
        void SetBodyType(PhysicsBodyType bodyType);

        /**
         * @brief Get body type
         * @return Current body type
         */
        PhysicsBodyType GetBodyType() const { return m_bodyType; }

        // Physics properties

        /**
         * @brief Set physics properties
         * @param properties New physics properties
         */
        void SetProperties(const PhysicsProperties& properties);

        /**
         * @brief Get physics properties
         * @return Current physics properties
         */
        const PhysicsProperties& GetProperties() const { return m_properties; }

        /**
         * @brief Set mass
         * @param mass New mass value
         */
        void SetMass(float mass);

        /**
         * @brief Get mass
         * @return Current mass
         */
        float GetMass() const { return m_properties.mass; }

        // Velocity operations

        /**
         * @brief Set linear velocity
         * @param velocity New linear velocity
         */
        void SetLinearVelocity(const glm::vec3& velocity);

        /**
         * @brief Get linear velocity
         * @return Current linear velocity
         */
        glm::vec3 GetLinearVelocity() const;

        /**
         * @brief Set angular velocity
         * @param velocity New angular velocity
         */
        void SetAngularVelocity(const glm::vec3& velocity);

        /**
         * @brief Get angular velocity
         * @return Current angular velocity
         */
        glm::vec3 GetAngularVelocity() const;

        /**
         * @brief Apply force to the body
         * @param force Force vector
         * @param relativePosition Relative position to apply force (optional)
         */
        void ApplyForce(const glm::vec3& force, const glm::vec3& relativePosition = glm::vec3(0.0f));

        /**
         * @brief Apply impulse to the body
         * @param impulse Impulse vector
         * @param relativePosition Relative position to apply impulse (optional)
         */
        void ApplyImpulse(const glm::vec3& impulse, const glm::vec3& relativePosition = glm::vec3(0.0f));

        /**
         * @brief Apply torque to the body
         * @param torque Torque vector
         */
        void ApplyTorque(const glm::vec3& torque);

        /**
         * @brief Clear all forces and velocities
         */
        void ClearForces();

        // Collision operations

        /**
         * @brief Add collider to the body
         * @param collider Collider to add
         * @return true if added, false otherwise
         */
        bool AddCollider(std::shared_ptr<Collider> collider);

        /**
         * @brief Remove collider from the body
         * @param collider Collider to remove
         * @return true if removed, false otherwise
         */
        bool RemoveCollider(std::shared_ptr<Collider> collider);

        /**
         * @brief Get all colliders
         * @return Vector of colliders
         */
        std::vector<std::shared_ptr<Collider>> GetColliders() const;

        /**
         * @brief Clear all colliders
         */
        void ClearColliders();

        // State operations

        /**
         * @brief Enable/disable the physics body
         * @param enabled Enabled state
         */
        void SetEnabled(bool enabled);

        /**
         * @brief Check if physics body is enabled
         * @return true if enabled, false otherwise
         */
        bool IsEnabled() const { return m_isEnabled; }

        /**
         * @brief Enable/disable gravity
         * @param gravityEnabled Gravity state
         */
        void SetGravityEnabled(bool gravityEnabled);

        /**
         * @brief Check if gravity is enabled
         * @return true if gravity enabled, false otherwise
         */
        bool IsGravityEnabled() const { return m_gravityEnabled; }

        /**
         * @brief Enable/disable kinematic mode
         * @param kinematic Kinematic state
         */
        void SetKinematic(bool kinematic);

        /**
         * @brief Check if body is kinematic
         * @return true if kinematic, false otherwise
         */
        bool IsKinematic() const { return m_isKinematic; }

        // Collision callbacks

        /**
         * @brief Set collision enter callback
         * @param callback Function called when collision starts
         */
        void SetCollisionEnterCallback(std::function<void(PhysicsComponent*)> callback);

        /**
         * @brief Set collision stay callback
         * @param callback Function called while collision continues
         */
        void SetCollisionStayCallback(std::function<void(PhysicsComponent*)> callback);

        /**
         * @brief Set collision exit callback
         * @param callback Function called when collision ends
         */
        void SetCollisionExitCallback(std::function<void(PhysicsComponent*)> callback);

        /**
         * @brief Set trigger enter callback
         * @param callback Function called when trigger is entered
         */
        void SetTriggerEnterCallback(std::function<void(PhysicsComponent*)> callback);

        /**
         * @brief Set trigger exit callback
         * @param callback Function called when trigger is exited
         */
        void SetTriggerExitCallback(std::function<void(PhysicsComponent*)> callback);

        // Physics state

        /**
         * @brief Get the underlying rigid body
         * @return Rigid body pointer
         */
        RigidBody* GetRigidBody() const { return m_rigidBody.get(); }

        /**
         * @brief Synchronize transform with physics body
         */
        void SyncTransform();

        /**
         * @brief Check if body is sleeping
         * @return true if sleeping, false otherwise
         */
        bool IsSleeping() const;

        /**
         * @brief Wake up the physics body
         */
        void WakeUp();

        /**
         * @brief Put the physics body to sleep
         */
        void Sleep();

    private:
        /**
         * @brief Create the underlying rigid body
         */
        void CreateRigidBody();

        /**
         * @brief Destroy the underlying rigid body
         */
        void DestroyRigidBody();

        /**
         * @brief Update physics properties on the rigid body
         */
        void UpdatePhysicsProperties();

        PhysicsBodyType m_bodyType;              ///< Type of physics body
        PhysicsProperties m_properties;          ///< Physics properties
        std::unique_ptr<RigidBody> m_rigidBody;  ///< Underlying rigid body
        std::vector<std::shared_ptr<Collider>> m_colliders; ///< Attached colliders

        bool m_isEnabled;                        ///< Enabled state
        bool m_gravityEnabled;                   ///< Gravity enabled state
        bool m_isKinematic;                      ///< Kinematic state

        // Collision callbacks
        std::function<void(PhysicsComponent*)> m_onCollisionEnter;
        std::function<void(PhysicsComponent*)> m_onCollisionStay;
        std::function<void(PhysicsComponent*)> m_onCollisionExit;
        std::function<void(PhysicsComponent*)> m_onTriggerEnter;
        std::function<void(PhysicsComponent*)> m_onTriggerExit;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_PHYSICS_COMPONENT_HPP
