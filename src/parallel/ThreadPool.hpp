/**
 * @file ThreadPool.hpp
 * @brief VoxelCraft Advanced Thread Pool with Work-Stealing
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the ThreadPool class that provides advanced thread pool
 * functionality for the VoxelCraft game engine, including work-stealing algorithms,
 * thread affinity, priority scheduling, and performance monitoring for optimal
 * parallel execution of game systems and tasks.
 */

#ifndef VOXELCRAFT_PARALLEL_THREAD_POOL_HPP
#define VOXELCRAFT_PARALLEL_THREAD_POOL_HPP

#include <memory>
#include <vector>
#include <queue>
#include <deque>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <future>
#include <chrono>
#include <optional>
#include <array>
#include <unordered_map>

#include "../core/Config.hpp"
#include "../core/System.hpp"

namespace VoxelCraft {

    /**
     * @enum TaskPriority
     * @brief Task execution priorities
     */
    enum class TaskPriority {
        Critical,               ///< Critical priority (execute immediately)
        High,                   ///< High priority
        Normal,                 ///< Normal priority
        Low,                    ///< Low priority
        Background              ///< Background priority (can be delayed)
    };

    /**
     * @enum ThreadAffinity
     * @brief CPU thread affinity settings
     */
    enum class ThreadAffinity {
        None,                   ///< No specific affinity
        Core,                   ///< Pin to specific core
        NUMA,                   ///< NUMA-aware placement
        Performance,            ///< Performance core preference
        Efficiency              ///< Efficiency core preference
    };

    /**
     * @enum WorkStealingStrategy
     * @brief Work-stealing algorithm strategies
     */
    enum class WorkStealingStrategy {
        LIFO,                   ///< Last In, First Out (stack-based)
        FIFO,                   ///< First In, First Out (queue-based)
        PriorityBased,          ///< Priority-based stealing
        Adaptive,               ///< Adaptive strategy based on load
        Custom                  ///< Custom strategy
    };

    /**
     * @struct ThreadPoolConfig
     * @brief Thread pool configuration settings
     */
    struct ThreadPoolConfig {
        // Basic settings
        int threadCount;                            ///< Number of worker threads
        int maxThreadCount;                         ///< Maximum number of threads
        bool enableDynamicScaling;                  ///< Enable dynamic thread scaling

        // Scheduling settings
        WorkStealingStrategy stealingStrategy;      ///< Work-stealing strategy
        bool enablePriorityScheduling;              ///< Enable priority-based scheduling
        int maxTaskQueueSize;                       ///< Maximum task queue size
        int maxBackgroundTasks;                     ///< Maximum background tasks

        // Performance settings
        int taskBatchSize;                          ///< Task batch processing size
        bool enableTaskGrouping;                    ///< Enable task grouping optimization
        bool enableLoadBalancing;                   ///< Enable load balancing
        float targetUtilization;                    ///< Target CPU utilization (0-1)

        // Thread settings
        ThreadAffinity threadAffinity;              ///< Thread CPU affinity
        int threadPriority;                         ///< Thread priority level
        size_t threadStackSize;                     ///< Thread stack size
        bool enableThreadNaming;                    ///< Enable thread naming

        // Monitoring settings
        bool enablePerformanceMonitoring;           ///< Enable performance monitoring
        float monitoringInterval;                   ///< Monitoring update interval
        bool enableTaskProfiling;                   ///< Enable task profiling
        int maxProfiledTasks;                       ///< Maximum profiled tasks

        // Error handling
        bool enableTaskTimeouts;                    ///< Enable task timeout detection
        float defaultTaskTimeout;                   ///< Default task timeout (seconds)
        bool enableDeadlockDetection;               ///< Enable deadlock detection
        float deadlockCheckInterval;                ///< Deadlock check interval

        ThreadPoolConfig()
            : threadCount(std::thread::hardware_concurrency())
            , maxThreadCount(std::thread::hardware_concurrency() * 2)
            , enableDynamicScaling(true)
            , stealingStrategy(WorkStealingStrategy::Adaptive)
            , enablePriorityScheduling(true)
            , maxTaskQueueSize(10000)
            , maxBackgroundTasks(1000)
            , taskBatchSize(64)
            , enableTaskGrouping(true)
            , enableLoadBalancing(true)
            , targetUtilization(0.8f)
            , threadAffinity(ThreadAffinity::Performance)
            , threadPriority(0)
            , threadStackSize(1048576)  // 1MB
            , enableThreadNaming(true)
            , enablePerformanceMonitoring(true)
            , monitoringInterval(1.0f)
            , enableTaskProfiling(true)
            , maxProfiledTasks(1000)
            , enableTaskTimeouts(true)
            , defaultTaskTimeout(30.0f)
            , enableDeadlockDetection(true)
            , deadlockCheckInterval(5.0f)
        {}
    };

    /**
     * @struct Task
     * @brief Thread pool task descriptor
     */
    struct Task {
        uint32_t taskId;                            ///< Unique task identifier
        TaskPriority priority;                      ///< Task priority
        std::function<void()> function;             ///< Task function
        std::string name;                           ///< Task name/description
        uint32_t groupId;                           ///< Task group identifier

        // Timing information
        double submitTime;                          ///< Task submission time
        double startTime;                           ///< Task start time
        double endTime;                             ///< Task completion time
        double duration;                            ///< Task execution duration

        // Dependencies
        std::vector<uint32_t> dependencies;         ///< Task dependencies
        std::vector<uint32_t> dependents;           ///< Tasks that depend on this

        // State
        bool completed;                             ///< Task completion flag
        bool started;                               ///< Task started flag
        bool cancelled;                             ///< Task cancelled flag
        std::string errorMessage;                   ///< Error message if failed

        // Profiling
        size_t memoryUsed;                          ///< Memory used by task
        int threadId;                               ///< Thread that executed task
        float cpuUtilization;                       ///< CPU utilization during task

        Task()
            : taskId(0)
            , priority(TaskPriority::Normal)
            , groupId(0)
            , submitTime(0.0)
            , startTime(0.0)
            , endTime(0.0)
            , duration(0.0)
            , completed(false)
            , started(false)
            , cancelled(false)
            , memoryUsed(0)
            , threadId(-1)
            , cpuUtilization(0.0f)
        {}
    };

    /**
     * @struct ThreadInfo
     * @brief Worker thread information
     */
    struct ThreadInfo {
        int threadId;                               ///< Thread identifier
        std::thread::id systemId;                   ///< System thread ID
        std::string name;                           ///< Thread name
        ThreadAffinity affinity;                    ///< CPU affinity

        // Performance metrics
        uint64_t tasksExecuted;                     ///< Total tasks executed
        double totalExecutionTime;                  ///< Total execution time
        double averageTaskTime;                     ///< Average task execution time
        float utilization;                          ///< CPU utilization (0-1)

        // Queue information
        size_t localQueueSize;                      ///< Local task queue size
        size_t stolenTasks;                         ///< Tasks stolen from other threads
        size_t sharedTasks;                         ///< Tasks taken from shared queue

        // State
        bool isActive;                              ///< Thread is active
        bool isSleeping;                            ///< Thread is sleeping
        double lastActivity;                        ///< Last activity timestamp

        ThreadInfo()
            : threadId(-1)
            , tasksExecuted(0)
            , totalExecutionTime(0.0)
            , averageTaskTime(0.0)
            , utilization(0.0f)
            , localQueueSize(0)
            , stolenTasks(0)
            , sharedTasks(0)
            , isActive(false)
            , isSleeping(false)
            , lastActivity(0.0)
        {}
    };

    /**
     * @struct ThreadPoolStats
     * @brief Thread pool performance statistics
     */
    struct ThreadPoolStats {
        // Pool statistics
        int activeThreads;                          ///< Number of active threads
        int totalThreads;                           ///< Total number of threads
        size_t queuedTasks;                         ///< Number of queued tasks
        size_t completedTasks;                      ///< Number of completed tasks

        // Performance metrics
        double averageTaskTime;                     ///< Average task execution time (ms)
        double maxTaskTime;                         ///< Maximum task execution time (ms)
        double totalExecutionTime;                  ///< Total execution time (ms)
        float poolUtilization;                      ///< Pool utilization (0-1)

        // Work-stealing statistics
        uint64_t stolenTasks;                       ///< Total stolen tasks
        uint64_t sharedTasks;                       ///< Total shared tasks
        float stealingEfficiency;                   ///< Stealing efficiency (0-1)

        // Priority statistics
        std::array<uint64_t, 5> priorityTasks;      ///< Tasks by priority level
        std::array<double, 5> priorityExecutionTime; ///< Execution time by priority

        // Memory statistics
        size_t memoryUsed;                          ///< Total memory used
        size_t peakMemoryUsed;                      ///< Peak memory usage
        uint32_t memoryAllocations;                 ///< Number of memory allocations

        // Error statistics
        uint32_t timeoutTasks;                      ///< Number of timeout tasks
        uint32_t failedTasks;                       ///< Number of failed tasks
        uint32_t cancelledTasks;                    ///< Number of cancelled tasks

        ThreadPoolStats()
            : activeThreads(0)
            , totalThreads(0)
            , queuedTasks(0)
            , completedTasks(0)
            , averageTaskTime(0.0)
            , maxTaskTime(0.0)
            , totalExecutionTime(0.0)
            , poolUtilization(0.0f)
            , stolenTasks(0)
            , sharedTasks(0)
            , stealingEfficiency(0.0f)
            , memoryUsed(0)
            , peakMemoryUsed(0)
            , memoryAllocations(0)
            , timeoutTasks(0)
            , failedTasks(0)
            , cancelledTasks(0)
        {
            priorityTasks.fill(0);
            priorityExecutionTime.fill(0.0);
        }
    };

    /**
     * @class ThreadPool
     * @brief Advanced thread pool with work-stealing
     *
     * The ThreadPool class provides advanced thread pool functionality for the
     * VoxelCraft game engine, featuring work-stealing algorithms, priority scheduling,
     * thread affinity management, and comprehensive performance monitoring for optimal
     * parallel execution of game systems and computational tasks.
     *
     * Key Features:
     * - Work-stealing with multiple strategies (LIFO, FIFO, Priority, Adaptive)
     * - Priority-based task scheduling with 5 priority levels
     * - Thread affinity and CPU core management
     * - Dynamic thread scaling based on load
     * - Task dependency management and grouping
     * - Performance monitoring and profiling
     * - Timeout detection and deadlock prevention
     * - Memory usage tracking and optimization
     * - Cross-platform thread management
     *
     * The thread pool is designed to maximize parallel execution efficiency
     * while maintaining thread safety and preventing common concurrency issues.
     */
    class ThreadPool : public System {
    public:
        /**
         * @brief Constructor
         * @param config Thread pool configuration
         */
        explicit ThreadPool(const ThreadPoolConfig& config = ThreadPoolConfig());

        /**
         * @brief Destructor
         */
        ~ThreadPool();

        /**
         * @brief Deleted copy constructor
         */
        ThreadPool(const ThreadPool&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        ThreadPool& operator=(const ThreadPool&) = delete;

        // Thread pool lifecycle

        /**
         * @brief Initialize thread pool
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown thread pool
         * @param force Force immediate shutdown
         * @return true if successful, false otherwise
         */
        bool Shutdown(bool force = false);

        /**
         * @brief Update thread pool
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get thread pool configuration
         * @return Current configuration
         */
        const ThreadPoolConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set thread pool configuration
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool SetConfig(const ThreadPoolConfig& config);

        /**
         * @brief Get thread pool statistics
         * @return Current statistics
         */
        const ThreadPoolStats& GetStats() const { return m_stats; }

        // Task submission

        /**
         * @brief Submit task for execution
         * @param function Task function
         * @param priority Task priority
         * @param name Task name (optional)
         * @return Task ID or 0 if failed
         */
        uint32_t SubmitTask(std::function<void()> function,
                          TaskPriority priority = TaskPriority::Normal,
                          const std::string& name = "");

        /**
         * @brief Submit task with dependencies
         * @param function Task function
         * @param dependencies Task dependencies
         * @param priority Task priority
         * @param name Task name
         * @return Task ID or 0 if failed
         */
        uint32_t SubmitTaskWithDependencies(std::function<void()> function,
                                          const std::vector<uint32_t>& dependencies,
                                          TaskPriority priority = TaskPriority::Normal,
                                          const std::string& name = "");

        /**
         * @brief Submit task group
         * @param tasks Vector of task functions
         * @param priority Group priority
         * @param groupName Group name
         * @return Group ID or 0 if failed
         */
        uint32_t SubmitTaskGroup(const std::vector<std::function<void()>>& tasks,
                               TaskPriority priority = TaskPriority::Normal,
                               const std::string& groupName = "");

        /**
         * @brief Submit fire-and-forget task
         * @param function Task function
         * @return true if submitted, false otherwise
         */
        bool SubmitFireAndForget(std::function<void()> function);

        // Future-based task submission

        /**
         * @brief Submit task with return value
         * @param function Task function with return value
         * @param priority Task priority
         * @param name Task name
         * @return std::future for the result
         */
        template<typename T>
        std::future<T> SubmitTaskWithResult(std::function<T()> function,
                                          TaskPriority priority = TaskPriority::Normal,
                                          const std::string& name = "");

        /**
         * @brief Submit parallel for loop
         * @param begin Start index
         * @param end End index
         * @param function Function to execute for each index
         * @param priority Task priority
         * @param name Task name
         * @return Task group ID or 0 if failed
         */
        uint32_t ParallelFor(size_t begin, size_t end,
                           std::function<void(size_t)> function,
                           TaskPriority priority = TaskPriority::Normal,
                           const std::string& name = "");

        /**
         * @brief Submit parallel reduce operation
         * @param begin Start index
         * @param end End index
         * @param function Function to execute for each index
         * @param reduceFunction Function to combine results
         * @param priority Task priority
         * @param name Task name
         * @return std::future for the final result
         */
        template<typename T, typename U>
        std::future<T> ParallelReduce(size_t begin, size_t end,
                                     std::function<U(size_t)> function,
                                     std::function<T(const std::vector<U>&)> reduceFunction,
                                     TaskPriority priority = TaskPriority::Normal,
                                     const std::string& name = "");

        // Task management

        /**
         * @brief Wait for task completion
         * @param taskId Task ID
         * @param timeout Timeout in seconds (0 = infinite)
         * @return true if completed, false if timeout or not found
         */
        bool WaitForTask(uint32_t taskId, float timeout = 0.0f);

        /**
         * @brief Wait for task group completion
         * @param groupId Group ID
         * @param timeout Timeout in seconds (0 = infinite)
         * @return true if completed, false if timeout or not found
         */
        bool WaitForTaskGroup(uint32_t groupId, float timeout = 0.0f);

        /**
         * @brief Cancel task
         * @param taskId Task ID
         * @return true if cancelled, false otherwise
         */
        bool CancelTask(uint32_t taskId);

        /**
         * @brief Cancel task group
         * @param groupId Group ID
         * @return true if cancelled, false otherwise
         */
        bool CancelTaskGroup(uint32_t groupId);

        /**
         * @brief Get task status
         * @param taskId Task ID
         * @return Task status information
         */
        std::optional<Task> GetTaskStatus(uint32_t taskId) const;

        /**
         * @brief Get task group status
         * @param groupId Group ID
         * @return Vector of task statuses in group
         */
        std::vector<Task> GetTaskGroupStatus(uint32_t groupId) const;

        // Thread management

        /**
         * @brief Get thread information
         * @param threadId Thread ID
         * @return Thread information or nullopt if not found
         */
        std::optional<ThreadInfo> GetThreadInfo(int threadId) const;

        /**
         * @brief Get all thread information
         * @return Vector of thread information
         */
        std::vector<ThreadInfo> GetAllThreadInfo() const;

        /**
         * @brief Set thread priority
         * @param threadId Thread ID
         * @param priority Thread priority
         * @return true if successful, false otherwise
         */
        bool SetThreadPriority(int threadId, int priority);

        /**
         * @brief Set thread affinity
         * @param threadId Thread ID
         * @param affinity CPU affinity
         * @return true if successful, false otherwise
         */
        bool SetThreadAffinity(int threadId, ThreadAffinity affinity);

        /**
         * @brief Scale thread count
         * @param newCount New thread count
         * @return true if successful, false otherwise
         */
        bool ScaleThreadCount(int newCount);

        // Work-stealing control

        /**
         * @brief Set work-stealing strategy
         * @param strategy New strategy
         * @return true if successful, false otherwise
         */
        bool SetWorkStealingStrategy(WorkStealingStrategy strategy);

        /**
         * @brief Enable work-stealing
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableWorkStealing(bool enabled);

        /**
         * @brief Get work-stealing statistics
         * @return Work-stealing performance data
         */
        std::unordered_map<std::string, uint64_t> GetWorkStealingStats() const;

        // Performance and monitoring

        /**
         * @brief Enable performance monitoring
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnablePerformanceMonitoring(bool enabled);

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Get load balancing report
         * @return Load balancing analysis
         */
        std::string GetLoadBalancingReport() const;

        /**
         * @brief Get bottleneck analysis
         * @return Bottleneck identification report
         */
        std::string GetBottleneckAnalysis() const;

        // Utility functions

        /**
         * @brief Get active task count
         * @return Number of active tasks
         */
        size_t GetActiveTaskCount() const;

        /**
         * @brief Get queued task count
         * @return Number of queued tasks
         */
        size_t GetQueuedTaskCount() const;

        /**
         * @brief Get completed task count
         * @return Number of completed tasks
         */
        size_t GetCompletedTaskCount() const;

        /**
         * @brief Is thread pool busy
         * @return true if busy, false otherwise
         */
        bool IsBusy() const;

        /**
         * @brief Get recommended thread count
         * @return Recommended thread count for current hardware
         */
        static int GetRecommendedThreadCount();

        /**
         * @brief Get hardware concurrency
         * @return Hardware thread count
         */
        static int GetHardwareConcurrency();

        /**
         * @brief Validate thread pool state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize thread pool
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Worker thread function
         * @param threadId Worker thread ID
         */
        void WorkerThreadFunction(int threadId);

        /**
         * @brief Find task for execution
         * @param threadId Worker thread ID
         * @return Task to execute or nullptr if none available
         */
        Task* FindTaskForExecution(int threadId);

        /**
         * @brief Execute task
         * @param task Task to execute
         * @param threadId Executing thread ID
         */
        void ExecuteTask(Task* task, int threadId);

        /**
         * @brief Steal task from another thread
         * @param thiefThreadId Stealing thread ID
         * @return Stolen task or nullptr if none available
         */
        Task* StealTask(int thiefThreadId);

        /**
         * @brief Try to steal task using LIFO strategy
         * @param thiefThreadId Stealing thread ID
         * @return Stolen task or nullptr if none available
         */
        Task* TryStealLIFO(int thiefThreadId);

        /**
         * @brief Try to steal task using FIFO strategy
         * @param thiefThreadId Stealing thread ID
         * @return Stolen task or nullptr if none available
         */
        Task* TryStealFIFO(int thiefThreadId);

        /**
         * @brief Try to steal task using priority strategy
         * @param thiefThreadId Stealing thread ID
         * @return Stolen task or nullptr if none available
         */
        Task* TryStealPriority(int thiefThreadId);

        /**
         * @brief Process completed task
         * @param task Completed task
         */
        void ProcessCompletedTask(Task* task);

        /**
         * @brief Check for task timeouts
         */
        void CheckTaskTimeouts();

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle thread pool errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Thread pool data
        ThreadPoolConfig m_config;                  ///< Thread pool configuration
        ThreadPoolStats m_stats;                    ///< Performance statistics

        // Thread management
        std::vector<std::unique_ptr<std::thread>> m_threads; ///< Worker threads
        std::vector<ThreadInfo> m_threadInfo;       ///< Thread information
        std::atomic<bool> m_shutdownRequested;      ///< Shutdown request flag
        std::atomic<int> m_activeThreads;           ///< Number of active threads

        // Task management
        std::deque<Task> m_sharedQueue;             ///< Shared task queue
        std::vector<std::deque<Task>> m_localQueues; ///< Local task queues per thread
        std::unordered_map<uint32_t, Task> m_tasks;  ///< All tasks by ID
        std::unordered_map<uint32_t, std::vector<uint32_t>> m_taskGroups; ///< Task groups

        // Synchronization
        mutable std::shared_mutex m_queueMutex;     ///< Queue synchronization
        mutable std::shared_mutex m_taskMutex;      ///< Task synchronization
        std::condition_variable_any m_taskAvailable; ///< Task available condition
        std::vector<std::unique_ptr<std::mutex>> m_localQueueMutexes; ///< Local queue mutexes

        // Task ID management
        std::atomic<uint32_t> m_nextTaskId;         ///< Next task ID counter
        std::atomic<uint32_t> m_nextGroupId;        ///< Next group ID counter

        // Performance tracking
        std::deque<double> m_executionTimes;        ///< Recent execution times
        std::chrono::steady_clock::time_point m_startTime; ///< Pool start time

        // State tracking
        bool m_isInitialized;                       ///< Pool is initialized
        double m_lastUpdateTime;                    ///< Last update time
        std::string m_lastError;                    ///< Last error message

        static constexpr int MAX_THREADS = 256;     ///< Maximum supported threads
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PARALLEL_THREAD_POOL_HPP
