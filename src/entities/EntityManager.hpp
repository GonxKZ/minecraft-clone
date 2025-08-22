/**
 * @file EntityManager.hpp
 * @brief VoxelCraft ECS - Entity Manager System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the EntityManager class that manages all entities
 * and provides the core functionality for the ECS architecture.
 */

#ifndef VOXELCRAFT_ENTITIES_ENTITY_MANAGER_HPP
#define VOXELCRAFT_ENTITIES_ENTITY_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <functional>
#include <mutex>
#include <atomic>
#include <shared_mutex>
#include <optional>
#include <string_view>

#include "Entity.hpp"
#include "System.hpp"

namespace VoxelCraft {

    /**
     * @struct EntityManagerStatistics
     * @brief Performance statistics for the entity manager
     */
    struct EntityManagerStatistics {
        uint64_t totalEntitiesCreated;         ///< Total entities created
        uint64_t totalEntitiesDestroyed;       ///< Total entities destroyed
        uint32_t activeEntities;               ///< Currently active entities
        uint32_t inactiveEntities;             ///< Currently inactive entities
        uint32_t entitiesPendingDestroy;       ///< Entities pending destruction
        uint32_t registeredSystems;            ///< Number of registered systems
        size_t memoryUsage;                    ///< Current memory usage
        size_t peakMemoryUsage;                ///< Peak memory usage
        double averageEntityLifetime;          ///< Average entity lifetime
        double entityCreationRate;             ///< Entities created per second
        double entityDestructionRate;          ///< Entities destroyed per second
    };

    /**
     * @struct EntityQuery
     * @brief Query structure for finding entities with specific components
     */
    struct EntityQuery {
        std::vector<std::string> requiredComponents;    ///< Components that must be present
        std::vector<std::string> optionalComponents;    ///< Components that may be present
        std::vector<std::string> excludedComponents;    ///< Components that must not be present
        std::vector<std::string> tags;                  ///< Required entity tags
        std::vector<std::string> excludedTags;          ///< Excluded entity tags
        std::string entityType;                         ///< Required entity type
        EntityState entityState;                        ///< Required entity state

        /**
         * @brief Default constructor
         */
        EntityQuery() : entityState(EntityState::Active) {}

        /**
         * @brief Constructor with required components
         * @param components Required component types
         */
        EntityQuery(const std::vector<std::string>& components)
            : requiredComponents(components), entityState(EntityState::Active) {}

        /**
         * @brief Add required component
         * @param component Component type name
         * @return Reference to this query
         */
        EntityQuery& WithComponent(const std::string& component) {
            requiredComponents.push_back(component);
            return *this;
        }

        /**
         * @brief Add optional component
         * @param component Component type name
         * @return Reference to this query
         */
        EntityQuery& WithOptionalComponent(const std::string& component) {
            optionalComponents.push_back(component);
            return *this;
        }

        /**
         * @brief Add excluded component
         * @param component Component type name
         * @return Reference to this query
         */
        EntityQuery& WithoutComponent(const std::string& component) {
            excludedComponents.push_back(component);
            return *this;
        }

        /**
         * @brief Add required tag
         * @param tag Entity tag
         * @return Reference to this query
         */
        EntityQuery& WithTag(const std::string& tag) {
            tags.push_back(tag);
            return *this;
        }

        /**
         * @brief Add excluded tag
         * @param tag Entity tag
         * @return Reference to this query
         */
        EntityQuery& WithoutTag(const std::string& tag) {
            excludedTags.push_back(tag);
            return *this;
        }

        /**
         * @brief Set entity type requirement
         * @param type Entity type
         * @return Reference to this query
         */
        EntityQuery& OfType(const std::string& type) {
            entityType = type;
            return *this;
        }

        /**
         * @brief Set entity state requirement
         * @param state Entity state
         * @return Reference to this query
         */
        EntityQuery& WithState(EntityState state) {
            entityState = state;
            return *this;
        }
    };

    /**
     * @typedef EntityQueryResult
     * @brief Result of an entity query
     */
    using EntityQueryResult = std::vector<Entity*>;

    /**
     * @class EntityManager
     * @brief Central manager for all entities and systems in the ECS
     *
     * The EntityManager is responsible for:
     * - Creating and destroying entities
     * - Managing entity lifecycle
     * - Registering and updating systems
     * - Querying entities by components and properties
     * - Managing entity pools and memory
     * - Providing statistics and debugging information
     */
    class EntityManager {
    public:
        /**
         * @brief Constructor
         * @param maxEntities Maximum number of entities
         */
        explicit EntityManager(size_t maxEntities = 100000);

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

        // Entity lifecycle management

        /**
         * @brief Create new entity
         * @param name Optional entity name
         * @return Pointer to created entity
         */
        Entity* CreateEntity(const std::string& name = "");

        /**
         * @brief Create entity with specific ID
         * @param id Entity ID to use
         * @param name Optional entity name
         * @return Pointer to created entity or nullptr if ID exists
         */
        Entity* CreateEntityWithID(EntityID id, const std::string& name = "");

        /**
         * @brief Destroy entity
         * @param entity Entity to destroy
         * @param immediate Destroy immediately or queue for destruction
         * @return true if destroyed/queued, false if not found
         */
        bool DestroyEntity(Entity* entity, bool immediate = false);

        /**
         * @brief Destroy entity by ID
         * @param id Entity ID to destroy
         * @param immediate Destroy immediately or queue for destruction
         * @return true if destroyed/queued, false if not found
         */
        bool DestroyEntityByID(EntityID id, bool immediate = false);

        /**
         * @brief Get entity by ID
         * @param id Entity ID
         * @return Pointer to entity or nullptr if not found
         */
        Entity* GetEntity(EntityID id);

        /**
         * @brief Get entity by ID (const version)
         * @param id Entity ID
         * @return Pointer to entity or nullptr if not found
         */
        const Entity* GetEntity(EntityID id) const;

        /**
         * @brief Check if entity exists
         * @param id Entity ID
         * @return true if exists, false otherwise
         */
        bool EntityExists(EntityID id) const;

        // Entity querying

        /**
         * @brief Query entities matching criteria
         * @param query Entity query
         * @return Vector of matching entities
         */
        EntityQueryResult QueryEntities(const EntityQuery& query);

        /**
         * @brief Query entities with specific component
         * @tparam T Component type
         * @return Vector of entities with component
         */
        template<typename T>
        std::vector<Entity*> GetEntitiesWithComponent();

        /**
         * @brief Query entities with specific tag
         * @param tag Entity tag
         * @return Vector of entities with tag
         */
        std::vector<Entity*> GetEntitiesWithTag(const std::string& tag);

        /**
         * @brief Query entities of specific type
         * @param type Entity type
         * @return Vector of entities of type
         */
        std::vector<Entity*> GetEntitiesOfType(const std::string& type);

        /**
         * @brief Query entities in specific state
         * @param state Entity state
         * @return Vector of entities in state
         */
        std::vector<Entity*> GetEntitiesInState(EntityState state);

        /**
         * @brief Get all entities
         * @return Vector of all entities
         */
        std::vector<Entity*> GetAllEntities();

        /**
         * @brief Get active entities count
         * @return Number of active entities
         */
        size_t GetActiveEntityCount() const;

        /**
         * @brief Get total entities count
         * @return Total number of entities (including inactive)
         */
        size_t GetTotalEntityCount() const;

        // System management

        /**
         * @brief Register system
         * @param system System to register
         * @return true if registered, false if already exists
         */
        bool RegisterSystem(std::unique_ptr<System> system);

        /**
         * @brief Unregister system
         * @param systemName System name
         * @return true if unregistered, false if not found
         */
        bool UnregisterSystem(const std::string& systemName);

        /**
         * @brief Get system by name
         * @param systemName System name
         * @return Pointer to system or nullptr if not found
         */
        System* GetSystem(const std::string& systemName);

        /**
         * @brief Get system by name (const version)
         * @param systemName System name
         * @return Pointer to system or nullptr if not found
         */
        const System* GetSystem(const std::string& systemName) const;

        /**
         * @brief Enable/disable system
         * @param systemName System name
         * @param enabled Enable state
         * @return true if successful, false if system not found
         */
        bool SetSystemEnabled(const std::string& systemName, bool enabled);

        // Update and processing

        /**
         * @brief Update all systems and entities
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Late update all systems
         * @param deltaTime Time elapsed since last update
         */
        void LateUpdate(double deltaTime);

        /**
         * @brief Fixed update all systems
         * @param fixedDeltaTime Fixed time step
         */
        void FixedUpdate(double fixedDeltaTime);

        /**
         * @brief Process pending entity destructions
         */
        void ProcessPendingDestructions();

        /**
         * @brief Clear all entities and systems
         */
        void Clear();

        // Statistics and debugging

        /**
         * @brief Get manager statistics
         * @return Entity manager statistics
         */
        EntityManagerStatistics GetStatistics() const;

        /**
         * @brief Reset statistics
         */
        void ResetStatistics();

        /**
         * @brief Validate entity manager state
         * @return true if valid, false if corrupted
         */
        bool Validate() const;

        /**
         * @brief Get memory usage information
         * @return Memory usage in bytes
         */
        size_t GetMemoryUsage() const;

        // Serialization

        /**
         * @brief Serialize all entities and systems
         * @param stream Output stream
         * @return true if successful, false otherwise
         */
        bool Serialize(std::ostream& stream) const;

        /**
         * @brief Deserialize entities and systems
         * @param stream Input stream
         * @return true if successful, false otherwise
         */
        bool Deserialize(std::istream& stream);

        /**
         * @brief Create entity from prefab
         * @param prefabName Prefab name
         * @return Pointer to created entity or nullptr if prefab not found
         */
        Entity* CreateEntityFromPrefab(const std::string& prefabName);

        /**
         * @brief Register entity prefab
         * @param name Prefab name
         * @param prefabData Prefab data
         * @return true if registered, false if name exists
         */
        bool RegisterPrefab(const std::string& name, const std::unordered_map<std::string, std::any>& prefabData);

        // Entity pools for memory management

        /**
         * @brief Create entity pool
         * @param poolName Pool name
         * @param initialSize Initial pool size
         * @param maxSize Maximum pool size
         */
        void CreateEntityPool(const std::string& poolName, size_t initialSize, size_t maxSize);

        /**
         * @brief Create entity from pool
         * @param poolName Pool name
         * @return Pointer to entity or nullptr if pool empty
         */
        Entity* CreateEntityFromPool(const std::string& poolName);

        /**
         * @brief Return entity to pool
         * @param entity Entity to return
         * @param poolName Pool name
         * @return true if returned, false if pool not found
         */
        bool ReturnEntityToPool(Entity* entity, const std::string& poolName);

        /**
         * @brief Get pool statistics
         * @param poolName Pool name
         * @return Pool statistics or empty optional if not found
         */
        std::optional<std::pair<size_t, size_t>> GetPoolStatistics(const std::string& poolName) const;

    private:
        /**
         * @brief Generate unique entity ID
         * @return Unique entity ID
         */
        EntityID GenerateEntityID();

        /**
         * @brief Process immediate entity destruction
         * @param entity Entity to destroy
         */
        void DestroyEntityImmediate(Entity* entity);

        /**
         * @brief Update entity-component mappings
         * @param entity Entity that changed
         * @param componentType Component type that was added/removed
         * @param added True if component was added, false if removed
         */
        void UpdateComponentMappings(Entity* entity, const std::string& componentType, bool added);

        /**
         * @brief Update entity-tag mappings
         * @param entity Entity that changed
         * @param tag Tag that was added/removed
         * @param added True if tag was added, false if removed
         */
        void UpdateTagMappings(Entity* entity, const std::string& tag, bool added);

        /**
         * @brief Update entity-type mappings
         * @param entity Entity that changed
         * @param type New entity type
         */
        void UpdateTypeMappings(Entity* entity, const std::string& type);

        /**
         * @brief Clean up destroyed entities
         */
        void CleanupDestroyedEntities();

        // Entity storage
        std::unordered_map<EntityID, std::unique_ptr<Entity>> m_entities;
        std::queue<EntityID> m_availableIDs;                    ///< Available entity IDs for reuse
        std::queue<Entity*> m_pendingDestruction;              ///< Entities pending destruction

        // Entity mappings for fast queries
        std::unordered_map<std::string, std::unordered_set<Entity*>> m_componentEntities;
        std::unordered_map<std::string, std::unordered_set<Entity*>> m_tagEntities;
        std::unordered_map<std::string, std::unordered_set<Entity*>> m_typeEntities;
        std::unordered_map<EntityState, std::unordered_set<Entity*>> m_stateEntities;

        // System management
        std::unordered_map<std::string, std::unique_ptr<System>> m_systems;
        std::vector<System*> m_updateOrder;                    ///< Systems in update order

        // Prefab system
        std::unordered_map<std::string, std::unordered_map<std::string, std::any>> m_prefabs;

        // Entity pools
        struct EntityPool {
            std::queue<Entity*> available;
            size_t maxSize;
            size_t created;
        };
        std::unordered_map<std::string, EntityPool> m_entityPools;

        // Threading and synchronization
        mutable std::shared_mutex m_entityMutex;               ///< Entity access synchronization
        mutable std::shared_mutex m_systemMutex;              ///< System access synchronization
        mutable std::shared_mutex m_mappingMutex;             ///< Mapping access synchronization

        // Statistics
        mutable EntityManagerStatistics m_statistics;
        mutable std::chrono::steady_clock::time_point m_startTime;

        // Configuration
        size_t m_maxEntities;
        bool m_immediateDestruction;                          ///< Destroy entities immediately
        size_t m_pendingDestructionLimit;                     ///< Maximum pending destructions

        // Next entity ID
        std::atomic<EntityID> m_nextEntityID;
    };

    // Template implementations

    template<typename T>
    std::vector<Entity*> EntityManager::GetEntitiesWithComponent() {
        std::shared_lock<std::shared_mutex> lock(m_mappingMutex);

        std::string componentType = typeid(T).name();
        auto it = m_componentEntities.find(componentType);

        if (it != m_componentEntities.end()) {
            return std::vector<Entity*>(it->second.begin(), it->second.end());
        }

        return {};
    }

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_ENTITY_MANAGER_HPP
