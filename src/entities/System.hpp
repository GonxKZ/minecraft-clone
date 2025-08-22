/**
 * @file System.hpp
 * @brief VoxelCraft ECS - System Base Class
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the System base class and system management for the ECS architecture.
 * Systems process entities with specific components and provide game logic.
 */

#ifndef VOXELCRAFT_ENTITIES_SYSTEM_HPP
#define VOXELCRAFT_ENTITIES_SYSTEM_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <chrono>

#include "Entity.hpp"
#include "EntityManager.hpp"

namespace VoxelCraft {

    /**
     * @enum SystemPriority
     * @brief System update priority levels
     */
    enum class SystemPriority {
        Lowest = 0,     ///< Updated last
        Low,            ///< Low priority
        Normal,         ///< Normal priority (default)
        High,           ///< High priority
        Highest,        ///< Updated first
        Critical        ///< Critical systems (immediate updates)
    };

    /**
     * @enum SystemState
     * @brief Current state of a system
     */
    enum class SystemState {
        Created,        ///< System created but not initialized
        Initialized,    ///< System initialized and ready
        Running,        ///< System is running and processing
        Paused,         ///< System is paused
        Error,          ///< System encountered an error
        Destroyed       ///< System is being destroyed
    };

    /**
     * @struct SystemStatistics
     * @brief Performance statistics for systems
     */
    struct SystemStatistics {
        uint64_t totalUpdates;                      ///< Total number of updates
        double totalUpdateTime;                     ///< Total time spent updating
        double averageUpdateTime;                   ///< Average time per update
        double maxUpdateTime;                       ///< Maximum update time
        double minUpdateTime;                       ///< Minimum update time
        uint64_t entitiesProcessed;                 ///< Total entities processed
        double averageEntitiesPerUpdate;            ///< Average entities per update
        uint64_t queriesExecuted;                   ///< Total queries executed
        double averageQueryTime;                    ///< Average query time
        double lastUpdateTime;                      ///< Time of last update
        double updateFrequency;                     ///< Updates per second
    };

    /**
     * @struct SystemDependencies
     * @brief System dependency information
     */
    struct SystemDependencies {
        std::vector<std::string> requiredSystems;   ///< Systems this system depends on
        std::vector<std::string> optionalSystems;   ///< Systems this system may use
        std::vector<std::string> conflictingSystems; ///< Systems this system conflicts with
        bool parallelExecution;                     ///< Can this system run in parallel
        uint32_t maxThreads;                        ///< Maximum threads for parallel execution
    };

    /**
     * @class System
     * @brief Base class for all ECS systems
     *
     * Systems in the ECS architecture are responsible for:
     * - Processing entities with specific components
     * - Implementing game logic and behavior
     * - Managing system-specific data and resources
     * - Providing debugging and profiling information
     *
     * System Lifecycle:
     * 1. Created - System instance created
     * 2. Initialized - System initialized with entity manager
     * 3. Started - System begins processing
     * 4. Updated - System processes entities each frame
     * 5. Stopped - System stops processing
     * 6. Destroyed - System cleanup and destruction
     */
    class System {
    public:
        /**
         * @brief Constructor
         * @param name System name
         * @param priority System priority
         */
        explicit System(const std::string& name, SystemPriority priority = SystemPriority::Normal);

        /**
         * @brief Destructor
         */
        virtual ~System();

        /**
         * @brief Deleted copy constructor
         */
        System(const System&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        System& operator=(const System&) = delete;

        // System identification

        /**
         * @brief Get system name
         * @return System name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Get system description
         * @return System description
         */
        virtual std::string GetDescription() const { return "Base system class"; }

        /**
         * @brief Get system priority
         * @return System priority
         */
        SystemPriority GetPriority() const { return m_priority; }

        // System state

        /**
         * @brief Get current system state
         * @return System state
         */
        SystemState GetState() const { return m_state; }

        /**
         * @brief Check if system is enabled
         * @return true if enabled, false otherwise
         */
        bool IsEnabled() const { return m_enabled; }

        /**
         * @brief Enable or disable system
         * @param enabled Enable state
         */
        void SetEnabled(bool enabled);

        // Lifecycle methods

        /**
         * @brief Initialize system with entity manager
         * @param entityManager Entity manager instance
         * @return true if initialization successful, false otherwise
         */
        virtual bool Initialize(EntityManager* entityManager);

        /**
         * @brief Shutdown system and cleanup resources
         */
        virtual void Shutdown();

        /**
         * @brief Start system processing
         * @return true if started successfully, false otherwise
         */
        virtual bool Start();

        /**
         * @brief Stop system processing
         */
        virtual void Stop();

        /**
         * @brief Pause system processing
         */
        virtual void Pause();

        /**
         * @brief Resume system processing
         */
        virtual void Resume();

        // Update methods

        /**
         * @brief Update system with delta time
         * @param deltaTime Time elapsed since last update
         */
        virtual void Update(double deltaTime);

        /**
         * @brief Late update system
         * @param deltaTime Time elapsed since last update
         */
        virtual void LateUpdate(double deltaTime);

        /**
         * @brief Fixed update system
         * @param fixedDeltaTime Fixed time step
         */
        virtual void FixedUpdate(double fixedDeltaTime);

        // Entity processing

        /**
         * @brief Process single entity
         * @param entity Entity to process
         * @param deltaTime Time elapsed since last update
         */
        virtual void ProcessEntity(Entity* entity, double deltaTime) {}

        /**
         * @brief Check if system should process entity
         * @param entity Entity to check
         * @return true if should process, false otherwise
         */
        virtual bool ShouldProcessEntity(Entity* entity) const { return true; }

        /**
         * @brief Get entities this system processes
         * @return Vector of entities to process
         */
        virtual std::vector<Entity*> GetEntitiesToProcess();

        // System configuration

        /**
         * @brief Get system dependencies
         * @return System dependencies
         */
        virtual SystemDependencies GetDependencies() const;

        /**
         * @brief Get required components for this system
         * @return Vector of required component type names
         */
        virtual std::vector<std::string> GetRequiredComponents() const { return {}; }

        /**
         * @brief Get optional components for this system
         * @return Vector of optional component type names
         */
        virtual std::vector<std::string> GetOptionalComponents() const { return {}; }

        /**
         * @brief Get update frequency
         * @return Updates per second (0 = every frame)
         */
        virtual double GetUpdateFrequency() const { return 0.0; }

        // Statistics and debugging

        /**
         * @brief Get system statistics
         * @return System statistics
         */
        SystemStatistics GetStatistics() const { return m_statistics; }

        /**
         * @brief Reset statistics
         */
        void ResetStatistics();

        /**
         * @brief Get system debug information
         * @return Debug information as key-value pairs
         */
        virtual std::unordered_map<std::string, std::string> GetDebugInfo() const;

        // Event handling

        /**
         * @brief Handle system-specific events
         * @param eventType Event type
         * @param data Event data
         */
        virtual void OnEvent(const std::string& eventType, std::any data) {}

        /**
         * @brief Send event from this system
         * @param eventType Event type
         * @param data Event data
         */
        void SendEvent(const std::string& eventType, std::any data = {});

        // Configuration

        /**
         * @brief Set system property
         * @tparam T Property type
         * @param name Property name
         * @param value Property value
         */
        template<typename T>
        void SetProperty(const std::string& name, const T& value);

        /**
         * @brief Get system property
         * @tparam T Property type
         * @param name Property name
         * @param defaultValue Default value if property doesn't exist
         * @return Property value or default
         */
        template<typename T>
        T GetProperty(const std::string& name, const T& defaultValue = T{}) const;

        /**
         * @brief Check if system has property
         * @param name Property name
         * @return true if has property, false otherwise
         */
        bool HasProperty(const std::string& name) const;

    protected:
        /**
         * @brief Update system statistics
         * @param deltaTime Time elapsed since last update
         * @param entityCount Number of entities processed
         */
        void UpdateStatistics(double deltaTime, size_t entityCount);

        /**
         * @brief Check if system should update this frame
         * @param deltaTime Time elapsed since last update
         * @return true if should update, false otherwise
         */
        bool ShouldUpdate(double deltaTime);

        /**
         * @brief Create entity query for this system
         * @return Entity query for system requirements
         */
        EntityQuery CreateEntityQuery() const;

        std::string m_name;                                   ///< System name
        SystemPriority m_priority;                            ///< System priority
        SystemState m_state;                                  ///< Current system state
        bool m_enabled;                                       ///< System enabled flag

        EntityManager* m_entityManager;                       ///< Entity manager instance
        SystemStatistics m_statistics;                        ///< System statistics
        std::unordered_map<std::string, std::any> m_properties; ///< System properties

        mutable std::mutex m_propertyMutex;                   ///< Property synchronization
        double m_lastUpdateTime;                              ///< Last update time
        double m_updateTimer;                                 ///< Update timer for frequency control

        friend class EntityManager;
    };

    // Template implementations

    template<typename T>
    void System::SetProperty(const std::string& name, const T& value) {
        std::lock_guard<std::mutex> lock(m_propertyMutex);
        m_properties[name] = value;
    }

    template<typename T>
    T System::GetProperty(const std::string& name, const T& defaultValue) const {
        std::lock_guard<std::mutex> lock(m_propertyMutex);
        auto it = m_properties.find(name);
        if (it != m_properties.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    // Example system implementations

    /**
     * @class TransformSystem
     * @brief System that handles entity transformations
     */
    class TransformSystem : public System {
    public:
        /**
         * @brief Constructor
         */
        TransformSystem();

        /**
         * @brief Get system description
         * @return System description
         */
        std::string GetDescription() const override { return "Handles entity position, rotation, and scale"; }

        /**
         * @brief Get required components
         * @return Required components
         */
        std::vector<std::string> GetRequiredComponents() const override;

        /**
         * @brief Process entity
         * @param entity Entity to process
         * @param deltaTime Time elapsed
         */
        void ProcessEntity(Entity* entity, double deltaTime) override;

        /**
         * @brief Get dependencies
         * @return System dependencies
         */
        SystemDependencies GetDependencies() const override;

    private:
        /**
         * @brief Update parent-child relationships
         */
        void UpdateHierarchy();

        /**
         * @brief Update transformation matrices
         */
        void UpdateMatrices();
    };

    /**
     * @class RenderSystem
     * @brief System that handles entity rendering
     */
    class RenderSystem : public System {
    public:
        /**
         * @brief Constructor
         */
        RenderSystem();

        /**
         * @brief Get system description
         * @return System description
         */
        std::string GetDescription() const override { return "Handles entity rendering and visual updates"; }

        /**
         * @brief Get required components
         * @return Required components
         */
        std::vector<std::string> GetRequiredComponents() const override;

        /**
         * @brief Process entity
         * @param entity Entity to process
         * @param deltaTime Time elapsed
         */
        void ProcessEntity(Entity* entity, double deltaTime) override;

        /**
         * @brief Get dependencies
         * @return System dependencies
         */
        SystemDependencies GetDependencies() const override;

        /**
         * @brief Set render distance
         * @param distance Render distance
         */
        void SetRenderDistance(float distance) { m_renderDistance = distance; }

        /**
         * @brief Get render distance
         * @return Render distance
         */
        float GetRenderDistance() const { return m_renderDistance; }

    private:
        /**
         * @brief Frustum culling
         * @param entity Entity to check
         * @return true if visible, false otherwise
         */
        bool IsVisible(Entity* entity) const;

        /**
         * @brief Sort entities by render order
         * @param entities Entities to sort
         */
        void SortRenderOrder(std::vector<Entity*>& entities);

        float m_renderDistance;                              ///< Maximum render distance
        // Add camera reference, render queue, etc.
    };

    /**
     * @class PhysicsSystem
     * @brief System that handles physics simulation
     */
    class PhysicsSystem : public System {
    public:
        /**
         * @brief Constructor
         */
        PhysicsSystem();

        /**
         * @brief Get system description
         * @return System description
         */
        std::string GetDescription() const override { return "Handles physics simulation and collision detection"; }

        /**
         * @brief Get required components
         * @return Required components
         */
        std::vector<std::string> GetRequiredComponents() const override;

        /**
         * @brief Process entity
         * @param entity Entity to process
         * @param deltaTime Time elapsed
         */
        void ProcessEntity(Entity* entity, double deltaTime) override;

        /**
         * @brief Get dependencies
         * @return System dependencies
         */
        SystemDependencies GetDependencies() const override;

        /**
         * @brief Get update frequency
         * @return Updates per second
         */
        double GetUpdateFrequency() const override { return 60.0; } // 60Hz physics

        /**
         * @brief Set gravity
         * @param gravity Gravity vector
         */
        void SetGravity(const glm::vec3& gravity) { m_gravity = gravity; }

        /**
         * @brief Get gravity
         * @return Gravity vector
         */
        const glm::vec3& GetGravity() const { return m_gravity; }

    private:
        /**
         * @brief Apply forces to entities
         */
        void ApplyForces();

        /**
         * @brief Detect collisions
         */
        void DetectCollisions();

        /**
         * @brief Resolve collisions
         */
        void ResolveCollisions();

        /**
         * @brief Integrate motion
         * @param deltaTime Time step
         */
        void IntegrateMotion(double deltaTime);

        glm::vec3 m_gravity;                                 ///< Gravity vector
        // Add physics world, collision detection, etc.
    };

    /**
     * @class AISystem
     * @brief System that handles AI behavior
     */
    class AISystem : public System {
    public:
        /**
         * @brief Constructor
         */
        AISystem();

        /**
         * @brief Get system description
         * @return System description
         */
        std::string GetDescription() const override { return "Handles AI behavior and decision making"; }

        /**
         * @brief Get required components
         * @return Required components
         */
        std::vector<std::string> GetRequiredComponents() const override;

        /**
         * @brief Process entity
         * @param entity Entity to process
         * @param deltaTime Time elapsed
         */
        void ProcessEntity(Entity* entity, double deltaTime) override;

        /**
         * @brief Get dependencies
         * @return System dependencies
         */
        SystemDependencies GetDependencies() const override;

        /**
         * @brief Get update frequency
         * @return Updates per second
         */
        double GetUpdateFrequency() const override { return 20.0; } // 20Hz AI

    private:
        /**
         * @brief Update behavior trees
         */
        void UpdateBehaviorTrees();

        /**
         * @brief Process pathfinding
         */
        void ProcessPathfinding();

        /**
         * @brief Update AI states
         */
        void UpdateAIStates();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_SYSTEM_HPP
