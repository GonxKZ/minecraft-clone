/**
 * @file PhysicsSystem.hpp
 * @brief VoxelCraft Physics System Main Coordinator
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the PhysicsSystem class that provides the main interface
 * for all physics functionality in the VoxelCraft engine, coordinating between
 * different physics subsystems and providing a unified API.
 */

#ifndef VOXELCRAFT_PHYSICS_PHYSICS_SYSTEM_HPP
#define VOXELCRAFT_PHYSICS_PHYSICS_SYSTEM_HPP

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
#include <any>

#include "../core/Config.hpp"
#include "../world/World.hpp"
#include "PhysicsTypes.hpp"
#include "PhysicsEngine.hpp"
#include "PhysicsWorld.hpp"
#include "CollisionSystem.hpp"
#include "RigidBody.hpp"
#include "Collider.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class PhysicsEngine;
    class PhysicsWorld;
    class CollisionSystem;
    class RigidBody;
    class Constraint;
    class Entity;
    class Player;

    /**
     * @enum PhysicsSystemState
     * @brief Current state of the physics system
     */
    enum class PhysicsSystemState {
        Uninitialized,        ///< System not initialized
        Initializing,         ///< System initializing
        Initialized,          ///< System initialized and ready
        Running,              ///< System running normally
        Paused,               ///< System paused
        Error,                ///< System in error state
        ShuttingDown          ///< System shutting down
    };

    /**
     * @enum PhysicsDebugMode
     * @brief Debug visualization modes
     */
    enum class PhysicsDebugMode {
        None,                ///< No debug visualization
        Basic,               ///< Basic wireframe visualization
        Detailed,            ///< Detailed visualization with normals
        Full,                ///< Full debug visualization
        Profiling            ///< Performance profiling overlay
    };

    /**
     * @struct PhysicsSystemConfig
     * @brief Configuration for the physics system
     */
    struct PhysicsSystemConfig {
        // Core settings
        bool enablePhysics;                   ///< Enable physics system
        bool enableCollisions;                ///< Enable collision detection
        bool enableConstraints;               ///< Enable constraint solving
        bool enableSleeping;                  ///< Enable object sleeping

        // Performance settings
        double fixedTimeStep;                 ///< Fixed timestep (seconds)
        int maxSubSteps;                      ///< Maximum substeps per frame
        int workerThreads;                    ///< Number of worker threads
        bool enableMultithreading;            ///< Enable multithreaded physics

        // Quality settings
        float simulationQuality;              ///< Simulation quality (0.0 - 1.0)
        bool enableCCD;                       ///< Enable continuous collision detection
        bool enableInterpolation;             ///< Enable state interpolation
        int positionIterations;               ///< Position correction iterations
        int velocityIterations;               ///< Velocity correction iterations

        // World settings
        glm::vec3 gravity;                    ///< Global gravity vector
        float airDensity;                     ///< Air density
        AABB worldBounds;                     ///< World bounds
        bool enableWorldBounds;               ///< Enable world bounds

        // Memory settings
        size_t maxRigidBodies;                ///< Maximum rigid bodies
        size_t maxConstraints;                ///< Maximum constraints
        size_t maxColliders;                  ///< Maximum colliders
        bool enableMemoryPooling;             ///< Enable memory pooling

        // Debug settings
        PhysicsDebugMode debugMode;           ///< Debug visualization mode
        bool enableDebugDrawing;              ///< Enable debug drawing
        bool enableProfiling;                 ///< Enable performance profiling
        bool enableValidation;                ///< Enable physics validation
        float debugDrawDistance;              ///< Maximum debug draw distance

        // Integration settings
        bool enableEntityIntegration;         ///< Enable entity physics integration
        bool enableVoxelIntegration;          ///< Enable voxel physics integration
        bool enableWeatherIntegration;        ///< Enable weather physics integration
        float integrationUpdateRate;          ///< Integration update rate (Hz)

        // Advanced settings
        bool enableDeterministicSimulation;   ///< Enable deterministic simulation
        uint32_t randomSeed;                  ///< Random seed for deterministic mode
        bool enableReplaySystem;              ///< Enable physics replay system
        size_t maxReplayFrames;               ///< Maximum replay frames
    };

    /**
     * @struct PhysicsSystemMetrics
     * @brief Comprehensive metrics for the physics system
     */
    struct PhysicsSystemMetrics {
        // System metrics
        PhysicsSystemState state;             ///< Current system state
        double uptime;                        ///< System uptime (seconds)
        uint64_t totalFrames;                 ///< Total frames processed
        double averageFrameTime;              ///< Average frame time (ms)
        double maxFrameTime;                  ///< Maximum frame time (ms)

        // Physics metrics (aggregated from subsystems)
        uint64_t totalSteps;                  ///< Total simulation steps
        double totalSimulationTime;           ///< Total simulation time
        double averageStepTime;              ///< Average step time (ms)
        uint32_t activeBodies;               ///< Currently active bodies
        uint32_t totalBodies;                ///< Total rigid bodies
        uint64_t collisionsDetected;         ///< Total collisions detected
        uint64_t constraintsSolved;          ///< Total constraints solved

        // Performance metrics
        double physicsTime;                  ///< Physics simulation time
        double collisionTime;                ///< Collision detection time
        double integrationTime;              ///< Integration time
        double synchronizationTime;          ///< World synchronization time
        double threadTime;                   ///< Multithreading time
        double totalFrameTime;               ///< Total frame time

        // Memory metrics
        size_t bodyMemoryUsage;              ///< Body memory usage
        size_t colliderMemoryUsage;          ///< Collider memory usage
        size_t constraintMemoryUsage;        ///< Constraint memory usage
        size_t totalMemoryUsage;             ///< Total memory usage
        size_t peakMemoryUsage;              ///< Peak memory usage

        // Threading metrics
        uint32_t activeThreads;              ///< Active physics threads
        double threadUtilization;            ///< Thread utilization (0.0 - 1.0)
        uint64_t tasksProcessed;             ///< Tasks processed
        uint64_t tasksQueued;                ///< Tasks queued

        // Integration metrics
        uint32_t entitiesIntegrated;         ///< Entities with physics integration
        uint32_t voxelsIntegrated;           ///< Voxels with physics integration
        uint32_t weatherEffects;             ///< Weather effects processed
        double integrationTime;              ///< Integration time

        // Error metrics
        uint64_t numericalErrors;            ///< Numerical integration errors
        uint64_t collisionErrors;            ///< Collision detection errors
        uint64_t constraintErrors;           ///< Constraint solving errors
        uint64_t memoryErrors;               ///< Memory allocation errors
        uint64_t threadErrors;               ///< Threading errors

        // Quality metrics
        double simulationAccuracy;           ///< Simulation accuracy (0.0 - 1.0)
        double collisionAccuracy;            ///< Collision detection accuracy
        double constraintAccuracy;           ///< Constraint solving accuracy
        uint64_t stabilityIssues;           ///< Simulation stability issues
    };

    /**
     * @struct PhysicsReplayFrame
     * @brief Physics state frame for replay system
     */
    struct PhysicsReplayFrame {
        double timestamp;                    ///< Frame timestamp
        std::vector<RigidBodyState> bodyStates;  ///< All body states
        std::vector<ConstraintState> constraintStates;  ///< All constraint states
        std::unordered_map<std::string, std::any> metadata;  ///< Frame metadata
    };

    /**
     * @class PhysicsSystem
     * @brief Main physics system coordinator for VoxelCraft
     *
     * The PhysicsSystem is the central hub for all physics functionality in the
     * VoxelCraft engine. It provides a unified interface to manage physics
     * simulation, collision detection, rigid body dynamics, and integration
     * with the game world.
     *
     * Key Features:
     * - Unified physics API for the entire engine
     * - Multiple physics worlds support
     * - Entity and voxel integration
     * - Advanced debugging and profiling
     * - Multithreaded simulation
     * - Deterministic simulation mode
     * - Physics replay and recording system
     * - Memory management and optimization
     * - Real-time performance monitoring
     *
     * The system coordinates between:
     * - PhysicsEngine: Core physics simulation
     * - PhysicsWorld: World-specific physics management
     * - CollisionSystem: Collision detection and response
     * - Entity integration: Game object physics
     * - Voxel integration: Block-based physics
     * - Weather integration: Environmental physics effects
     */
    class PhysicsSystem {
    public:
        /**
         * @brief Constructor
         * @param config Physics system configuration
         */
        explicit PhysicsSystem(const PhysicsSystemConfig& config);

        /**
         * @brief Destructor
         */
        ~PhysicsSystem();

        /**
         * @brief Deleted copy constructor
         */
        PhysicsSystem(const PhysicsSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        PhysicsSystem& operator=(const PhysicsSystem&) = delete;

        // System lifecycle

        /**
         * @brief Initialize physics system
         * @param world Parent game world
         * @return true if successful, false otherwise
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown physics system
         */
        void Shutdown();

        /**
         * @brief Update physics system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Render debug information
         */
        void RenderDebug();

        /**
         * @brief Get system state
         * @return Current system state
         */
        PhysicsSystemState GetState() const { return m_state; }

        // Simulation control

        /**
         * @brief Start physics simulation
         * @return true if started, false otherwise
         */
        bool StartSimulation();

        /**
         * @brief Stop physics simulation
         */
        void StopSimulation();

        /**
         * @brief Pause physics simulation
         */
        void PauseSimulation();

        /**
         * @brief Resume physics simulation
         */
        void ResumeSimulation();

        /**
         * @brief Check if simulation is running
         * @return true if running, false otherwise
         */
        bool IsSimulationRunning() const;

        /**
         * @brief Check if simulation is paused
         * @return true if paused, false otherwise
         */
        bool IsSimulationPaused() const;

        /**
         * @brief Set simulation time scale
         * @param scale Time scale (1.0 = normal speed)
         */
        void SetTimeScale(float scale);

        /**
         * @brief Get simulation time scale
         * @return Current time scale
         */
        float GetTimeScale() const { return m_timeScale; }

        /**
         * @brief Reset physics simulation
         */
        void ResetSimulation();

        // World management

        /**
         * @brief Create new physics world
         * @param name World name
         * @param config World configuration
         * @return World instance or nullptr on failure
         */
        std::unique_ptr<PhysicsWorld> CreateWorld(const std::string& name,
                                                 const PhysicsWorldConfig& config);

        /**
         * @brief Get physics world by name
         * @param name World name
         * @return World instance or nullptr if not found
         */
        PhysicsWorld* GetWorld(const std::string& name) const;

        /**
         * @brief Get all physics worlds
         * @return Vector of world pointers
         */
        std::vector<PhysicsWorld*> GetAllWorlds() const;

        /**
         * @brief Remove physics world
         * @param name World name
         * @return true if removed, false otherwise
         */
        bool RemoveWorld(const std::string& name);

        /**
         * @brief Get default physics world
         * @return Default world or nullptr
         */
        PhysicsWorld* GetDefaultWorld() const { return m_defaultWorld; }

        // Rigid body operations

        /**
         * @brief Create rigid body in default world
         * @param position Initial position
         * @param orientation Initial orientation
         * @param mass Body mass (0 for static)
         * @return Rigid body instance or nullptr on failure
         */
        std::unique_ptr<RigidBody> CreateRigidBody(
            const glm::vec3& position = glm::vec3(0.0f),
            const glm::quat& orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            float mass = 1.0f);

        /**
         * @brief Create rigid body in specific world
         * @param worldName World name
         * @param position Initial position
         * @param orientation Initial orientation
         * @param mass Body mass
         * @return Rigid body instance or nullptr on failure
         */
        std::unique_ptr<RigidBody> CreateRigidBodyInWorld(
            const std::string& worldName,
            const glm::vec3& position = glm::vec3(0.0f),
            const glm::quat& orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            float mass = 1.0f);

        /**
         * @brief Find rigid body by ID across all worlds
         * @param id Body ID
         * @return Body pointer or nullptr if not found
         */
        RigidBody* FindRigidBody(uint32_t id) const;

        /**
         * @brief Get all rigid bodies across all worlds
         * @return Vector of body pointers
         */
        std::vector<RigidBody*> GetAllRigidBodies() const;

        // Entity integration

        /**
         * @brief Add entity to physics simulation
         * @param entity Entity to add
         * @param worldName Optional world name (default world if empty)
         * @return true if added, false otherwise
         */
        bool AddEntityToPhysics(Entity* entity, const std::string& worldName = "");

        /**
         * @brief Remove entity from physics simulation
         * @param entity Entity to remove
         * @return true if removed, false otherwise
         */
        bool RemoveEntityFromPhysics(Entity* entity);

        /**
         * @brief Get rigid body for entity
         * @param entity Entity
         * @return Rigid body or nullptr if not found
         */
        RigidBody* GetRigidBodyForEntity(Entity* entity) const;

        /**
         * @brief Update entity physics
         * @param entity Entity to update
         * @param deltaTime Time elapsed
         */
        void UpdateEntityPhysics(Entity* entity, double deltaTime);

        // Voxel integration

        /**
         * @brief Add voxel collision to physics
         * @param position Voxel position
         * @param size Voxel size
         * @param worldName Optional world name
         * @return true if added, false otherwise
         */
        bool AddVoxelCollision(const glm::vec3& position, const glm::vec3& size,
                              const std::string& worldName = "");

        /**
         * @brief Remove voxel collision from physics
         * @param position Voxel position
         * @param worldName Optional world name
         * @return true if removed, false otherwise
         */
        bool RemoveVoxelCollision(const glm::vec3& position, const std::string& worldName = "");

        /**
         * @brief Update voxel collisions in area
         * @param minPos Minimum position
         * @param maxPos Maximum position
         * @param worldName Optional world name
         * @return Number of voxels updated
         */
        size_t UpdateVoxelCollisions(const glm::vec3& minPos, const glm::vec3& maxPos,
                                    const std::string& worldName = "");

        // Query operations

        /**
         * @brief Perform raycast in default world
         * @param ray Ray to cast
         * @param maxDistance Maximum distance
         * @param hitInfo Output hit information
         * @return true if hit something, false otherwise
         */
        bool Raycast(const Ray& ray, float maxDistance, RaycastHit& hitInfo) const;

        /**
         * @brief Perform raycast in specific world
         * @param worldName World name
         * @param ray Ray to cast
         * @param maxDistance Maximum distance
         * @param hitInfo Output hit information
         * @return true if hit something, false otherwise
         */
        bool RaycastInWorld(const std::string& worldName, const Ray& ray,
                           float maxDistance, RaycastHit& hitInfo) const;

        /**
         * @brief Perform overlap test in default world
         * @param collider Test collider
         * @param overlapping Output overlapping bodies
         * @return Number of overlapping bodies
         */
        size_t OverlapTest(const Collider* collider, std::vector<RigidBody*>& overlapping) const;

        /**
         * @brief Perform sweep test in default world
         * @param collider Test collider
         * @param direction Sweep direction
         * @param distance Sweep distance
         * @param hitInfo Output hit information
         * @return true if hit something, false otherwise
         */
        bool SweepTest(const Collider* collider, const glm::vec3& direction,
                      float distance, RaycastHit& hitInfo) const;

        // Configuration

        /**
         * @brief Get physics system configuration
         * @return Current configuration
         */
        const PhysicsSystemConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set physics system configuration
         * @param config New configuration
         */
        void SetConfig(const PhysicsSystemConfig& config);

        /**
         * @brief Get physics system metrics
         * @return Performance metrics
         */
        const PhysicsSystemMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Debug and profiling

        /**
         * @brief Set debug mode
         * @param mode Debug mode
         */
        void SetDebugMode(PhysicsDebugMode mode);

        /**
         * @brief Get current debug mode
         * @return Debug mode
         */
        PhysicsDebugMode GetDebugMode() const { return m_config.debugMode; }

        /**
         * @brief Enable/disable debug drawing
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
         * @brief Get combined debug render data from all worlds
         * @return Debug render data
         */
        std::vector<DebugRenderData> GetDebugRenderData() const;

        /**
         * @brief Get profiling information
         * @return Profiling data
         */
        std::string GetProfilingData() const;

        // Replay system

        /**
         * @brief Start physics recording
         * @return true if started, false otherwise
         */
        bool StartRecording();

        /**
         * @brief Stop physics recording
         * @return true if stopped, false otherwise
         */
        bool StopRecording();

        /**
         * @brief Start physics replay
         * @return true if started, false otherwise
         */
        bool StartReplay();

        /**
         * @brief Stop physics replay
         * @return true if stopped, false otherwise
         */
        bool StopReplay();

        /**
         * @brief Check if recording
         * @return true if recording, false otherwise
         */
        bool IsRecording() const { return m_recording; }

        /**
         * @brief Check if replaying
         * @return true if replaying, false otherwise
         */
        bool IsReplaying() const { return m_replaying; }

        /**
         * @brief Get recorded frames count
         * @return Number of recorded frames
         */
        size_t GetRecordedFramesCount() const { return m_recordedFrames.size(); }

        // Utility functions

        /**
         * @brief Get system uptime
         * @return Uptime in seconds
         */
        double GetUptime() const;

        /**
         * @brief Get total simulation time
         * @return Total simulation time
         */
        double GetTotalSimulationTime() const;

        /**
         * @brief Get physics system status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Validate physics system integrity
         * @return Vector of validation errors
         */
        std::vector<std::string> ValidateSystem() const;

        /**
         * @brief Optimize physics system performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> OptimizeSystem();

        /**
         * @brief Clear all physics data
         */
        void ClearAll();

    private:
        /**
         * @brief Initialize subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Initialize default physics world
         * @return true if successful, false otherwise
         */
        bool InitializeDefaultWorld();

        /**
         * @brief Update all physics worlds
         * @param deltaTime Time elapsed
         */
        void UpdatePhysicsWorlds(double deltaTime);

        /**
         * @brief Update entity integration
         * @param deltaTime Time elapsed
         */
        void UpdateEntityIntegration(double deltaTime);

        /**
         * @brief Update voxel integration
         * @param deltaTime Time elapsed
         */
        void UpdateVoxelIntegration(double deltaTime);

        /**
         * @brief Update weather integration
         * @param deltaTime Time elapsed
         */
        void UpdateWeatherIntegration(double deltaTime);

        /**
         * @brief Process replay system
         * @param deltaTime Time elapsed
         */
        void ProcessReplay(double deltaTime);

        /**
         * @brief Record physics frame
         */
        void RecordPhysicsFrame();

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle system errors
         * @param error Error message
         */
        void HandleSystemError(const std::string& error);

        /**
         * @brief Validate system configuration
         * @return true if valid, false otherwise
         */
        bool ValidateConfiguration() const;

        // Configuration and state
        PhysicsSystemConfig m_config;              ///< System configuration
        PhysicsSystemMetrics m_metrics;            ///< Performance metrics
        PhysicsSystemState m_state;                ///< Current system state
        World* m_world;                            ///< Parent game world

        // Physics worlds
        std::unordered_map<std::string, std::unique_ptr<PhysicsWorld>> m_physicsWorlds;
        PhysicsWorld* m_defaultWorld;              ///< Default physics world
        mutable std::shared_mutex m_worldsMutex;   ///< Worlds synchronization

        // Entity integration
        std::unordered_map<Entity*, std::string> m_entityWorlds;  ///< Entity to world mapping
        mutable std::shared_mutex m_entityMutex;   ///< Entity synchronization

        // Simulation state
        float m_timeScale;                         ///< Time scale multiplier
        bool m_simulationRunning;                  ///< Simulation running flag
        double m_lastUpdateTime;                   ///< Last update timestamp
        double m_startTime;                        ///< System start time

        // Replay system
        bool m_recording;                          ///< Recording active flag
        bool m_replaying;                          ///< Replay active flag
        size_t m_replayFrameIndex;                 ///< Current replay frame
        std::vector<PhysicsReplayFrame> m_recordedFrames;  ///< Recorded frames
        double m_lastRecordTime;                   ///< Last record timestamp

        // Threading
        std::atomic<bool> m_systemRunning;         ///< System running flag

        // Debug and profiling
        mutable std::shared_mutex m_debugMutex;    ///< Debug synchronization
        std::unordered_map<std::string, double> m_profileSections;
        mutable std::mutex m_profileMutex;         ///< Profile synchronization

        // Error tracking
        std::vector<std::string> m_systemErrors;   ///< System errors
        mutable std::mutex m_errorMutex;           ///< Error synchronization
    };

    /**
     * @class PhysicsSystemFactory
     * @brief Factory for creating physics system configurations
     */
    class PhysicsSystemFactory {
    public:
        /**
         * @brief Create default physics system configuration
         * @return Default configuration
         */
        static PhysicsSystemConfig CreateDefaultConfig();

        /**
         * @brief Create high-performance physics system configuration
         * @return High-performance configuration
         */
        static PhysicsSystemConfig CreateHighPerformanceConfig();

        /**
         * @brief Create low-memory physics system configuration
         * @return Low-memory configuration
         */
        static PhysicsSystemConfig CreateLowMemoryConfig();

        /**
         * @brief Create custom physics system configuration
         * @param baseConfig Base configuration
         * @param overrides Configuration overrides
         * @return Custom configuration
         */
        static PhysicsSystemConfig CreateCustomConfig(
            const PhysicsSystemConfig& baseConfig,
            const std::unordered_map<std::string, std::any>& overrides);

        /**
         * @brief Load configuration from file
         * @param filename Configuration file
         * @return Loaded configuration
         */
        static PhysicsSystemConfig LoadConfigFromFile(const std::string& filename);

        /**
         * @brief Save configuration to file
         * @param config Configuration to save
         * @param filename Output filename
         * @return true if successful, false otherwise
         */
        static bool SaveConfigToFile(const PhysicsSystemConfig& config, const std::string& filename);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PHYSICS_PHYSICS_SYSTEM_HPP
