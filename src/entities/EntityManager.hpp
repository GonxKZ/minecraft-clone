#pragma once
#include "Entity.hpp"
#include "Component.hpp"
#include "System.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <mutex>
#include <atomic>
#include <functional>
#include <condition_variable>
#include "core/Logger.hpp"

namespace VoxelCraft {

	class ComponentManager;
	class SystemManager;

	/**
	 * @brief Entity creation parameters
	 */
	struct EntityCreationParams
	{
		std::string name;
		std::string tag;
		Entity::Type type = Entity::Type::CUSTOM;
		Entity::Priority priority = Entity::Priority::NORMAL;
		uint32_t flags = 0;
		bool deferredCreation = false;
	};

	/**
	 * @brief Entity query structure for filtering entities
	 */
	struct EntityQuery
	{
		std::vector<std::string> requiredComponents;
		std::vector<std::string> anyComponents;
		std::vector<std::string> excludedComponents;
		std::vector<Entity::Type> types;
		std::vector<Entity::State> states;
		std::vector<std::string> tags;
		std::string namePattern;
		bool caseSensitive = false;

		// Performance options
		bool sortByPriority = false;
		bool sortByCreationTime = false;
		size_t maxResults = 0;
	};

	/**
	 * @brief Entity manager statistics
	 */
	struct EntityManagerStats
	{
		uint64_t totalEntitiesCreated;
		uint64_t totalEntitiesDestroyed;
		uint64_t activeEntities;
		uint64_t inactiveEntities;
		uint32_t maxEntities;
		uint32_t entityPoolSize;
		uint32_t entityPoolUsage;
		float averageEntityLifetime;
		uint64_t totalEntityUpdates;
		float averageUpdateTime;
		size_t memoryUsage;
		size_t peakMemoryUsage;
	};

	/**
	 * @brief Entity processing queue item
	 */
	struct EntityQueueItem
	{
		EntityID entityID;
		std::function<void(EntityID)> processor;
		int priority;
	};

	/**
	 * @brief Entity manager for handling entity lifecycle
	 *
	 * The EntityManager is responsible for:
	 * - Creating and destroying entities
	 * - Managing entity pools for performance
	 * - Processing entity queries
	 * - Handling entity serialization
	 * - Managing entity updates and rendering
	 * - Coordinating with ComponentManager and SystemManager
	 */
	class EntityManager
	{
	public:
		/**
		 * @brief Entity manager configuration
		 */
		struct Config
		{
			size_t initialEntityPoolSize = 1000;
			size_t maxEntityPoolSize = 10000;
			size_t entityPoolGrowthSize = 500;
			bool enableEntityPooling = true;
			bool enableParallelProcessing = true;
			size_t maxProcessingThreads = 4;
			size_t maxEntitiesPerFrame = 1000;
			bool enableSerialization = true;
			bool enableProfiling = true;
			float cleanupInterval = 30.0f; // seconds
		};

	private:
		Config m_config;
		std::unordered_map<EntityID, std::shared_ptr<Entity>> m_entities;
		std::unordered_map<EntityID, std::shared_ptr<Entity>> m_entityPool;
		std::queue<EntityID> m_availableIDs;
		std::vector<EntityID> m_pendingDestruction;
		std::vector<std::shared_ptr<Entity>> m_pendingCreation;
		std::queue<EntityQueueItem> m_processingQueue;

		ComponentManager* m_componentManager;
		SystemManager* m_systemManager;

		// Threading
		std::mutex m_entityMutex;
		std::mutex m_poolMutex;
		std::mutex m_processingMutex;
		std::condition_variable m_processingCV;
		std::atomic<bool> m_processingActive;
		std::vector<std::thread> m_processingThreads;

		// Statistics
		EntityManagerStats m_stats;
		std::chrono::high_resolution_clock::time_point m_lastCleanupTime;

		// Entity indexing for queries
		std::unordered_multimap<std::string, EntityID> m_tagIndex;
		std::unordered_multimap<Entity::Type, EntityID> m_typeIndex;
		std::unordered_multimap<Entity::State, EntityID> m_stateIndex;
		std::unordered_multimap<EntitySignature, EntityID> m_signatureIndex;

	public:
		/**
		 * @brief Constructor
		 */
		EntityManager(const Config& config = Config());

		/**
		 * @brief Destructor
		 */
		~EntityManager();

		/**
		 * @brief Initialize entity manager
		 */
		bool Initialize(ComponentManager* componentManager, SystemManager* systemManager);

		/**
		 * @brief Shutdown entity manager
		 */
		void Shutdown();

		/**
		 * @brief Update entity manager
		 */
		void Update(float deltaTime);

		/**
		 * @brief Render entities
		 */
		void Render();

		/**
		 * @brief Create entity
		 */
		std::shared_ptr<Entity> CreateEntity(const EntityCreationParams& params = EntityCreationParams());

		/**
		 * @brief Destroy entity
		 */
		void DestroyEntity(EntityID entityID);

		/**
		 * @brief Get entity by ID
		 */
		std::shared_ptr<Entity> GetEntity(EntityID entityID) const;

		/**
		 * @brief Check if entity exists
		 */
		bool EntityExists(EntityID entityID) const;

		/**
		 * @brief Get all entities
		 */
		std::vector<std::shared_ptr<Entity>> GetAllEntities() const;

		/**
		 * @brief Get entities by query
		 */
		std::vector<std::shared_ptr<Entity>> QueryEntities(const EntityQuery& query) const;

		/**
		 * @brief Get entities by tag
		 */
		std::vector<std::shared_ptr<Entity>> GetEntitiesByTag(const std::string& tag) const;

		/**
		 * @brief Get entities by type
		 */
		std::vector<std::shared_ptr<Entity>> GetEntitiesByType(Entity::Type type) const;

		/**
		 * @brief Get entities by state
		 */
		std::vector<std::shared_ptr<Entity>> GetEntitiesByState(Entity::State state) const;

		/**
		 * @brief Get entities with components
		 */
		std::vector<std::shared_ptr<Entity>> GetEntitiesWithComponents(
			const std::vector<std::string>& componentTypes) const;

		/**
		 * @brief Process entities with function
		 */
		void ProcessEntities(const std::function<void(EntityID)>& processor, int priority = 0);

		/**
		 * @brief Process entities in parallel
		 */
		void ProcessEntitiesParallel(const std::function<void(EntityID)>& processor,
			size_t maxConcurrency = 0);

		/**
		 * @brief Serialize all entities
		 */
		std::vector<uint8_t> SerializeAllEntities() const;

		/**
		 * @brief Deserialize entities
		 */
		void DeserializeEntities(const std::vector<uint8_t>& data);

		/**
		 * @brief Clear all entities
		 */
		void ClearAllEntities();

		/**
		 * @brief Get entity count
		 */
		size_t GetEntityCount() const;

		/**
		 * @brief Get active entity count
		 */
		size_t GetActiveEntityCount() const;

		/**
		 * @brief Get entity manager statistics
		 */
		const EntityManagerStats& GetStatistics() const { return m_stats; }

		/**
		 * @brief Reset statistics
		 */
		void ResetStatistics();

		/**
		 * @brief Get memory usage
		 */
		size_t GetMemoryUsage() const;

		/**
		 * @brief Set entity manager configuration
		 */
		void SetConfig(const Config& config) { m_config = config; }

		/**
		 * @brief Get entity manager configuration
		 */
		const Config& GetConfig() const { return m_config; }

		/**
		 * @brief Enable/disable entity pooling
		 */
		void SetEntityPoolingEnabled(bool enabled) { m_config.enableEntityPooling = enabled; }

		/**
		 * @brief Check if entity pooling is enabled
		 */
		bool IsEntityPoolingEnabled() const { return m_config.enableEntityPooling; }

		/**
		 * @brief Enable/disable parallel processing
		 */
		void SetParallelProcessingEnabled(bool enabled) { m_config.enableParallelProcessing = enabled; }

		/**
		 * @brief Check if parallel processing is enabled
		 */
		bool IsParallelProcessingEnabled() const { return m_config.enableParallelProcessing; }

	private:
		/**
		 * @brief Generate new entity ID
		 */
		EntityID GenerateEntityID();

		/**
		 * @brief Create entity instance
		 */
		std::shared_ptr<Entity> CreateEntityInstance(EntityID entityID, const EntityCreationParams& params);

		/**
		 * @brief Process pending creations
		 */
		void ProcessPendingCreations();

		/**
		 * @brief Process pending destructions
		 */
		void ProcessPendingDestructions();

		/**
		 * @brief Perform cleanup operations
		 */
		void PerformCleanup();

		/**
		 * @brief Update entity indices
		 */
		void UpdateEntityIndices(EntityID entityID, bool add);

		/**
		 * @brief Processing thread function
		 */
		void ProcessingThreadFunction();

		/**
		 * @brief Process entity queue item
		 */
		void ProcessEntityQueueItem(const EntityQueueItem& item);

		/**
		 * @brief Update statistics
		 */
		void UpdateStatistics();

		/**
		 * @brief Validate entity
		 */
		bool ValidateEntity(EntityID entityID) const;

		/**
		 * @brief Get entity from pool or create new
		 */
		std::shared_ptr<Entity> GetEntityFromPool();

		/**
		 * @brief Return entity to pool
		 */
		void ReturnEntityToPool(std::shared_ptr<Entity> entity);

		/**
		 * @brief Grow entity pool
		 */
		void GrowEntityPool(size_t additionalSize);

		/**
		 * @brief Cleanup entity pool
		 */
		void CleanupEntityPool();
	};

} // namespace VoxelCraft