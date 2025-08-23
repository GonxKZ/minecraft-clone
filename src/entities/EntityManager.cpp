#include "EntityManager.hpp"
#include <algorithm>
#include <regex>
#include <chrono>

namespace VoxelCraft {

	EntityManager::EntityManager(const Config& config)
		: m_config(config)
		, m_componentManager(nullptr)
		, m_systemManager(nullptr)
		, m_processingActive(false)
		, m_stats({0, 0, 0, 0, 0, 0, 0, 0.0f, 0, 0.0f, 0, 0})
	{
		m_lastCleanupTime = std::chrono::high_resolution_clock::now();

		// Initialize entity pool
		if (m_config.enableEntityPooling) {
			m_entityPool.reserve(m_config.initialEntityPoolSize);
			for (size_t i = 0; i < m_config.initialEntityPoolSize; ++i) {
				m_availableIDs.push(i + 1);
			}
		}

		VOXEL_LOG_INFO("EntityManager initialized with pool size: {}", m_config.initialEntityPoolSize);
	}

	EntityManager::~EntityManager()
	{
		Shutdown();
	}

	bool EntityManager::Initialize(ComponentManager* componentManager, SystemManager* systemManager)
	{
		m_componentManager = componentManager;
		m_systemManager = systemManager;

		// Start processing threads
		if (m_config.enableParallelProcessing) {
			m_processingActive.store(true);
			for (size_t i = 0; i < m_config.maxProcessingThreads; ++i) {
				m_processingThreads.emplace_back(&EntityManager::ProcessingThreadFunction, this);
			}
			VOXEL_LOG_INFO("Started {} processing threads", m_config.maxProcessingThreads);
		}

		VOXEL_LOG_INFO("EntityManager initialized successfully");
		return true;
	}

	void EntityManager::Shutdown()
	{
		VOXEL_LOG_INFO("Shutting down EntityManager...");

		// Stop processing threads
		if (m_processingActive.load()) {
			m_processingActive.store(false);
			m_processingCV.notify_all();

			for (auto& thread : m_processingThreads) {
				if (thread.joinable()) {
					thread.join();
				}
			}
			m_processingThreads.clear();
			VOXEL_LOG_INFO("Processing threads stopped");
		}

		// Clear all entities
		ClearAllEntities();

		// Clear entity pool
		{
			std::lock_guard<std::mutex> lock(m_poolMutex);
			m_entityPool.clear();
			while (!m_availableIDs.empty()) {
				m_availableIDs.pop();
			}
		}

		// Clear indices
		m_tagIndex.clear();
		m_typeIndex.clear();
		m_stateIndex.clear();
		m_signatureIndex.clear();

		VOXEL_LOG_INFO("EntityManager shutdown complete");
	}

	void EntityManager::Update(float deltaTime)
	{
		auto startTime = std::chrono::high_resolution_clock::now();

		// Process pending operations
		ProcessPendingCreations();
		ProcessPendingDestructions();

		// Update entities
		std::vector<EntityID> entitiesToUpdate;
		{
			std::lock_guard<std::mutex> lock(m_entityMutex);
			for (const auto& pair : m_entities) {
				if (pair.second && pair.second->IsActive()) {
					entitiesToUpdate.push_back(pair.first);
				}
			}
		}

		// Update entities with limit per frame
		size_t updateCount = 0;
		for (EntityID entityID : entitiesToUpdate) {
			if (updateCount >= m_config.maxEntitiesPerFrame) {
				break;
			}

			auto entity = GetEntity(entityID);
			if (entity) {
				entity->Update(deltaTime);
				updateCount++;
			}
		}

		// Perform periodic cleanup
		auto currentTime = std::chrono::high_resolution_clock::now();
		float timeSinceCleanup = std::chrono::duration_cast<std::chrono::seconds>(
			currentTime - m_lastCleanupTime).count();

		if (timeSinceCleanup >= m_config.cleanupInterval) {
			PerformCleanup();
			m_lastCleanupTime = currentTime;
		}

		// Update statistics
		auto endTime = std::chrono::high_resolution_clock::now();
		float updateTime = std::chrono::duration_cast<std::chrono::microseconds>(
			endTime - startTime).count() / 1000.0f;

		UpdateStatistics();
		m_stats.averageUpdateTime = (m_stats.averageUpdateTime * m_stats.totalEntityUpdates +
			updateTime) / (m_stats.totalEntityUpdates + 1);
		m_stats.totalEntityUpdates++;
	}

	EntityID EntityManager::GenerateEntityID()
	{
		if (m_config.enableEntityPooling && !m_availableIDs.empty()) {
			std::lock_guard<std::mutex> lock(m_poolMutex);
			if (!m_availableIDs.empty()) {
				EntityID id = m_availableIDs.front();
				m_availableIDs.pop();
				return id;
			}
		}

		// Generate new ID
		static EntityID nextID = 1;
		return nextID++;
	}

} // namespace VoxelCraft