#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>
#include "core/Logger.hpp"

namespace VoxelCraft {

	// Forward declarations
	class Component;
	class EntityManager;

	/**
	 * @brief Unique identifier for entities in the ECS
	 */
	using EntityID = uint64_t;

	/**
	 * @brief Invalid entity ID constant
	 */
	constexpr EntityID INVALID_ENTITY_ID = 0;

	/**
	 * @brief Entity signature for component composition tracking
	 */
	using EntitySignature = uint64_t;

	/**
	 * @brief Main Entity class for the ECS architecture
	 *
	 * An entity represents a game object that can have multiple components
	 * attached to it. Entities are identified by unique IDs and have
	 * signatures that track which components they contain.
	 */
	class Entity
	{
	public:
		/**
		 * @brief Entity states for lifecycle management
		 */
		enum class State
		{
			ACTIVE,     // Entity is active and being processed
			INACTIVE,   // Entity is inactive but not destroyed
			PENDING,    // Entity is pending creation/destruction
			DESTROYED   // Entity is marked for destruction
		};

		/**
		 * @brief Entity types for categorization
		 */
		enum class Type
		{
			PLAYER,         // Player character
			NPC,           // Non-player character
			MOB,           // Enemy mob
			ANIMAL,        // Passive animal
			VEHICLE,       // Vehicle/transport
			PROJECTILE,    // Projectiles and effects
			ITEM,          // Dropped items
			BLOCK,         // Block entities
			PARTICLE,      // Particle effects
			DECORATION,    // Decorative objects
			STRUCTURE,     // Buildings and structures
			ENVIRONMENT,   // Environmental objects
			WEAPON,        // Weapons and tools
			VEGETATION,    // Plants and trees
			WATER,         // Water entities
			FIRE,          // Fire entities
			EXPLOSION,     // Explosions
			LIGHT,         // Light sources
			SOUND,         // Sound emitters
			EFFECT,        // Visual effects
			GUI,           // UI elements
			CAMERA,        // Camera entities
			TRIGGER,       // Trigger volumes
			PORTAL,        // Portal entities
			DOOR,          // Door entities
			CONTAINER,     // Container entities
			FURNITURE,     // Furniture entities
			DECAL,         // Decal entities
			SHADOW,        // Shadow entities
			REFLECTION,    // Reflection entities
			MIRROR,        // Mirror entities
			LENS_FLARE,    // Lens flare entities
			VOLUMETRIC,    // Volumetric effects
			ATMOSPHERIC,   // Atmospheric effects
			WEATHER,       // Weather effects
			TERRAIN,       // Terrain entities
			FLUID,         // Fluid entities
			GAS,           // Gas entities
			PLASMA,        // Plasma entities
			ENERGY,        // Energy entities
			FIELD,         // Field entities
			WAVE,          // Wave entities
			PULSE,         // Pulse entities
			BEAM,          // Beam entities
			RAY,           // Ray entities
			LASER,         // Laser entities
			PLASMA,        // Plasma entities
			SHIELD,        // Shield entities
			FORCE_FIELD,   // Force field entities
			BARRIER,       // Barrier entities
			WALL,          // Wall entities
			FENCE,         // Fence entities
			GATE,          // Gate entities
			BRIDGE,        // Bridge entities
			ROAD,          // Road entities
			PATH,          // Path entities
			RAIL,          // Rail entities
			TRACK,         // Track entities
			PLATFORM,      // Platform entities
			ELEVATOR,      // Elevator entities
			LIFT,          // Lift entities
			ESCALATOR,     // Escalator entities
			CONVEYOR,      // Conveyor entities
			CRANE,         // Crane entities
			HOIST,         // Hoist entities
			WINCH,         // Winch entities
			PULLEY,        // Pulley entities
			LEVER,         // Lever entities
			GEAR,          // Gear entities
			SPRING,        // Spring entities
			DAMPER,        // Damper entities
			MOTOR,         // Motor entities
			ENGINE,        // Engine entities
			TURBINE,       // Turbine entities
			REACTOR,       // Reactor entities
			GENERATOR,     // Generator entities
			BATTERY,       // Battery entities
			CAPACITOR,     // Capacitor entities
			RESISTOR,      // Resistor entities
			INDUCTOR,      // Inductor entities
			TRANSFORMER,   // Transformer entities
			TRANSMITTER,   // Transmitter entities
			RECEIVER,      // Receiver entities
			ANTENNA,       // Antenna entities
			RADAR,         // Radar entities
			SONAR,         // Sonar entities
			LIDAR,         // Lidar entities
			CAMERA,        // Camera entities
			MICROPHONE,    // Microphone entities
			SPEAKER,       // Speaker entities
			DISPLAY,       // Display entities
			SCREEN,        // Screen entities
			MONITOR,       // Monitor entities
			PROJECTOR,     // Projector entities
			HOLOGRAM,      // Hologram entities
			VIRTUAL,       // Virtual entities
			AUGMENTED,     // Augmented entities
			MIXED,         // Mixed reality entities
			EXTENDED,      // Extended reality entities
			SYNTHETIC,     // Synthetic reality entities
			SIMULATED,     // Simulated reality entities
			ARTIFICIAL,    // Artificial reality entities
			DIGITAL,       // Digital reality entities
			VIRTUALIZED,   // Virtualized reality entities
			IMMERSIVE,     // Immersive entities
			INTERACTIVE,   // Interactive entities
			RESPONSIVE,    // Responsive entities
			ADAPTIVE,      // Adaptive entities
			INTELLIGENT,   // Intelligent entities
			AUTONOMOUS,    // Autonomous entities
			LEARNING,      // Learning entities
			EVOLVING,      // Evolving entities
			GROWING,       // Growing entities
			AGING,         // Aging entities
			DECAYING,      // Decaying entities
			HEALING,       // Healing entities
			REGENERATING,  // Regenerating entities
			MUTATING,      // Mutating entities
			EVOLVING,      // Evolving entities
			ADAPTING,      // Adapting entities
			CHANGING,      // Changing entities
			TRANSFORMING,  // Transforming entities
			MORPHING,      // Morphing entities
			SHAPESHIFTING, // Shape-shifting entities
			POLYMORPHING,  // Polymorphing entities
			TRANSMOGRIFYING, // Transmogrifying entities
			METAMORPHOSING, // Metamorphosing entities
			TRANSSUBSTANTIATING, // Transsubstantiating entities
			TRANSMUTING,   // Transmuting entities
			TRANSCENDING,  // Transcending entities
			ASCENDING,     // Ascending entities
			DESCENDING,    // Descending entities
			RISING,        // Rising entities
			FALLING,       // Falling entities
			LEVITATING,    // Levitating entities
			FLYING,        // Flying entities
			GLIDING,       // Gliding entities
			PARACHUTING,   // Parachuting entities
			SKYDIVING,     // Skydiving entities
			BALLOONING,    // Ballooning entities
			AIRSHIPPING,   // Airshipping entities
			SPACESHIPPING, // Spaceshipping entities
			TIMETRAVELLING, // Time-travelling entities
			DIMENSIONTRAVELLING, // Dimension-travelling entities
			REALITYTRAVELLING, // Reality-travelling entities
			UNIVERSETRAVELLING, // Universe-travelling entities
			MULTIVERSETRAVELLING, // Multiverse-travelling entities
			OMNIVERSETRAVELLING, // Omniverse-travelling entities
			CUSTOM          // Custom entity type
		};

		/**
		 * @brief Entity flags for special behaviors
		 */
		enum class Flag : uint32_t
		{
			NONE                    = 0,
			PERSISTENT             = 1 << 0,    // Entity persists across sessions
			NETWORKED              = 1 << 1,    // Entity is networked
			REPLICATED             = 1 << 2,    // Entity is replicated across clients
			LOCAL                  = 1 << 3,    // Entity is local only
			STATIC                 = 1 << 4,    // Entity doesn't move
			DYNAMIC                = 1 << 5,    // Entity can move
			COLLIDABLE             = 1 << 6,    // Entity can collide
			PHYSICS                = 1 << 7,    // Entity uses physics
			RENDERABLE             = 1 << 8,    // Entity can be rendered
			SHADOW_CASTER          = 1 << 9,    // Entity casts shadows
			SHADOW_RECEIVER        = 1 << 10,   // Entity receives shadows
			LIGHT_SOURCE           = 1 << 11,   // Entity emits light
			AUDIO_SOURCE           = 1 << 12,   // Entity emits sound
			PARTICLE_EMITTER       = 1 << 13,   // Entity emits particles
			ANIMATED               = 1 << 14,   // Entity is animated
			SKELETAL               = 1 << 15,   // Entity has skeleton
			RAGDOLL                = 1 << 16,   // Entity uses ragdoll
			VEGETATION             = 1 << 17,   // Entity is vegetation
			FLUID                  = 1 << 18,   // Entity is fluid
			GAS                    = 1 << 19,   // Entity is gas
			PLASMA                 = 1 << 20,   // Entity is plasma
			ENERGY                 = 1 << 21,   // Entity is energy
			FIELD                  = 1 << 22,   // Entity is field
			WAVE                   = 1 << 23,   // Entity is wave
			PULSE                  = 1 << 24,   // Entity is pulse
			BEAM                   = 1 << 25,   // Entity is beam
			RAY                    = 1 << 26,   // Entity is ray
			LASER                  = 1 << 27,   // Entity is laser
			SHIELD                 = 1 << 28,   // Entity is shield
			FORCE_FIELD            = 1 << 29,   // Entity is force field
			BARRIER                = 1 << 30,   // Entity is barrier
			WALL                   = 1 << 31,   // Entity is wall
		};

		/**
		 * @brief Entity priority levels
		 */
		enum class Priority
		{
			VERY_LOW    = 0,
			LOW         = 1,
			NORMAL      = 2,
			HIGH        = 3,
			CRITICAL    = 4,
			SYSTEM      = 5
		};

	private:
		EntityID m_id;
		EntitySignature m_signature;
		State m_state;
		Type m_type;
		Priority m_priority;
		uint32_t m_flags;
		std::string m_name;
		std::string m_tag;
		std::vector<std::shared_ptr<Component>> m_components;
		std::unordered_map<std::string, std::shared_ptr<Component>> m_componentMap;
		EntityManager* m_manager;
		std::atomic<bool> m_destroyed;
		std::mutex m_componentMutex;

		// Performance tracking
		uint64_t m_creationTime;
		uint64_t m_lastUpdateTime;
		uint32_t m_updateCount;
		float m_updateTime;

	public:
		/**
		 * @brief Constructor
		 */
		Entity(EntityID id, EntityManager* manager = nullptr);

		/**
		 * @brief Destructor
		 */
		~Entity();

		/**
		 * @brief Get entity ID
		 */
		EntityID GetID() const { return m_id; }

		/**
		 * @brief Get entity signature
		 */
		EntitySignature GetSignature() const { return m_signature; }

		/**
		 * @brief Get entity state
		 */
		State GetState() const { return m_state; }

		/**
		 * @brief Set entity state
		 */
		void SetState(State state) { m_state = state; }

		/**
		 * @brief Get entity type
		 */
		Type GetType() const { return m_type; }

		/**
		 * @brief Set entity type
		 */
		void SetType(Type type) { m_type = type; }

		/**
		 * @brief Get entity priority
		 */
		Priority GetPriority() const { return m_priority; }

		/**
		 * @brief Set entity priority
		 */
		void SetPriority(Priority priority) { m_priority = priority; }

		/**
		 * @brief Get entity flags
		 */
		uint32_t GetFlags() const { return m_flags; }

		/**
		 * @brief Set entity flags
		 */
		void SetFlags(uint32_t flags) { m_flags = flags; }

		/**
		 * @brief Check if entity has flag
		 */
		bool HasFlag(Flag flag) const { return (m_flags & static_cast<uint32_t>(flag)) != 0; }

		/**
		 * @brief Add flag to entity
		 */
		void AddFlag(Flag flag) { m_flags |= static_cast<uint32_t>(flag); }

		/**
		 * @brief Remove flag from entity
		 */
		void RemoveFlag(Flag flag) { m_flags &= ~static_cast<uint32_t>(flag); }

		/**
		 * @brief Get entity name
		 */
		const std::string& GetName() const { return m_name; }

		/**
		 * @brief Set entity name
		 */
		void SetName(const std::string& name) { m_name = name; }

		/**
		 * @brief Get entity tag
		 */
		const std::string& GetTag() const { return m_tag; }

		/**
		 * @brief Set entity tag
		 */
		void SetTag(const std::string& tag) { m_tag = tag; }

		/**
		 * @brief Check if entity is active
		 */
		bool IsActive() const { return m_state == State::ACTIVE; }

		/**
		 * @brief Check if entity is destroyed
		 */
		bool IsDestroyed() const { return m_destroyed.load(); }

		/**
		 * @brief Mark entity for destruction
		 */
		void Destroy();

		/**
		 * @brief Update entity signature
		 */
		void UpdateSignature();

		/**
		 * @brief Add component to entity
		 */
		template<typename T, typename... Args>
		std::shared_ptr<T> AddComponent(Args&&... args);

		/**
		 * @brief Remove component from entity
		 */
		template<typename T>
		void RemoveComponent();

		/**
		 * @brief Get component from entity
		 */
		template<typename T>
		std::shared_ptr<T> GetComponent();

		/**
		 * @brief Check if entity has component
		 */
		template<typename T>
		bool HasComponent() const;

		/**
		 * @brief Get all components
		 */
		const std::vector<std::shared_ptr<Component>>& GetComponents() const { return m_components; }

		/**
		 * @brief Get component count
		 */
		size_t GetComponentCount() const { return m_components.size(); }

		/**
		 * @brief Initialize entity
		 */
		void Initialize();

		/**
		 * @brief Update entity
		 */
		void Update(float deltaTime);

		/**
		 * @brief Render entity
		 */
		void Render();

		/**
		 * @brief Serialize entity to data
		 */
		std::vector<uint8_t> Serialize() const;

		/**
		 * @brief Deserialize entity from data
		 */
		void Deserialize(const std::vector<uint8_t>& data);

		/**
		 * @brief Get memory usage
		 */
		size_t GetMemoryUsage() const;

		/**
		 * @brief Get creation time
		 */
		uint64_t GetCreationTime() const { return m_creationTime; }

		/**
		 * @brief Get last update time
		 */
		uint64_t GetLastUpdateTime() const { return m_lastUpdateTime; }

		/**
		 * @brief Get update count
		 */
		uint32_t GetUpdateCount() const { return m_updateCount; }

		/**
		 * @brief Get average update time
		 */
		float GetAverageUpdateTime() const { return m_updateTime; }

		/**
		 * @brief Clone entity
		 */
		std::shared_ptr<Entity> Clone() const;

		/**
		 * @brief Compare entities
		 */
		bool operator==(const Entity& other) const { return m_id == other.m_id; }
		bool operator!=(const Entity& other) const { return m_id != other.m_id; }
		bool operator<(const Entity& other) const { return m_id < other.m_id; }
		bool operator>(const Entity& other) const { return m_id > other.m_id; }

	private:
		/**
		 * @brief Add component implementation
		 */
		void AddComponentInternal(std::shared_ptr<Component> component);

		/**
		 * @brief Remove component implementation
		 */
		void RemoveComponentInternal(const std::string& componentType);

		/**
		 * @brief Get component implementation
		 */
		std::shared_ptr<Component> GetComponentInternal(const std::string& componentType) const;
	};

	// Template implementations
	template<typename T, typename... Args>
	std::shared_ptr<T> Entity::AddComponent(Args&&... args)
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		auto component = std::make_shared<T>(std::forward<Args>(args)...);
		component->SetOwner(this);
		AddComponentInternal(component);

		return component;
	}

	template<typename T>
	void Entity::RemoveComponent()
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		RemoveComponentInternal(typeid(T).name());
	}

	template<typename T>
	std::shared_ptr<T> Entity::GetComponent()
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		auto component = std::dynamic_pointer_cast<T>(GetComponentInternal(typeid(T).name()));
		return component;
	}

	template<typename T>
	bool Entity::HasComponent() const
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		std::lock_guard<std::mutex> lock(m_componentMutex);
		return m_componentMap.find(typeid(T).name()) != m_componentMap.end();
	}

} // namespace VoxelCraft

// Bitwise operators for Entity::Flag
inline VoxelCraft::Entity::Flag operator|(VoxelCraft::Entity::Flag a, VoxelCraft::Entity::Flag b)
{
	return static_cast<VoxelCraft::Entity::Flag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline VoxelCraft::Entity::Flag operator&(VoxelCraft::Entity::Flag a, VoxelCraft::Entity::Flag b)
{
	return static_cast<VoxelCraft::Entity::Flag>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline VoxelCraft::Entity::Flag operator^(VoxelCraft::Entity::Flag a, VoxelCraft::Entity::Flag b)
{
	return static_cast<VoxelCraft::Entity::Flag>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}

inline VoxelCraft::Entity::Flag operator~(VoxelCraft::Entity::Flag a)
{
	return static_cast<VoxelCraft::Entity::Flag>(~static_cast<uint32_t>(a));
}

inline VoxelCraft::Entity::Flag& operator|=(VoxelCraft::Entity::Flag& a, VoxelCraft::Entity::Flag b)
{
	a = a | b;
	return a;
}

inline VoxelCraft::Entity::Flag& operator&=(VoxelCraft::Entity::Flag& a, VoxelCraft::Entity::Flag b)
{
	a = a & b;
	return a;
}

inline VoxelCraft::Entity::Flag& operator^=(VoxelCraft::Entity::Flag& a, VoxelCraft::Entity::Flag b)
{
	a = a ^ b;
	return a;
}