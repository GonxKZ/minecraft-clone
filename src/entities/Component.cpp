#include "Component.hpp"
#include <algorithm>
#include <chrono>

namespace VoxelCraft {

	// Static member initialization
	static ComponentID s_nextComponentID = 1;
	static std::unordered_map<std::string, ComponentID> s_componentTypeMap;
	static std::mutex s_componentTypeMutex;

	Component::Component(const Properties& properties)
		: m_properties(properties)
		, m_state(ComponentState::ACTIVE)
		, m_owner(nullptr)
		, m_enabled(true)
		, m_destroyed(false)
		, m_creationTime(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count())
		, m_lastUpdateTime(m_creationTime)
		, m_updateCount(0)
		, m_updateTime(0.0f)
		, m_memoryUsage(sizeof(Component))
	{
		std::lock_guard<std::mutex> lock(s_componentTypeMutex);
		m_id = s_nextComponentID++;
	}

	std::vector<uint8_t> Component::Serialize() const
	{
		std::vector<uint8_t> data;

		// Serialize component header
		uint32_t headerSize = sizeof(ComponentID) + sizeof(ComponentState) +
			sizeof(uint32_t) + sizeof(uint64_t) * 3 + sizeof(float);

		data.resize(headerSize);
		uint8_t* ptr = data.data();

		// Component ID
		std::memcpy(ptr, &m_id, sizeof(ComponentID));
		ptr += sizeof(ComponentID);

		// State
		std::memcpy(ptr, &m_state, sizeof(ComponentState));
		ptr += sizeof(ComponentState);

		// Version
		std::memcpy(ptr, &m_properties.version, sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		// Creation time
		std::memcpy(ptr, &m_creationTime, sizeof(uint64_t));
		ptr += sizeof(uint64_t);

		// Last update time
		std::memcpy(ptr, &m_lastUpdateTime, sizeof(uint64_t));
		ptr += sizeof(uint64_t);

		// Update count
		std::memcpy(ptr, &m_updateCount, sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		// Update time
		std::memcpy(ptr, &m_updateTime, sizeof(float));
		ptr += sizeof(float);

		// Serialize properties
		auto serializeString = [&](const std::string& str) {
			uint32_t size = static_cast<uint32_t>(str.size());
			data.insert(data.end(), reinterpret_cast<uint8_t*>(&size),
				reinterpret_cast<uint8_t*>(&size) + sizeof(uint32_t));
			data.insert(data.end(), str.begin(), str.end());
		};

		serializeString(m_properties.name);
		serializeString(m_properties.description);

		// Serialize dependencies
		uint32_t depCount = static_cast<uint32_t>(m_properties.dependencies.size());
		data.insert(data.end(), reinterpret_cast<uint8_t*>(&depCount),
			reinterpret_cast<uint8_t*>(&depCount) + sizeof(uint32_t));

		for (const auto& dep : m_properties.dependencies) {
			serializeString(dep);
		}

		return data;
	}

	void Component::Deserialize(const std::vector<uint8_t>& data)
	{
		if (data.size() < sizeof(ComponentID) + sizeof(ComponentState) + sizeof(uint32_t) +
			sizeof(uint64_t) * 3 + sizeof(float)) {
			VOXEL_LOG_ERROR("Component deserialization failed: insufficient data");
			return;
		}

		const uint8_t* ptr = data.data();

		// Component ID
		std::memcpy(&m_id, ptr, sizeof(ComponentID));
		ptr += sizeof(ComponentID);

		// State
		std::memcpy(&m_state, ptr, sizeof(ComponentState));
		ptr += sizeof(ComponentState);

		// Version
		std::memcpy(&m_properties.version, ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		// Creation time
		std::memcpy(&m_creationTime, ptr, sizeof(uint64_t));
		ptr += sizeof(uint64_t);

		// Last update time
		std::memcpy(&m_lastUpdateTime, ptr, sizeof(uint64_t));
		ptr += sizeof(uint64_t);

		// Update count
		std::memcpy(&m_updateCount, ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		// Update time
		std::memcpy(&m_updateTime, ptr, sizeof(float));
		ptr += sizeof(float);

		// Deserialize strings
		auto deserializeString = [&](std::string& str) -> bool {
			if (ptr + sizeof(uint32_t) > data.data() + data.size()) return false;
			uint32_t size;
			std::memcpy(&size, ptr, sizeof(uint32_t));
			ptr += sizeof(uint32_t);

			if (ptr + size > data.data() + data.size()) return false;
			str.assign(reinterpret_cast<const char*>(ptr), size);
			ptr += size;
			return true;
		};

		if (!deserializeString(m_properties.name) ||
			!deserializeString(m_properties.description)) {
			VOXEL_LOG_ERROR("Component deserialization failed: string data corrupted");
			return;
		}

		// Deserialize dependencies
		if (ptr + sizeof(uint32_t) > data.data() + data.size()) {
			VOXEL_LOG_ERROR("Component deserialization failed: dependency count corrupted");
			return;
		}

		uint32_t depCount;
		std::memcpy(&depCount, ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		m_properties.dependencies.clear();
		for (uint32_t i = 0; i < depCount; ++i) {
			std::string dep;
			if (!deserializeString(dep)) {
				VOXEL_LOG_ERROR("Component deserialization failed: dependency {} corrupted", i);
				return;
			}
			m_properties.dependencies.push_back(dep);
		}
	}

	bool Component::HasDependency(const std::string& dependency) const
	{
		return std::find(m_properties.dependencies.begin(),
			m_properties.dependencies.end(), dependency) != m_properties.dependencies.end();
	}

	void Component::UpdatePerformanceStats(float deltaTime)
	{
		m_lastUpdateTime = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
		m_updateCount++;
		m_updateTime = (m_updateTime * (m_updateCount - 1) + deltaTime * 1000.0f) / m_updateCount;
	}

	void Component::UpdateMemoryUsage()
	{
		m_memoryUsage = sizeof(Component);
		m_memoryUsage += m_properties.name.capacity();
		m_memoryUsage += m_properties.description.capacity();

		for (const auto& dep : m_properties.dependencies) {
			m_memoryUsage += dep.capacity();
		}
	}

	ComponentID Component::RegisterComponentType(const std::string& typeName)
	{
		std::lock_guard<std::mutex> lock(s_componentTypeMutex);

		auto it = s_componentTypeMap.find(typeName);
		if (it != s_componentTypeMap.end()) {
			return it->second;
		}

		ComponentID newID = static_cast<ComponentID>(s_componentTypeMap.size()) + 1;
		s_componentTypeMap[typeName] = newID;

		VOXEL_LOG_DEBUG("Component type '{}' registered with ID: {}", typeName, newID);
		return newID;
	}

	ComponentID Component::GetComponentTypeID(const std::string& typeName)
	{
		std::lock_guard<std::mutex> lock(s_componentTypeMutex);

		auto it = s_componentTypeMap.find(typeName);
		return (it != s_componentTypeMap.end()) ? it->second : INVALID_COMPONENT_ID;
	}

} // namespace VoxelCraft