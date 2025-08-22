/**
 * @file Component.hpp
 * @brief VoxelCraft ECS - Component System Base
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Component base class and component system architecture
 * for the VoxelCraft ECS. Components provide data and behavior to entities.
 */

#ifndef VOXELCRAFT_ENTITIES_COMPONENT_HPP
#define VOXELCRAFT_ENTITIES_COMPONENT_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <typeinfo>
#include <any>
#include <functional>
#include <mutex>

namespace VoxelCraft {

    class Entity;
    class ComponentRegistry;

    /**
     * @typedef ComponentID
     * @brief Unique identifier for components
     */
    using ComponentID = uint32_t;

    /**
     * @typedef ComponentTypeID
     * @brief Type identifier for component types
     */
    using ComponentTypeID = uint32_t;

    /**
     * @enum ComponentState
     * @brief Current state of a component
     */
    enum class ComponentState {
        Created,        ///< Component has been created
        Attached,       ///< Component is attached to an entity
        Detached,       ///< Component is detached from entity
        Destroyed       ///< Component is being destroyed
    };

    /**
     * @struct ComponentMetadata
     * @brief Metadata for component types
     */
    struct ComponentTypeMetadata {
        std::string name;                    ///< Human-readable component name
        std::string description;             ///< Component description
        std::string category;                ///< Component category
        bool isUnique;                       ///< Only one instance per entity
        bool isSerializable;                 ///< Component can be serialized
        std::vector<std::string> dependencies; ///< Required component types
    };

    /**
     * @class Component
     * @brief Base class for all components in the ECS system
     *
     * Components provide data and behavior to entities. They can be attached,
     * detached, and communicate with other components through the entity.
     *
     * Component Lifecycle:
     * 1. Created - Component is instantiated
     * 2. Attached - Component is attached to an entity
     * 3. Initialized - Component is initialized
     * 4. Updated - Component is updated each frame
     * 5. Detached - Component is detached from entity
     * 6. Destroyed - Component is destroyed
     */
    class Component {
    public:
        /**
         * @brief Constructor
         */
        Component();

        /**
         * @brief Destructor
         */
        virtual ~Component();

        /**
         * @brief Deleted copy constructor
         */
        Component(const Component&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Component& operator=(const Component&) = delete;

        // Component identification

        /**
         * @brief Get component unique ID
         * @return Component ID
         */
        ComponentID GetID() const { return m_id; }

        /**
         * @brief Get component type ID
         * @return Component type ID
         */
        ComponentTypeID GetTypeID() const { return m_typeId; }

        /**
         * @brief Get component type name
         * @return Type name
         */
        const std::string& GetTypeName() const { return m_typeName; }

        /**
         * @brief Get human-readable component name
         * @return Component name
         */
        virtual std::string GetName() const { return "Component"; }

        /**
         * @brief Get component description
         * @return Component description
         */
        virtual std::string GetDescription() const { return "Base component class"; }

        // Entity relationship

        /**
         * @brief Get entity this component is attached to
         * @return Entity pointer or nullptr if not attached
         */
        Entity* GetEntity() const { return m_entity; }

        /**
         * @brief Check if component is attached to an entity
         * @return true if attached, false otherwise
         */
        bool IsAttached() const { return m_entity != nullptr; }

        // Component state

        /**
         * @brief Get current component state
         * @return Component state
         */
        ComponentState GetState() const { return m_state; }

        /**
         * @brief Check if component is enabled
         * @return true if enabled, false otherwise
         */
        bool IsEnabled() const { return m_enabled; }

        /**
         * @brief Enable or disable component
         * @param enabled Enable state
         */
        void SetEnabled(bool enabled);

        // Lifecycle methods

        /**
         * @brief Called when component is attached to an entity
         */
        virtual void OnAttach() {}

        /**
         * @brief Called when component is detached from entity
         */
        virtual void OnDetach() {}

        /**
         * @brief Called to initialize component after attachment
         */
        virtual void Initialize() {}

        /**
         * @brief Called to cleanup component before detachment
         */
        virtual void Cleanup() {}

        /**
         * @brief Called every frame to update component
         * @param deltaTime Time elapsed since last update
         */
        virtual void Update(double deltaTime) {}

        /**
         * @brief Called after all regular updates
         * @param deltaTime Time elapsed since last update
         */
        virtual void LateUpdate(double deltaTime) {}

        /**
         * @brief Called at fixed time intervals
         * @param fixedDeltaTime Fixed time step
         */
        virtual void FixedUpdate(double fixedDeltaTime) {}

        /**
         * @brief Called when component receives an event
         * @param eventType Event type
         * @param data Event data
         */
        virtual void OnEvent(const std::string& eventType, std::any data) {}

        // Component communication

        /**
         * @brief Send event to owning entity
         * @param eventType Event type
         * @param data Event data
         */
        void SendEntityEvent(const std::string& eventType, std::any data = {});

        /**
         * @brief Get component from same entity
         * @tparam T Component type
         * @return Component pointer or nullptr if not found
         */
        template<typename T>
        T* GetComponentFromEntity();

        /**
         * @brief Get component from same entity (const version)
         * @tparam T Component type
         * @return Component pointer or nullptr if not found
         */
        template<typename T>
        const T* GetComponentFromEntity() const;

        // Properties system

        /**
         * @brief Set component property
         * @tparam T Property type
         * @param name Property name
         * @param value Property value
         */
        template<typename T>
        void SetProperty(const std::string& name, const T& value);

        /**
         * @brief Get component property
         * @tparam T Property type
         * @param name Property name
         * @param defaultValue Default value if property doesn't exist
         * @return Property value or default
         */
        template<typename T>
        T GetProperty(const std::string& name, const T& defaultValue = T{}) const;

        /**
         * @brief Check if component has property
         * @param name Property name
         * @return true if has property, false otherwise
         */
        bool HasProperty(const std::string& name) const;

        /**
         * @brief Remove component property
         * @param name Property name
         */
        void RemoveProperty(const std::string& name);

        // Serialization

        /**
         * @brief Serialize component data
         * @param data Output data map
         * @return true if successful, false otherwise
         */
        virtual bool Serialize(std::unordered_map<std::string, std::any>& data) const;

        /**
         * @brief Deserialize component data
         * @param data Input data map
         * @return true if successful, false otherwise
         */
        virtual bool Deserialize(const std::unordered_map<std::string, std::any>& data);

        // Type information

        /**
         * @brief Get component type metadata
         * @return Type metadata
         */
        static ComponentTypeMetadata GetTypeMetadata();

        /**
         * @brief Get component type ID (must be implemented by derived classes)
         * @return Component type ID
         */
        static ComponentTypeID GetStaticTypeID();

    protected:
        /**
         * @brief Set component type ID
         * @param typeId Type ID
         */
        void SetTypeID(ComponentTypeID typeId) { m_typeId = typeId; }

        ComponentID m_id;                       ///< Unique component ID
        ComponentTypeID m_typeId;               ///< Component type ID
        std::string m_typeName;                 ///< Component type name

        Entity* m_entity;                       ///< Owning entity
        ComponentState m_state;                 ///< Current component state
        bool m_enabled;                         ///< Component enabled flag

        std::unordered_map<std::string, std::any> m_properties; ///< Component properties
        mutable std::mutex m_propertiesMutex;   ///< Properties synchronization

        // Statistics
        double m_creationTime;                  ///< Component creation time
        uint64_t m_updateCount;                 ///< Number of updates
        double m_totalUpdateTime;               ///< Total update time

        friend class Entity;
        friend class ComponentRegistry;
    };

    // Template implementations

    template<typename T>
    T* Component::GetComponentFromEntity() {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        return m_entity ? m_entity->GetComponent<T>() : nullptr;
    }

    template<typename T>
    const T* Component::GetComponentFromEntity() const {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        return m_entity ? m_entity->GetComponent<T>() : nullptr;
    }

    template<typename T>
    void Component::SetProperty(const std::string& name, const T& value) {
        std::lock_guard<std::mutex> lock(m_propertiesMutex);
        m_properties[name] = value;
    }

    template<typename T>
    T Component::GetProperty(const std::string& name, const T& defaultValue) const {
        std::lock_guard<std::mutex> lock(m_propertiesMutex);
        auto it = m_properties.find(name);
        if (it != m_properties.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    /**
     * @class ComponentRegistry
     * @brief Registry for component types and factories
     */
    class ComponentRegistry {
    public:
        /**
         * @brief Register component type
         * @tparam T Component type
         * @param metadata Component metadata
         */
        template<typename T>
        static void RegisterComponent(const ComponentTypeMetadata& metadata = ComponentTypeMetadata{});

        /**
         * @brief Create component by type name
         * @param typeName Component type name
         * @return Component pointer or nullptr if type not found
         */
        static std::unique_ptr<Component> CreateComponent(const std::string& typeName);

        /**
         * @brief Get component type metadata
         * @param typeName Component type name
         * @return Optional metadata
         */
        static std::optional<ComponentTypeMetadata> GetComponentMetadata(const std::string& typeName);

        /**
         * @brief Get all registered component types
         * @return Vector of type names
         */
        static std::vector<std::string> GetRegisteredTypes();

        /**
         * @brief Check if component type is registered
         * @param typeName Component type name
         * @return true if registered, false otherwise
         */
        static bool IsTypeRegistered(const std::string& typeName);

    private:
        /**
         * @typedef ComponentFactory
         * @brief Function type for component creation
         */
        using ComponentFactory = std::function<std::unique_ptr<Component>()>;

        static std::unordered_map<std::string, ComponentFactory> s_factories;
        static std::unordered_map<std::string, ComponentTypeMetadata> s_metadata;
        static std::mutex s_registryMutex;
        static ComponentTypeID s_nextTypeId;
    };

    // Helper macro for component registration
    #define REGISTER_COMPONENT(ComponentType, Name, Description) \
        static struct ComponentType##Registrar { \
            ComponentType##Registrar() { \
                VoxelCraft::ComponentTypeMetadata metadata; \
                metadata.name = Name; \
                metadata.description = Description; \
                metadata.category = #ComponentType; \
                metadata.isUnique = true; \
                metadata.isSerializable = true; \
                VoxelCraft::ComponentRegistry::RegisterComponent<ComponentType>(metadata); \
            } \
        } ComponentType##RegistrarInstance;

    // Example component classes

    /**
     * @class TransformComponent
     * @brief Component that provides position, rotation, and scale
     */
    class TransformComponent : public Component {
    public:
        /**
         * @brief Get component name
         * @return Component name
         */
        std::string GetName() const override { return "Transform"; }

        /**
         * @brief Get component description
         * @return Component description
         */
        std::string GetDescription() const override { return "Provides 3D transformation data"; }

        // Transform data
        glm::vec3 position{0.0f, 0.0f, 0.0f};    ///< Position in world space
        glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f}; ///< Rotation quaternion
        glm::vec3 scale{1.0f, 1.0f, 1.0f};       ///< Scale factors

        /**
         * @brief Get forward vector
         * @return Forward direction vector
         */
        glm::vec3 GetForward() const;

        /**
         * @brief Get right vector
         * @return Right direction vector
         */
        glm::vec3 GetRight() const;

        /**
         * @brief Get up vector
         * @return Up direction vector
         */
        glm::vec3 GetUp() const;

        /**
         * @brief Get transformation matrix
         * @return 4x4 transformation matrix
         */
        glm::mat4 GetTransformMatrix() const;

        /**
         * @brief Set position
         * @param pos New position
         */
        void SetPosition(const glm::vec3& pos);

        /**
         * @brief Set rotation from quaternion
         * @param rot New rotation
         */
        void SetRotation(const glm::quat& rot);

        /**
         * @brief Set rotation from euler angles
         * @param euler Euler angles in degrees
         */
        void SetRotation(const glm::vec3& euler);

        /**
         * @brief Set scale
         * @param scl New scale
         */
        void SetScale(const glm::vec3& scl);

        /**
         * @brief Translate by offset
         * @param offset Translation offset
         */
        void Translate(const glm::vec3& offset);

        /**
         * @brief Rotate by quaternion
         * @param rot Rotation quaternion
         */
        void Rotate(const glm::quat& rot);

        /**
         * @brief Rotate by euler angles
         * @param euler Euler angles in degrees
         */
        void Rotate(const glm::vec3& euler);

        /**
         * @brief Scale by factor
         * @param factor Scale factor
         */
        void Scale(const glm::vec3& factor);

        /**
         * @brief Look at target position
         * @param target Target position
         * @param up Up vector
         */
        void LookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

        // Serialization
        bool Serialize(std::unordered_map<std::string, std::any>& data) const override;
        bool Deserialize(const std::unordered_map<std::string, std::any>& data) override;

        static ComponentTypeID GetStaticTypeID();
    };

    /**
     * @class RenderComponent
     * @brief Component that provides rendering data
     */
    class RenderComponent : public Component {
    public:
        /**
         * @brief Get component name
         * @return Component name
         */
        std::string GetName() const override { return "Render"; }

        /**
         * @brief Get component description
         * @return Component description
         */
        std::string GetDescription() const override { return "Provides rendering and visual data"; }

        // Render data
        std::string meshName;                  ///< Mesh resource name
        std::string materialName;              ///< Material resource name
        bool visible{true};                    ///< Visibility flag
        bool castShadows{true};                ///< Shadow casting flag
        bool receiveShadows{true};             ///< Shadow receiving flag
        int renderLayer{0};                    ///< Render layer
        float renderDistance{1000.0f};         ///< Maximum render distance

        // Animation data
        bool animated{false};                  ///< Animation flag
        std::string currentAnimation;          ///< Current animation name
        float animationTime{0.0f};             ///< Current animation time
        float animationSpeed{1.0f};            ///< Animation playback speed

        /**
         * @brief Set mesh
         * @param name Mesh resource name
         */
        void SetMesh(const std::string& name);

        /**
         * @brief Set material
         * @param name Material resource name
         */
        void SetMaterial(const std::string& name);

        /**
         * @brief Play animation
         * @param name Animation name
         * @param loop Loop animation flag
         */
        void PlayAnimation(const std::string& name, bool loop = false);

        /**
         * @brief Stop animation
         */
        void StopAnimation();

        /**
         * @brief Pause animation
         */
        void PauseAnimation();

        /**
         * @brief Resume animation
         */
        void ResumeAnimation();

        // Serialization
        bool Serialize(std::unordered_map<std::string, std::any>& data) const override;
        bool Deserialize(const std::unordered_map<std::string, std::any>& data) override;

        static ComponentTypeID GetStaticTypeID();
    };

    /**
     * @class PhysicsComponent
     * @brief Component that provides physics simulation data
     */
    class PhysicsComponent : public Component {
    public:
        /**
         * @brief Get component name
         * @return Component name
         */
        std::string GetName() const override { return "Physics"; }

        /**
         * @brief Get component description
         * @return Component description
         */
        std::string GetDescription() const override { return "Provides physics simulation data"; }

        // Physics properties
        float mass{1.0f};                      ///< Object mass
        float friction{0.5f};                  ///< Friction coefficient
        float restitution{0.3f};               ///< Restitution (bounciness)
        float linearDamping{0.0f};             ///< Linear damping
        float angularDamping{0.0f};            ///< Angular damping

        // Motion data
        glm::vec3 velocity{0.0f, 0.0f, 0.0f};   ///< Linear velocity
        glm::vec3 angularVelocity{0.0f, 0.0f, 0.0f}; ///< Angular velocity
        glm::vec3 force{0.0f, 0.0f, 0.0f};      ///< Applied force
        glm::vec3 torque{0.0f, 0.0f, 0.0f};     ///< Applied torque

        // Collision data
        std::string collisionShape;            ///< Collision shape type
        glm::vec3 collisionSize{1.0f, 1.0f, 1.0f}; ///< Collision shape size
        bool isStatic{false};                  ///< Static object flag
        bool isTrigger{false};                 ///< Trigger object flag
        bool isKinematic{false};               ///< Kinematic object flag

        /**
         * @brief Apply force to object
         * @param force Force vector
         */
        void ApplyForce(const glm::vec3& force);

        /**
         * @brief Apply impulse to object
         * @param impulse Impulse vector
         */
        void ApplyImpulse(const glm::vec3& impulse);

        /**
         * @brief Apply torque to object
         * @param torque Torque vector
         */
        void ApplyTorque(const glm::vec3& torque);

        /**
         * @brief Set velocity
         * @param vel New velocity
         */
        void SetVelocity(const glm::vec3& vel);

        /**
         * @brief Set angular velocity
         * @param angVel New angular velocity
         */
        void SetAngularVelocity(const glm::vec3& angVel);

        /**
         * @brief Set physics properties
         * @param props Physics properties
         */
        void SetProperties(const std::unordered_map<std::string, float>& props);

        // Serialization
        bool Serialize(std::unordered_map<std::string, std::any>& data) const override;
        bool Deserialize(const std::unordered_map<std::string, std::any>& data) override;

        static ComponentTypeID GetStaticTypeID();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_COMPONENT_HPP
