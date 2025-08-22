/**
 * @file ThreadPool.cpp
 * @brief VoxelCraft Thread Pool Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "ThreadPool.hpp"
#include "Logger.hpp"

#include <algorithm>

namespace VoxelCraft {

ThreadPool::ThreadPool(size_t numThreads)
    : m_running(false)
    , m_initialized(false)
    , m_maxQueueSize(0) // Unlimited by default
    , m_monitoringEnabled(true)
    , m_nextTaskId(1)
{
    std::memset(&m_metrics, 0, sizeof(ThreadPoolMetrics));

    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) {
            numThreads = 4; // Fallback to 4 threads
        }
    }

    m_threads.reserve(numThreads);
    m_metrics.totalThreads = numThreads;
}

ThreadPool::~ThreadPool() {
    Shutdown();
}

bool ThreadPool::Initialize() {
    if (m_initialized) {
        return true;
    }

    VOXELCRAFT_INFO("Initializing ThreadPool with {} threads", m_threads.capacity());

    try {
        m_running = true;
        m_initialized = true;

        // Create worker threads
        for (size_t i = 0; i < m_threads.capacity(); ++i) {
            m_threads.emplace_back(&ThreadPool::WorkerThread, this);
        }

        VOXELCRAFT_INFO("ThreadPool initialized successfully");
        return true;

    } catch (const std::exception& e) {
        VOXELCRAFT_ERROR("Failed to initialize ThreadPool: {}", e.what());
        m_running = false;
        m_initialized = false;
        return false;
    }
}

void ThreadPool::Shutdown() {
    if (!m_initialized) {
        return;
    }

    VOXELCRAFT_INFO("Shutting down ThreadPool");

    // Stop all threads
    m_running = false;
    m_queueCondition.notify_all();

    // Wait for threads to finish
    for (std::thread& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    m_threads.clear();

    // Clear remaining tasks
    while (!m_taskQueue.empty()) {
        m_taskQueue.pop();
    }

    m_initialized = false;
    VOXELCRAFT_INFO("ThreadPool shutdown complete");
}

std::future<void> ThreadPool::SubmitTask(std::function<void()> function,
                                        TaskPriority priority,
                                        const std::string& name) {
    if (!m_initialized || !m_running) {
        VOXELCRAFT_ERROR("Cannot submit task: ThreadPool not initialized or running");
        return std::future<void>();
    }

    // Check queue size limit
    if (m_maxQueueSize > 0) {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_taskQueue.size() >= m_maxQueueSize) {
            VOXELCRAFT_WARNING("Task queue full, rejecting task");
            m_metrics.totalTasksRejected++;
            return std::future<void>();
        }
    }

    Task task;
    task.function = function;
    task.priority = priority;
    task.name = name;
    task.id = m_nextTaskId++;
    task.submitTime = std::chrono::steady_clock::now();

    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();

    // Wrap function to handle promise
    task.function = [function, promise]() {
        try {
            function();
            promise->set_value();
        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Task execution error: {}", e.what());
            try {
                promise->set_exception(std::current_exception());
            } catch (...) {
                // Exception setting failed
            }
        } catch (...) {
            VOXELCRAFT_ERROR("Task execution error: Unknown exception");
            try {
                promise->set_exception(std::current_exception());
            } catch (...) {
                // Exception setting failed
            }
        }
    };

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_taskQueue.push({task, priority});
        m_metrics.totalTasksQueued++;
    }

    m_queueCondition.notify_one();

    return future;
}

void ThreadPool::WaitAll() {
    if (!m_initialized) {
        return;
    }

    // Wait until queue is empty and all threads are idle
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_queueCondition.wait(lock, [this]() {
        return m_taskQueue.empty() && m_metrics.activeThreads == 0;
    });
}

void ThreadPool::WorkerThread() {
    VOXELCRAFT_TRACE("Worker thread started");

    while (m_running) {
        std::function<void()> task = GetNextTask();

        if (task) {
            // Execute task
            auto startTime = std::chrono::steady_clock::now();

            {
                std::lock_guard<std::mutex> lock(m_metricsMutex);
                m_metrics.activeThreads++;
                m_metrics.idleThreads = std::max(0, (int)m_metrics.totalThreads - (int)m_metrics.activeThreads);
            }

            try {
                task();
            } catch (const std::exception& e) {
                VOXELCRAFT_ERROR("Unhandled exception in worker thread: {}", e.what());
            } catch (...) {
                VOXELCRAFT_ERROR("Unhandled exception in worker thread: Unknown");
            }

            auto endTime = std::chrono::steady_clock::now();
            double executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime
            ).count();

            {
                std::lock_guard<std::mutex> lock(m_metricsMutex);
                m_metrics.activeThreads--;
                m_metrics.totalTasksExecuted++;

                // Update average execution time
                double totalTime = m_metrics.averageTaskTime * (m_metrics.totalTasksExecuted - 1);
                m_metrics.averageTaskTime = (totalTime + executionTime) / m_metrics.totalTasksExecuted;
            }

        } else {
            // No task available, update idle threads
            {
                std::lock_guard<std::mutex> lock(m_metricsMutex);
                m_metrics.idleThreads = m_metrics.totalThreads - m_metrics.activeThreads;
            }
        }
    }

    VOXELCRAFT_TRACE("Worker thread stopped");
}

std::function<void()> ThreadPool::GetNextTask() {
    std::unique_lock<std::mutex> lock(m_queueMutex);

    // Wait for task or shutdown signal
    m_queueCondition.wait(lock, [this]() {
        return !m_running || !m_taskQueue.empty();
    });

    if (!m_running) {
        return nullptr;
    }

    if (m_taskQueue.empty()) {
        return nullptr;
    }

    PriorityTask priorityTask = m_taskQueue.top();
    m_taskQueue.pop();

    Task task = priorityTask.task;

    // Calculate queue time
    auto now = std::chrono::steady_clock::now();
    double queueTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - task.submitTime
    ).count();

    // Update queue time average
    {
        std::lock_guard<std::mutex> metricsLock(m_metricsMutex);
        size_t totalTasks = m_metrics.totalTasksExecuted + m_metrics.totalTasksQueued;
        if (totalTasks > 0) {
            double totalQueueTime = m_metrics.averageQueueTime * (totalTasks - 1);
            m_metrics.averageQueueTime = (totalQueueTime + queueTime) / totalTasks;
        }
    }

    return task.function;
}

void ThreadPool::ExecuteTask(const Task& task) {
    // Task execution is handled in SubmitTask wrapper
    // This method is kept for potential future enhancements
}

void ThreadPool::UpdateMetrics(const Task& task, double executionTime, double queueTime) {
    std::lock_guard<std::mutex> lock(m_metricsMutex);

    // Update execution time average
    double totalTime = m_metrics.averageTaskTime * (m_metrics.totalTasksExecuted - 1);
    m_metrics.averageTaskTime = (totalTime + executionTime) / m_metrics.totalTasksExecuted;

    // Update queue time average
    size_t totalTasks = m_metrics.totalTasksExecuted + m_metrics.totalTasksQueued;
    if (totalTasks > 0) {
        double totalQueueTime = m_metrics.averageQueueTime * (totalTasks - 1);
        m_metrics.averageQueueTime = (totalQueueTime + queueTime) / totalTasks;
    }

    // Update queue size
    {
        std::lock_guard<std::mutex> queueLock(m_queueMutex);
        m_metrics.queueSize = m_taskQueue.size();
        m_metrics.maxQueueSize = std::max(m_metrics.maxQueueSize, m_metrics.queueSize);
    }
}

} // namespace VoxelCraft
