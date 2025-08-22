/**
 * @file EntityManager.hpp
 * @brief VoxelCraft Entity System - Entity Manager Header
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the EntityManager class for managing entities in the ECS system.
 */

#ifndef VOXELCRAFT_ENTITIES_ENTITY_MANAGER_HPP
#define VOXELCRAFT_ENTITIES_ENTITY_MANAGER_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <functional>

#include "Entity.hpp"

namespace VoxelCraft {

    /**
     * @struct EntityManagerStats
     * @brief Statistics for the entity manager
     */
    struct EntityManagerStats {
        size_t totalEntities;                ///< Total number of entities (active + inactive)
        size_t activeEntities;               ///< Number of active entities
        size_t inactiveEntities;             ///< Number of inactive entities
        size_t pendingDestroyEntities;       ///< Number of entities pending destruction
        size_t totalComponents;              ///< Total number of components across all entities
        double averageComponentsPerEntity;   ///< Average number of components per entity
        size_t destroyedEntities;            ///< Total entities destroyed
        size_t createdEntities;              ///< Total entities created
    };

    /**
     * @typedef EntityFilter
     * @brief Function type for filtering entities
     */
    using EntityFilter = std::function<bool(const Entity*)>;

    /**
     * @typedef EntityProcessor
     * @brief Function type for processing entities
     */
    using EntityProcessor = std::function<void(Entity*)>;

    /**
     * @class EntityManager
     * @brief Manages entities and their lifecycle in the ECS system
     *
     * The EntityManager is responsible for:
     * - Creating and destroying entities
     * - Managing entity lifecycle
     * - Providing entity lookup and iteration
     * - Maintaining entity statistics
     * - Handling entity cleanup
     */
    class EntityManager {
    public:
        /**
         * @brief Constructor
         */
        EntityManager();

        /**
         * @brief Destructor
         */
        ~EntityManager();

        /**
         * @brief Deleted copy constructor
         */
        EntityManager(const EntityManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        EntityManager& operator=(const EntityManager&) = delete;

        // Entity creation and destruction

        /**
         * @brief Create a new entity
         * @param name Entity name
         * @return Pointer to the created entity
         */
        Entity* CreateEntity(const std::string& name = "");

        /**
         * @brief Destroy an entity
         * @param entity Entity to destroy
         * @return true if destroyed, false if not found
         */
        bool DestroyEntity(Entity* entity);

        /**
         * @brief Destroy an entity by ID
         * @param entityId Entity ID to destroy
         * @return true if destroyed, false if not found
         */
        bool DestroyEntity(EntityID entityId);

        /**
         * @brief Mark entity for destruction (deferred)
         * @param entity Entity to mark for destruction
         * @return true if marked, false if not found
         */
        bool MarkEntityForDestruction(Entity* entity);

        // Entity lookup

        /**
         * @brief Get entity by ID
         * @param entityId Entity ID
         * @return Entity pointer or nullptr if not found
         */
        Entity* GetEntity(EntityID entityId);

        /**
         * @brief Get entity by ID (const version)
         * @param entityId Entity ID
         * @return Entity pointer or nullptr if not found
         */
        const Entity* GetEntity(EntityID entityId) const;

        /**
         * @brief Find entity by name
         * @param name Entity name
         * @return Entity pointer or nullptr if not found
         */
        Entity* FindEntity(const std::string& name);

        /**
         * @brief Check if entity exists
         * @param entityId Entity ID
         * @return true if exists, false otherwise
         */
        bool EntityExists(EntityID entityId) const;

        // Entity iteration and processing

        /**
         * @brief Get all entities
         * @return Vector of entity pointers
         */
        std::vector<Entity*> GetAllEntities();

        /**
         * @brief Get all active entities
         * @return Vector of active entity pointers
         */
        std::vector<Entity*> GetActiveEntities();

        /**
         * @brief Get entities that match a filter
         * @param filter Filter function
         * @return Vector of matching entity pointers
         */
        std::vector<Entity*> GetEntities(const EntityFilter& filter);

        /**
         * @brief Process all active entities
         * @param processor Processing function
         */
        void ProcessActiveEntities(const EntityProcessor& processor);

        /**
         * @brief Process entities that match a filter
         * @param filter Filter function
         * @param processor Processing function
         */
        void ProcessEntities(const EntityFilter& filter, const EntityProcessor& processor);

        // Entity management

        /**
         * @brief Update all active entities
         * @param deltaTime Time elapsed since last update
         */
        void UpdateEntities(double deltaTime);

        /**
         * @brief Render all active entities
         */
        void RenderEntities();

        /**
         * @brief Cleanup destroyed entities
         * @return Number of entities cleaned up
         */
        size_t CleanupDestroyedEntities();

        /**
         * @brief Clear all entities
         * @param force Force destruction of all entities
         */
        void ClearAllEntities(bool force = false);

        // Statistics and monitoring

        /**
         * @brief Get entity manager statistics
         * @return Entity manager statistics
         */
        EntityManagerStats GetStatistics() const;

        /**
         * @brief Reset statistics
         */
        void ResetStatistics();

        /**
         * @brief Get total number of entities
         * @return Total entity count
         */
        size_t GetEntityCount() const;

        /**
         * @brief Get number of active entities
         * @return Active entity count
         */
        size_t GetActiveEntityCount() const;

        /**
         * @brief Get number of entities pending destruction
         * @return Pending destruction count
         */
        size_t GetPendingDestroyCount() const;

    private:
        /**
         * @brief Generate unique entity ID
         * @return Unique entity ID
         */
        EntityID GenerateEntityId();

        /**
         * @brief Add entity to internal storage
         * @param entity Entity to add
         */
        void AddEntity(Entity* entity);

        /**
         * @brief Remove entity from internal storage
         * @param entityId Entity ID to remove
         */
        void RemoveEntity(EntityID entityId);

        std::unordered_map<EntityID, std::unique_ptr<Entity>> m_entities;    ///< All entities by ID
        std::unordered_map<std::string, EntityID> m_entityNames;             ///< Entity names to IDs
        std::queue<EntityID> m_pendingDestroy;                               ///< Entities pending destruction

        mutable std::shared_mutex m_entityMutex;                             ///< Entity synchronization
        static std::atomic<EntityID> s_nextEntityId;                         ///< Next entity ID

        // Statistics
        mutable EntityManagerStats m_stats;                                   ///< Current statistics
        mutable std::mutex m_statsMutex;                                      ///< Statistics synchronization
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_ENTITY_MANAGER_HPP