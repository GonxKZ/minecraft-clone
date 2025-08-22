/**
 * @file ParallelTaskSystem.hpp
 * @brief VoxelCraft High-Level Parallel Task System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the ParallelTaskSystem class that provides high-level parallel
 * task management for the VoxelCraft game engine, including task graphs, dependencies,
 * parallel algorithms, data parallelism, and integration with game systems for
 * optimal parallel execution of complex game operations.
 */

#ifndef VOXELCRAFT_PARALLEL_PARALLEL_TASK_SYSTEM_HPP
#define VOXELCRAFT_PARALLEL_PARALLEL_TASK_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <deque>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <optional>
#include <algorithm>
#include <execution>
#include <numeric>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../core/System.hpp"

namespace VoxelCraft {

    // Forward declarations
    class ThreadPool;
    class SIMDMath;

    /**
     * @enum TaskGraphNodeType
     * @brief Types of task graph nodes
     */
    enum class TaskGraphNodeType {
        Task,                   ///< Regular task node
        Barrier,                ///< Synchronization barrier
        Conditional,            ///< Conditional execution
        Loop,                   ///< Loop node
        Fork,                   ///< Fork execution
        Join,                   ///< Join execution
        Custom                  ///< Custom node type
    };

    /**
     * @enum DataAccessPattern
     * @brief Data access patterns for parallel execution
     */
    enum class DataAccessPattern {
        ReadOnly,               ///< Read-only access
        WriteOnly,              ///< Write-only access
        ReadWrite,              ///< Read-write access
        Atomic,                 ///< Atomic operations
        None                    ///< No data access
    };

    /**
     * @enum ParallelPattern
     * @brief Common parallel execution patterns
     */
    enum class ParallelPattern {
        Map,                    ///< Map pattern (apply function to all elements)
        Reduce,                 ///< Reduce pattern (combine elements)
        Scan,                   ///< Scan/prefix sum pattern
        Sort,                   ///< Parallel sort
        Search,                 ///< Parallel search
        Transform,              ///< Transform pattern
        Custom                  ///< Custom pattern
    };

    /**
     * @struct TaskGraphNode
     * @brief Node in a task graph
     */
    struct TaskGraphNode {
        uint32_t nodeId;                            ///< Unique node identifier
        TaskGraphNodeType type;                     ///< Node type
        std::string name;                           ///< Node name

        // Task execution
        std::function<void()> taskFunction;         ///< Task function to execute
        std::vector<uint32_t> dependencies;         ///< Node dependencies
        std::vector<uint32_t> dependents;           ///< Nodes that depend on this

        // Data dependencies
        std::vector<DataAccessPattern> dataAccess;  ///< Data access patterns
        std::vector<std::string> dataDependencies;  ///< Named data dependencies

        // Execution control
        bool canExecute;                            ///< Node can be executed
        bool isExecuting;                            ///< Node is currently executing
        bool isCompleted;                            ///< Node execution completed
        double startTime;                            ///< Execution start time
        double endTime;                              ///< Execution end time
        double duration;                             ///< Execution duration

        // Conditional execution
        std::function<bool()> conditionFunction;    ///< Condition for execution
        bool conditionResult;                        ///< Condition evaluation result

        // Loop execution
        int loopIterations;                          ///< Number of loop iterations
        int currentIteration;                        ///< Current iteration
        std::function<void(int)> loopBody;           ///< Loop body function

        // Error handling
        bool hasError;                               ///< Node has error
        std::string errorMessage;                    ///< Error message

        TaskGraphNode()
            : nodeId(0)
            , type(TaskGraphNodeType::Task)
            , canExecute(false)
            , isExecuting(false)
            , isCompleted(false)
            , startTime(0.0)
            , endTime(0.0)
            , duration(0.0)
            , conditionResult(false)
            , loopIterations(0)
            , currentIteration(0)
            , hasError(false)
        {}
    };

    /**
     * @struct TaskGraph
     * @brief Directed acyclic graph of tasks
     */
    struct TaskGraph {
        uint32_t graphId;                           ///< Unique graph identifier
        std::string name;                           ///< Graph name
        std::unordered_map<uint32_t, TaskGraphNode> nodes; ///< Graph nodes
        std::vector<uint32_t> entryNodes;           ///< Entry point nodes
        std::vector<uint32_t> exitNodes;            ///< Exit point nodes

        // Execution state
        bool isExecuting;                           ///< Graph is executing
        bool isCompleted;                           ///< Graph execution completed
        double startTime;                           ///< Execution start time
        double endTime;                             ///< Execution end time
        double duration;                            ///< Total execution duration

        // Statistics
        size_t totalNodes;                          ///< Total number of nodes
        size_t executedNodes;                       ///< Number of executed nodes
        size_t failedNodes;                         ///< Number of failed nodes
        double averageNodeTime;                     ///< Average node execution time

        TaskGraph()
            : graphId(0)
            , isExecuting(false)
            , isCompleted(false)
            , startTime(0.0)
            , endTime(0.0)
            , duration(0.0)
            , totalNodes(0)
            , executedNodes(0)
            , failedNodes(0)
            , averageNodeTime(0.0)
        {}
    };

    /**
     * @struct ParallelAlgorithmParams
     * @brief Parameters for parallel algorithms
     */
    struct ParallelAlgorithmParams {
        size_t dataSize;                            ///< Size of data to process
        size_t chunkSize;                           ///< Processing chunk size
        int threadCount;                            ///< Number of threads to use
        bool enableSIMD;                            ///< Enable SIMD optimization
        bool enableCacheOptimization;               ///< Enable cache optimization
        size_t memoryAlignment;                     ///< Memory alignment requirement
        std::string algorithm;                      ///< Algorithm name

        ParallelAlgorithmParams()
            : dataSize(0)
            , chunkSize(1024)
            , threadCount(std::thread::hardware_concurrency())
            , enableSIMD(true)
            , enableCacheOptimization(true)
            , memoryAlignment(64)
        {}
    };

    /**
     * @struct ParallelTaskSystemStats
     * @brief Parallel task system performance statistics
     */
    struct ParallelTaskSystemStats {
        // Graph execution statistics
        uint64_t totalGraphsExecuted;               ///< Total graphs executed
        uint64_t successfulGraphs;                  ///< Successfully executed graphs
        uint64_t failedGraphs;                      ///< Failed graph executions
        double averageGraphTime;                    ///< Average graph execution time

        // Node execution statistics
        uint64_t totalNodesExecuted;                ///< Total nodes executed
        uint64_t successfulNodes;                   ///< Successfully executed nodes
        uint64_t failedNodes;                       ///< Failed node executions
        double averageNodeTime;                     ///< Average node execution time

        // Parallel algorithm statistics
        uint64_t mapOperations;                     ///< Map operations performed
        uint64_t reduceOperations;                  ///< Reduce operations performed
        uint64_t sortOperations;                    ///< Sort operations performed
        uint64_t searchOperations;                  ///< Search operations performed

        // Performance metrics
        double totalExecutionTime;                  ///< Total execution time
        double parallelEfficiency;                  ///< Parallel efficiency (0-1)
        double loadBalance;                         ///< Load balancing factor (0-1)
        size_t peakMemoryUsage;                     ///< Peak memory usage

        // SIMD statistics
        uint64_t simdOperations;                    ///< SIMD operations performed
        double simdSpeedup;                         ///< SIMD speedup factor
        size_t simdDataProcessed;                   ///< SIMD data processed

        // Cache statistics
        uint64_t cacheMisses;                       ///< Cache misses detected
        uint64_t cacheHits;                         ///< Cache hits
        double cacheHitRate;                        ///< Cache hit rate (0-1)
    };

    /**
     * @class ParallelTaskSystem
     * @brief High-level parallel task management system
     *
     * The ParallelTaskSystem class provides high-level parallel task management for the
     * VoxelCraft game engine, including task graphs with dependencies, parallel algorithms,
     * data parallelism, and seamless integration with game systems for optimal parallel
     * execution of complex game operations and computational tasks.
     *
     * Key Features:
     * - Task graph execution with automatic dependency resolution
     * - High-level parallel algorithms (map, reduce, sort, search)
     * - Data parallelism with SIMD optimization
     * - Task scheduling with load balancing
     * - Memory management for parallel operations
     * - Performance monitoring and bottleneck analysis
     * - Integration with game engine systems
     * - Cache-aware parallel processing
     * - Error handling and recovery
     *
     * The parallel task system is designed to make parallel programming accessible
     * while maintaining high performance and scalability for game development.
     */
    class ParallelTaskSystem : public System {
    public:
        /**
         * @brief Constructor
         * @param threadPool Thread pool instance
         */
        explicit ParallelTaskSystem(ThreadPool* threadPool);

        /**
         * @brief Destructor
         */
        ~ParallelTaskSystem();

        /**
         * @brief Deleted copy constructor
         */
        ParallelTaskSystem(const ParallelTaskSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        ParallelTaskSystem& operator=(const ParallelTaskSystem&) = delete;

        // Parallel task system lifecycle

        /**
         * @brief Initialize parallel task system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown parallel task system
         */
        void Shutdown();

        /**
         * @brief Update parallel task system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get parallel task system statistics
         * @return Current statistics
         */
        const ParallelTaskSystemStats& GetStats() const { return m_stats; }

        // Task graph management

        /**
         * @brief Create task graph
         * @param name Graph name
         * @return Graph ID or 0 if failed
         */
        uint32_t CreateTaskGraph(const std::string& name);

        /**
         * @brief Add task node to graph
         * @param graphId Graph ID
         * @param taskFunction Task function
         * @param dependencies Node dependencies
         * @param name Node name
         * @return Node ID or 0 if failed
         */
        uint32_t AddTaskNode(uint32_t graphId,
                           std::function<void()> taskFunction,
                           const std::vector<uint32_t>& dependencies = {},
                           const std::string& name = "");

        /**
         * @brief Add conditional node to graph
         * @param graphId Graph ID
         * @param conditionFunction Condition function
         * @param taskFunction Task function (executed if condition true)
         * @param dependencies Node dependencies
         * @param name Node name
         * @return Node ID or 0 if failed
         */
        uint32_t AddConditionalNode(uint32_t graphId,
                                  std::function<bool()> conditionFunction,
                                  std::function<void()> taskFunction,
                                  const std::vector<uint32_t>& dependencies = {},
                                  const std::string& name = "");

        /**
         * @brief Add loop node to graph
         * @param graphId Graph ID
         * @param iterations Number of iterations
         * @param loopBody Loop body function
         * @param dependencies Node dependencies
         * @param name Node name
         * @return Node ID or 0 if failed
         */
        uint32_t AddLoopNode(uint32_t graphId,
                           int iterations,
                           std::function<void(int)> loopBody,
                           const std::vector<uint32_t>& dependencies = {},
                           const std::string& name = "");

        /**
         * @brief Execute task graph
         * @param graphId Graph ID
         * @return true if execution started, false otherwise
         */
        bool ExecuteTaskGraph(uint32_t graphId);

        /**
         * @brief Wait for task graph completion
         * @param graphId Graph ID
         * @param timeout Timeout in seconds (0 = infinite)
         * @return true if completed, false if timeout or failed
         */
        bool WaitForTaskGraph(uint32_t graphId, float timeout = 0.0f);

        /**
         * @brief Get task graph status
         * @param graphId Graph ID
         * @return Task graph status
         */
        std::optional<TaskGraph> GetTaskGraphStatus(uint32_t graphId) const;

        /**
         * @brief Cancel task graph execution
         * @param graphId Graph ID
         * @return true if cancelled, false otherwise
         */
        bool CancelTaskGraph(uint32_t graphId);

        // High-level parallel algorithms

        /**
         * @brief Execute parallel map operation
         * @param input Input data
         * @param function Map function
         * @param params Algorithm parameters
         * @return std::future for the result
         */
        template<typename T, typename U>
        std::future<std::vector<U>> ParallelMap(const std::vector<T>& input,
                                               std::function<U(const T&)> function,
                                               const ParallelAlgorithmParams& params = ParallelAlgorithmParams());

        /**
         * @brief Execute parallel reduce operation
         * @param input Input data
         * @param function Reduce function
         * @param initialValue Initial value
         * @param params Algorithm parameters
         * @return std::future for the result
         */
        template<typename T, typename U>
        std::future<U> ParallelReduce(const std::vector<T>& input,
                                     std::function<U(U, const T&)> function,
                                     U initialValue,
                                     const ParallelAlgorithmParams& params = ParallelAlgorithmParams());

        /**
         * @brief Execute parallel sort operation
         * @param data Data to sort
         * @param comparator Comparison function
         * @param params Algorithm parameters
         * @return std::future for completion
         */
        template<typename T>
        std::future<void> ParallelSort(std::vector<T>& data,
                                      std::function<bool(const T&, const T&)> comparator,
                                      const ParallelAlgorithmParams& params = ParallelAlgorithmParams());

        /**
         * @brief Execute parallel search operation
         * @param data Data to search
         * @param predicate Search predicate
         * @param params Algorithm parameters
         * @return std::future for search results
         */
        template<typename T>
        std::future<std::vector<T>> ParallelSearch(const std::vector<T>& data,
                                                  std::function<bool(const T&)> predicate,
                                                  const ParallelAlgorithmParams& params = ParallelAlgorithmParams());

        /**
         * @brief Execute parallel transform operation
         * @param input Input data
         * @param output Output data
         * @param function Transform function
         * @param params Algorithm parameters
         * @return std::future for completion
         */
        template<typename T, typename U>
        std::future<void> ParallelTransform(const std::vector<T>& input,
                                          std::vector<U>& output,
                                          std::function<U(const T&)> function,
                                          const ParallelAlgorithmParams& params = ParallelAlgorithmParams());

        // Game-specific parallel operations

        /**
         * @brief Execute parallel world update
         * @param worldChunks Chunks to update
         * @param updateFunction Update function
         * @return std::future for completion
         */
        std::future<void> ParallelWorldUpdate(const std::vector<uint32_t>& worldChunks,
                                            std::function<void(uint32_t)> updateFunction);

        /**
         * @brief Execute parallel physics simulation
         * @param physicsObjects Physics objects to simulate
         * @param simulationFunction Simulation function
         * @param deltaTime Time step
         * @return std::future for completion
         */
        std::future<void> ParallelPhysicsSimulation(const std::vector<uint32_t>& physicsObjects,
                                                  std::function<void(uint32_t, float)> simulationFunction,
                                                  float deltaTime);

        /**
         * @brief Execute parallel AI update
         * @param aiEntities AI entities to update
         * @param updateFunction Update function
         * @return std::future for completion
         */
        std::future<void> ParallelAIUpdate(const std::vector<uint32_t>& aiEntities,
                                         std::function<void(uint32_t)> updateFunction);

        /**
         * @brief Execute parallel rendering
         * @param renderCommands Render commands to execute
         * @param renderFunction Render function
         * @return std::future for completion
         */
        std::future<void> ParallelRendering(const std::vector<uint32_t>& renderCommands,
                                          std::function<void(uint32_t)> renderFunction);

        // Memory management for parallel operations

        /**
         * @brief Allocate memory for parallel operation
         * @param size Memory size in bytes
         * @param alignment Memory alignment
         * @return Aligned memory pointer
         */
        void* AllocateParallelMemory(size_t size, size_t alignment = 64);

        /**
         * @brief Deallocate parallel memory
         * @param memory Memory pointer
         */
        void DeallocateParallelMemory(void* memory);

        /**
         * @brief Get memory usage for parallel operations
         * @return Memory usage in bytes
         */
        size_t GetParallelMemoryUsage() const;

        // SIMD operations

        /**
         * @brief Execute SIMD-optimized operation
         * @param data Input data
         * @param operation SIMD operation function
         * @return std::future for completion
         */
        template<typename T>
        std::future<void> ExecuteSIMDOperation(std::vector<T>& data,
                                             std::function<void(T*, size_t)> operation);

        /**
         * @brief Check SIMD support
         * @return SIMD capabilities
         */
        std::unordered_map<std::string, bool> GetSIMDSupport() const;

        /**
         * @brief Enable SIMD optimization
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableSIMDOptimization(bool enabled);

        // Performance analysis

        /**
         * @brief Get performance analysis report
         * @return Detailed performance analysis
         */
        std::string GetPerformanceAnalysis() const;

        /**
         * @brief Get bottleneck identification
         * @return Bottleneck analysis report
         */
        std::string GetBottleneckAnalysis() const;

        /**
         * @brief Get optimization recommendations
         * @return Optimization recommendations
         */
        std::vector<std::string> GetOptimizationRecommendations() const;

        // Utility functions

        /**
         * @brief Get optimal chunk size for data
         * @param dataSize Total data size
         * @param threadCount Number of threads
         * @return Optimal chunk size
         */
        size_t GetOptimalChunkSize(size_t dataSize, int threadCount) const;

        /**
         * @brief Get optimal thread count for operation
         * @param operationType Operation type
         * @param dataSize Data size
         * @return Optimal thread count
         */
        int GetOptimalThreadCount(const std::string& operationType, size_t dataSize) const;

        /**
         * @brief Calculate parallel efficiency
         * @param sequentialTime Sequential execution time
         * @param parallelTime Parallel execution time
         * @param threadCount Number of threads
         * @return Parallel efficiency (0-1)
         */
        float CalculateParallelEfficiency(double sequentialTime, double parallelTime, int threadCount) const;

        /**
         * @brief Validate parallel task system state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Optimize parallel task system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Execute task graph node
         * @param graphId Graph ID
         * @param nodeId Node ID
         * @return true if successful, false otherwise
         */
        bool ExecuteTaskGraphNode(uint32_t graphId, uint32_t nodeId);

        /**
         * @brief Check if node can be executed
         * @param graphId Graph ID
         * @param nodeId Node ID
         * @return true if can execute, false otherwise
         */
        bool CanExecuteNode(uint32_t graphId, uint32_t nodeId) const;

        /**
         * @brief Update task graph execution
         * @param graphId Graph ID
         */
        void UpdateTaskGraphExecution(uint32_t graphId);

        /**
         * @brief Process completed task graph node
         * @param graphId Graph ID
         * @param nodeId Node ID
         */
        void ProcessCompletedNode(uint32_t graphId, uint32_t nodeId);

        /**
         * @brief Split data for parallel processing
         * @param dataSize Total data size
         * @param chunkCount Number of chunks
         * @return Vector of chunk ranges
         */
        std::vector<std::pair<size_t, size_t>> SplitDataForParallel(size_t dataSize, int chunkCount) const;

        /**
         * @brief Merge parallel results
         * @param results Vector of partial results
         * @param mergeFunction Merge function
         * @return Merged result
         */
        template<typename T, typename U>
        T MergeParallelResults(const std::vector<U>& results, std::function<T(const std::vector<U>&)> mergeFunction) const;

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle parallel task system errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Parallel task system data
        ThreadPool* m_threadPool;                    ///< Thread pool instance
        ParallelTaskSystemStats m_stats;             ///< Performance statistics

        // Task graph management
        std::unordered_map<uint32_t, TaskGraph> m_taskGraphs; ///< Active task graphs
        mutable std::shared_mutex m_graphsMutex;     ///< Graph synchronization

        // Memory management
        std::vector<void*> m_allocatedMemory;        ///< Allocated memory blocks
        mutable std::shared_mutex m_memoryMutex;     ///< Memory synchronization
        size_t m_totalAllocatedMemory;               ///< Total allocated memory

        // SIMD support
        std::unique_ptr<SIMDMath> m_simdMath;        ///< SIMD math operations
        bool m_simdEnabled;                          ///< SIMD optimization enabled

        // State tracking
        bool m_isInitialized;                        ///< System is initialized
        double m_lastUpdateTime;                     ///< Last update time
        std::string m_lastError;                     ///< Last error message

        static std::atomic<uint32_t> s_nextGraphId;  ///< Next graph ID counter
        static std::atomic<uint32_t> s_nextNodeId;   ///< Next node ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PARALLEL_PARALLEL_TASK_SYSTEM_HPP
