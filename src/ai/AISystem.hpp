/**
 * @file AISystem.hpp
 * @brief VoxelCraft AI System - Main AI Controller
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the AISystem class that coordinates all AI functionality
 * in the VoxelCraft game world including mob behaviors, pathfinding, procedural
 * generation, and village systems.
 */

#ifndef VOXELCRAFT_AI_AI_SYSTEM_HPP
#define VOXELCRAFT_AI_AI_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../entities/System.hpp"
#include "../world/World.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class EntityManager;
    class Mob;
    class BehaviorTree;
    class PathfindingSystem;
    class NavigationSystem;
    class ProceduralGenerator;
    class VillageSystem;
    class SensorySystem;
    class DecisionSystem;
    class LearningSystem;
    class BlackboardSystem;

    /**
     * @enum AIState
     * @brief Global AI system state
     */
    enum class AIState {
        Active,                 ///< AI system is active
        Paused,                 ///< AI system is paused
        Inactive,               ///< AI system is inactive
        Error                   ///< AI system is in error state
    };

    /**
     * @enum AISystemMode
     * @brief AI system operational modes
     */
    enum class AISystemMode {
        Normal,                 ///< Normal AI operation
        Debug,                  ///< Debug mode with extra logging
        Performance,            ///< Performance monitoring mode
        Learning,               ///< Learning and adaptation mode
        Minimal                ///< Minimal AI for low-end systems
    };

    /**
     * @enum AIUpdatePriority
     * @brief Priority levels for AI updates
     */
    enum class AIUpdatePriority {
        Critical,               ///< Critical updates (player threats, etc.)
        High,                   ///< High priority updates
        Medium,                 ///< Medium priority updates
        Low,                    ///< Low priority updates
        Background              ///< Background updates
    };

    /**
     * @struct AISystemConfig
     * @brief AI system configuration settings
     */
    struct AISystemConfig {
        // Basic settings
        AISystemMode mode;                      ///< AI system mode
        float updateInterval;                   ///< AI update interval (seconds)
        int maxConcurrentUpdates;               ///< Maximum concurrent AI updates
        bool enableMultithreading;              ///< Enable multithreaded AI updates

        // Performance settings
        float maxUpdateTime;                    ///< Maximum time per AI update (seconds)
        int maxActiveMobs;                      ///< Maximum active mobs
        float mobUpdateDistance;                ///< Maximum distance for mob updates
        float mobDeactivationDistance;          ///< Distance to deactivate mobs

        // Pathfinding settings
        bool enablePathfinding;                 ///< Enable pathfinding system
        float pathfindingTimeout;               ///< Pathfinding timeout (seconds)
        int maxPathLength;                      ///< Maximum path length
        bool enablePathCaching;                 ///< Enable path caching

        // Procedural generation settings
        bool enableProceduralGeneration;        ///< Enable procedural generation
        float structureGenerationRadius;        ///< Structure generation radius
        int maxStructuresPerChunk;              ///< Maximum structures per chunk
        bool enableVillageGeneration;           ///< Enable village generation

        // Sensory settings
        float sightRange;                       ///< Default sight range
        float hearingRange;                     ///< Default hearing range
        float smellRange;                       ///< Default smell range
        int sensoryUpdateFrequency;             ///< Sensory update frequency

        // Learning settings
        bool enableLearning;                    ///< Enable learning system
        float learningRate;                     ///< Learning rate for AI adaptation
        bool enableBehaviorRecording;           ///< Enable behavior recording
        std::string learningDataPath;           ///< Path for learning data

        // Debug settings
        bool enableDebugDrawing;                ///< Enable AI debug drawing
        bool showBehaviorTrees;                 ///< Show behavior tree visualization
        bool showPathfinding;                   ///< Show pathfinding visualization
        bool showSensoryData;                   ///< Show sensory data visualization
        bool enablePerformanceLogging;          ///< Enable performance logging
    };

    /**
     * @struct AISystemMetrics
     * @brief Performance metrics for the AI system
     */
    struct AISystemMetrics {
        // Performance metrics
        uint64_t updateCount;                   ///< Number of AI updates performed
        double totalUpdateTime;                 ///< Total update time (ms)
        double averageUpdateTime;               ///< Average update time (ms)
        double maxUpdateTime;                   ///< Maximum update time (ms)

        // Mob metrics
        uint32_t activeMobs;                    ///< Currently active mobs
        uint32_t totalMobs;                     ///< Total mobs in system
        uint64_t mobUpdates;                    ///< Total mob updates
        uint64_t mobStateChanges;               ///< Mob state changes

        // Pathfinding metrics
        uint64_t pathRequests;                  ///< Pathfinding requests
        uint64_t successfulPaths;               ///< Successful pathfinds
        uint64_t failedPaths;                   ///< Failed pathfinds
        double averagePathfindingTime;          ///< Average pathfinding time

        // Procedural generation metrics
        uint64_t structuresGenerated;           ///< Structures generated
        uint64_t villagesGenerated;             ///< Villages generated
        uint64_t chunksProcessed;               ///< Chunks processed for generation
        double generationTime;                  ///< Total generation time

        // Sensory metrics
        uint64_t sensoryUpdates;                ///< Sensory system updates
        uint64_t stimuliDetected;               ///< Stimuli detected
        uint64_t alertsTriggered;               ///< AI alerts triggered
        double sensoryProcessingTime;           ///< Sensory processing time

        // Learning metrics
        uint64_t learningEvents;                ///< Learning events processed
        uint64_t behaviorAdaptations;           ///< Behavior adaptations made
        float averageLearningTime;              ///< Average learning time
        bool learningSystemActive;              ///< Learning system active

        // Memory metrics
        size_t memoryUsage;                     ///< Current memory usage
        size_t peakMemoryUsage;                 ///< Peak memory usage
        uint32_t allocations;                   ///< Memory allocations
        uint32_t deallocations;                 ///< Memory deallocations

        // Error metrics
        uint64_t systemErrors;                  ///< System errors
        uint64_t mobErrors;                     ///< Mob-related errors
        uint64_t pathfindingErrors;             ///< Pathfinding errors
        uint64_t generationErrors;              ///< Generation errors
    };

    /**
     * @struct AITask
     * @brief Task for AI system processing
     */
    struct AITask {
        uint32_t taskId;                        ///< Unique task ID
        AIUpdatePriority priority;              ///< Task priority
        std::function<void()> function;         ///< Task function
        double scheduledTime;                   ///< Scheduled execution time
        std::string description;                ///< Task description
        bool isRecurring;                       ///< Whether task is recurring
        double interval;                        ///< Recurring task interval
        bool isCompleted;                       ///< Task completion flag
    };

    /**
     * @struct AILogEntry
     * @brief AI system log entry
     */
    struct AILogEntry {
        double timestamp;                       ///< Log timestamp
        std::string level;                      ///< Log level (INFO, WARNING, ERROR)
        std::string component;                  ///< AI component name
        std::string message;                    ///< Log message
        std::unordered_map<std::string, std::any> data; ///< Additional log data
    };

    /**
     * @class AISystem
     * @brief Main AI System Controller
     *
     * The AISystem class is the central coordinator for all AI functionality in VoxelCraft.
     * It manages mob behaviors, pathfinding, procedural generation, village systems,
     * and provides a unified interface for AI operations across the game world.
     *
     * Key Features:
     * - Comprehensive AI system management and coordination
     * - Multi-threaded AI updates with priority queuing
     * - Performance monitoring and optimization
     * - Modular subsystem integration (pathfinding, navigation, procedural generation)
     * - Learning and adaptation capabilities
     * - Debug visualization and profiling
     * - Event-driven architecture for AI interactions
     *
     * The system is designed to handle hundreds to thousands of AI entities efficiently
     * while maintaining real-time performance and providing rich, emergent behaviors.
     */
    class AISystem : public System {
    public:
        /**
         * @brief Constructor
         * @param config AI system configuration
         */
        explicit AISystem(const AISystemConfig& config);

        /**
         * @brief Destructor
         */
        ~AISystem() override;

        /**
         * @brief Deleted copy constructor
         */
        AISystem(const AISystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        AISystem& operator=(const AISystem&) = delete;

        // System interface implementation

        /**
         * @brief Initialize AI system
         * @return true if successful, false otherwise
         */
        bool Initialize() override;

        /**
         * @brief Shutdown AI system
         */
        void Shutdown() override;

        /**
         * @brief Update AI system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime) override;

        /**
         * @brief Get system type
         * @return System type
         */
        SystemType GetType() const override { return SystemType::AI; }

        // AI system lifecycle

        /**
         * @brief Start AI system
         * @return true if successful, false otherwise
         */
        bool Start();

        /**
         * @brief Stop AI system
         */
        void Stop();

        /**
         * @brief Pause AI system
         */
        void Pause();

        /**
         * @brief Resume AI system
         */
        void Resume();

        /**
         * @brief Get current AI system state
         * @return AI system state
         */
        AIState GetState() const { return m_state; }

        // Configuration management

        /**
         * @brief Get AI system configuration
         * @return Current configuration
         */
        const AISystemConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set AI system configuration
         * @param config New configuration
         */
        void SetConfig(const AISystemConfig& config);

        /**
         * @brief Get AI system mode
         * @return Current mode
         */
        AISystemMode GetMode() const { return m_config.mode; }

        /**
         * @brief Set AI system mode
         * @param mode New mode
         */
        void SetMode(AISystemMode mode);

        // Metrics and monitoring

        /**
         * @brief Get AI system metrics
         * @return Performance metrics
         */
        const AISystemMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        // Mob management

        /**
         * @brief Register mob type
         * @param mobType Mob type name
         * @param factory Mob factory function
         * @return true if registered, false otherwise
         */
        bool RegisterMobType(const std::string& mobType, std::function<std::shared_ptr<Mob>()> factory);

        /**
         * @brief Unregister mob type
         * @param mobType Mob type name
         * @return true if unregistered, false otherwise
         */
        bool UnregisterMobType(const std::string& mobType);

        /**
         * @brief Spawn mob
         * @param mobType Mob type name
         * @param position Spawn position
         * @param rotation Spawn rotation
         * @return Spawned mob ID or 0 if failed
         */
        uint32_t SpawnMob(const std::string& mobType, const glm::vec3& position, const glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f));

        /**
         * @brief Despawn mob
         * @param mobId Mob ID
         * @return true if despawned, false otherwise
         */
        bool DespawnMob(uint32_t mobId);

        /**
         * @brief Get mob by ID
         * @param mobId Mob ID
         * @return Mob instance or nullptr
         */
        std::shared_ptr<Mob> GetMob(uint32_t mobId) const;

        /**
         * @brief Get all active mobs
         * @return Vector of active mobs
         */
        std::vector<std::shared_ptr<Mob>> GetActiveMobs() const;

        /**
         * @brief Get mobs in radius
         * @param position Center position
         * @param radius Search radius
         * @return Vector of mobs in radius
         */
        std::vector<std::shared_ptr<Mob>> GetMobsInRadius(const glm::vec3& position, float radius) const;

        // Subsystem access

        /**
         * @brief Get pathfinding system
         * @return Pathfinding system instance
         */
        PathfindingSystem* GetPathfindingSystem() const { return m_pathfindingSystem.get(); }

        /**
         * @brief Get navigation system
         * @return Navigation system instance
         */
        NavigationSystem* GetNavigationSystem() const { return m_navigationSystem.get(); }

        /**
         * @brief Get procedural generator
         * @return Procedural generator instance
         */
        ProceduralGenerator* GetProceduralGenerator() const { return m_proceduralGenerator.get(); }

        /**
         * @brief Get village system
         * @return Village system instance
         */
        VillageSystem* GetVillageSystem() const { return m_villageSystem.get(); }

        /**
         * @brief Get sensory system
         * @return Sensory system instance
         */
        SensorySystem* GetSensorySystem() const { return m_sensorySystem.get(); }

        /**
         * @brief Get decision system
         * @return Decision system instance
         */
        DecisionSystem* GetDecisionSystem() const { return m_decisionSystem.get(); }

        /**
         * @brief Get learning system
         * @return Learning system instance
         */
        LearningSystem* GetLearningSystem() const { return m_learningSystem.get(); }

        /**
         * @brief Get blackboard system
         * @return Blackboard system instance
         */
        BlackboardSystem* GetBlackboardSystem() const { return m_blackboardSystem.get(); }

        // Task management

        /**
         * @brief Schedule AI task
         * @param task AI task to schedule
         * @return Task ID
         */
        uint32_t ScheduleTask(const AITask& task);

        /**
         * @brief Cancel AI task
         * @param taskId Task ID to cancel
         * @return true if cancelled, false otherwise
         */
        bool CancelTask(uint32_t taskId);

        /**
         * @brief Get pending tasks count
         * @return Number of pending tasks
         */
        size_t GetPendingTasksCount() const;

        // Event system

        /**
         * @brief Add AI event listener
         * @param eventType Event type
         * @param listener Listener function
         */
        void AddEventListener(const std::string& eventType, std::function<void(const std::string&)> listener);

        /**
         * @brief Remove AI event listener
         * @param eventType Event type
         */
        void RemoveEventListener(const std::string& eventType);

        /**
         * @brief Trigger AI event
         * @param eventType Event type
         * @param eventData Event data
         */
        void TriggerEvent(const std::string& eventType, const std::string& eventData = "");

        // World integration

        /**
         * @brief Get AI world
         * @return World instance
         */
        World* GetWorld() const { return m_world; }

        /**
         * @brief Set AI world
         * @param world World instance
         */
        void SetWorld(World* world);

        /**
         * @brief Get entity manager
         * @return Entity manager instance
         */
        EntityManager* GetEntityManager() const { return m_entityManager; }

        /**
         * @brief Set entity manager
         * @param entityManager Entity manager instance
         */
        void SetEntityManager(EntityManager* entityManager);

        // Logging and debugging

        /**
         * @brief Log AI message
         * @param level Log level
         * @param component Component name
         * @param message Log message
         * @param data Additional data
         */
        void LogMessage(const std::string& level, const std::string& component, const std::string& message,
                       const std::unordered_map<std::string, std::any>& data = {});

        /**
         * @brief Get recent log entries
         * @param count Number of entries to retrieve
         * @return Vector of log entries
         */
        std::vector<AILogEntry> GetRecentLogEntries(size_t count = 100) const;

        /**
         * @brief Enable debug drawing
         * @param enabled Enable state
         * @return Previous state
         */
        bool SetDebugDrawingEnabled(bool enabled);

        /**
         * @brief Check if debug drawing is enabled
         * @return true if enabled, false otherwise
         */
        bool IsDebugDrawingEnabled() const { return m_config.enableDebugDrawing; }

        /**
         * @brief Get debug render data
         * @return Debug render data
         */
        const std::vector<DebugRenderData>& GetDebugRenderData() const;

        /**
         * @brief Clear debug render data
         */
        void ClearDebugRenderData();

        // Utility functions

        /**
         * @brief Validate AI system state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get AI system status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize AI system performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> OptimizeSystem();

        /**
         * @brief Get memory usage information
         * @return Memory usage data
         */
        std::unordered_map<std::string, size_t> GetMemoryUsage() const;

    private:
        /**
         * @brief Initialize AI subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Initialize worker threads
         * @return true if successful, false otherwise
         */
        bool InitializeWorkerThreads();

        /**
         * @brief Worker thread function
         * @param threadId Thread ID
         */
        void WorkerThreadFunction(int threadId);

        /**
         * @brief Process AI tasks
         */
        void ProcessTasks();

        /**
         * @brief Update active mobs
         * @param deltaTime Time elapsed
         */
        void UpdateMobs(double deltaTime);

        /**
         * @brief Update AI subsystems
         * @param deltaTime Time elapsed
         */
        void UpdateSubsystems(double deltaTime);

        /**
         * @brief Cleanup inactive mobs
         */
        void CleanupInactiveMobs();

        /**
         * @brief Process scheduled tasks
         * @param currentTime Current time
         */
        void ProcessScheduledTasks(double currentTime);

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Collect debug render data
         */
        void CollectDebugRenderData();

        /**
         * @brief Handle AI system errors
         * @param error Error message
         */
        void HandleSystemError(const std::string& error);

        // AI system data
        AISystemConfig m_config;                        ///< AI system configuration
        AISystemMetrics m_metrics;                      ///< Performance metrics
        AIState m_state;                                ///< Current system state

        // World integration
        World* m_world;                                 ///< Game world
        EntityManager* m_entityManager;                 ///< Entity manager

        // AI subsystems
        std::unique_ptr<PathfindingSystem> m_pathfindingSystem;    ///< Pathfinding system
        std::unique_ptr<NavigationSystem> m_navigationSystem;      ///< Navigation system
        std::unique_ptr<ProceduralGenerator> m_proceduralGenerator; ///< Procedural generator
        std::unique_ptr<VillageSystem> m_villageSystem;           ///< Village system
        std::unique_ptr<SensorySystem> m_sensorySystem;           ///< Sensory system
        std::unique_ptr<DecisionSystem> m_decisionSystem;         ///< Decision system
        std::unique_ptr<LearningSystem> m_learningSystem;         ///< Learning system
        std::unique_ptr<BlackboardSystem> m_blackboardSystem;     ///< Blackboard system

        // Mob management
        std::unordered_map<uint32_t, std::shared_ptr<Mob>> m_activeMobs;    ///< Active mobs
        std::unordered_map<std::string, std::function<std::shared_ptr<Mob>()>> m_mobFactories; ///< Mob factories
        mutable std::shared_mutex m_mobMutex;              ///< Mob synchronization

        // Task management
        std::priority_queue<AITask, std::vector<AITask>, std::function<bool(const AITask&, const AITask&)>> m_taskQueue;
        std::unordered_map<uint32_t, AITask> m_scheduledTasks;
        mutable std::shared_mutex m_taskMutex;             ///< Task synchronization
        std::condition_variable m_taskCondition;           ///< Task condition variable
        static std::atomic<uint32_t> s_nextTaskId;        ///< Next task ID

        // Threading
        std::vector<std::thread> m_workerThreads;         ///< Worker threads
        std::atomic<bool> m_workersRunning;               ///< Workers running flag
        std::atomic<int> m_activeThreads;                ///< Active thread count

        // Event system
        std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>> m_eventListeners;
        mutable std::shared_mutex m_eventMutex;           ///< Event synchronization

        // Logging
        std::vector<AILogEntry> m_logEntries;            ///< Recent log entries
        mutable std::shared_mutex m_logMutex;            ///< Log synchronization
        size_t m_maxLogEntries;                          ///< Maximum log entries

        // Debug data
        std::vector<DebugRenderData> m_debugRenderData;
        mutable std::shared_mutex m_debugMutex;          ///< Debug synchronization

        // Timing
        double m_lastUpdateTime;                         ///< Last update timestamp
        double m_creationTime;                           ///< System creation timestamp
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_AI_SYSTEM_HPP
