/**
 * @file ConstraintSolver.hpp
 * @brief VoxelCraft Physics Constraint Solver System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the ConstraintSolver class that handles the solving
 * of physics constraints using various numerical methods and algorithms.
 */

#ifndef VOXELCRAFT_PHYSICS_CONSTRAINT_SOLVER_HPP
#define VOXELCRAFT_PHYSICS_CONSTRAINT_SOLVER_HPP

#include <memory>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>

#include "../core/Config.hpp"
#include "Constraint.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Constraint;
    class RigidBody;
    class PhysicsEngine;

    /**
     * @enum SolverType
     * @brief Type of constraint solver
     */
    enum class SolverType {
        PGS,                    ///< Projected Gauss-Seidel (default)
        Jacobi,                 ///< Jacobi iteration
        Impulse,                ///< Impulse-based solver
        XPBD,                   ///< Extended Position Based Dynamics
        SequentialImpulse,      ///< Sequential impulse solver
        Hybrid,                 ///< Hybrid approach
        Custom                  ///< Custom solver
    };

    /**
     * @enum SolverMode
     * @brief Solver operational mode
     */
    enum class SolverMode {
        Sequential,             ///< Solve constraints sequentially
        Parallel,               ///< Solve constraints in parallel
        Batched,                ///< Solve constraints in batches
        Island,                 ///< Solve constraint islands
        Adaptive                ///< Adaptive solving strategy
    };

    /**
     * @enum SolverPriority
     * @brief Constraint solving priority
     */
    enum class SolverPriority {
        High,                   ///< High priority constraint
        Normal,                 ///< Normal priority constraint
        Low,                    ///< Low priority constraint
        Background              ///< Background priority constraint
    };

    /**
     * @struct SolverConfig
     * @brief Constraint solver configuration
     */
    struct SolverConfig {
        // Basic settings
        SolverType solverType;              ///< Solver algorithm type
        SolverMode solverMode;              ///< Solver operational mode

        // Performance settings
        int maxIterations;                  ///< Maximum solver iterations
        int minIterations;                  ///< Minimum solver iterations
        double timeStep;                    ///< Solver timestep
        double warmStartFactor;             ///< Warm starting factor (0.0 - 1.0)

        // Convergence settings
        double convergenceThreshold;        ///< Convergence threshold
        double errorReductionParameter;     ///< Error reduction parameter (SOR)
        bool useSuccessiveOverRelaxation;   ///< Use successive over-relaxation

        // Island settings (for island-based solving)
        bool enableIslandSplitting;         ///< Enable constraint island splitting
        int minIslandSize;                  ///< Minimum island size
        int maxIslandSize;                  ///< Maximum island size
        bool mergeSmallIslands;             ///< Merge small islands

        // Parallel settings
        bool enableParallelSolving;         ///< Enable parallel constraint solving
        int workerThreads;                  ///< Number of solver worker threads
        int batchSize;                      ///< Batch size for parallel processing
        bool useSIMD;                       ///< Use SIMD instructions

        // Advanced settings
        bool enableFriction;                ///< Enable friction in solving
        bool enableWarmStarting;            ///< Enable constraint warm starting
        bool enableCaching;                 ///< Enable solution caching
        bool enablePreprocessing;           ///< Enable constraint preprocessing

        // Stabilization
        double baumgarteFactor;             ///< Baumgarte stabilization factor
        double velocityDamping;             ///< Velocity damping factor
        double positionDamping;             ///< Position damping factor

        // Debugging
        bool enableDebugDrawing;            ///< Enable solver debug drawing
        bool enableProfiling;               ///< Enable performance profiling
        bool enableValidation;              ///< Enable solver validation
    };

    /**
     * @struct SolverMetrics
     * @brief Performance metrics for constraint solver
     */
    struct SolverMetrics {
        // Iteration metrics
        uint64_t totalIterations;           ///< Total solver iterations
        uint64_t totalSteps;                ///< Total solving steps
        double totalSolveTime;              ///< Total time spent solving
        double averageSolveTime;            ///< Average solve time (ms)
        double maxSolveTime;                ///< Maximum solve time (ms)

        // Convergence metrics
        uint64_t convergenceCount;          ///< Number of converged solutions
        uint64_t divergenceCount;           ///< Number of diverged solutions
        double averageError;                ///< Average constraint error
        double maxError;                    ///< Maximum constraint error
        double convergenceRate;             ///< Convergence rate (0.0 - 1.0)

        // Constraint metrics
        uint32_t activeConstraints;         ///< Currently active constraints
        uint32_t solvedConstraints;         ///< Constraints solved this frame
        uint32_t brokenConstraints;         ///< Constraints broken this frame
        uint64_t totalConstraintEvaluations; ///< Total constraint evaluations

        // Island metrics
        uint32_t islandCount;               ///< Number of constraint islands
        uint32_t averageIslandSize;         ///< Average island size
        uint32_t maxIslandSize;             ///< Maximum island size
        double islandSplitTime;             ///< Time spent splitting islands

        // Parallel metrics
        uint32_t activeThreads;             ///< Active solver threads
        double threadUtilization;           ///< Thread utilization (0.0 - 1.0)
        uint64_t parallelTasks;             ///< Total parallel tasks
        double parallelEfficiency;          ///< Parallel efficiency

        // Memory metrics
        size_t memoryUsage;                 ///< Current memory usage
        size_t peakMemoryUsage;             ///< Peak memory usage
        uint32_t allocations;               ///< Memory allocations
        uint32_t deallocations;             ///< Memory deallocations

        // Cache metrics
        uint64_t cacheHits;                 ///< Solution cache hits
        uint64_t cacheMisses;               ///< Solution cache misses
        double cacheHitRate;                ///< Cache hit rate (0.0 - 1.0)
        uint32_t cachedSolutions;           ///< Number of cached solutions

        // Error metrics
        uint64_t numericalErrors;           ///< Numerical computation errors
        uint64_t jacobianErrors;            ///< Jacobian computation errors
        uint64_t matrixErrors;              ///< Matrix operation errors
        uint64_t preconditionerErrors;      ///< Preconditioner errors
    };

    /**
     * @struct ConstraintIsland
     * @brief Group of connected constraints for efficient solving
     */
    struct ConstraintIsland {
        std::vector<Constraint*> constraints;  ///< Constraints in island
        std::vector<RigidBody*> bodies;        ///< Bodies affected by constraints
        uint32_t islandId;                     ///< Unique island ID
        float totalMass;                       ///< Total mass of bodies in island
        bool isActive;                         ///< Island is active
        bool needsSplitting;                   ///< Island needs to be split
        double lastSolveTime;                  ///< Last solve timestamp
    };

    /**
     * @struct SolverTask
     * @brief Task for parallel constraint solving
     */
    struct SolverTask {
        std::function<void()> function;        ///< Task function
        SolverPriority priority;               ///< Task priority
        std::string name;                      ///< Task name for debugging
        double timestamp;                      ///< Task creation timestamp
        std::atomic<bool> completed;           ///< Completion flag
        ConstraintIsland* island;              ///< Associated constraint island
    };

    /**
     * @class ConstraintSolver
     * @brief Advanced constraint solver for physics simulation
     *
     * The ConstraintSolver handles the solving of physics constraints using
     * various numerical methods. It supports multiple solver algorithms,
     * parallel solving, constraint islands, and advanced optimization techniques.
     *
     * Key Features:
     * - Multiple solver algorithms (PGS, Jacobi, XPBD, Sequential Impulse)
     * - Constraint island splitting for efficiency
     * - Parallel constraint solving
     * - Warm starting and caching
     * - Successive over-relaxation (SOR)
     * - Baumgarte stabilization
     * - Comprehensive performance profiling
     *
     * Supported Solver Types:
     * - **PGS (Projected Gauss-Seidel)**: Iterative method with projection
     * - **Jacobi**: Parallel iterative method
     * - **Sequential Impulse**: Impulse-based sequential solving
     * - **XPBD (Extended Position Based Dynamics)**: Position-based approach
     * - **Hybrid**: Adaptive combination of methods
     */
    class ConstraintSolver {
    public:
        /**
         * @brief Constructor
         * @param config Solver configuration
         */
        explicit ConstraintSolver(const SolverConfig& config);

        /**
         * @brief Destructor
         */
        ~ConstraintSolver();

        /**
         * @brief Deleted copy constructor
         */
        ConstraintSolver(const ConstraintSolver&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        ConstraintSolver& operator=(const ConstraintSolver&) = delete;

        // Solver lifecycle

        /**
         * @brief Initialize constraint solver
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown constraint solver
         */
        void Shutdown();

        /**
         * @brief Update solver
         * @param deltaTime Time elapsed
         */
        void Update(double deltaTime);

        // Constraint solving

        /**
         * @brief Solve all constraints
         * @param constraints Vector of constraints to solve
         * @param timeStep Simulation time step
         * @return true if successful, false otherwise
         */
        bool SolveConstraints(const std::vector<Constraint*>& constraints, double timeStep);

        /**
         * @brief Solve constraint islands
         * @param islands Vector of constraint islands
         * @param timeStep Simulation time step
         * @return Number of islands solved
         */
        size_t SolveIslands(const std::vector<ConstraintIsland>& islands, double timeStep);

        /**
         * @brief Solve single constraint
         * @param constraint Constraint to solve
         * @param timeStep Simulation time step
         * @param iteration Current iteration
         * @return true if constraint was solved, false otherwise
         */
        bool SolveSingleConstraint(Constraint* constraint, double timeStep, int iteration);

        /**
         * @brief Prepare constraints for solving
         * @param constraints Vector of constraints
         * @param timeStep Simulation time step
         */
        void PrepareConstraints(const std::vector<Constraint*>& constraints, double timeStep);

        /**
         * @brief Finalize constraint solving
         * @param constraints Vector of constraints
         */
        void FinalizeConstraints(const std::vector<Constraint*>& constraints);

        // Island management

        /**
         * @brief Build constraint islands
         * @param constraints Vector of constraints
         * @param bodies Vector of rigid bodies
         * @param islands Output vector of islands
         * @return Number of islands created
         */
        size_t BuildIslands(const std::vector<Constraint*>& constraints,
                           const std::vector<RigidBody*>& bodies,
                           std::vector<ConstraintIsland>& islands);

        /**
         * @brief Split constraint island
         * @param island Island to split
         * @param subIslands Output vector of sub-islands
         * @return Number of sub-islands created
         */
        size_t SplitIsland(const ConstraintIsland& island, std::vector<ConstraintIsland>& subIslands);

        /**
         * @brief Merge small islands
         * @param islands Vector of islands to merge
         * @param maxIslandSize Maximum island size
         * @return Number of merges performed
         */
        size_t MergeSmallIslands(std::vector<ConstraintIsland>& islands, size_t maxIslandSize);

        /**
         * @brief Get active constraint islands
         * @return Vector of active islands
         */
        std::vector<ConstraintIsland> GetActiveIslands() const;

        // Parallel solving

        /**
         * @brief Enable parallel solving
         * @param enabled Enable state
         * @return Previous state
         */
        bool SetParallelSolvingEnabled(bool enabled);

        /**
         * @brief Check if parallel solving is enabled
         * @return true if enabled, false otherwise
         */
        bool IsParallelSolvingEnabled() const { return m_config.enableParallelSolving; }

        /**
         * @brief Set number of worker threads
         * @param threads Number of threads
         */
        void SetWorkerThreads(int threads);

        /**
         * @brief Get number of worker threads
         * @return Number of threads
         */
        int GetWorkerThreads() const { return m_config.workerThreads; }

        // Configuration

        /**
         * @brief Get solver configuration
         * @return Current configuration
         */
        const SolverConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set solver configuration
         * @param config New configuration
         */
        void SetConfig(const SolverConfig& config);

        /**
         * @brief Get solver type
         * @return Current solver type
         */
        SolverType GetSolverType() const { return m_config.solverType; }

        /**
         * @brief Set solver type
         * @param type New solver type
         */
        void SetSolverType(SolverType type);

        /**
         * @brief Set maximum iterations
         * @param iterations Maximum iterations
         */
        void SetMaxIterations(int iterations);

        /**
         * @brief Get maximum iterations
         * @return Maximum iterations
         */
        int GetMaxIterations() const { return m_config.maxIterations; }

        // Warm starting and caching

        /**
         * @brief Enable warm starting
         * @param enabled Enable state
         * @return Previous state
         */
        bool SetWarmStartingEnabled(bool enabled);

        /**
         * @brief Check if warm starting is enabled
         * @return true if enabled, false otherwise
         */
        bool IsWarmStartingEnabled() const { return m_config.enableWarmStarting; }

        /**
         * @brief Clear solution cache
         */
        void ClearCache();

        /**
         * @brief Get cache size
         * @return Number of cached solutions
         */
        size_t GetCacheSize() const;

        // Metrics and profiling

        /**
         * @brief Get solver metrics
         * @return Performance metrics
         */
        const SolverMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

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

        // Debug and visualization

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
         * @brief Validate solver state
         * @return true if valid, false otherwise
         */
        bool ValidateSolverState() const;

        /**
         * @brief Get solver status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize solver performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> OptimizeSolver();

    private:
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
         * @brief Process solver tasks
         */
        void ProcessTasks();

        /**
         * @brief Solve constraints using PGS algorithm
         * @param constraints Vector of constraints
         * @param timeStep Simulation time step
         * @return true if successful, false otherwise
         */
        bool SolvePGS(const std::vector<Constraint*>& constraints, double timeStep);

        /**
         * @brief Solve constraints using Jacobi algorithm
         * @param constraints Vector of constraints
         * @param timeStep Simulation time step
         * @return true if successful, false otherwise
         */
        bool SolveJacobi(const std::vector<Constraint*>& constraints, double timeStep);

        /**
         * @brief Solve constraints using sequential impulse
         * @param constraints Vector of constraints
         * @param timeStep Simulation time step
         * @return true if successful, false otherwise
         */
        bool SolveSequentialImpulse(const std::vector<Constraint*>& constraints, double timeStep);

        /**
         * @brief Solve constraints using XPBD
         * @param constraints Vector of constraints
         * @param timeStep Simulation time step
         * @return true if successful, false otherwise
         */
        bool SolveXPBD(const std::vector<Constraint*>& constraints, double timeStep);

        /**
         * @brief Apply warm starting to constraints
         * @param constraints Vector of constraints
         */
        void ApplyWarmStarting(const std::vector<Constraint*>& constraints);

        /**
         * @brief Build constraint graph for island detection
         * @param constraints Vector of constraints
         * @param bodies Vector of bodies
         * @return Constraint graph
         */
        std::unordered_map<uint32_t, std::vector<uint32_t>> BuildConstraintGraph(
            const std::vector<Constraint*>& constraints,
            const std::vector<RigidBody*>& bodies);

        /**
         * @brief Find connected components in constraint graph
         * @param graph Constraint graph
         * @param components Output vector of components
         */
        void FindConnectedComponents(const std::unordered_map<uint32_t, std::vector<uint32_t>>& graph,
                                   std::vector<std::vector<uint32_t>>& components);

        /**
         * @brief Update performance metrics
         * @param solveTime Time spent solving
         * @param iterations Number of iterations
         * @param constraintsSolved Number of constraints solved
         */
        void UpdateMetrics(double solveTime, int iterations, size_t constraintsSolved);

        /**
         * @brief Collect debug render data
         */
        void CollectDebugRenderData();

        /**
         * @brief Handle solver errors
         * @param error Error message
         */
        void HandleSolverError(const std::string& error);

        // Configuration and state
        SolverConfig m_config;                      ///< Solver configuration
        SolverMetrics m_metrics;                    ///< Performance metrics

        // Constraint management
        std::vector<ConstraintIsland> m_activeIslands;  ///< Active constraint islands
        mutable std::shared_mutex m_islandsMutex;   ///< Islands synchronization

        // Cache system
        std::unordered_map<uint32_t, ConstraintState> m_solutionCache;
        mutable std::shared_mutex m_cacheMutex;     ///< Cache synchronization

        // Threading
        std::vector<std::thread> m_workerThreads;  ///< Worker threads
        std::atomic<bool> m_workersRunning;        ///< Workers running flag
        std::queue<SolverTask> m_taskQueue;        ///< Task queue
        mutable std::mutex m_taskMutex;            ///< Task synchronization
        std::condition_variable m_taskCondition;   ///< Task condition variable

        // Profiling
        std::unordered_map<std::string, double> m_profileSections;
        mutable std::mutex m_profileMutex;         ///< Profile synchronization
        std::vector<DebugRenderData> m_debugRenderData;
        mutable std::shared_mutex m_debugMutex;    ///< Debug synchronization

        // Solver state
        bool m_isInitialized;                       ///< Solver is initialized
        double m_lastUpdateTime;                    ///< Last update timestamp
        uint64_t m_solveCount;                      ///< Total solve operations
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PHYSICS_CONSTRAINT_SOLVER_HPP
