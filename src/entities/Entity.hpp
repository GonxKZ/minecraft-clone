/**
 * @file Entity.hpp
 * @brief VoxelCraft Entity System - Entity Header
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Entity class for the VoxelCraft ECS system.
 */

#ifndef VOXELCRAFT_ENTITIES_ENTITY_HPP
#define VOXELCRAFT_ENTITIES_ENTITY_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <atomic>

namespace VoxelCraft {

    class Component;
    class EntityManager;

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
     * @class Entity
     * @brief Represents a game entity in the ECS system
     *
     * An entity is a container for components that define its behavior
     * and properties. Entities have a unique ID and can be in different states.
     */
    class Entity {
    public:
        /**
         * @brief Constructor
         * @param id Unique entity ID
         * @param name Entity name
         * @param manager Entity manager that owns this entity
         */
        Entity(EntityID id, const std::string& name, EntityManager* manager);

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
         * @brief Update the entity
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Render the entity
         */
        void Render();

        /**
         * @brief Add a component to this entity
         * @tparam T Component type
         * @tparam Args Component constructor arguments
         * @param args Constructor arguments
         * @return Pointer to the created component
         */
        template<typename T, typename... Args>
        T* AddComponent(Args&&... args);

        /**
         * @brief Get a component from this entity
         * @tparam T Component type
         * @return Pointer to the component or nullptr if not found
         */
        template<typename T>
        T* GetComponent();

        /**
         * @brief Get a component from this entity (const version)
         * @tparam T Component type
         * @return Pointer to the component or nullptr if not found
         */
        template<typename T>
        const T* GetComponent() const;

        /**
         * @brief Check if entity has a component
         * @tparam T Component type
         * @return true if entity has the component, false otherwise
         */
        template<typename T>
        bool HasComponent() const;

        /**
         * @brief Remove a component from this entity
         * @tparam T Component type
         * @return true if component was removed, false if not found
         */
        template<typename T>
        bool RemoveComponent();

        /**
         * @brief Destroy this entity
         */
        void Destroy();

        /**
         * @brief Get entity ID
         * @return Entity ID
         */
        EntityID GetID() const { return m_id; }

        /**
         * @brief Get entity name
         * @return Entity name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Set entity name
         * @param name New entity name
         */
        void SetName(const std::string& name) { m_name = name; }

        /**
         * @brief Get entity state
         * @return Entity state
         */
        EntityState GetState() const { return m_state; }

        /**
         * @brief Set entity state
         * @param state New entity state
         */
        void SetState(EntityState state) { m_state = state; }

        /**
         * @brief Check if entity is active
         * @return true if active, false otherwise
         */
        bool IsActive() const { return m_state == EntityState::Active; }

        /**
         * @brief Check if entity is marked for destruction
         * @return true if pending destruction, false otherwise
         */
        bool IsPendingDestroy() const { return m_state == EntityState::PendingDestroy; }

        /**
         * @brief Get number of components
         * @return Number of components attached to this entity
         */
        size_t GetComponentCount() const { return m_components.size(); }

        /**
         * @brief Get all components
         * @return Vector of component pointers
         */
        std::vector<Component*> GetComponents();

        /**
         * @brief Get entity manager
         * @return Entity manager pointer
         */
        EntityManager* GetManager() const { return m_manager; }

    private:
        /**
         * @brief Add component to internal storage
         * @param component Component to add
         */
        void AddComponentInternal(Component* component);

        /**
         * @brief Remove component from internal storage
         * @param component Component to remove
         */
        void RemoveComponentInternal(Component* component);

        EntityID m_id;                                           ///< Unique entity ID
        std::string m_name;                                      ///< Entity name
        EntityState m_state;                                     ///< Current entity state
        EntityManager* m_manager;                                ///< Entity manager owner
        std::unordered_map<std::string, std::unique_ptr<Component>> m_components; ///< Components by type

        // Friend classes for internal access
        friend class EntityManager;
        friend class Component;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_ENTITY_HPP