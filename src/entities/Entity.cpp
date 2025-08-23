#include "Entity.hpp"
#include "Component.hpp"
#include <algorithm>
#include <chrono>

namespace VoxelCraft {

	// Static member initialization
	static EntityID s_nextEntityID = 1;
	static std::mutex s_entityIDMutex;

	Entity::Entity(EntityID id, EntityManager* manager)
		: m_id(id)
		, m_signature(0)
		, m_state(State::ACTIVE)
		, m_type(Type::CUSTOM)
		, m_priority(Priority::NORMAL)
		, m_flags(0)
		, m_name("")
		, m_tag("")
		, m_manager(manager)
		, m_destroyed(false)
		, m_creationTime(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count())
		, m_lastUpdateTime(m_creationTime)
		, m_updateCount(0)
		, m_updateTime(0.0f)
	{
		if (m_id == 0) {
			std::lock_guard<std::mutex> lock(s_entityIDMutex);
			m_id = s_nextEntityID++;
		}

		VOXEL_LOG_INFO("Entity created with ID: {}", m_id);
	}

	Entity::~Entity()
	{
		VOXEL_LOG_INFO("Entity destroyed with ID: {}", m_id);

		// Clean up components
		std::lock_guard<std::mutex> lock(m_componentMutex);
		for (auto& component : m_components) {
			if (component) {
				component->Destroy();
			}
		}
		m_components.clear();
		m_componentMap.clear();
	}

	void Entity::Destroy()
	{
		if (!m_destroyed.exchange(true)) {
			m_state = State::DESTROYED;
			VOXEL_LOG_INFO("Entity marked for destruction: {}", m_id);
		}
	}

	void Entity::UpdateSignature()
	{
		m_signature = 0;
		std::lock_guard<std::mutex> lock(m_componentMutex);

		for (const auto& component : m_components) {
			if (component && component->IsEnabled()) {
				ComponentID componentId = component->GetTypeID();
				if (componentId < sizeof(EntitySignature) * 8) {
					m_signature |= (static_cast<EntitySignature>(1) << componentId);
				}
			}
		}
	}

	void Entity::AddComponentInternal(std::shared_ptr<Component> component)
	{
		if (!component) {
			VOXEL_LOG_ERROR("Cannot add null component to entity {}", m_id);
			return;
		}

		std::lock_guard<std::mutex> lock(m_componentMutex);

		// Check if component type is unique and already exists
		if (component->IsUnique()) {
			auto it = m_componentMap.find(component->GetTypeName());
			if (it != m_componentMap.end()) {
				VOXEL_LOG_WARNING("Entity {} already has unique component of type {}",
					m_id, component->GetTypeName());
				return;
			}
		}

		// Add component
		m_components.push_back(component);
		m_componentMap[component->GetTypeName()] = component;

		// Initialize component
		component->Initialize();

		// Update signature
		UpdateSignature();

		VOXEL_LOG_DEBUG("Component {} added to entity {}",
			component->GetTypeName(), m_id);
	}

	void Entity::RemoveComponentInternal(const std::string& componentType)
	{
		std::lock_guard<std::mutex> lock(m_componentMutex);

		auto it = m_componentMap.find(componentType);
		if (it == m_componentMap.end()) {
			VOXEL_LOG_WARNING("Component {} not found on entity {}",
				componentType, m_id);
			return;
		}

		// Find and remove component
		auto componentIt = std::find_if(m_components.begin(), m_components.end(),
			[&](const std::shared_ptr<Component>& comp) {
				return comp && comp->GetTypeName() == componentType;
			});

		if (componentIt != m_components.end()) {
			(*componentIt)->Destroy();
			m_components.erase(componentIt);
		}

		m_componentMap.erase(it);

		// Update signature
		UpdateSignature();

		VOXEL_LOG_DEBUG("Component {} removed from entity {}",
			componentType, m_id);
	}

	std::shared_ptr<Component> Entity::GetComponentInternal(const std::string& componentType) const
	{
		std::lock_guard<std::mutex> lock(m_componentMutex);

		auto it = m_componentMap.find(componentType);
		return (it != m_componentMap.end()) ? it->second : nullptr;
	}

	void Entity::Initialize()
	{
		if (m_state != State::CREATED) {
			return;
		}

		VOXEL_LOG_DEBUG("Initializing entity {}", m_id);

		std::lock_guard<std::mutex> lock(m_componentMutex);
		for (auto& component : m_components) {
			if (component && component->IsEnabled()) {
				component->Initialize();
			}
		}

		m_state = State::ACTIVE;
	}

	void Entity::Update(float deltaTime)
	{
		if (m_state != State::ACTIVE || m_destroyed.load()) {
			return;
		}

		auto startTime = std::chrono::high_resolution_clock::now();

		std::lock_guard<std::mutex> lock(m_componentMutex);
		for (auto& component : m_components) {
			if (component && component->IsEnabled()) {
				component->Update(deltaTime);
			}
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		float updateTime = std::chrono::duration_cast<std::chrono::microseconds>(
			endTime - startTime).count() / 1000.0f;

		// Update performance statistics
		m_lastUpdateTime = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
		m_updateCount++;
		m_updateTime = (m_updateTime * (m_updateCount - 1) + updateTime) / m_updateCount;
	}

	void Entity::Render()
	{
		if (m_state != State::ACTIVE || m_destroyed.load()) {
			return;
		}

		std::lock_guard<std::mutex> lock(m_componentMutex);
		for (auto& component : m_components) {
			if (component && component->IsEnabled()) {
				component->Render();
			}
		}
	}

	std::vector<uint8_t> Entity::Serialize() const
	{
		std::vector<uint8_t> data;

		// Serialize entity header
		uint32_t headerSize = sizeof(EntityID) + sizeof(State) + sizeof(Type) +
			sizeof(Priority) + sizeof(uint32_t) + sizeof(uint64_t);

		data.resize(headerSize);
		uint8_t* ptr = data.data();

		// Entity ID
		std::memcpy(ptr, &m_id, sizeof(EntityID));
		ptr += sizeof(EntityID);

		// State
		std::memcpy(ptr, &m_state, sizeof(State));
		ptr += sizeof(State);

		// Type
		std::memcpy(ptr, &m_type, sizeof(Type));
		ptr += sizeof(Type);

		// Priority
		std::memcpy(ptr, &m_priority, sizeof(Priority));
		ptr += sizeof(Priority);

		// Flags
		std::memcpy(ptr, &m_flags, sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		// Signature
		std::memcpy(ptr, &m_signature, sizeof(EntitySignature));
		ptr += sizeof(EntitySignature);

		// Serialize strings
		auto serializeString = [&](const std::string& str) {
			uint32_t size = static_cast<uint32_t>(str.size());
			data.insert(data.end(), reinterpret_cast<uint8_t*>(&size),
				reinterpret_cast<uint8_t*>(&size) + sizeof(uint32_t));
			data.insert(data.end(), str.begin(), str.end());
		};

		serializeString(m_name);
		serializeString(m_tag);

		// Serialize components
		std::lock_guard<std::mutex> lock(m_componentMutex);
		uint32_t componentCount = static_cast<uint32_t>(m_components.size());
		data.insert(data.end(), reinterpret_cast<uint8_t*>(&componentCount),
			reinterpret_cast<uint8_t*>(&componentCount) + sizeof(uint32_t));

		for (const auto& component : m_components) {
			if (component && component->IsSerializable()) {
				auto componentData = component->Serialize();

				// Component type name
				std::string typeName = component->GetTypeName();
				uint32_t nameSize = static_cast<uint32_t>(typeName.size());
				data.insert(data.end(), reinterpret_cast<uint8_t*>(&nameSize),
					reinterpret_cast<uint8_t*>(&nameSize) + sizeof(uint32_t));
				data.insert(data.end(), typeName.begin(), typeName.end());

				// Component data size
				uint32_t dataSize = static_cast<uint32_t>(componentData.size());
				data.insert(data.end(), reinterpret_cast<uint8_t*>(&dataSize),
					reinterpret_cast<uint8_t*>(&dataSize) + sizeof(uint32_t));

				// Component data
				data.insert(data.end(), componentData.begin(), componentData.end());
			}
		}

		return data;
	}

	void Entity::Deserialize(const std::vector<uint8_t>& data)
	{
		if (data.size() < sizeof(EntityID) + sizeof(State) + sizeof(Type) +
			sizeof(Priority) + sizeof(uint32_t) + sizeof(EntitySignature)) {
			VOXEL_LOG_ERROR("Entity deserialization failed: insufficient data");
			return;
		}

		const uint8_t* ptr = data.data();

		// Entity ID
		std::memcpy(&m_id, ptr, sizeof(EntityID));
		ptr += sizeof(EntityID);

		// State
		std::memcpy(&m_state, ptr, sizeof(State));
		ptr += sizeof(State);

		// Type
		std::memcpy(&m_type, ptr, sizeof(Type));
		ptr += sizeof(Type);

		// Priority
		std::memcpy(&m_priority, ptr, sizeof(Priority));
		ptr += sizeof(Priority);

		// Flags
		std::memcpy(&m_flags, ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		// Signature
		std::memcpy(&m_signature, ptr, sizeof(EntitySignature));
		ptr += sizeof(EntitySignature);

		// Deserialize strings
		auto deserializeString = [&](std::string& str) {
			if (ptr + sizeof(uint32_t) > data.data() + data.size()) return false;
			uint32_t size;
			std::memcpy(&size, ptr, sizeof(uint32_t));
			ptr += sizeof(uint32_t);

			if (ptr + size > data.data() + data.size()) return false;
			str.assign(reinterpret_cast<const char*>(ptr), size);
			ptr += size;
			return true;
		};

		if (!deserializeString(m_name) || !deserializeString(m_tag)) {
			VOXEL_LOG_ERROR("Entity deserialization failed: string data corrupted");
			return;
		}

		// Clear existing components
		{
			std::lock_guard<std::mutex> lock(m_componentMutex);
			m_components.clear();
			m_componentMap.clear();
		}

		// Deserialize components
		if (ptr + sizeof(uint32_t) > data.data() + data.size()) {
			VOXEL_LOG_ERROR("Entity deserialization failed: component count corrupted");
			return;
		}

		uint32_t componentCount;
		std::memcpy(&componentCount, ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);

		for (uint32_t i = 0; i < componentCount; ++i) {
			if (ptr + sizeof(uint32_t) > data.data() + data.size()) break;

			// Component type name
			uint32_t nameSize;
			std::memcpy(&nameSize, ptr, sizeof(uint32_t));
			ptr += sizeof(uint32_t);

			if (ptr + nameSize > data.data() + data.size()) break;
			std::string typeName(reinterpret_cast<const char*>(ptr), nameSize);
			ptr += nameSize;

			// Component data size
			if (ptr + sizeof(uint32_t) > data.data() + data.size()) break;
			uint32_t dataSize;
			std::memcpy(&dataSize, ptr, sizeof(uint32_t));
			ptr += sizeof(uint32_t);

			if (ptr + dataSize > data.data() + data.size()) break;

			// Component data
			std::vector<uint8_t> componentData(ptr, ptr + dataSize);
			ptr += dataSize;

			// Note: Component creation would need a component factory
			// This is a placeholder for the actual component deserialization
			VOXEL_LOG_DEBUG("Component {} deserialized ({} bytes)", typeName, dataSize);
		}
	}

	size_t Entity::GetMemoryUsage() const
	{
		size_t totalUsage = sizeof(Entity);
		totalUsage += m_name.capacity() + m_tag.capacity();

		std::lock_guard<std::mutex> lock(m_componentMutex);
		for (const auto& component : m_components) {
			if (component) {
				totalUsage += component->GetMemoryUsage();
			}
		}

		return totalUsage;
	}

	std::shared_ptr<Entity> Entity::Clone() const
	{
		auto clonedEntity = std::make_shared<Entity>(0, m_manager);

		clonedEntity->m_state = m_state;
		clonedEntity->m_type = m_type;
		clonedEntity->m_priority = m_priority;
		clonedEntity->m_flags = m_flags;
		clonedEntity->m_name = m_name + "_clone";
		clonedEntity->m_tag = m_tag;

		// Clone components
		std::lock_guard<std::mutex> lock(m_componentMutex);
		for (const auto& component : m_components) {
			if (component) {
				auto clonedComponent = component->Clone();
				if (clonedComponent) {
					clonedEntity->AddComponentInternal(clonedComponent);
				}
			}
		}

		VOXEL_LOG_INFO("Entity {} cloned to {}", m_id, clonedEntity->m_id);
		return clonedEntity;
	}

} // namespace VoxelCraft