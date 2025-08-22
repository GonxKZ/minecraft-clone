/**
 * @file RigidBody.hpp
 * @brief VoxelCraft Rigid Body Physics System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the RigidBody class that represents rigid body objects
 * in the physics simulation with full 6DOF dynamics, collision detection,
 * and constraint support.
 */

#ifndef VOXELCRAFT_PHYSICS_RIGID_BODY_HPP
#define VOXELCRAFT_PHYSICS_RIGID_BODY_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <any>

#include "../core/Config.hpp"
#include "PhysicsTypes.hpp"
#include "Collider.hpp"
#include "PhysicsMaterial.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Collider;
    class PhysicsMaterial;
    class Constraint;
    class Entity;

    /**
     * @enum RigidBodyType
     * @brief Type of rigid body
     */
    enum class RigidBodyType {
        Static,             ///< Static body (infinite mass, no motion)
        Kinematic,          ///< Kinematic body (motion controlled by user)
        Dynamic             ///< Dynamic body (fully simulated)
    };

    /**
     * @enum RigidBodyFlags
     * @brief Rigid body behavior flags
     */
    enum RigidBodyFlags : uint32_t {
        FLAG_NONE           = 0x0000,
        FLAG_DISABLE_GRAVITY = 0x0001,      ///< Disable gravity for this body
        FLAG_DISABLE_COLLISION = 0x0002,    ///< Disable collision detection
        FLAG_DISABLE_INTEGRATION = 0x0004,  ///< Disable motion integration
        FLAG_ENABLE_CCD      = 0x0008,      ///< Enable continuous collision detection
        FLAG_IS_SLEEPING     = 0x0010,      ///< Body is sleeping
        FLAG_AUTO_SLEEP      = 0x0020,      ///< Enable automatic sleeping
        FLAG_FIXED_ROTATION  = 0x0040,      ///< Fixed rotation (no rotation)
        FLAG_HIGH_PRECISION  = 0x0080,      ///< High precision simulation
        FLAG_DEBUG_DRAW      = 0x0100,      ///< Enable debug drawing
        FLAG_USER_FLAG_0     = 0x1000,      ///< User-defined flag 0
        FLAG_USER_FLAG_1     = 0x2000,      ///< User-defined flag 1
        FLAG_USER_FLAG_2     = 0x4000,      ///< User-defined flag 2
        FLAG_USER_FLAG_3     = 0x8000       ///< User-defined flag 3
    };

    /**
     * @enum ActivationState
     * @brief Rigid body activation state
     */
    enum class ActivationState {
        Active,             ///< Body is active and simulated
        Sleeping,           ///< Body is sleeping (not simulated)
        AlwaysActive,       ///< Body is always active
        AlwaysSleeping      ///< Body is always sleeping
    };

    /**
     * @struct RigidBodyState
     * @brief Complete state of a rigid body
     */
    struct RigidBodyState {
        // Position and orientation
        glm::vec3 position;                 ///< World space position
        glm::quat orientation;              ///< World space orientation

        // Linear motion
        glm::vec3 linearVelocity;           ///< Linear velocity
        glm::vec3 linearAcceleration;       ///< Linear acceleration

        // Angular motion
        glm::vec3 angularVelocity;          ///< Angular velocity (radians/sec)
        glm::vec3 angularAcceleration;      ///< Angular acceleration

        // Forces and torques
        glm::vec3 totalForce;               ///< Total force applied
        glm::vec3 totalTorque;              ///< Total torque applied

        // Timestamps
        double lastUpdateTime;              ///< Last update timestamp
        double creationTime;                ///< Body creation timestamp
    };

    /**
     * @struct RigidBodyProperties
     * @brief Physical properties of a rigid body
     */
    struct RigidBodyProperties {
        // Mass properties
        float mass;                         ///< Body mass (kg)
        float inverseMass;                  ///< Inverse mass (1/mass)
        glm::mat3 inertiaTensor;            ///< Inertia tensor
        glm::mat3 inverseInertiaTensor;     ///< Inverse inertia tensor
        glm::vec3 centerOfMass;             ///< Center of mass (local space)

        // Material properties
        float linearDamping;                ///< Linear damping coefficient
        float angularDamping;               ///< Angular damping coefficient
        float friction;                     ///< Friction coefficient
        float restitution;                  ///< Restitution coefficient
        float rollingFriction;              ///< Rolling friction coefficient

        // Physical limits
        float maxLinearVelocity;            ///< Maximum linear velocity
        float maxAngularVelocity;           ///< Maximum angular velocity
        float maxForce;                     ///< Maximum force magnitude
        float maxTorque;                    ///< Maximum torque magnitude

        // Simulation settings
        float sleepThreshold;               ///< Sleep threshold
        float ccdMotionThreshold;           ///< CCD motion threshold
        int collisionGroup;                 ///< Collision group
        int collisionMask;                  ///< Collision mask
    };

    /**
     * @struct RigidBodyMetrics
     * @brief Performance and simulation metrics for a rigid body
     */
    struct RigidBodyMetrics {
        uint64_t updateCount;               ///< Number of updates performed
        uint64_t collisionCount;            ///< Number of collisions detected
        uint64_t constraintCount;           ///< Number of constraints applied
        double totalActiveTime;             ///< Total time body was active
        double totalSleepTime;              ///< Total time body was sleeping
        double averageUpdateTime;           ///< Average update time (ms)
        double maxUpdateTime;               ///< Maximum update time (ms)
        glm::vec3 maxLinearVelocity;        ///< Maximum linear velocity reached
        glm::vec3 maxAngularVelocity;       ///< Maximum angular velocity reached
        float maxForce;                     ///< Maximum force applied
        float maxTorque;                    ///< Maximum torque applied
        size_t memoryUsage;                 ///< Memory usage estimate
    };

    /**
     * @class RigidBody
     * @brief Rigid body object with full 6DOF physics simulation
     *
     * A RigidBody represents a physical object in the simulation with:
     * - Position, orientation, and scale
     * - Linear and angular velocity/acceleration
     * - Mass and inertia properties
     * - Collision detection and response
     * - Constraint support
     * - Force and torque application
     * - Sleeping for performance
     * - Multiple simulation modes
     *
     * Key Features:
     * - Full 6 degrees of freedom (3 linear + 3 angular)
     * - Multiple body types (static, kinematic, dynamic)
     * - Configurable physical properties
     * - Multiple collider support
     * - Force and impulse application
     * - Velocity and position control
     * - Sleep management for performance
     * - Thread-safe operations
     */
    class RigidBody {
    public:
        /**
         * @brief Constructor
         * @param type Body type
         * @param mass Body mass (0 for static)
         */
        RigidBody(RigidBodyType type = RigidBodyType::Dynamic, float mass = 1.0f);

        /**
         * @brief Destructor
         */
        ~RigidBody();

        /**
         * @brief Deleted copy constructor
         */
        RigidBody(const RigidBody&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        RigidBody& operator=(const RigidBody&) = delete;

        // Body identification

        /**
         * @brief Get body ID
         * @return Unique body ID
         */
        uint32_t GetID() const { return m_id; }

        /**
         * @brief Get body type
         * @return Body type
         */
        RigidBodyType GetType() const { return m_type; }

        /**
         * @brief Set body type
         * @param type New body type
         */
        void SetType(RigidBodyType type);

        /**
         * @brief Get body name
         * @return Body name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Set body name
         * @param name New body name
         */
        void SetName(const std::string& name) { m_name = name; }

        // Transform access

        /**
         * @brief Get world position
         * @return World position
         */
        const glm::vec3& GetPosition() const { return m_state.position; }

        /**
         * @brief Set world position
         * @param position New position
         */
        void SetPosition(const glm::vec3& position);

        /**
         * @brief Get world orientation
         * @return World orientation quaternion
         */
        const glm::quat& GetOrientation() const { return m_state.orientation; }

        /**
         * @brief Set world orientation
         * @param orientation New orientation quaternion
         */
        void SetOrientation(const glm::quat& orientation);

        /**
         * @brief Get world transform matrix
         * @return 4x4 transform matrix
         */
        glm::mat4 GetTransformMatrix() const;

        /**
         * @brief Get forward direction vector
         * @return Forward direction
         */
        glm::vec3 GetForwardDirection() const;

        /**
         * @brief Get up direction vector
         * @return Up direction
         */
        glm::vec3 GetUpDirection() const;

        /**
         * @brief Get right direction vector
         * @return Right direction
         */
        glm::vec3 GetRightDirection() const;

        // Motion access

        /**
         * @brief Get linear velocity
         * @return Linear velocity
         */
        const glm::vec3& GetLinearVelocity() const { return m_state.linearVelocity; }

        /**
         * @brief Set linear velocity
         * @param velocity New linear velocity
         */
        void SetLinearVelocity(const glm::vec3& velocity);

        /**
         * @brief Get angular velocity
         * @return Angular velocity (radians/sec)
         */
        const glm::vec3& GetAngularVelocity() const { return m_state.angularVelocity; }

        /**
         * @brief Set angular velocity
         * @param velocity New angular velocity (radians/sec)
         */
        void SetAngularVelocity(const glm::vec3& velocity);

        /**
         * @brief Get linear acceleration
         * @return Linear acceleration
         */
        const glm::vec3& GetLinearAcceleration() const { return m_state.linearAcceleration; }

        /**
         * @brief Get angular acceleration
         * @return Angular acceleration
         */
        const glm::vec3& GetAngularAcceleration() const { return m_state.angularAcceleration; }

        // Properties access

        /**
         * @brief Get body mass
         * @return Body mass (kg)
         */
        float GetMass() const { return m_properties.mass; }

        /**
         * @brief Set body mass
         * @param mass New mass (kg)
         */
        void SetMass(float mass);

        /**
         * @brief Get inverse mass
         * @return Inverse mass
         */
        float GetInverseMass() const { return m_properties.inverseMass; }

        /**
         * @brief Get inertia tensor
         * @return Inertia tensor
         */
        const glm::mat3& GetInertiaTensor() const { return m_properties.inertiaTensor; }

        /**
         * @brief Set inertia tensor
         * @param tensor New inertia tensor
         */
        void SetInertiaTensor(const glm::mat3& tensor);

        /**
         * @brief Get center of mass (local space)
         * @return Center of mass
         */
        const glm::vec3& GetCenterOfMass() const { return m_properties.centerOfMass; }

        /**
         * @brief Set center of mass
         * @param com New center of mass (local space)
         */
        void SetCenterOfMass(const glm::vec3& com);

        /**
         * @brief Get linear damping
         * @return Linear damping coefficient
         */
        float GetLinearDamping() const { return m_properties.linearDamping; }

        /**
         * @brief Set linear damping
         * @param damping New linear damping coefficient
         */
        void SetLinearDamping(float damping);

        /**
         * @brief Get angular damping
         * @return Angular damping coefficient
         */
        float GetAngularDamping() const { return m_properties.angularDamping; }

        /**
         * @brief Set angular damping
         * @param damping New angular damping coefficient
         */
        void SetAngularDamping(float damping);

        /**
         * @brief Get friction coefficient
         * @return Friction coefficient
         */
        float GetFriction() const { return m_properties.friction; }

        /**
         * @brief Set friction coefficient
         * @param friction New friction coefficient
         */
        void SetFriction(float friction);

        /**
         * @brief Get restitution coefficient
         * @return Restitution coefficient
         */
        float GetRestitution() const { return m_properties.restitution; }

        /**
         * @brief Set restitution coefficient
         * @param restitution New restitution coefficient
         */
        void SetRestitution(float restitution);

        // Force and torque application

        /**
         * @brief Apply force at world position
         * @param force Force vector
         * @param position World position to apply force
         */
        void ApplyForce(const glm::vec3& force, const glm::vec3& position);

        /**
         * @brief Apply force at center of mass
         * @param force Force vector
         */
        void ApplyCentralForce(const glm::vec3& force);

        /**
         * @brief Apply torque
         * @param torque Torque vector
         */
        void ApplyTorque(const glm::vec3& torque);

        /**
         * @brief Apply impulse at world position
         * @param impulse Impulse vector
         * @param position World position to apply impulse
         */
        void ApplyImpulse(const glm::vec3& impulse, const glm::vec3& position);

        /**
         * @brief Apply impulse at center of mass
         * @param impulse Impulse vector
         */
        void ApplyCentralImpulse(const glm::vec3& impulse);

        /**
         * @brief Apply angular impulse
         * @param impulse Angular impulse vector
         */
        void ApplyAngularImpulse(const glm::vec3& impulse);

        /**
         * @brief Clear all forces and torques
         */
        void ClearForces();

        // Velocity and position control

        /**
         * @brief Set body to kinematic mode and move to position
         * @param position Target position
         * @param deltaTime Time step
         */
        void MoveToPosition(const glm::vec3& position, float deltaTime);

        /**
         * @brief Set body to kinematic mode and rotate to orientation
         * @param orientation Target orientation
         * @param deltaTime Time step
         */
        void RotateToOrientation(const glm::quat& orientation, float deltaTime);

        // Collider management

        /**
         * @brief Add collider to body
         * @param collider Collider to add
         * @return true if added, false otherwise
         */
        bool AddCollider(std::unique_ptr<Collider> collider);

        /**
         * @brief Remove collider from body
         * @param collider Collider to remove
         * @return true if removed, false otherwise
         */
        bool RemoveCollider(Collider* collider);

        /**
         * @brief Get all colliders
         * @return Vector of colliders
         */
        std::vector<Collider*> GetColliders() const;

        /**
         * @brief Get collider count
         * @return Number of colliders
         */
        size_t GetColliderCount() const;

        /**
         * @brief Get world space AABB
         * @return World space AABB
         */
        AABB GetWorldAABB() const;

        /**
         * @brief Get world space bounding sphere
         * @return World space bounding sphere
         */
        BoundingSphere GetWorldBoundingSphere() const;

        // Constraint management

        /**
         * @brief Add constraint to body
         * @param constraint Constraint to add
         * @return true if added, false otherwise
         */
        bool AddConstraint(std::unique_ptr<Constraint> constraint);

        /**
         * @brief Remove constraint from body
         * @param constraint Constraint to remove
         * @return true if removed, false otherwise
         */
        bool RemoveConstraint(Constraint* constraint);

        /**
         * @brief Get all constraints
         * @return Vector of constraints
         */
        std::vector<Constraint*> GetConstraints() const;

        // Activation and sleeping

        /**
         * @brief Get activation state
         * @return Current activation state
         */
        ActivationState GetActivationState() const { return m_activationState; }

        /**
         * @brief Set activation state
         * @param state New activation state
         */
        void SetActivationState(ActivationState state);

        /**
         * @brief Activate body (wake up from sleep)
         */
        void Activate();

        /**
         * @brief Deactivate body (put to sleep)
         */
        void Deactivate();

        /**
         * @brief Check if body is sleeping
         * @return true if sleeping, false otherwise
         */
        bool IsSleeping() const { return m_activationState == ActivationState::Sleeping; }

        /**
         * @brief Check if body should sleep
         * @param deltaTime Time elapsed
         * @return true if should sleep, false otherwise
         */
        bool ShouldSleep(float deltaTime) const;

        // Flags and settings

        /**
         * @brief Check if flag is set
         * @param flag Flag to check
         * @return true if set, false otherwise
         */
        bool HasFlag(RigidBodyFlags flag) const { return (m_flags & flag) != 0; }

        /**
         * @brief Set flag
         * @param flag Flag to set
         */
        void SetFlag(RigidBodyFlags flag);

        /**
         * @brief Clear flag
         * @param flag Flag to clear
         */
        void ClearFlag(RigidBodyFlags flag);

        /**
         * @brief Toggle flag
         * @param flag Flag to toggle
         */
        void ToggleFlag(RigidBodyFlags flag);

        /**
         * @brief Get all flags
         * @return Current flags
         */
        uint32_t GetFlags() const { return m_flags; }

        /**
         * @brief Set all flags
         * @param flags New flags
         */
        void SetFlags(uint32_t flags);

        // Entity integration

        /**
         * @brief Get associated entity
         * @return Associated entity or nullptr
         */
        Entity* GetEntity() const { return m_entity; }

        /**
         * @brief Set associated entity
         * @param entity Entity to associate
         */
        void SetEntity(Entity* entity) { m_entity = entity; }

        // Update and simulation

        /**
         * @brief Update body state
         * @param deltaTime Time elapsed since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Integrate motion using current forces
         * @param deltaTime Time step
         */
        void Integrate(float deltaTime);

        /**
         * @brief Apply damping to motion
         * @param deltaTime Time step
         */
        void ApplyDamping(float deltaTime);

        // Custom properties

        /**
         * @brief Set custom property
         * @tparam T Property type
         * @param key Property key
         * @param value Property value
         */
        template<typename T>
        void SetProperty(const std::string& key, const T& value);

        /**
         * @brief Get custom property
         * @tparam T Property type
         * @param key Property key
         * @param defaultValue Default value
         * @return Property value or default
         */
        template<typename T>
        T GetProperty(const std::string& key, const T& defaultValue = T{}) const;

        /**
         * @brief Check if property exists
         * @param key Property key
         * @return true if exists, false otherwise
         */
        bool HasProperty(const std::string& key) const;

        /**
         * @brief Remove custom property
         * @param key Property key
         */
        void RemoveProperty(const std::string& key);

        // Metrics and debugging

        /**
         * @brief Get body metrics
         * @return Performance metrics
         */
        const RigidBodyMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get debug information
         * @return Debug information string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Validate body state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

    private:
        /**
         * @brief Initialize body with default values
         */
        void InitializeDefaults();

        /**
         * @brief Update inertia tensor based on colliders
         */
        void UpdateInertiaTensor();

        /**
         * @brief Update world space properties
         */
        void UpdateWorldProperties();

        /**
         * @brief Clamp velocities to limits
         */
        void ClampVelocities();

        /**
         * @brief Update activation state
         * @param deltaTime Time elapsed
         */
        void UpdateActivation(float deltaTime);

        /**
         * @brief Update metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(float deltaTime);

        // Body data
        uint32_t m_id;                                      ///< Unique body ID
        std::string m_name;                                ///< Body name
        RigidBodyType m_type;                              ///< Body type
        uint32_t m_flags;                                  ///< Body flags
        ActivationState m_activationState;                 ///< Activation state

        // Physical state
        RigidBodyState m_state;                            ///< Current state
        RigidBodyState m_previousState;                    ///< Previous state for interpolation
        RigidBodyProperties m_properties;                  ///< Physical properties

        // Colliders
        std::vector<std::unique_ptr<Collider>> m_colliders;
        mutable std::shared_mutex m_collidersMutex;        ///< Collider synchronization

        // Constraints
        std::vector<std::unique_ptr<Constraint>> m_constraints;
        mutable std::shared_mutex m_constraintsMutex;      ///< Constraint synchronization

        // Entity integration
        Entity* m_entity;                                  ///< Associated entity

        // Custom properties
        std::unordered_map<std::string, std::any> m_properties;
        mutable std::shared_mutex m_propertiesMutex;       ///< Properties synchronization

        // Metrics
        RigidBodyMetrics m_metrics;
        mutable std::shared_mutex m_metricsMutex;          ///< Metrics synchronization

        // Statistics
        static std::atomic<uint32_t> s_nextBodyId;        ///< Next body ID counter
    };

    // Template implementations

    template<typename T>
    void RigidBody::SetProperty(const std::string& key, const T& value) {
        std::unique_lock<std::shared_mutex> lock(m_propertiesMutex);
        m_properties[key] = value;
    }

    template<typename T>
    T RigidBody::GetProperty(const std::string& key, const T& defaultValue) const {
        std::shared_lock<std::shared_mutex> lock(m_propertiesMutex);
        auto it = m_properties.find(key);
        if (it != m_properties.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

} // namespace VoxelCraft

#endif // VOXELCRAFT_PHYSICS_RIGID_BODY_HPP
