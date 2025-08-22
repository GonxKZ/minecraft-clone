/**
 * @file Entity.hpp
 * @brief VoxelCraft ECS - Entity System Core
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Entity class and core ECS architecture for VoxelCraft.
 * The Entity Component System provides a flexible and performant way to manage
 * game objects and their behaviors.
 */

#ifndef VOXELCRAFT_ENTITIES_ENTITY_HPP
#define VOXELCRAFT_ENTITIES_ENTITY_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <typeinfo>
#include <functional>
#include <mutex>
#include <atomic>
#include <optional>

#include "Component.hpp"

namespace VoxelCraft {

    /**
     * @typedef EntityID
     * @brief Unique identifier for entities
     */
    using EntityID = uint64_t;

    /**
     * @enum EntityState
     * @brief Current state of an entity
     */
    enum class EntityState {
        Active,         ///< Entity is active and being processed
        Inactive,       ///< Entity is inactive but still exists
        PendingDestroy, ///< Entity is marked for destruction
        Destroyed       ///< Entity has been destroyed
    };

    /**
     * @struct EntityMetadata
     * @brief Metadata associated with an entity
     */
    struct EntityMetadata {
        std::string name;                    ///< Human-readable name
        std::string type;                    ///< Entity type identifier
        std::string prefab;                  ///< Prefab this entity was created from
        std::unordered_map<std::string, std::string> tags; ///< Entity tags
        std::unordered_map<std::string, std::any> properties; ///< Custom properties
        double creationTime;                 ///< Time when entity was created
        double lastUpdateTime;               ///< Last time entity was updated
    };

    /**
     * @class Entity
     * @brief Core entity class in the ECS architecture
     *
     * An Entity represents any game object that can have components attached to it.
     * Entities themselves contain no logic - all behavior is provided by components
     * and processed by systems.
     *
     * Features:
     * - Unique identification
     * - Dynamic component attachment/detachment
     * - Metadata and properties
     * - State management
     * - Serialization support
     * - Event system integration
     */
    class Entity {
    public:
        /**
         * @brief Constructor
         * @param id Unique entity ID
         * @param name Optional entity name
         */
        explicit Entity(EntityID id, const std::string& name = "");

        /**
         * @brief Destructor
         */
        ~Entity();

        /**
         * @brief Deleted copy constructor
         */
        Entity(const Entity&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Entity& operator=(const Entity&) = delete;

        /**
         * @brief Move constructor
         */
        Entity(Entity&& other) noexcept;

        /**
         * @brief Move assignment operator
         */
        Entity& operator=(Entity&& other) noexcept;

        // Entity identification

        /**
         * @brief Get entity unique ID
         * @return Entity ID
         */
        EntityID GetID() const { return m_id; }

        /**
         * @brief Get entity name
         * @return Entity name
         */
        const std::string& GetName() const { return m_metadata.name; }

        /**
         * @brief Set entity name
         * @param name New entity name
         */
        void SetName(const std::string& name) { m_metadata.name = name; }

        // Entity state

        /**
         * @brief Get current entity state
         * @return Entity state
         */
        EntityState GetState() const { return m_state; }

        /**
         * @brief Set entity state
         * @param state New entity state
         */
        void SetState(EntityState state);

        /**
         * @brief Check if entity is active
         * @return true if active, false otherwise
         */
        bool IsActive() const { return m_state == EntityState::Active; }

        /**
         * @brief Activate entity
         */
        void Activate();

        /**
         * @brief Deactivate entity
         */
        void Deactivate();

        /**
         * @brief Mark entity for destruction
         */
        void Destroy();

        // Component management

        /**
         * @brief Add component to entity
         * @tparam T Component type
         * @tparam Args Constructor argument types
         * @param args Constructor arguments
         * @return Pointer to created component
         */
        template<typename T, typename... Args>
        T* AddComponent(Args&&... args);

        /**
         * @brief Get component from entity
         * @tparam T Component type
         * @return Pointer to component or nullptr if not found
         */
        template<typename T>
        T* GetComponent();

        /**
         * @brief Get component from entity (const version)
         * @tparam T Component type
         * @return Pointer to component or nullptr if not found
         */
        template<typename T>
        const T* GetComponent() const;

        /**
         * @brief Remove component from entity
         * @tparam T Component type
         * @return true if component was removed, false if not found
         */
        template<typename T>
        bool RemoveComponent();

        /**
         * @brief Check if entity has component
         * @tparam T Component type
         * @return true if has component, false otherwise
         */
        template<typename T>
        bool HasComponent() const;

        /**
         * @brief Get all components of entity
         * @return Vector of component pointers
         */
        std::vector<Component*> GetComponents();

        /**
         * @brief Get all components of entity (const version)
         * @return Vector of component pointers
         */
        std::vector<const Component*> GetComponents() const;

        /**
         * @brief Remove all components from entity
         */
        void RemoveAllComponents();

        // Metadata and properties

        /**
         * @brief Get entity metadata
         * @return Reference to metadata
         */
        EntityMetadata& GetMetadata() { return m_metadata; }

        /**
         * @brief Get entity metadata (const version)
         * @return Reference to metadata
         */
        const EntityMetadata& GetMetadata() const { return m_metadata; }

        /**
         * @brief Set entity type
         * @param type Entity type identifier
         */
        void SetType(const std::string& type) { m_metadata.type = type; }

        /**
         * @brief Get entity type
         * @return Entity type identifier
         */
        const std::string& GetType() const { return m_metadata.type; }

        /**
         * @brief Add tag to entity
         * @param tag Tag to add
         * @param value Optional tag value
         */
        void AddTag(const std::string& tag, const std::string& value = "");

        /**
         * @brief Remove tag from entity
         * @param tag Tag to remove
         */
        void RemoveTag(const std::string& tag);

        /**
         * @brief Check if entity has tag
         * @param tag Tag to check
         * @return true if has tag, false otherwise
         */
        bool HasTag(const std::string& tag) const;

        /**
         * @brief Get tag value
         * @param tag Tag name
         * @return Optional tag value
         */
        std::optional<std::string> GetTag(const std::string& tag) const;

        /**
         * @brief Set custom property
         * @tparam T Property type
         * @param name Property name
         * @param value Property value
         */
        template<typename T>
        void SetProperty(const std::string& name, const T& value);

        /**
         * @brief Get custom property
         * @tparam T Property type
         * @param name Property name
         * @return Optional property value
         */
        template<typename T>
        std::optional<T> GetProperty(const std::string& name) const;

        /**
         * @brief Remove custom property
         * @param name Property name
         */
        void RemoveProperty(const std::string& name);

        // Entity lifecycle

        /**
         * @brief Update entity (called every frame)
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Late update entity (called after all regular updates)
         * @param deltaTime Time elapsed since last update
         */
        void LateUpdate(double deltaTime);

        /**
         * @brief Fixed update entity (called at fixed intervals)
         * @param fixedDeltaTime Fixed time step
         */
        void FixedUpdate(double fixedDeltaTime);

        // Serialization

        /**
         * @brief Serialize entity to data stream
         * @param stream Output stream
         * @return true if successful, false otherwise
         */
        bool Serialize(std::ostream& stream) const;

        /**
         * @brief Deserialize entity from data stream
         * @param stream Input stream
         * @return true if successful, false otherwise
         */
        bool Deserialize(std::istream& stream);

        // Event system integration

        /**
         * @brief Send event from this entity
         * @param eventType Event type
         * @param data Event data
         */
        void SendEvent(const std::string& eventType, std::any data = {});

        /**
         * @brief Broadcast event to all components
         * @param eventType Event type
         * @param data Event data
         */
        void BroadcastEvent(const std::string& eventType, std::any data = {});

    private:
        /**
         * @brief Initialize entity
         */
        void Initialize();

        /**
         * @brief Cleanup entity resources
         */
        void Cleanup();

        /**
         * @brief Notify components about state change
         */
        void NotifyStateChange();

        EntityID m_id;                                    ///< Unique entity ID
        EntityState m_state;                              ///< Current entity state
        EntityMetadata m_metadata;                        ///< Entity metadata

        std::unordered_map<std::string, std::unique_ptr<Component>> m_components; ///< Attached components
        mutable std::mutex m_componentsMutex;             ///< Components synchronization

        // Cached component pointers for fast access
        std::unordered_map<std::string, Component*> m_componentCache;

        // Entity hierarchy (future feature)
        Entity* m_parent;                                 ///< Parent entity
        std::vector<Entity*> m_children;                  ///< Child entities

        // Statistics
        uint64_t m_updateCount;                           ///< Number of updates
        double m_totalUpdateTime;                         ///< Total update time

        // Next component ID for this entity
        uint32_t m_nextComponentId;
    };

    // Template implementations

    template<typename T, typename... Args>
    T* Entity::AddComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

        std::lock_guard<std::mutex> lock(m_componentsMutex);

        std::string typeName = typeid(T).name();

        // Check if component already exists
        if (m_components.count(typeName) > 0) {
            return static_cast<T*>(m_components[typeName].get());
        }

        // Create new component
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* componentPtr = component.get();

        // Initialize component
        component->m_entity = this;
        component->m_id = m_nextComponentId++;
        component->m_typeName = typeName;
        component->OnAttach();

        // Store component
        m_components[typeName] = std::move(component);
        m_componentCache[typeName] = componentPtr;

        return componentPtr;
    }

    template<typename T>
    T* Entity::GetComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

        std::lock_guard<std::mutex> lock(m_componentsMutex);

        std::string typeName = typeid(T).name();
        auto it = m_components.find(typeName);

        if (it != m_components.end()) {
            return static_cast<T*>(it->second.get());
        }

        return nullptr;
    }

    template<typename T>
    const T* Entity::GetComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

        std::lock_guard<std::mutex> lock(m_componentsMutex);

        std::string typeName = typeid(T).name();
        auto it = m_components.find(typeName);

        if (it != m_components.end()) {
            return static_cast<const T*>(it->second.get());
        }

        return nullptr;
    }

    template<typename T>
    bool Entity::RemoveComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

        std::lock_guard<std::mutex> lock(m_componentsMutex);

        std::string typeName = typeid(T).name();
        auto it = m_components.find(typeName);

        if (it != m_components.end()) {
            it->second->OnDetach();
            m_componentCache.erase(typeName);
            m_components.erase(it);
            return true;
        }

        return false;
    }

    template<typename T>
    bool Entity::HasComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

        std::lock_guard<std::mutex> lock(m_componentsMutex);
        return m_components.count(typeid(T).name()) > 0;
    }

    template<typename T>
    void Entity::SetProperty(const std::string& name, const T& value) {
        m_metadata.properties[name] = value;
    }

    template<typename T>
    std::optional<T> Entity::GetProperty(const std::string& name) const {
        auto it = m_metadata.properties.find(name);
        if (it != m_metadata.properties.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_ENTITY_HPP
