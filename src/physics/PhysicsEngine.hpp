/**
 * @file PhysicsEngine.hpp
 * @brief VoxelCraft Advanced Physics Engine
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the PhysicsEngine class that provides a complete physics
 * simulation system with rigid body dynamics, collision detection, and
 * constraint solving for voxel-based worlds.
 */

#ifndef VOXELCRAFT_PHYSICS_PHYSICS_ENGINE_HPP
#define VOXELCRAFT_PHYSICS_PHYSICS_ENGINE_HPP

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
#include "RigidBody.hpp"
#include "CollisionSystem.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class RigidBody;
    class CollisionSystem;
    class ConstraintSolver;
    class PhysicsWorld;
    class BroadPhase;
    class NarrowPhase;
    class PhysicsMaterial;
    class Constraint;

    /**
     * @enum PhysicsSolver
     * @brief Physics solver algorithms
     */
    enum class PhysicsSolver {
        PGS,                    ///< Projected Gauss-Seidel (default)
        Jacobi,                 ///< Jacobi iteration
        Impulse,                ///< Impulse-based solver
        XPBD,                   ///< Extended Position Based Dynamics
        SequentialImpulse,      ///< Sequential impulse solver
        Hybrid                  ///< Hybrid approach
    };

    /**
     * @enum IntegrationMethod
     * @brief Numerical integration methods
     */
    enum class IntegrationMethod {
        SemiImplicitEuler,      ///< Semi-implicit Euler (default)
        ExplicitEuler,          ///< Explicit Euler
        Verlet,                 ///< Verlet integration
        RK4,                    ///< Runge-Kutta 4th order
        Adaptive,               ///< Adaptive timestep
        Symplectic,             ///< Symplectic Euler
        Implicit,               ///< Implicit Euler
    };

    /**
     * @enum PhysicsQuality
     * @brief Physics simulation quality levels
     */
    enum class PhysicsQuality {
        Fast,                   ///< Fast simulation, low accuracy
        Normal,                 ///< Balanced performance/accuracy
        High,                   ///< High accuracy, moderate performance
        Ultra,                  ///< Ultra-high accuracy, low performance
        Custom                  ///< Custom settings
    };

    /**
     * @struct PhysicsConfig
     * @brief Physics engine configuration
     */
    struct PhysicsConfig {
        // Basic settings
        PhysicsSolver solver;                   ///< Physics solver algorithm
        IntegrationMethod integrator;           ///< Integration method
        PhysicsQuality quality;                 ///< Simulation quality

        // Performance settings
        double fixedTimeStep;                   ///< Fixed timestep (seconds)
        int maxSubSteps;                        ///< Maximum substeps per frame
        int positionIterations;                 ///< Position correction iterations
        int velocityIterations;                 ///< Velocity correction iterations
        double baumgarteFactor;                 ///< Baumgarte stabilization factor

        // World settings
        glm::vec3 gravity;                      ///< Global gravity vector
        double airDensity;                      ///< Air density for drag
        double globalFriction;                  ///< Global friction coefficient
        double globalRestitution;               ///< Global restitution coefficient

        // Constraints
        int maxConstraints;                     ///< Maximum number of constraints
        double constraintWarmStart;             ///< Constraint warm starting factor
        double constraintBias;                  ///< Constraint bias factor

        // Bodies
        int maxRigidBodies;                     ///< Maximum rigid bodies
        int maxSleepingBodies;                  ///< Maximum sleeping bodies
        double sleepThreshold;                  ///< Sleep threshold
        double sleepTimeThreshold;              ///< Time to sleep (seconds)

        // Performance
        bool enableMultithreading;              ///< Enable multithreaded physics
        int workerThreads;                      ///< Number of physics worker threads
        int batchSize;                          ///< Batch size for parallel processing
        bool enableSIMD;                        ///< Enable SIMD optimizations

        // Debugging
        bool enableDebugDrawing;                ///< Enable debug visualization
        bool enableProfiling;                   ///< Enable performance profiling
        bool enableValidation;                  ///< Enable physics validation

        // Voxel integration
        bool enableVoxelCollisions;             ///< Enable voxel block collisions
        bool enableVoxelPhysics;                ///< Enable voxel physics
        double voxelCollisionMargin;            ///< Voxel collision margin
        bool enableTerrainDeformation;          ///< Enable terrain deformation
    };

    /**
     * @struct PhysicsMetrics
     * @brief Performance metrics for physics engine
     */
    struct PhysicsMetrics {
        // Simulation metrics
        uint64_t totalSteps;                    ///< Total simulation steps
        double totalTime;                       ///< Total simulation time (ms)
        double averageStepTime;                 ///< Average step time (ms)
        double maxStepTime;                     ///< Maximum step time (ms)
        double minStepTime;                     ///< Minimum step time (ms)

        // Bodies and collisions
        uint32_t activeBodies;                  ///< Currently active bodies
        uint32_t sleepingBodies;                ///< Sleeping bodies
        uint32_t totalBodies;                   ///< Total rigid bodies
        uint64_t collisionsDetected;            ///< Total collisions detected
        uint64_t collisionsResolved;            ///< Total collisions resolved

        // Constraints
        uint32_t activeConstraints;             ///< Active constraints
        uint32_t totalConstraints;              ///< Total constraints
        uint64_t constraintIterations;          ///< Total constraint iterations
        double constraintSolveTime;             ///< Constraint solving time (ms)

        // Performance
        double broadPhaseTime;                  ///< Broad phase time (ms)
        double narrowPhaseTime;                 ///< Narrow phase time (ms)
        double solveTime;                       ///< Solver time (ms)
        double integrationTime;                 ///< Integration time (ms)
        double syncTime;                        ///< Synchronization time (ms)

        // Threading
        uint32_t activeThreads;                 ///< Active physics threads
        double threadUtilization;               ///< Thread utilization (0.0 - 1.0)
        uint64_t threadTasks;                   ///< Total threaded tasks

        // Memory
        size_t memoryUsage;                     ///< Physics memory usage (bytes)
        size_t peakMemoryUsage;                 ///< Peak memory usage (bytes)
        uint32_t allocations;                   ///< Total memory allocations
        uint32_t deallocations;                 ///< Total memory deallocations

        // Errors and warnings
        uint64_t numericalErrors;              ///< Numerical integration errors
        uint64_t solverFailures;               ///< Solver convergence failures
        uint64_t constraintViolations;         ///< Constraint violations
        uint64_t penetrationErrors;            ///< Penetration detection errors
    };

    /**
     * @struct PhysicsTask
     * @brief Physics computation task for multithreading
     */
    struct PhysicsTask {
        std::function<void()> function;        ///< Task function
        int priority;                          ///< Task priority
        std::string name;                      ///< Task name for debugging
        double timestamp;                      ///< Task creation time
        std::atomic<bool> completed;           ///< Completion flag
    };

    /**
     * @class PhysicsEngine
     * @brief Advanced physics engine for voxel worlds
     *
     * The PhysicsEngine provides a complete physics simulation system with:
     * - Rigid body dynamics with realistic motion
     * - Advanced collision detection and response
     * - Constraint systems for joints and connections
     * - Voxel integration for block-based worlds
     * - Multithreaded simulation for performance
     * - Debug visualization and profiling
     *
     * Key Features:
     * - Multiple physics solvers (PGS, Jacobi, XPBD)
     * - Various integration methods (Euler, Verlet, RK4)
     * - Broad-phase and narrow-phase collision detection
     * - Constraint-based rigid body simulation
     * - Voxel-block collision detection
     * - Sleeping objects for performance
     * - Multithreaded computation
     * - Real-time debugging and visualization
     */
    class PhysicsEngine {
    public:
        /**
         * @brief Constructor
         * @param config Physics configuration
         */
        explicit PhysicsEngine(const PhysicsConfig& config);

        /**
         * @brief Destructor
         */
        ~PhysicsEngine();

        /**
         * @brief Deleted copy constructor
         */
        PhysicsEngine(const PhysicsEngine&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        PhysicsEngine& operator=(const PhysicsEngine&) = delete;

        // Engine lifecycle

        /**
         * @brief Initialize the physics engine
         * @param world Parent world
         * @return true if successful, false otherwise
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown the physics engine
         */
        void Shutdown();

        /**
         * @brief Update physics simulation
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Perform a single physics step
         * @param timeStep Time step for simulation
         */
        void Step(double timeStep);

        // Body management

        /**
         * @brief Create a new rigid body
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
         * @brief Add rigid body to simulation
         * @param body Body to add
         * @return true if added, false otherwise
         */
        bool AddRigidBody(std::unique_ptr<RigidBody> body);

        /**
         * @brief Remove rigid body from simulation
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
         * @brief Get active rigid bodies count
         * @return Number of active bodies
         */
        size_t GetActiveBodiesCount() const;

        // Constraint management

        /**
         * @brief Add constraint between bodies
         * @param constraint Constraint to add
         * @return true if added, false otherwise
         */
        bool AddConstraint(std::unique_ptr<Constraint> constraint);

        /**
         * @brief Remove constraint
         * @param constraint Constraint to remove
         * @return true if removed, false otherwise
         */
        bool RemoveConstraint(Constraint* constraint);

        /**
         * @brief Get all constraints
         * @return Vector of constraint pointers
         */
        std::vector<Constraint*> GetConstraints() const;

        // Collision queries

        /**
         * @brief Perform raycast in the physics world
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
         * @brief Perform sphere overlap test
         * @param center Sphere center
         * @param radius Sphere radius
         * @param overlapping Output overlapping bodies
         * @return Number of overlapping bodies
         */
        size_t OverlapSphere(const glm::vec3& center, float radius,
                            std::vector<RigidBody*>& overlapping) const;

        /**
         * @brief Perform AABB overlap test
         * @param aabb AABB to test
         * @param overlapping Output overlapping bodies
         * @return Number of overlapping bodies
         */
        size_t OverlapAABB(const AABB& aabb, std::vector<RigidBody*>& overlapping) const;

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
         * @brief Update voxel collisions for area
         * @param minPos Minimum position
         * @param maxPos Maximum position
         * @return Number of voxel collisions updated
         */
        size_t UpdateVoxelCollisions(const glm::vec3& minPos, const glm::vec3& maxPos);

        /**
         * @brief Check voxel collision at position
         * @param position Position to check
         * @return true if collision exists, false otherwise
         */
        bool HasVoxelCollision(const glm::vec3& position) const;

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
        bool IsPaused() const { return m_paused; }

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
         * @brief Set gravity
         * @param gravity Gravity vector
         */
        void SetGravity(const glm::vec3& gravity);

        /**
         * @brief Get gravity
         * @return Current gravity vector
         */
        const glm::vec3& GetGravity() const { return m_config.gravity; }

        // Configuration

        /**
         * @brief Get physics configuration
         * @return Current configuration
         */
        const PhysicsConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set physics configuration
         * @param config New configuration
         */
        void SetConfig(const PhysicsConfig& config);

        /**
         * @brief Get physics metrics
         * @return Performance metrics
         */
        const PhysicsMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Debug and visualization

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

        // Profiling

        /**
         * @brief Get profiling data
         * @return Profiling information
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
         * @brief Perform broad phase collision detection
         */
        void PerformBroadPhase();

        /**
         * @brief Perform narrow phase collision detection
         */
        void PerformNarrowPhase();

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
         * @brief Synchronize physics state with game world
         */
        void SynchronizeState();

        /**
         * @brief Update sleeping bodies
         */
        void UpdateSleepingBodies();

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
         * @brief Validate physics state
         * @return true if valid, false otherwise
         */
        bool ValidatePhysicsState() const;

        /**
         * @brief Handle physics errors
         * @param error Error message
         */
        void HandlePhysicsError(const std::string& error);

        // Configuration and state
        PhysicsConfig m_config;                      ///< Physics configuration
        PhysicsMetrics m_metrics;                    ///< Performance metrics
        World* m_world;                             ///< Parent world

        // Core systems
        std::unique_ptr<CollisionSystem> m_collisionSystem;  ///< Collision detection
        std::unique_ptr<ConstraintSolver> m_constraintSolver; ///< Constraint solving
        std::unique_ptr<BroadPhase> m_broadPhase;    ///< Broad phase collision detection
        std::unique_ptr<NarrowPhase> m_narrowPhase;  ///< Narrow phase collision detection

        // Bodies and constraints
        std::unordered_map<uint32_t, std::unique_ptr<RigidBody>> m_rigidBodies;
        std::unordered_map<uint32_t, std::unique_ptr<Constraint>> m_constraints;
        mutable std::shared_mutex m_bodiesMutex;     ///< Bodies synchronization
        mutable std::shared_mutex m_constraintsMutex; ///< Constraints synchronization
        std::atomic<uint32_t> m_nextBodyId;         ///< Next body ID
        std::atomic<uint32_t> m_nextConstraintId;   ///< Next constraint ID

        // Voxel collisions
        std::unordered_map<glm::vec3, VoxelCollisionData> m_voxelCollisions;
        mutable std::shared_mutex m_voxelMutex;     ///< Voxel synchronization

        // Simulation state
        bool m_paused;                             ///< Simulation paused flag
        float m_timeScale;                         ///< Time scale multiplier
        double m_accumulator;                      ///< Time accumulator for fixed timestep
        double m_lastTime;                         ///< Last update time

        // Threading
        std::vector<std::thread> m_workerThreads;  ///< Worker threads
        std::atomic<bool> m_workersRunning;        ///< Workers running flag
        std::queue<PhysicsTask> m_taskQueue;       ///< Task queue
        mutable std::mutex m_taskMutex;            ///< Task synchronization
        std::condition_variable m_taskCondition;   ///< Task condition variable

        // Debug and profiling
        std::vector<DebugRenderData> m_debugRenderData;
        mutable std::shared_mutex m_debugMutex;    ///< Debug synchronization
        std::unordered_map<std::string, double> m_profileSections;
        mutable std::mutex m_profileMutex;         ///< Profile synchronization

        // Statistics
        uint64_t m_totalSteps;                     ///< Total simulation steps
        uint64_t m_totalCollisions;                ///< Total collisions processed
        uint64_t m_totalConstraintIterations;      ///< Total constraint iterations
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PHYSICS_PHYSICS_ENGINE_HPP
