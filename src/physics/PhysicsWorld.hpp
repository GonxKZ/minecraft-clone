/**
 * @file PhysicsWorld.hpp
 * @brief VoxelCraft Physics World Coordinator
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the PhysicsWorld class that coordinates all physics
 * simulation, including rigid bodies, constraints, collision detection,
 * and integration with the game world.
 */

#ifndef VOXELCRAFT_PHYSICS_PHYSICS_WORLD_HPP
#define VOXELCRAFT_PHYSICS_PHYSICS_WORLD_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <queue>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>

#include "../core/Config.hpp"
#include "../world/World.hpp"
#include "PhysicsTypes.hpp"
#include "PhysicsEngine.hpp"
#include "CollisionSystem.hpp"
#include "RigidBody.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class PhysicsEngine;
    class CollisionSystem;
    class RigidBody;
    class Constraint;
    class Entity;

    /**
     * @struct PhysicsWorldConfig
     * @brief Configuration for the physics world
     */
    struct PhysicsWorldConfig {
        // Simulation settings
        double fixedTimeStep;                   ///< Fixed timestep (seconds)
        int maxSubSteps;                        ///< Maximum substeps per frame
        double maxTimeStep;                     ///< Maximum timestep
        bool enableInterpolation;               ///< Enable state interpolation

        // World settings
        glm::vec3 gravity;                      ///< World gravity vector
        float airDensity;                       ///< Air density
        float globalFriction;                   ///< Global friction coefficient
        float globalRestitution;                ///< Global restitution coefficient

        // Performance settings
        bool enableMultithreading;              ///< Enable multithreaded physics
        int workerThreads;                      ///< Number of physics worker threads
        size_t maxRigidBodies;                  ///< Maximum rigid bodies
        size_t maxConstraints;                  ///< Maximum constraints

        // Quality settings
        bool enableCCD;                         ///< Enable continuous collision detection
        bool enableSleeping;                    ///< Enable object sleeping
        float sleepThreshold;                   ///< Sleep threshold
        float sleepTimeThreshold;               ///< Time to sleep

        // Debug settings
        bool enableDebugDrawing;                ///< Enable debug visualization
        bool enableProfiling;                   ///< Enable performance profiling
        bool enableValidation;                  ///< Enable physics validation
        float debugDrawDistance;                ///< Debug draw distance

        // Integration settings
        bool enableEntityIntegration;           ///< Enable entity integration
        bool enableVoxelIntegration;            ///< Enable voxel integration
        bool enableWeatherIntegration;          ///< Enable weather integration
        float integrationUpdateRate;            ///< Integration update rate
    };

    /**
     * @struct PhysicsWorldMetrics
     * @brief Performance metrics for the physics world
     */
    struct PhysicsWorldMetrics {
        // Simulation metrics
        uint64_t totalSteps;                    ///< Total simulation steps
        double totalTime;                       ///< Total simulation time
        double averageStepTime;                 ///< Average step time
        double maxStepTime;                     ///< Maximum step time
        double minStepTime;                     ///< Minimum step time

        // Object metrics
        uint32_t activeBodies;                  ///< Currently active bodies
        uint32_t sleepingBodies;                ///< Sleeping bodies
        uint32_t totalBodies;                   ///< Total rigid bodies
        uint32_t activeConstraints;             ///< Active constraints
        uint32_t totalConstraints;              ///< Total constraints

        // Performance metrics
        double collisionTime;                   ///< Collision detection time
        double integrationTime;                 ///< Integration time
        double constraintTime;                  ///< Constraint solving time
        double synchronizationTime;             ///< World synchronization time
        double totalFrameTime;                  ///< Total frame time

        // Threading metrics
        uint32_t activeThreads;                 ///< Active physics threads
        double threadUtilization;               ///< Thread utilization
        uint32_t tasksProcessed;                ///< Tasks processed
        uint32_t tasksQueued;                   ///< Tasks queued

        // Memory metrics
        size_t bodyMemoryUsage;                 ///< Body memory usage
        size_t constraintMemoryUsage;           ///< Constraint memory usage
        size_t totalMemoryUsage;                ///< Total memory usage
        size_t peakMemoryUsage;                 ///< Peak memory usage

        // Integration metrics
        uint32_t entitiesUpdated;               ///< Entities updated
        uint32_t voxelsUpdated;                 ///< Voxels updated
        uint32_t weatherEffects;                ///< Weather effects processed
        double integrationTime;                 ///< Integration time

        // Error metrics
        uint64_t numericalErrors;              ///< Numerical integration errors
        uint64_t collisionErrors;              ///< Collision detection errors
        uint64_t constraintErrors;             ///< Constraint solving errors
        uint64_t synchronizationErrors;         ///< Synchronization errors
    };

    /**
     * @struct PhysicsTask
     * @brief Physics computation task for multithreading
     */
    struct PhysicsTask {
        std::function<void()> function;        ///< Task function
        int priority;                          ///< Task priority (0 = highest)
        std::string name;                      ///< Task name for debugging
        double timestamp;                      ///< Task creation timestamp
        std::atomic<bool> completed;           ///< Completion flag
        std::atomic<bool> started;             ///< Started flag
    };

    /**
     * @struct PhysicsWorldState
     * @brief Current state of the physics world
     */
    struct PhysicsWorldState {
        double currentTime;                    ///< Current simulation time
        double deltaTime;                      ///< Last frame delta time
        double fixedTimeStep;                  ///< Fixed timestep
        double accumulator;                    ///< Time accumulator
        bool isPaused;                         ///< Simulation paused flag
        bool isInitialized;                    ///< World initialized flag
        uint64_t stepCount;                    ///< Total steps taken
    };

    /**
     * @class PhysicsWorld
     * @brief Main physics world coordinator
     *
     * The PhysicsWorld is the central coordinator for all physics simulation
     * in the game world. It manages rigid bodies, constraints, collision detection,
     * and integration with the game world and entities.
     *
     * Key Features:
     * - Unified physics simulation interface
     * - Entity integration (game objects with physics)
     * - Voxel world integration (block physics)
     * - Weather system integration
     * - Multithreaded simulation
     * - Real-time debugging and profiling
     * - Deterministic simulation (when needed)
     * - Memory management and optimization
     */
    class PhysicsWorld {
    public:
        /**
         * @brief Constructor
         * @param config Physics world configuration
         */
        explicit PhysicsWorld(const PhysicsWorldConfig& config);

        /**
         * @brief Destructor
         */
        ~PhysicsWorld();

        /**
         * @brief Deleted copy constructor
         */
        PhysicsWorld(const PhysicsWorld&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        PhysicsWorld& operator=(const PhysicsWorld&) = delete;

        // World lifecycle

        /**
         * @brief Initialize physics world
         * @param world Parent game world
         * @return true if successful, false otherwise
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown physics world
         */
        void Shutdown();

        /**
         * @brief Update physics simulation
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Perform single physics step
         * @param timeStep Time step for simulation
         */
        void Step(double timeStep);

        /**
         * @brief Render debug information
         */
        void RenderDebug();

        // Simulation control

        /**
         * @brief Pause physics simulation
         */
        void Pause();

        /**
         * @brief Resume physics simulation
         */
        void Resume();

        /**
         * @brief Check if simulation is paused
         * @return true if paused, false otherwise
         */
        bool IsPaused() const { return m_state.isPaused; }

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
         * @brief Get current simulation time
         * @return Current time
         */
        double GetCurrentTime() const { return m_state.currentTime; }

        /**
         * @brief Get physics world state
         * @return Current state
         */
        const PhysicsWorldState& GetState() const { return m_state; }

        // Rigid body management

        /**
         * @brief Create rigid body
         * @param position Initial position
         * @param orientation Initial orientation
         * @param mass Body mass (0 for static)
         * @return Rigid body instance
         */
        std::unique_ptr<RigidBody> CreateRigidBody(
            const glm::vec3& position = glm::vec3(0.0f),
            const glm::quat& orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            float mass = 1.0f);

        /**
         * @brief Add rigid body to world
         * @param body Body to add
         * @return true if added, false otherwise
         */
        bool AddRigidBody(std::unique_ptr<RigidBody> body);

        /**
         * @brief Remove rigid body from world
         * @param body Body to remove
         * @return true if removed, false otherwise
         */
        bool RemoveRigidBody(RigidBody* body);

        /**
         * @brief Get rigid body by ID
         * @param id Body ID
         * @return Body pointer or nullptr if not found
         */
        RigidBody* GetRigidBody(uint32_t id) const;

        /**
         * @brief Get all rigid bodies
         * @return Vector of body pointers
         */
        std::vector<RigidBody*> GetRigidBodies() const;

        /**
         * @brief Get rigid bodies in area
         * @param center Center position
         * @param radius Search radius
         * @return Vector of bodies in area
         */
        std::vector<RigidBody*> GetRigidBodiesInArea(const glm::vec3& center, float radius) const;

        /**
         * @brief Get active rigid bodies
         * @return Vector of active bodies
         */
        std::vector<RigidBody*> GetActiveRigidBodies() const;

        // Entity integration

        /**
         * @brief Add entity to physics world
         * @param entity Entity to add
         * @return true if added, false otherwise
         */
        bool AddEntity(Entity* entity);

        /**
         * @brief Remove entity from physics world
         * @param entity Entity to remove
         * @return true if removed, false otherwise
         */
        bool RemoveEntity(Entity* entity);

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
         * @brief Add voxel collision at position
         * @param position Voxel position
         * @param size Voxel size
         * @return true if added, false otherwise
         */
        bool AddVoxelCollision(const glm::vec3& position, const glm::vec3& size);

        /**
         * @brief Remove voxel collision at position
         * @param position Voxel position
         * @return true if removed, false otherwise
         */
        bool RemoveVoxelCollision(const glm::vec3& position);

        /**
         * @brief Update voxel collisions in area
         * @param minPos Minimum position
         * @param maxPos Maximum position
         * @return Number of voxels updated
         */
        size_t UpdateVoxelCollisions(const glm::vec3& minPos, const glm::vec3& maxPos);

        /**
         * @brief Handle voxel destruction physics
         * @param position Voxel position
         * @param explosionForce Explosion force
         */
        void HandleVoxelDestruction(const glm::vec3& position, float explosionForce);

        // Query operations

        /**
         * @brief Perform raycast
         * @param ray Ray to cast
         * @param maxDistance Maximum distance
         * @param hitInfo Output hit information
         * @return true if hit something, false otherwise
         */
        bool Raycast(const Ray& ray, float maxDistance, RaycastHit& hitInfo) const;

        /**
         * @brief Perform multiple raycasts
         * @param rays Rays to cast
         * @param maxDistance Maximum distance
         * @param hitInfos Output hit information
         * @return Number of hits
         */
        size_t MultiRaycast(const std::vector<Ray>& rays, float maxDistance,
                           std::vector<RaycastHit>& hitInfos) const;

        /**
         * @brief Perform overlap test
         * @param collider Test collider
         * @param overlapping Output overlapping bodies
         * @return Number of overlapping bodies
         */
        size_t OverlapTest(const Collider* collider, std::vector<RigidBody*>& overlapping) const;

        /**
         * @brief Perform sweep test
         * @param collider Test collider
         * @param direction Sweep direction
         * @param distance Sweep distance
         * @param hitInfo Output hit information
         * @return true if hit something, false otherwise
         */
        bool SweepTest(const Collider* collider, const glm::vec3& direction,
                      float distance, RaycastHit& hitInfo) const;

        // Weather integration

        /**
         * @brief Apply weather forces to bodies
         * @param deltaTime Time elapsed
         */
        void ApplyWeatherForces(double deltaTime);

        /**
         * @brief Update weather effects on physics
         * @param weatherType Current weather type
         * @param intensity Weather intensity
         */
        void UpdateWeatherEffects(const std::string& weatherType, float intensity);

        // Configuration

        /**
         * @brief Get physics world configuration
         * @return Current configuration
         */
        const PhysicsWorldConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set physics world configuration
         * @param config New configuration
         */
        void SetConfig(const PhysicsWorldConfig& config);

        /**
         * @brief Get physics world metrics
         * @return Performance metrics
         */
        const PhysicsWorldMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Debug and profiling

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
         * @brief Get debug render data
         * @return Debug render data
         */
        const std::vector<DebugRenderData>& GetDebugRenderData() const;

        /**
         * @brief Clear debug render data
         */
        void ClearDebugRenderData();

        /**
         * @brief Get profiling information
         * @return Profiling data
         */
        std::string GetProfilingData() const;

        /**
         * @brief Start profiling section
         * @param name Section name
         */
        void StartProfilingSection(const std::string& name);

        /**
         * @brief End profiling section
         * @param name Section name
         */
        void EndProfilingSection(const std::string& name);

        // Utility functions

        /**
         * @brief Get world gravity
         * @return Gravity vector
         */
        const glm::vec3& GetGravity() const { return m_config.gravity; }

        /**
         * @brief Set world gravity
         * @param gravity New gravity vector
         */
        void SetGravity(const glm::vec3& gravity);

        /**
         * @brief Get air density
         * @return Air density
         */
        float GetAirDensity() const { return m_config.airDensity; }

        /**
         * @brief Set air density
         * @param density New air density
         */
        void SetAirDensity(float density);

        /**
         * @brief Get physics world bounds
         * @return World AABB
         */
        AABB GetWorldBounds() const;

        /**
         * @brief Set physics world bounds
         * @param bounds New world bounds
         */
        void SetWorldBounds(const AABB& bounds);

        /**
         * @brief Validate physics world state
         * @return true if valid, false otherwise
         */
        bool ValidateWorldState() const;

        /**
         * @brief Get physics world status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

    private:
        /**
         * @brief Initialize physics subsystems
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
         * @brief Process physics tasks
         */
        void ProcessTasks();

        /**
         * @brief Perform collision detection
         */
        void PerformCollisionDetection();

        /**
         * @brief Solve constraints
         */
        void SolveConstraints();

        /**
         * @brief Integrate rigid body motion
         * @param timeStep Time step
         */
        void IntegrateMotion(double timeStep);

        /**
         * @brief Synchronize with game world
         */
        void SynchronizeWorld();

        /**
         * @brief Update sleeping bodies
         */
        void UpdateSleepingBodies();

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
         * @brief Collect debug render data
         */
        void CollectDebugRenderData();

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle physics errors
         * @param error Error message
         */
        void HandlePhysicsError(const std::string& error);

        // Configuration and state
        PhysicsWorldConfig m_config;              ///< World configuration
        PhysicsWorldMetrics m_metrics;            ///< Performance metrics
        PhysicsWorldState m_state;                ///< World state
        World* m_world;                           ///< Parent game world

        // Core systems
        std::unique_ptr<PhysicsEngine> m_physicsEngine;    ///< Physics engine
        std::unique_ptr<CollisionSystem> m_collisionSystem; ///< Collision system

        // Object management
        std::unordered_map<uint32_t, std::unique_ptr<RigidBody>> m_rigidBodies;
        std::unordered_map<Entity*, RigidBody*> m_entityBodies;
        mutable std::shared_mutex m_bodiesMutex;  ///< Bodies synchronization

        // Voxel integration
        std::unordered_map<glm::vec3, VoxelCollisionData> m_voxelCollisions;
        mutable std::shared_mutex m_voxelMutex;   ///< Voxel synchronization

        // Simulation state
        float m_timeScale;                        ///< Time scale multiplier
        AABB m_worldBounds;                       ///< World bounds
        bool m_boundsEnabled;                     ///< World bounds enabled

        // Threading
        std::vector<std::thread> m_workerThreads; ///< Worker threads
        std::atomic<bool> m_workersRunning;       ///< Workers running flag
        std::queue<PhysicsTask> m_taskQueue;      ///< Task queue
        mutable std::mutex m_taskMutex;           ///< Task synchronization
        std::condition_variable m_taskCondition;  ///< Task condition variable

        // Debug and profiling
        std::vector<DebugRenderData> m_debugRenderData;
        mutable std::shared_mutex m_debugMutex;   ///< Debug synchronization
        std::unordered_map<std::string, double> m_profileSections;
        mutable std::mutex m_profileMutex;        ///< Profile synchronization

        // Weather integration
        std::string m_currentWeatherType;         ///< Current weather type
        float m_currentWeatherIntensity;          ///< Current weather intensity
        double m_lastWeatherUpdate;               ///< Last weather update time

        // Statistics
        uint64_t m_totalSteps;                    ///< Total simulation steps
        uint64_t m_totalCollisions;               ///< Total collisions processed
        uint64_t m_totalIntegrations;             ///< Total integrations performed
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PHYSICS_PHYSICS_WORLD_HPP
