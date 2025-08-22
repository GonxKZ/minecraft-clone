/**
 * @file ThreadPool.hpp
 * @brief VoxelCraft Thread Pool for Parallel Processing
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_THREAD_POOL_HPP
#define VOXELCRAFT_CORE_THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <type_traits>

namespace VoxelCraft {

/**
 * @class ThreadPool
 * @brief High-performance thread pool for parallel task execution
 *
 * The ThreadPool provides:
 * - Dynamic thread management
 * - Task queuing and execution
 * - Load balancing
 * - Task priorities
 * - Performance monitoring
 * - Graceful shutdown
 * - Exception handling
 */
class ThreadPool {
public:
    /**
     * @enum TaskPriority
     * @brief Task execution priorities
     */
    enum class TaskPriority {
        LOW = 0,     ///< Low priority tasks
        NORMAL,      ///< Normal priority tasks
        HIGH,        ///< High priority tasks
        CRITICAL     ///< Critical priority tasks (execute immediately)
    };

    /**
     * @struct Task
     * @brief Represents a task in the thread pool
     */
    struct Task {
        std::function<void()> function;    ///< Task function
        TaskPriority priority;              ///< Task priority
        std::string name;                   ///< Task name for debugging
        uint64_t id;                        ///< Unique task ID
        std::chrono::steady_clock::time_point submitTime; ///< Submission time
    };

    /**
     * @struct ThreadPoolMetrics
     * @brief Performance metrics for the thread pool
     */
    struct ThreadPoolMetrics {
        uint64_t totalTasksExecuted;        ///< Total tasks executed
        uint64_t totalTasksQueued;          ///< Total tasks queued
        uint64_t totalTasksRejected;        ///< Total tasks rejected
        double averageTaskTime;             ///< Average task execution time (ms)
        double averageQueueTime;            ///< Average time tasks spend in queue (ms)
        uint32_t activeThreads;             ///< Number of active threads
        uint32_t idleThreads;               ///< Number of idle threads
        uint32_t totalThreads;              ///< Total number of threads
        size_t queueSize;                   ///< Current queue size
        size_t maxQueueSize;                ///< Maximum queue size reached
    };

    /**
     * @brief Constructor
     * @param numThreads Number of worker threads (0 = hardware concurrency)
     */
    explicit ThreadPool(size_t numThreads = 0);

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

    /**
     * @brief Initialize the thread pool
     * @return true if successful, false otherwise
     */
    bool Initialize();

    /**
     * @brief Shutdown the thread pool
     */
    void Shutdown();

    /**
     * @brief Submit a task for execution
     * @param function Task function
     * @param priority Task priority
     * @param name Task name for debugging
     * @return Future for task result (void for non-returning tasks)
     */
    std::future<void> SubmitTask(std::function<void()> function,
                                TaskPriority priority = TaskPriority::NORMAL,
                                const std::string& name = "");

    /**
     * @brief Submit a task with return value
     * @tparam F Function type
     * @tparam Args Argument types
     * @param function Task function
     * @param priority Task priority
     * @param name Task name for debugging
     * @param args Function arguments
     * @return Future for task result
     */
    template<typename F, typename... Args>
    auto SubmitTask(F&& function, TaskPriority priority, const std::string& name, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    /**
     * @brief Submit a task with return value (normal priority)
     * @tparam F Function type
     * @tparam Args Argument types
     * @param function Task function
     * @param args Function arguments
     * @return Future for task result
     */
    template<typename F, typename... Args>
    auto SubmitTask(F&& function, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    /**
     * @brief Wait for all tasks to complete
     */
    void WaitAll();

    /**
     * @brief Get current metrics
     * @return Thread pool metrics
     */
    const ThreadPoolMetrics& GetMetrics() const { return m_metrics; }

    /**
     * @brief Get number of worker threads
     * @return Number of threads
     */
    size_t GetThreadCount() const { return m_threads.size(); }

    /**
     * @brief Check if thread pool is running
     * @return true if running, false otherwise
     */
    bool IsRunning() const { return m_running; }

    /**
     * @brief Set maximum queue size
     * @param maxSize Maximum queue size (0 = unlimited)
     */
    void SetMaxQueueSize(size_t maxSize) { m_maxQueueSize = maxSize; }

    /**
     * @brief Get maximum queue size
     * @return Maximum queue size
     */
    size_t GetMaxQueueSize() const { return m_maxQueueSize; }

    /**
     * @brief Enable/disable performance monitoring
     * @param enabled Enable monitoring
     */
    void SetMonitoringEnabled(bool enabled) { m_monitoringEnabled = enabled; }

    /**
     * @brief Check if monitoring is enabled
     * @return true if enabled
     */
    bool IsMonitoringEnabled() const { return m_monitoringEnabled; }

private:
    // Thread management
    std::vector<std::thread> m_threads;            ///< Worker threads
    std::atomic<bool> m_running;                   ///< Running flag
    std::atomic<bool> m_initialized;               ///< Initialization flag

    // Task queue
    struct PriorityTask {
        Task task;
        TaskPriority priority;

        bool operator<(const PriorityTask& other) const {
            return static_cast<int>(priority) < static_cast<int>(other.priority);
        }
    };

    std::priority_queue<PriorityTask> m_taskQueue;  ///< Task priority queue
    mutable std::mutex m_queueMutex;                ///< Queue synchronization
    std::condition_variable m_queueCondition;       ///< Queue condition variable

    // Metrics
    ThreadPoolMetrics m_metrics;                    ///< Performance metrics
    mutable std::mutex m_metricsMutex;              ///< Metrics synchronization

    // Configuration
    size_t m_maxQueueSize;                          ///< Maximum queue size
    bool m_monitoringEnabled;                       ///< Enable performance monitoring
    uint64_t m_nextTaskId;                          ///< Next task ID counter

    /**
     * @brief Worker thread function
     */
    void WorkerThread();

    /**
     * @brief Get next task from queue
     * @return Task to execute, or empty function if none available
     */
    std::function<void()> GetNextTask();

    /**
     * @brief Execute a task and measure performance
     * @param task Task to execute
     */
    void ExecuteTask(const Task& task);

    /**
     * @brief Update performance metrics
     * @param task Task that was executed
     * @param executionTime Execution time in milliseconds
     * @param queueTime Time spent in queue in milliseconds
     */
    void UpdateMetrics(const Task& task, double executionTime, double queueTime);
};

// Template implementations

template<typename F, typename... Args>
auto ThreadPool::SubmitTask(F&& function, TaskPriority priority, const std::string& name, Args&&... args)
    -> std::future<typename std::invoke_result<F, Args...>::type>
{
    using ReturnType = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(function), std::forward<Args>(args)...)
    );

    std::future<ReturnType> result = task->get_future();

    SubmitTask([task]() { (*task)(); }, priority, name);

    return result;
}

template<typename F, typename... Args>
auto ThreadPool::SubmitTask(F&& function, Args&&... args)
    -> std::future<typename std::invoke_result<F, Args...>::type>
{
    return SubmitTask(std::forward<F>(function), TaskPriority::NORMAL, "", std::forward<Args>(args)...);
}

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_THREAD_POOL_HPP
