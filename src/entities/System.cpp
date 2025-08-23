#include "System.hpp"
#include <algorithm>
#include <chrono>

namespace VoxelCraft {

	// Static member initialization
	static SystemID s_nextSystemID = 1;
	static std::unordered_map<std::string, SystemID> s_systemTypeMap;
	static std::mutex s_systemTypeMutex;

	System::System(const SystemProperties& properties)
		: m_properties(properties)
		, m_state(SystemState::CREATED)
		, m_entityManager(nullptr)
		, m_enabled(true)
		, m_paused(false)
		, m_destroyed(false)
		, m_signature(0)
		, m_statistics({0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0})
		, m_accumulatedTime(0.0f)
	{
		std::lock_guard<std::mutex> lock(s_systemTypeMutex);
		m_id = s_nextSystemID++;
	}

	bool System::Initialize()
	{
		if (m_state != SystemState::CREATED) {
			VOXEL_LOG_WARNING("System {} already initialized", m_properties.name);
			return false;
		}

		VOXEL_LOG_INFO("Initializing system: {}", m_properties.name);

		m_state = SystemState::INITIALIZED;
		m_lastUpdateTime = std::chrono::high_resolution_clock::now();

		return true;
	}

	void System::Shutdown()
	{
		if (m_state == SystemState::DESTROYED) {
			return;
		}

		VOXEL_LOG_INFO("Shutting down system: {}", m_properties.name);

		// Clear tracked entities
		std::lock_guard<std::mutex> lock(m_entityMutex);
		m_trackedEntities.clear();

		m_state = SystemState::DESTROYED;
	}

	void System::Update(float deltaTime)
	{
		if (m_state != SystemState::ACTIVE || !m_enabled.load() || m_paused.load()) {
			return;
		}

		auto startTime = std::chrono::high_resolution_clock::now();

		// Handle update interval
		if (m_properties.updateInterval > 0.0f) {
			m_accumulatedTime += deltaTime;
			if (m_accumulatedTime < m_properties.updateInterval) {
				return;
			}
			deltaTime = m_accumulatedTime;
			m_accumulatedTime = 0.0f;
		}

		// Process entities
		std::unique_lock<std::mutex> entityLock(m_entityMutex, std::try_to_lock);
		if (!entityLock.owns_lock()) {
			return; // Skip update if can't lock
		}

		uint32_t entitiesProcessed = 0;
		for (EntityID entityID : m_trackedEntities) {
			ProcessEntity(entityID, deltaTime);
			entitiesProcessed++;
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		float updateTime = std::chrono::duration_cast<std::chrono::microseconds>(
			endTime - startTime).count() / 1000.0f;

		UpdateStatistics(updateTime, entitiesProcessed);
	}

	void System::UpdateVariable(float deltaTime)
	{
		if (m_state != SystemState::ACTIVE || !m_enabled.load() || m_paused.load()) {
			return;
		}

		// Default implementation - can be overridden
		Update(deltaTime);
	}

	void System::UpdateRender()
	{
		if (m_state != SystemState::ACTIVE || !m_enabled.load() || m_paused.load()) {
			return;
		}

		// Default implementation - can be overridden
	}

	void System::Render()
	{
		if (m_state != SystemState::ACTIVE || !m_enabled.load() || m_paused.load()) {
			return;
		}

		// Default implementation - can be overridden
	}

	void System::OnEvent(const std::string& eventType, void* eventData)
	{
		// Default implementation - can be overridden
		VOXEL_LOG_DEBUG("System {} received event: {}", m_properties.name, eventType);
	}

	bool System::MatchesSignature(EntityID entityID, EntitySignature entitySignature) const
	{
		return (entitySignature & m_signature) == m_signature;
	}

	void System::AddEntity(EntityID entityID)
	{
		std::lock_guard<std::mutex> lock(m_entityMutex);

		if (m_trackedEntities.find(entityID) == m_trackedEntities.end()) {
			m_trackedEntities.insert(entityID);
			VOXEL_LOG_DEBUG("Entity {} added to system {}", entityID, m_properties.name);
		}
	}

	void System::RemoveEntity(EntityID entityID)
	{
		std::lock_guard<std::mutex> lock(m_entityMutex);

		auto it = m_trackedEntities.find(entityID);
		if (it != m_trackedEntities.end()) {
			m_trackedEntities.erase(it);
			VOXEL_LOG_DEBUG("Entity {} removed from system {}", entityID, m_properties.name);
		}
	}

	bool System::HasEntity(EntityID entityID) const
	{
		std::lock_guard<std::mutex> lock(m_entityMutex);
		return m_trackedEntities.find(entityID) != m_trackedEntities.end();
	}

	void System::ProcessEntity(EntityID entityID, float deltaTime)
	{
		// Default implementation - should be overridden by specific systems
		VOXEL_LOG_DEBUG("System {} processing entity {} (default implementation)",
			m_properties.name, entityID);
	}

	void System::ProcessAllEntities(float deltaTime)
	{
		std::lock_guard<std::mutex> lock(m_entityMutex);

		for (EntityID entityID : m_trackedEntities) {
			ProcessEntity(entityID, deltaTime);
		}
	}

	void System::ResetStatistics()
	{
		m_statistics.totalUpdates = 0;
		m_statistics.totalEntitiesProcessed = 0;
		m_statistics.averageUpdateTime = 0.0f;
		m_statistics.maxUpdateTime = 0.0f;
		m_statistics.minUpdateTime = 0.0f;
		m_statistics.totalUpdateTime = 0.0f;
		m_statistics.memoryUsage = 0;
		m_statistics.peakMemoryUsage = 0;
	}

	bool System::HasDependency(const std::string& dependency) const
	{
		return std::find(m_properties.dependencies.begin(),
			m_properties.dependencies.end(), dependency) != m_properties.dependencies.end();
	}

	bool System::HasConflict(const std::string& conflict) const
	{
		return std::find(m_properties.conflicts.begin(),
			m_properties.conflicts.end(), conflict) != m_properties.conflicts.end();
	}

	size_t System::GetMemoryUsage() const
	{
		size_t totalUsage = sizeof(System);
		totalUsage += m_properties.name.capacity();
		totalUsage += m_properties.description.capacity();

		for (const auto& dep : m_properties.dependencies) {
			totalUsage += dep.capacity();
		}

		for (const auto& conflict : m_properties.conflicts) {
			totalUsage += conflict.capacity();
		}

		// Add tracked entities size
		std::lock_guard<std::mutex> lock(m_entityMutex);
		totalUsage += m_trackedEntities.size() * sizeof(EntityID);

		return totalUsage;
	}

	SystemID System::RegisterSystemType(const std::string& typeName)
	{
		std::lock_guard<std::mutex> lock(s_systemTypeMutex);

		auto it = s_systemTypeMap.find(typeName);
		if (it != s_systemTypeMap.end()) {
			return it->second;
		}

		SystemID newID = static_cast<SystemID>(s_systemTypeMap.size()) + 1;
		s_systemTypeMap[typeName] = newID;

		VOXEL_LOG_DEBUG("System type '{}' registered with ID: {}", typeName, newID);
		return newID;
	}

	SystemID System::GetSystemTypeID(const std::string& typeName)
	{
		std::lock_guard<std::mutex> lock(s_systemTypeMutex);

		auto it = s_systemTypeMap.find(typeName);
		return (it != s_systemTypeMap.end()) ? it->second : INVALID_SYSTEM_ID;
	}

} // namespace VoxelCraft