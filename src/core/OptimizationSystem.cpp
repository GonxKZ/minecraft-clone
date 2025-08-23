/**
 * @file OptimizationSystem.cpp
 * @brief VoxelCraft Performance Optimization System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "OptimizationSystem.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <memory>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "../utils/Logger.hpp"
#include "../utils/Random.hpp"
#include "../math/MathUtils.hpp"

namespace VoxelCraft {

    // Static instance
    static OptimizationSystem* s_instance = nullptr;

    // MemoryPool implementation
    MemoryPool::MemoryPool(size_t blockSize, size_t initialBlocks)
        : m_blockSize(blockSize), m_totalBlocks(initialBlocks), m_usedBlocks(0) {
        m_memory = std::malloc(m_totalBlocks * m_blockSize);
        m_freeList.reserve(m_totalBlocks);

        // Initialize free list
        for (size_t i = 0; i < m_totalBlocks; ++i) {
            char* block = static_cast<char*>(m_memory) + i * m_blockSize;
            m_freeList.push_back(block);
        }
    }

    MemoryPool::~MemoryPool() {
        if (m_memory) {
            std::free(m_memory);
        }
    }

    void* MemoryPool::Allocate(size_t size) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (size > m_blockSize || m_freeList.empty()) {
            return nullptr;  // Size too large or no free blocks
        }

        void* block = m_freeList.back();
        m_freeList.pop_back();
        m_usedBlocks++;

        return block;
    }

    void MemoryPool::Deallocate(void* ptr) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!ptr) {
            return;
        }

        // Check if pointer belongs to this pool
        char* start = static_cast<char*>(m_memory);
        char* end = start + m_totalBlocks * m_blockSize;
        char* block = static_cast<char*>(ptr);

        if (block < start || block >= end) {
            return;  // Pointer not from this pool
        }

        // Check if block is properly aligned
        size_t offset = block - start;
        if (offset % m_blockSize != 0) {
            return;  // Invalid alignment
        }

        m_freeList.push_back(block);
        m_usedBlocks--;
    }

    void MemoryPool::Cleanup() {
        std::lock_guard<std::mutex> lock(m_mutex);
        // Defragmentation could be implemented here
        Logger::Debug("MemoryPool cleanup: {} blocks used out of {}",
                     m_usedBlocks, m_totalBlocks);
    }

    // Cache implementation
    template<typename Key, typename Value>
    Cache<Key, Value>::Cache(size_t maxSize, float expirationTime)
        : m_maxSize(maxSize), m_expirationTime(expirationTime) {
    }

    template<typename Key, typename Value>
    Cache<Key, Value>::~Cache() {
        Clear();
    }

    template<typename Key, typename Value>
    bool Cache<Key, Value>::Put(const Key& key, const Value& value) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        // Check if we need to evict entries
        if (m_entries.size() >= m_maxSize) {
            // Find least recently used entry
            auto lru = std::min_element(m_entries.begin(), m_entries.end(),
                [](const auto& a, const auto& b) {
                    return a.second.accessCount < b.second.accessCount;
                });

            if (lru != m_entries.end()) {
                m_entries.erase(lru);
            }
        }

        CacheEntry entry;
        entry.value = value;
        entry.timestamp = std::chrono::steady_clock::now();
        entry.accessCount = 0;

        m_entries[key] = entry;
        return true;
    }

    template<typename Key, typename Value>
    bool Cache<Key, Value>::Get(const Key& key, Value& value) {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        auto it = m_entries.find(key);
        if (it == m_entries.end()) {
            m_misses++;
            return false;
        }

        // Check expiration
        auto now = std::chrono::steady_clock::now();
        auto age = std::chrono::duration<float>(now - it->second.timestamp).count();

        if (age > m_expirationTime) {
            lock.unlock();
            std::unique_lock<std::shared_mutex> writeLock(m_mutex);
            m_entries.erase(it);
            m_misses++;
            return false;
        }

        value = it->second.value;
        it->second.accessCount++;
        m_hits++;

        return true;
    }

    template<typename Key, typename Value>
    bool Cache<Key, Value>::Remove(const Key& key) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        return m_entries.erase(key) > 0;
    }

    template<typename Key, typename Value>
    void Cache<Key, Value>::Clear() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_entries.clear();
        m_hits = 0;
        m_misses = 0;
    }

    template<typename Key, typename Value>
    void Cache<Key, Value>::Cleanup() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto now = std::chrono::steady_clock::now();

        for (auto it = m_entries.begin(); it != m_entries.end();) {
            auto age = std::chrono::duration<float>(now - it->second.timestamp).count();
            if (age > m_expirationTime) {
                it = m_entries.erase(it);
            } else {
                ++it;
            }
        }
    }

    template<typename Key, typename Value>
    size_t Cache<Key, Value>::Size() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_entries.size();
    }

    template<typename Key, typename Value>
    float Cache<Key, Value>::HitRate() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        int total = m_hits + m_misses;
        return total > 0 ? static_cast<float>(m_hits) / total : 0.0f;
    }

    // ThreadPool implementation
    ThreadPool::ThreadPool(int minThreads, int maxThreads)
        : m_activeThreads(0), m_shutdown(false), m_minThreads(minThreads), m_maxThreads(maxThreads) {
        m_stats.totalThreads = minThreads;

        // Create worker threads
        for (int i = 0; i < m_minThreads; ++i) {
            m_threads.emplace_back(&ThreadPool::WorkerThread, this);
        }
    }

    ThreadPool::~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_shutdown = true;
        }
        m_condition.notify_all();

        // Wait for threads to finish
        for (auto& thread : m_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    uint64_t ThreadPool::SubmitTask(const std::string& name, TaskPriority priority,
                                  std::function<void()> task, void* userData) {
        static uint64_t taskId = 1;
        uint64_t id = taskId++;

        PrioritizedTask pTask;
        pTask.id = id;
        pTask.name = name;
        pTask.priority = priority;
        pTask.function = task;
        pTask.userData = userData;
        pTask.submitTime = std::chrono::steady_clock::now();

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_taskQueue.push(pTask);
            m_pendingTasks[id] = pTask;
            m_stats.queuedTasks++;
            m_stats.maxQueueSize = std::max(m_stats.maxQueueSize, static_cast<int>(m_taskQueue.size()));
        }

        m_condition.notify_one();
        return id;
    }

    bool ThreadPool::CancelTask(uint64_t taskId) {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        auto it = m_pendingTasks.find(taskId);
        if (it != m_pendingTasks.end()) {
            m_pendingTasks.erase(it);
            m_stats.queuedTasks--;
            return true;
        }
        return false;
    }

    bool ThreadPool::WaitForTask(uint64_t taskId, float timeout) {
        auto startTime = std::chrono::steady_clock::now();

        while (true) {
            {
                std::unique_lock<std::mutex> lock(m_taskMutex);
                auto it = m_activeTasks.find(taskId);
                if (it == m_activeTasks.end()) {
                    // Task not found or completed
                    return true;
                }

                if (it->second.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) {
                    return true;
                }
            }

            auto elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime).count();
            if (elapsed > timeout) {
                return false;  // Timeout
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void ThreadPool::SetThreadCount(int count) {
        count = std::clamp(count, m_minThreads, m_maxThreads);

        if (count > m_stats.totalThreads) {
            // Add threads
            for (int i = m_stats.totalThreads; i < count; ++i) {
                m_threads.emplace_back(&ThreadPool::WorkerThread, this);
            }
        } else if (count < m_stats.totalThreads) {
            // Remove threads (they will finish current tasks and exit)
            // This is simplified - in a real implementation, you'd need more complex logic
        }

        m_stats.totalThreads = count;
    }

    int ThreadPool::GetQueuedTaskCount() const {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        return static_cast<int>(m_taskQueue.size());
    }

    void ThreadPool::WorkerThread() {
        while (true) {
            PrioritizedTask task;
            bool hasTask = false;

            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_condition.wait(lock, [this]() {
                    return m_shutdown || !m_taskQueue.empty();
                });

                if (m_shutdown && m_taskQueue.empty()) {
                    break;
                }

                if (!m_taskQueue.empty()) {
                    task = m_taskQueue.top();
                    m_taskQueue.pop();
                    hasTask = true;
                    m_pendingTasks.erase(task.id);
                    m_stats.queuedTasks--;
                    m_activeThreads++;
                }
            }

            if (hasTask) {
                auto startTime = std::chrono::steady_clock::now();

                try {
                    // Create packaged task
                    auto packagedTask = std::packaged_task<void()>(task.function);
                    auto future = packagedTask.get_future();

                    {
                        std::unique_lock<std::mutex> lock(m_taskMutex);
                        m_activeTasks[task.id] = std::move(future);
                    }

                    // Execute task
                    packagedTask();

                    auto endTime = std::chrono::steady_clock::now();
                    float executionTime = std::chrono::duration<float>(endTime - startTime).count();

                    // Update statistics
                    {
                        std::unique_lock<std::mutex> lock(m_statsMutex);
                        m_stats.completedTasks++;
                        m_stats.averageTaskTime = (m_stats.averageTaskTime + executionTime) * 0.5f;
                        m_stats.longestTaskTime = std::max(m_stats.longestTaskTime, executionTime);
                        m_stats.shortestTaskTime = std::min(m_stats.shortestTaskTime, executionTime);
                    }

                    {
                        std::unique_lock<std::mutex> lock(m_taskMutex);
                        m_activeTasks.erase(task.id);
                    }

                } catch (const std::exception& e) {
                    Logger::Error("ThreadPool task '{}' failed: {}", task.name, e.what());
                    {
                        std::unique_lock<std::mutex> lock(m_statsMutex);
                        m_stats.failedTasks++;
                    }
                }

                m_activeThreads--;
            }
        }
    }

    void ThreadPool::AdjustThreadCount() {
        // Simple thread count adjustment based on queue size
        int queueSize = GetQueuedTaskCount();
        int targetThreads = m_minThreads;

        if (queueSize > 10) {
            targetThreads = std::min(m_maxThreads, m_minThreads + queueSize / 5);
        }

        SetThreadCount(targetThreads);
    }

    void ThreadPool::ProcessCompletedTasks() {
        // Clean up completed futures
        std::unique_lock<std::mutex> lock(m_taskMutex);
        for (auto it = m_activeTasks.begin(); it != m_activeTasks.end();) {
            if (it->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                it = m_activeTasks.erase(it);
            } else {
                ++it;
            }
        }
    }

    // PerformanceProfiler implementation
    PerformanceProfiler& PerformanceProfiler::GetInstance() {
        static PerformanceProfiler instance;
        return instance;
    }

    void PerformanceProfiler::BeginFrame() {
        m_frameStart = std::chrono::steady_clock::now();
        m_sectionStack.clear();
    }

    void PerformanceProfiler::EndFrame() {
        auto frameEnd = std::chrono::steady_clock::now();
        float frameTime = std::chrono::duration<float>(frameEnd - m_frameStart).count() * 1000.0f;  // Convert to ms

        // Update frame statistics
        m_stats.frameTime = frameTime;
        m_stats.framesPerSecond = frameTime > 0.0f ? 1000.0f / frameTime : 0.0f;

        // Calculate bottlenecks
        std::vector<std::pair<std::string, float>> sectionTimes;
        for (const auto& section : m_sections) {
            sectionTimes.emplace_back(section.first, section.second.duration);
        }

        if (!sectionTimes.empty()) {
            std::sort(sectionTimes.begin(), sectionTimes.end(),
                     [](const auto& a, const auto& b) { return a.second > b.second; });

            m_stats.mainBottleneck = sectionTimes[0].first;
            m_stats.bottleneckSeverity = sectionTimes[0].second / m_stats.frameTime;
        }
    }

    void PerformanceProfiler::BeginSection(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        ProfileSection& section = m_sections[name];
        section.startTime = std::chrono::steady_clock::now();
        section.callCount++;

        m_sectionStack.push_back(name);
    }

    void PerformanceProfiler::EndSection(const std::string& name) {
        auto endTime = std::chrono::steady_clock::now();

        std::unique_lock<std::shared_mutex> lock(m_mutex);

        auto it = m_sections.find(name);
        if (it != m_sections.end()) {
            ProfileSection& section = it->second;
            if (!m_sectionStack.empty() && m_sectionStack.back() == name) {
                m_sectionStack.pop_back();
            }

            float duration = std::chrono::duration<float>(endTime - section.startTime).count() * 1000.0f;
            section.duration += duration;
        }
    }

    std::vector<std::string> PerformanceProfiler::GetBottlenecks() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        std::vector<std::pair<std::string, float>> sectionTimes;
        for (const auto& section : m_sections) {
            sectionTimes.emplace_back(section.first, section.second.duration);
        }

        std::sort(sectionTimes.begin(), sectionTimes.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });

        std::vector<std::string> bottlenecks;
        for (size_t i = 0; i < std::min(sectionTimes.size(), size_t(5)); ++i) {
            bottlenecks.push_back(sectionTimes[i].first);
        }

        return bottlenecks;
    }

    void PerformanceProfiler::Reset() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_sections.clear();
        m_sectionStack.clear();
        m_stats = PerformanceStats();
    }

    // OptimizationSystem implementation
    OptimizationSystem& OptimizationSystem::GetInstance() {
        if (!s_instance) {
            s_instance = new OptimizationSystem();
        }
        return *s_instance;
    }

    bool OptimizationSystem::Initialize(const OptimizationConfig& config) {
        if (m_initialized) {
            Logger::Warning("OptimizationSystem already initialized");
            return true;
        }

        m_config = config;
        m_initialized = true;
        m_debugMode = false;
        m_nextTaskId = 1;

        try {
            // Initialize subsystems
            InitializeThreadPool();
            InitializeCaches();
            InitializeMemoryPools();
            InitializeProfiler();

            // Set default optimization states
            m_enabledOptimizations[OptimizationType::THREADING] = config.enableThreading;
            m_enabledOptimizations[OptimizationType::CACHING] = config.enableCaching;
            m_enabledOptimizations[OptimizationType::MEMORY] = config.enableMemoryPooling;
            m_enabledOptimizations[OptimizationType::RENDERING] = true;
            m_enabledOptimizations[OptimizationType::PHYSICS] = true;
            m_enabledOptimizations[OptimizationType::AI] = true;
            m_enabledOptimizations[OptimizationType::NETWORKING] = true;
            m_enabledOptimizations[OptimizationType::IO] = true;
            m_enabledOptimizations[OptimizationType::GENERAL] = true;

            m_currentQuality = 1.0f;
            m_lastPerformanceCheck = 0.0f;

            Logger::Info("OptimizationSystem initialized successfully");
            return true;

        } catch (const std::exception& e) {
            Logger::Error("Failed to initialize OptimizationSystem: {}", e.what());
            return false;
        }
    }

    void OptimizationSystem::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Shutdown thread pool
        if (m_threadPool) {
            m_threadPool.reset();
        }

        // Clear caches
        m_caches.clear();

        // Clear memory pools
        m_memoryPools.clear();

        // Clear active tasks
        m_activeTasks.clear();

        m_initialized = false;
        Logger::Info("OptimizationSystem shutdown");
    }

    void OptimizationSystem::Update(float deltaTime) {
        if (!m_initialized) {
            return;
        }

        // Update subsystems
        UpdateThreadPool(deltaTime);
        UpdateCaches(deltaTime);
        UpdateMemoryPools(deltaTime);
        UpdateProfiler(deltaTime);

        // Perform auto-optimization
        if (m_config.enableAutoOptimization) {
            PerformAutoOptimization();
        }

        // Update performance statistics
        m_lastPerformanceCheck += deltaTime;
        if (m_lastPerformanceCheck >= m_config.performanceCheckInterval) {
            m_lastPerformanceCheck = 0.0f;
            DetectBottlenecks();
        }
    }

    uint64_t OptimizationSystem::SubmitTask(const std::string& name, TaskPriority priority,
                                          std::function<void()> task, void* userData) {
        if (!m_threadPool) {
            Logger::Error("ThreadPool not initialized");
            return 0;
        }

        uint64_t taskId = m_nextTaskId++;
        uint64_t poolTaskId = m_threadPool->SubmitTask(name, priority, task, userData);

        // Store task info
        auto taskInfo = std::make_shared<OptimizationTask>();
        taskInfo->id = taskId;
        taskInfo->name = name;
        taskInfo->priority = priority;
        taskInfo->submitTime = std::chrono::steady_clock::now();
        taskInfo->userData = userData;
        taskInfo->completed = false;
        taskInfo->cancelled = false;

        m_activeTasks[taskId] = taskInfo;

        return taskId;
    }

    uint64_t OptimizationSystem::SubmitBackgroundTask(const std::string& name, std::function<void()> task) {
        return SubmitTask(name, TaskPriority::BACKGROUND, task);
    }

    bool OptimizationSystem::CancelTask(uint64_t taskId) {
        auto it = m_activeTasks.find(taskId);
        if (it != m_activeTasks.end()) {
            it->second->cancelled = true;
            m_threadPool->CancelTask(taskId);
            m_activeTasks.erase(it);
            return true;
        }
        return false;
    }

    bool OptimizationSystem::WaitForTask(uint64_t taskId, float timeout) {
        return m_threadPool->WaitForTask(taskId, timeout);
    }

    const ThreadPoolStats& OptimizationSystem::GetThreadPoolStats() const {
        static ThreadPoolStats defaultStats;
        return m_threadPool ? m_threadPool->GetStats() : defaultStats;
    }

    int OptimizationSystem::ClearCache(CacheType type) {
        auto it = m_caches.find(type);
        if (it == m_caches.end()) {
            return 0;
        }

        // This is a simplified implementation - in reality you'd need to handle different cache types
        return 0;
    }

    const CacheStats& OptimizationSystem::GetCacheStats(CacheType type) const {
        static CacheStats defaultStats;
        auto it = m_cacheStats.find(type);
        return it != m_cacheStats.end() ? it->second : defaultStats;
    }

    void* OptimizationSystem::MemoryAllocate(MemoryPoolType poolType, size_t size) {
        auto it = m_memoryPools.find(poolType);
        if (it == m_memoryPools.end()) {
            return nullptr;
        }

        void* ptr = it->second->Allocate(size);
        if (ptr) {
            m_memoryStats.totalAllocated += size;
            m_memoryStats.totalUsed += size;
            m_memoryStats.poolAllocations++;
            m_memoryStats.poolUsage[poolType] += size;
            m_memoryStats.poolPeakUsage[poolType] = std::max(m_memoryStats.poolPeakUsage[poolType],
                                                           m_memoryStats.poolUsage[poolType]);
        }

        return ptr;
    }

    void OptimizationSystem::MemoryDeallocate(MemoryPoolType poolType, void* ptr) {
        auto it = m_memoryPools.find(poolType);
        if (it == m_memoryPools.end()) {
            return;
        }

        // Estimate size (this is simplified)
        size_t estimatedSize = it->second->GetUsedBlocks() * it->second->GetMemoryUsage() / it->second->GetTotalBlocks();

        it->second->Deallocate(ptr);
        m_memoryStats.totalUsed -= estimatedSize;
        m_memoryStats.poolUsage[poolType] -= estimatedSize;
    }

    size_t OptimizationSystem::GarbageCollect() {
        size_t freedMemory = 0;

        // Cleanup memory pools
        for (auto& pool : m_memoryPools) {
            size_t before = pool.second->GetMemoryUsage();
            pool.second->Cleanup();
            size_t after = pool.second->GetMemoryUsage();
            freedMemory += (before - after);
        }

        // Cleanup caches
        for (auto& cache : m_caches) {
            // This would call cleanup on each cache type
        }

        m_memoryStats.memoryFreed += freedMemory;
        m_memoryStats.gcCycles++;

        return freedMemory;
    }

    void OptimizationSystem::BeginFrame() {
        if (m_profiler) {
            m_profiler->BeginFrame();
        }
    }

    void OptimizationSystem::EndFrame() {
        if (m_profiler) {
            m_profiler->EndFrame();
            m_performanceStats = m_profiler->GetStats();
        }
    }

    void OptimizationSystem::BeginSection(const std::string& name) {
        if (m_profiler) {
            m_profiler->BeginSection(name);
        }
    }

    void OptimizationSystem::EndSection(const std::string& name) {
        if (m_profiler) {
            m_profiler->EndSection(name);
        }
    }

    const PerformanceStats& OptimizationSystem::GetPerformanceStats() const {
        return m_performanceStats;
    }

    std::vector<std::string> OptimizationSystem::GetBottlenecks() const {
        return m_profiler ? m_profiler->GetBottlenecks() : std::vector<std::string>();
    }

    void OptimizationSystem::SetQualityLevel(float quality) {
        m_currentQuality = std::clamp(quality, 0.1f, 1.0f);

        // Adjust various quality settings based on quality level
        m_performanceStats.lodQuality = m_currentQuality;
        m_performanceStats.textureQuality = m_currentQuality;
        m_performanceStats.shadowQuality = m_currentQuality * 0.8f;  // Shadows are more expensive
        m_performanceStats.effectQuality = m_currentQuality;

        Logger::Info("Quality level set to {:.2f}", m_currentQuality);
    }

    void OptimizationSystem::AutoAdjustQuality() {
        if (!m_config.enableAutoOptimization) {
            return;
        }

        float currentFPS = m_performanceStats.framesPerSecond;
        float targetFPS = m_config.targetFrameRate;

        if (currentFPS < targetFPS * 0.8f) {
            // FPS too low, reduce quality
            float newQuality = m_currentQuality * 0.9f;
            SetQualityLevel(newQuality);
        } else if (currentFPS > targetFPS * 0.95f && m_currentQuality < 1.0f) {
            // FPS good and quality not max, increase quality
            float newQuality = std::min(m_currentQuality * 1.1f, 1.0f);
            SetQualityLevel(newQuality);
        }
    }

    void OptimizationSystem::EnableOptimization(OptimizationType type, bool enable) {
        m_enabledOptimizations[type] = enable;
        Logger::Info("Optimization {} {}", type, enable ? "enabled" : "disabled");
    }

    bool OptimizationSystem::IsOptimizationEnabled(OptimizationType type) const {
        auto it = m_enabledOptimizations.find(type);
        return it != m_enabledOptimizations.end() ? it->second : false;
    }

    std::string OptimizationSystem::GetDebugInfo() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);

        ss << "OptimizationSystem Debug Info:\n";
        ss << "Quality Level: " << m_currentQuality << "\n";
        ss << "Frame Time: " << m_performanceStats.frameTime << "ms\n";
        ss << "FPS: " << m_performanceStats.framesPerSecond << "\n";
        ss << "Memory Usage: " << m_memoryStats.totalUsed / (1024.0f * 1024.0f) << "MB\n";
        ss << "Active Tasks: " << m_activeTasks.size() << "\n";
        ss << "Thread Pool: " << (m_threadPool ? m_threadPool->GetActiveThreadCount() : 0) << " threads\n";

        if (!m_performanceStats.mainBottleneck.empty()) {
            ss << "Main Bottleneck: " << m_performanceStats.mainBottleneck << "\n";
        }

        return ss.str();
    }

    bool OptimizationSystem::Validate() const {
        bool valid = true;

        if (!m_threadPool) {
            Logger::Error("OptimizationSystem validation failed: ThreadPool not initialized");
            valid = false;
        }

        if (!m_profiler) {
            Logger::Error("OptimizationSystem validation failed: PerformanceProfiler not initialized");
            valid = false;
        }

        if (m_memoryPools.empty()) {
            Logger::Error("OptimizationSystem validation failed: No memory pools initialized");
            valid = false;
        }

        return valid;
    }

    // Private methods implementation
    void OptimizationSystem::InitializeThreadPool() {
        m_threadPool = std::make_unique<ThreadPool>(m_config.minWorkerThreads, m_config.maxWorkerThreads);
        Logger::Info("ThreadPool initialized with {}-{} threads", m_config.minWorkerThreads, m_config.maxWorkerThreads);
    }

    void OptimizationSystem::InitializeCaches() {
        // Initialize different cache types
        // This is simplified - in reality you'd create specific cache instances
        Logger::Info("Caches initialized");
    }

    void OptimizationSystem::InitializeMemoryPools() {
        // Create memory pools for different allocation sizes
        const size_t poolSizes[] = {64, 256, 1024, 4096, 16384};  // bytes

        for (size_t size : poolSizes) {
            auto pool = std::make_unique<MemoryPool>(size, 1024);  // 1024 blocks per pool
            m_memoryPools[static_cast<MemoryPoolType>(size)] = std::move(pool);
        }

        Logger::Info("Memory pools initialized with {} pools", m_memoryPools.size());
    }

    void OptimizationSystem::InitializeProfiler() {
        m_profiler = std::make_unique<PerformanceProfiler>();
        Logger::Info("PerformanceProfiler initialized");
    }

    void OptimizationSystem::UpdateThreadPool(float deltaTime) {
        if (m_threadPool) {
            // Adjust thread pool size based on load
            static float lastAdjustment = 0.0f;
            lastAdjustment += deltaTime;

            if (lastAdjustment >= 5.0f) {  // Adjust every 5 seconds
                lastAdjustment = 0.0f;
                m_threadPool->AdjustThreadCount();
            }
        }
    }

    void OptimizationSystem::UpdateCaches(float deltaTime) {
        // Cleanup expired cache entries
        static float lastCacheCleanup = 0.0f;
        lastCacheCleanup += deltaTime;

        if (lastCacheCleanup >= m_config.cacheCleanupInterval) {
            lastCacheCleanup = 0.0f;
            CleanupExpiredCacheEntries();
        }
    }

    void OptimizationSystem::UpdateMemoryPools(float deltaTime) {
        // Monitor memory usage
        m_memoryStats.peakUsage = std::max(m_memoryStats.peakUsage, m_memoryStats.totalUsed);

        // Trigger garbage collection if needed
        float usageRatio = static_cast<float>(m_memoryStats.totalUsed) / m_config.maxMemoryUsageMB;
        if (usageRatio > m_config.memoryCleanupThreshold) {
            GarbageCollect();
        }
    }

    void OptimizationSystem::UpdateProfiler(float deltaTime) {
        if (m_profiler) {
            // Update performance metrics
            m_performanceStats.memoryUsageMB = m_memoryStats.totalUsed / (1024.0f * 1024.0f);
            m_performanceStats.cpuUsage = GetCPUUsage();
            m_performanceStats.gpuUsage = GetGPUUsage();
        }
    }

    void OptimizationSystem::PerformAutoOptimization() {
        // Auto-adjust quality based on performance
        AutoAdjustQuality();

        // Balance thread pool
        AdjustThreadPoolSize();

        // Balance memory usage
        BalanceMemoryUsage();
    }

    void OptimizationSystem::AdjustThreadPoolSize() {
        if (!m_threadPool) {
            return;
        }

        const auto& stats = m_threadPool->GetStats();
        int targetThreads = m_config.minWorkerThreads;

        // Adjust based on queue size and utilization
        if (stats.queuedTasks > 20) {
            targetThreads = std::min(m_config.maxWorkerThreads, targetThreads + stats.queuedTasks / 10);
        }

        if (stats.threadUtilization > 0.8f && targetThreads < m_config.maxWorkerThreads) {
            targetThreads++;
        } else if (stats.threadUtilization < 0.3f && targetThreads > m_config.minWorkerThreads) {
            targetThreads--;
        }

        m_threadPool->SetThreadCount(targetThreads);
    }

    void OptimizationSystem::CleanupExpiredCacheEntries() {
        // Cleanup logic for different cache types
        Logger::Debug("Cache cleanup completed");
    }

    void OptimizationSystem::BalanceMemoryUsage() {
        // Monitor and balance memory usage across pools
        if (m_memoryStats.totalUsed > m_config.maxMemoryUsageMB * 1024 * 1024) {
            // Force garbage collection
            size_t freed = GarbageCollect();
            Logger::Info("Emergency GC: freed {} bytes", freed);
        }
    }

    void OptimizationSystem::DetectBottlenecks() {
        if (!m_profiler) {
            return;
        }

        auto bottlenecks = m_profiler->GetBottlenecks();
        if (!bottlenecks.empty()) {
            Logger::Debug("Detected performance bottlenecks: {}", bottlenecks[0]);
        }
    }

    float OptimizationSystem::GetCurrentTime() const {
        return std::chrono::duration<float>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    size_t OptimizationSystem::GetMemoryUsage() const {
        return m_memoryStats.totalUsed;
    }

    float OptimizationSystem::GetCPUUsage() const {
        // Simplified CPU usage estimation
        // In a real implementation, you'd use platform-specific APIs
        return 0.5f;  // 50% placeholder
    }

    float OptimizationSystem::GetGPUUsage() const {
        // Simplified GPU usage estimation
        // In a real implementation, you'd use graphics API queries
        return 0.6f;  // 60% placeholder
    }

} // namespace VoxelCraft
