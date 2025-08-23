#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <chrono>
#include "core/Logger.hpp"
#include "Entity.hpp"

namespace VoxelCraft {

	class EntityManager;

	/**
	 * @brief Unique identifier for system types
	 */
	using SystemID = uint32_t;

	/**
	 * @brief Invalid system ID constant
	 */
	constexpr SystemID INVALID_SYSTEM_ID = 0;

	/**
	 * @brief System type enumeration for categorization
	 */
	enum class SystemType
	{
		RENDER,         // Rendering system
		PHYSICS,        // Physics simulation
		INPUT,          // Input handling
		AI,            // Artificial intelligence
		ANIMATION,      // Animation system
		AUDIO,          // Audio system
		NETWORK,        // Network system
		SAVE,          // Save/load system
		SCRIPT,         // Scripting system
		PARTICLE,       // Particle system
		LIGHTING,       // Lighting system
		SHADOW,         // Shadow system
		COLLISION,      // Collision detection
		TRANSFORM,      // Transform system
		CAMERA,         // Camera system
		GUI,            // GUI system
		TERRAIN,        // Terrain system
		WATER,          // Water system
		WEATHER,        // Weather system
		VEGETATION,     // Vegetation system
		FLUID,          // Fluid dynamics
		COMBAT,         // Combat system
		INVENTORY,      // Inventory system
		CRAFTING,       // Crafting system
		BUILDING,       // Building system
		ECONOMY,        // Economy system
		QUEST,          // Quest system
		DIALOGUE,       // Dialogue system
		CUTSCENE,       // Cutscene system
		CINEMATIC,      // Cinematic system
		TRIGGER,        // Trigger system
		EVENT,          // Event system
		PROFILE,        // Profiling system
		DEBUG,          // Debug system
		EDITOR,         // Editor system
		TOOL,           // Tool system
		UTILITY,        // Utility system
		CUSTOM          // Custom system type
	};

	/**
	 * @brief System state enumeration
	 */
	enum class SystemState
	{
		CREATED,      // System created but not initialized
		INITIALIZED,  // System initialized
		ACTIVE,       // System active and running
		INACTIVE,     // System inactive
		PAUSED,       // System paused
		ERROR,        // System in error state
		DESTROYED     // System destroyed
	};

	/**
	 * @brief System update priority
	 */
	enum class SystemPriority
	{
		VERY_LOW    = -100,
		LOW         = -50,
		NORMAL      = 0,
		HIGH        = 50,
		CRITICAL    = 100,
		SYSTEM      = 1000
	};

	/**
	 * @brief System update frequency
	 */
	enum class SystemUpdateFrequency
	{
		FIXED,        // Fixed timestep updates
		VARIABLE,     // Variable timestep updates
		RENDER,       // Render frequency updates
		NEVER         // No automatic updates
	};

	/**
	 * @brief System properties structure
	 */
	struct SystemProperties
	{
		std::string name;
		std::string description;
		SystemType type;
		SystemPriority priority;
		SystemUpdateFrequency updateFrequency;
		bool isEnabled;
		bool isParallel;         // Can run in parallel
		bool isThreadSafe;       // Thread-safe operations
		bool requiresRendering;  // Requires rendering context
		bool requiresPhysics;    // Requires physics context
		bool requiresAudio;      // Requires audio context
		bool requiresNetwork;    // Requires network context
		std::vector<std::string> dependencies; // System dependencies
		std::vector<std::string> conflicts;    // System conflicts
		float updateInterval;    // Update interval in seconds
	};

	/**
	 * @brief Base System class for the ECS architecture
	 *
	 * Systems contain logic that operates on entities with specific
	 * component combinations. They implement the game logic.
	 */
	class System
	{
	public:
		/**
		 * @brief System statistics structure
		 */
		struct Statistics
		{
			uint64_t totalUpdates;
			uint64_t totalEntitiesProcessed;
			float averageUpdateTime;
			float maxUpdateTime;
			float minUpdateTime;
			float totalUpdateTime;
			uint32_t memoryUsage;
			uint32_t peakMemoryUsage;
		};

	protected:
		SystemID m_id;
		SystemProperties m_properties;
		SystemState m_state;
		EntityManager* m_entityManager;
		std::atomic<bool> m_enabled;
		std::atomic<bool> m_paused;
		std::atomic<bool> m_destroyed;

		// Entity tracking
		std::unordered_set<EntityID> m_trackedEntities;
		EntitySignature m_signature; // Required component signature
		std::mutex m_entityMutex;

		// Performance tracking
		Statistics m_statistics;
		std::chrono::high_resolution_clock::time_point m_lastUpdateTime;
		float m_accumulatedTime;

		// Threading
		std::mutex m_updateMutex;

	public:
		/**
		 * @brief Constructor
		 */
		System(const SystemProperties& properties);

		/**
		 * @brief Virtual destructor
		 */
		virtual ~System() = default;

		/**
		 * @brief Get system ID
		 */
		SystemID GetID() const { return m_id; }

		/**
		 * @brief Get system properties
		 */
		const SystemProperties& GetProperties() const { return m_properties; }

		/**
		 * @brief Get system state
		 */
		SystemState GetState() const { return m_state; }

		/**
		 * @brief Set system state
		 */
		void SetState(SystemState state) { m_state = state; }

		/**
		 * @brief Get entity manager
		 */
		EntityManager* GetEntityManager() const { return m_entityManager; }

		/**
		 * @brief Set entity manager
		 */
		void SetEntityManager(EntityManager* manager) { m_entityManager = manager; }

		/**
		 * @brief Check if system is enabled
		 */
		bool IsEnabled() const { return m_enabled.load(); }

		/**
		 * @brief Set system enabled state
		 */
		void SetEnabled(bool enabled) { m_enabled.store(enabled); }

		/**
		 * @brief Enable system
		 */
		void Enable() { SetEnabled(true); }

		/**
		 * @brief Disable system
		 */
		void Disable() { SetEnabled(false); }

		/**
		 * @brief Toggle system enabled state
		 */
		void Toggle() { SetEnabled(!IsEnabled()); }

		/**
		 * @brief Check if system is paused
		 */
		bool IsPaused() const { return m_paused.load(); }

		/**
		 * @brief Set system paused state
		 */
		void SetPaused(bool paused) { m_paused.store(paused); }

		/**
		 * @brief Pause system
		 */
		void Pause() { SetPaused(true); }

		/**
		 * @brief Resume system
		 */
		void Resume() { SetPaused(false); }

		/**
		 * @brief Toggle system paused state
		 */
		void TogglePaused() { SetPaused(!IsPaused()); }

		/**
		 * @brief Check if system is destroyed
		 */
		bool IsDestroyed() const { return m_destroyed.load(); }

		/**
		 * @brief Mark system for destruction
		 */
		void Destroy() { m_destroyed.store(true); }

		/**
		 * @brief Initialize system
		 */
		virtual bool Initialize();

		/**
		 * @brief Shutdown system
		 */
		virtual void Shutdown();

		/**
		 * @brief Update system with fixed timestep
		 */
		virtual void Update(float deltaTime);

		/**
		 * @brief Update system with variable timestep
		 */
		virtual void UpdateVariable(float deltaTime);

		/**
		 * @brief Update system for rendering
		 */
		virtual void UpdateRender();

		/**
		 * @brief Render system
		 */
		virtual void Render();

		/**
		 * @brief Handle system events
		 */
		virtual void OnEvent(const std::string& eventType, void* eventData);

		/**
		 * @brief Get required component signature
		 */
		virtual EntitySignature GetSignature() const { return m_signature; }

		/**
		 * @brief Set required component signature
		 */
		void SetSignature(EntitySignature signature) { m_signature = signature; }

		/**
		 * @brief Check if entity matches system signature
		 */
		bool MatchesSignature(EntityID entityID, EntitySignature entitySignature) const;

		/**
		 * @brief Add entity to system tracking
		 */
		virtual void AddEntity(EntityID entityID);

		/**
		 * @brief Remove entity from system tracking
		 */
		virtual void RemoveEntity(EntityID entityID);

		/**
		 * @brief Check if system tracks entity
		 */
		bool HasEntity(EntityID entityID) const;

		/**
		 * @brief Get tracked entities count
		 */
		size_t GetTrackedEntityCount() const { return m_trackedEntities.size(); }

		/**
		 * @brief Get tracked entities
		 */
		const std::unordered_set<EntityID>& GetTrackedEntities() const { return m_trackedEntities; }

		/**
		 * @brief Process entity
		 */
		virtual void ProcessEntity(EntityID entityID, float deltaTime);

		/**
		 * @brief Process all tracked entities
		 */
		virtual void ProcessAllEntities(float deltaTime);

		/**
		 * @brief Get system statistics
		 */
		const Statistics& GetStatistics() const { return m_statistics; }

		/**
		 * @brief Reset system statistics
		 */
		void ResetStatistics();

		/**
		 * @brief Get system type name
		 */
		virtual std::string GetTypeName() const = 0;

		/**
		 * @brief Get system type ID
		 */
		virtual SystemID GetTypeID() const = 0;

		/**
		 * @brief Get system dependencies
		 */
		virtual std::vector<std::string> GetDependencies() const { return m_properties.dependencies; }

		/**
		 * @brief Get system conflicts
		 */
		virtual std::vector<std::string> GetConflicts() const { return m_properties.conflicts; }

		/**
		 * @brief Check if system has dependency
		 */
		virtual bool HasDependency(const std::string& dependency) const;

		/**
		 * @brief Check if system has conflict
		 */
		virtual bool HasConflict(const std::string& conflict) const;

		/**
		 * @brief Validate system configuration
		 */
		virtual bool Validate() const { return true; }

		/**
		 * @brief Get memory usage
		 */
		virtual size_t GetMemoryUsage() const;

		/**
		 * @brief Serialize system state
		 */
		virtual std::vector<uint8_t> Serialize() const;

		/**
		 * @brief Deserialize system state
		 */
		virtual void Deserialize(const std::vector<uint8_t>& data);

	protected:
		/**
		 * @brief Update performance statistics
		 */
		void UpdateStatistics(float deltaTime, uint32_t entitiesProcessed);

		/**
		 * @brief Register system type
		 */
		static SystemID RegisterSystemType(const std::string& typeName);

		/**
		 * @brief Get system type ID by name
		 */
		static SystemID GetSystemTypeID(const std::string& typeName);

		/**
		 * @brief Lock entity mutex
		 */
		std::unique_lock<std::mutex> LockEntities() { return std::unique_lock<std::mutex>(m_entityMutex); }

		/**
		 * @brief Lock update mutex
		 */
		std::unique_lock<std::mutex> LockUpdate() { return std::unique_lock<std::mutex>(m_updateMutex); }
	};

	/**
	 * @brief Template helper for system type registration
	 */
	template<typename T>
	class SystemRegistry
	{
	public:
		static SystemID GetTypeID()
		{
			static SystemID typeID = System::RegisterSystemType(typeid(T).name());
			return typeID;
		}

		static std::string GetTypeName()
		{
			return typeid(T).name();
		}
	};

	/**
	 * @brief Macro for easy system definition
	 */
#define VOXEL_SYSTEM(SystemClass, SystemProps) \
	class SystemClass : public VoxelCraft::System { \
	public: \
		SystemClass() : VoxelCraft::System(SystemProps) {} \
		SystemClass(const SystemProps& props) : VoxelCraft::System(props) {} \
		\
		std::string GetTypeName() const override { return #SystemClass; } \
		VoxelCraft::SystemID GetTypeID() const override { return SystemRegistry<SystemClass>::GetTypeID(); }

#define END_VOXEL_SYSTEM() };

	/**
	 * @brief System update context structure
	 */
	struct SystemUpdateContext
	{
		float deltaTime;
		float fixedDeltaTime;
		float variableDeltaTime;
		float renderDeltaTime;
		uint64_t frameCount;
		uint64_t updateCount;
		std::chrono::high_resolution_clock::time_point currentTime;
		void* userData;
	};

	/**
	 * @brief System render context structure
	 */
	struct SystemRenderContext
	{
		void* renderDevice;
		void* renderContext;
		void* camera;
		void* viewport;
		uint32_t width;
		uint32_t height;
		float aspectRatio;
		float fieldOfView;
		float nearPlane;
		float farPlane;
		void* userData;
	};

} // namespace VoxelCraft