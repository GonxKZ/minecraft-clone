#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <cstdint>
#include "core/Logger.hpp"

namespace VoxelCraft {

	class Entity;

	/**
	 * @brief Unique identifier for component types
	 */
	using ComponentID = uint32_t;

	/**
	 * @brief Invalid component ID constant
	 */
	constexpr ComponentID INVALID_COMPONENT_ID = 0;

	/**
	 * @brief Component type enumeration for categorization
	 */
	enum class ComponentType
	{
		TRANSFORM,      // Position, rotation, scale
		RENDER,         // Rendering properties
		PHYSICS,        // Physics properties
		COLLISION,      // Collision detection
		ANIMATION,      // Animation controller
		AUDIO,          // Audio properties
		LIGHT,          // Light properties
		PARTICLE,       // Particle system
		AI,            // Artificial intelligence
		INPUT,         // Input handling
		CAMERA,        // Camera properties
		NETWORK,       // Network synchronization
		SAVE,          // Save/load properties
		SCRIPT,        // Scripting properties
		EFFECT,        // Visual effects
		WEAPON,        // Weapon properties
		ARMOR,         // Armor properties
		INVENTORY,     // Inventory management
		CRAFTING,      // Crafting properties
		BUILDING,      // Building properties
		TERRAIN,       // Terrain interaction
		WATER,         // Water interaction
		FIRE,          // Fire properties
		EXPLOSION,     // Explosion properties
		PROJECTILE,    // Projectile properties
		VEHICLE,       // Vehicle properties
		MOUNT,         // Mount properties
		PET,           // Pet properties
		NPC,           // NPC properties
		MOB,           // Mob properties
		PLAYER,        // Player properties
		HEALTH,        // Health system
		STAMINA,       // Stamina system
		MANA,          // Mana system
		ENERGY,        // Energy system
		HUNGER,        // Hunger system
		THIRST,        // Thirst system
		TEMPERATURE,   // Temperature system
		RADIATION,     // Radiation system
		POISON,        // Poison system
		DISEASE,       // Disease system
		CURSE,         // Curse system
		BLESSING,      // Blessing system
		BUFF,          // Buff system
		DEBUFF,        // Debuff system
		STATUS,        // Status effects
		SKILL,         // Skill system
		ABILITY,       // Ability system
		TALENT,        // Talent system
		PERSONALITY,   // Personality system
		RELATIONSHIP,  // Relationship system
		FACTION,       // Faction system
		REPUTATION,    // Reputation system
		QUEST,         // Quest system
		ACHIEVEMENT,   // Achievement system
		STATISTICS,    // Statistics system
		PROGRESSION,   // Progression system
		LEVEL,         // Level system
		EXPERIENCE,    // Experience system
		CURRENCY,      // Currency system
		TRADING,       // Trading system
		MARKET,        // Market system
		ECONOMY,       // Economy system
		GUILD,         // Guild system
		PARTY,         // Party system
		TEAM,          // Team system
		CLAN,          // Clan system
		ALLIANCE,      // Alliance system
		WAR,           // War system
		PEACE,         // Peace system
		DIPLOMACY,     // Diplomacy system
		TRADE,         // Trade system
		RESOURCE,      // Resource system
		PRODUCTION,    // Production system
		CONSTRUCTION,  // Construction system
		DEMOLITION,    // Demolition system
		REPAIR,        // Repair system
		MAINTENANCE,   // Maintenance system
		UPGRADE,       // Upgrade system
		ENHANCEMENT,   // Enhancement system
		ENCHANTMENT,   // Enchantment system
		RUNE,          // Rune system
		GEM,           // Gem system
		SOCKET,        // Socket system
		ATTACHMENT,    // Attachment system
		MODIFICATION,  // Modification system
		CUSTOMIZATION, // Customization system
		APPEARANCE,    // Appearance system
		COLOR,         // Color system
		TEXTURE,       // Texture system
		MATERIAL,      // Material system
		SHADER,        // Shader system
		EFFECT,        // Effect system
		PARTICLE,      // Particle system
		DECAL,         // Decal system
		OVERLAY,       // Overlay system
		SHADOW,        // Shadow system
		REFLECTION,    // Reflection system
		REFRACTION,    // Refraction system
		TRANSPARENCY,  // Transparency system
		OPACITY,       // Opacity system
		BLENDING,      // Blending system
		FOG,           // Fog system
		ATMOSPHERE,    // Atmosphere system
		WEATHER,       // Weather system
		CLIMATE,       // Climate system
		SEASON,        // Season system
		TIME,          // Time system
		DAY_NIGHT,     // Day/night system
		LIGHTING,      // Lighting system
		SHADOW,        // Shadow system
		AMBIENT,       // Ambient system
		DIRECTIONAL,   // Directional system
		POINT,         // Point system
		SPOT,          // Spot system
		VOLUMETRIC,    // Volumetric system
		LENS_FLARE,    // Lens flare system
		GLOW,          // Glow system
		BLOOM,         // Bloom system
		DOF,           // Depth of field system
		MOTION_BLUR,   // Motion blur system
		SSAO,          // SSAO system
		SSR,           // SSR system
		RAY_TRACING,   // Ray tracing system
		REFLECTION,    // Reflection system
		REFRACTION,    // Refraction system
		DIFFRACTION,   // Diffraction system
		INTERFERENCE,  // Interference system
		POLARIZATION,  // Polarization system
		DISPERSION,    // Dispersion system
		ABSORPTION,    // Absorption system
		EMISSION,      // Emission system
		SCATTERING,    // Scattering system
		TRANSMISSION,  // Transmission system
		REFLECTION,    // Reflection system
		REFRACTION,    // Refraction system
		DIFFRACTION,   // Diffraction system
		INTERFERENCE,  // Interference system
		POLARIZATION,  // Polarization system
		DISPERSION,    // Dispersion system
		ABSORPTION,    // Absorption system
		EMISSION,      // Emission system
		SCATTERING,    // Scattering system
		TRANSMISSION,  // Transmission system
		ABSORPTION,    // Absorption system
		EMISSION,      // Emission system
		SCATTERING,    // Scattering system
		TRANSMISSION,  // Transmission system
		CUSTOM         // Custom component type
	};

	/**
	 * @brief Component state enumeration
	 */
	enum class ComponentState
	{
		ACTIVE,       // Component is active
		INACTIVE,     // Component is inactive
		PENDING,      // Component is pending
		DESTROYED     // Component is destroyed
	};

	/**
	 * @brief Component update frequency
	 */
	enum class UpdateFrequency
	{
		FIXED,        // Fixed timestep updates
		VARIABLE,     // Variable timestep updates
		RENDER,       // Render frequency updates
		NEVER         // No automatic updates
	};

	/**
	 * @brief Base Component class for the ECS architecture
	 *
	 * Components contain data and behavior for entities. They can be
	 * attached to entities to provide specific functionality.
	 */
	class Component
	{
	public:
		/**
		 * @brief Component properties structure
		 */
		struct Properties
		{
			std::string name;
			std::string description;
			ComponentType type;
			UpdateFrequency updateFrequency;
			bool isUnique;          // Only one instance per entity
			bool isPersistent;      // Persists across sessions
			bool isNetworked;       // Synchronized over network
			bool isSerializable;    // Can be serialized
			uint32_t version;       // Component version
			std::vector<std::string> dependencies; // Required components
		};

	protected:
		ComponentID m_id;
		Properties m_properties;
		ComponentState m_state;
		Entity* m_owner;
		std::atomic<bool> m_enabled;
		std::atomic<bool> m_destroyed;

		// Performance tracking
		uint64_t m_creationTime;
		uint64_t m_lastUpdateTime;
		uint32_t m_updateCount;
		float m_updateTime;

		// Memory management
		size_t m_memoryUsage;

		// Thread safety
		std::mutex m_updateMutex;

	public:
		/**
		 * @brief Constructor
		 */
		Component(const Properties& properties);

		/**
		 * @brief Virtual destructor
		 */
		virtual ~Component() = default;

		/**
		 * @brief Get component ID
		 */
		ComponentID GetID() const { return m_id; }

		/**
		 * @brief Get component properties
		 */
		const Properties& GetProperties() const { return m_properties; }

		/**
		 * @brief Get component state
		 */
		ComponentState GetState() const { return m_state; }

		/**
		 * @brief Set component state
		 */
		void SetState(ComponentState state) { m_state = state; }

		/**
		 * @brief Get owner entity
		 */
		Entity* GetOwner() const { return m_owner; }

		/**
		 * @brief Set owner entity
		 */
		void SetOwner(Entity* owner) { m_owner = owner; }

		/**
		 * @brief Check if component is enabled
		 */
		bool IsEnabled() const { return m_enabled.load(); }

		/**
		 * @brief Set component enabled state
		 */
		void SetEnabled(bool enabled) { m_enabled.store(enabled); }

		/**
		 * @brief Enable component
		 */
		void Enable() { SetEnabled(true); }

		/**
		 * @brief Disable component
		 */
		void Disable() { SetEnabled(false); }

		/**
		 * @brief Toggle component enabled state
		 */
		void Toggle() { SetEnabled(!IsEnabled()); }

		/**
		 * @brief Check if component is destroyed
		 */
		bool IsDestroyed() const { return m_destroyed.load(); }

		/**
		 * @brief Mark component for destruction
		 */
		void Destroy() { m_destroyed.store(true); }

		/**
		 * @brief Initialize component
		 */
		virtual void Initialize() {}

		/**
		 * @brief Update component with fixed timestep
		 */
		virtual void Update(float deltaTime) {}

		/**
		 * @brief Update component with variable timestep
		 */
		virtual void UpdateVariable(float deltaTime) {}

		/**
		 * @brief Update component for rendering
		 */
		virtual void UpdateRender() {}

		/**
		 * @brief Render component
		 */
		virtual void Render() {}

		/**
		 * @brief Handle component events
		 */
		virtual void OnEvent(const std::string& eventType, void* eventData) {}

		/**
		 * @brief Serialize component to data
		 */
		virtual std::vector<uint8_t> Serialize() const;

		/**
		 * @brief Deserialize component from data
		 */
		virtual void Deserialize(const std::vector<uint8_t>& data);

		/**
		 * @brief Get component type name
		 */
		virtual std::string GetTypeName() const = 0;

		/**
		 * @brief Get component type ID
		 */
		virtual ComponentID GetTypeID() const = 0;

		/**
		 * @brief Clone component
		 */
		virtual std::shared_ptr<Component> Clone() const = 0;

		/**
		 * @brief Get memory usage
		 */
		virtual size_t GetMemoryUsage() const { return m_memoryUsage; }

		/**
		 * @brief Validate component
		 */
		virtual bool Validate() const { return true; }

		/**
		 * @brief Reset component to default state
		 */
		virtual void Reset() {}

		/**
		 * @brief Get component dependencies
		 */
		virtual std::vector<std::string> GetDependencies() const { return m_properties.dependencies; }

		/**
		 * @brief Check if component has dependency
		 */
		virtual bool HasDependency(const std::string& dependency) const;

		/**
		 * @brief Get component version
		 */
		uint32_t GetVersion() const { return m_properties.version; }

		/**
		 * @brief Check if component is unique
		 */
		bool IsUnique() const { return m_properties.isUnique; }

		/**
		 * @brief Check if component is persistent
		 */
		bool IsPersistent() const { return m_properties.isPersistent; }

		/**
		 * @brief Check if component is networked
		 */
		bool IsNetworked() const { return m_properties.isNetworked; }

		/**
		 * @brief Check if component is serializable
		 */
		bool IsSerializable() const { return m_properties.isSerializable; }

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
		 * @brief Update performance statistics
		 */
		void UpdatePerformanceStats(float deltaTime);

	protected:
		/**
		 * @brief Set memory usage
		 */
		void SetMemoryUsage(size_t usage) { m_memoryUsage = usage; }

		/**
		 * @brief Update memory usage automatically
		 */
		void UpdateMemoryUsage();

		/**
		 * @brief Register component type
		 */
		static ComponentID RegisterComponentType(const std::string& typeName);

		/**
		 * @brief Get component type ID by name
		 */
		static ComponentID GetComponentTypeID(const std::string& typeName);
	};

	/**
	 * @brief Template helper for component type registration
	 */
	template<typename T>
	class ComponentRegistry
	{
	public:
		static ComponentID GetTypeID()
		{
			static ComponentID typeID = Component::RegisterComponentType(typeid(T).name());
			return typeID;
		}

		static std::string GetTypeName()
		{
			return typeid(T).name();
		}
	};

	/**
	 * @brief Macro for easy component definition
	 */
#define VOXEL_COMPONENT(ComponentClass, ComponentProps) \
	class ComponentClass : public VoxelCraft::Component { \
	public: \
		ComponentClass() : VoxelCraft::Component(ComponentProps) {} \
		ComponentClass(const ComponentProps& props) : VoxelCraft::Component(props) {} \
		\
		std::string GetTypeName() const override { return #ComponentClass; } \
		VoxelCraft::ComponentID GetTypeID() const override { return ComponentRegistry<ComponentClass>::GetTypeID(); } \
		std::shared_ptr<VoxelCraft::Component> Clone() const override { return std::make_shared<ComponentClass>(*this); }

#define END_VOXEL_COMPONENT() };

} // namespace VoxelCraft