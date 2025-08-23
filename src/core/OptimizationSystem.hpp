/**
 * @file OptimizationSystem.hpp
 * @brief VoxelCraft Performance Optimization System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_OPTIMIZATION_SYSTEM_HPP
#define VOXELCRAFT_CORE_OPTIMIZATION_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <deque>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <future>
#include <chrono>
#include <string>
#include <type_traits>

#include "../math/Vec3.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Entity;
    class Player;
    struct OptimizationConfig;
    struct PerformanceStats;
    struct MemoryStats;
    struct ThreadPoolStats;
    struct CacheStats;
    struct OptimizationTask;

    /**
     * @enum TaskPriority
     * @brief Priority levels for optimization tasks
     */
    enum class TaskPriority {
        CRITICAL,           ///< Critical tasks (immediate execution)
        HIGH,               ///< High priority tasks
        NORMAL,             ///< Normal priority tasks
        LOW,                ///< Low priority tasks
        BACKGROUND          ///< Background tasks (can be delayed)
    };

    /**
     * @enum OptimizationType
     * @brief Types of optimizations
     */
    enum class OptimizationType {
        THREADING,          ///< Multi-threading optimizations
        CACHING,            ///< Cache optimizations
        MEMORY,             ///< Memory management
        RENDERING,          ///< Rendering optimizations
        PHYSICS,            ///< Physics optimizations
        AI,                 ///< AI optimizations
        NETWORKING,         ///< Network optimizations
        IO,                 ///< I/O optimizations
        GENERAL             ///< General optimizations
    };

    /**
     * @enum CacheType
     * @brief Types of caches
     */
    enum class CacheType {
        TEXTURE,            ///< Texture cache
        MESH,               ///< Mesh cache
        CHUNK,              ///< Chunk cache
        ENTITY,             ///< Entity cache
        PATHFINDING,        ///< Pathfinding cache
        PHYSICS,            ///< Physics cache
        NETWORK,            ///< Network cache
        CUSTOM              ///< Custom cache
    };

    /**
     * @enum MemoryPoolType
     * @brief Types of memory pools
     */
    enum class MemoryPoolType {
        GENERAL,            ///< General purpose memory pool
        TEXTURE,            ///< Texture memory pool
        MESH,               ///< Mesh memory pool
        CHUNK,              ///< Chunk memory pool
        ENTITY,             ///< Entity memory pool
        TEMPORARY,          ///< Temporary memory pool
        CUSTOM              ///< Custom memory pool
    };

    /**
     * @struct OptimizationConfig
     * @brief Configuration for optimization system
     */
    struct OptimizationConfig {
        bool enableThreading = true;
        bool enableCaching = true;
        bool enableMemoryPooling = true;
        bool enableProfiling = false;
        bool enableAutoOptimization = true;

        // Threading settings
        int maxWorkerThreads = 8;
        int minWorkerThreads = 2;
        int maxQueueSize = 1000;
        float taskTimeout = 30.0f;  // seconds

        // Caching settings
        size_t maxCacheSizeMB = 512;
        float cacheExpirationTime = 300.0f;  // 5 minutes
        float cacheCleanupInterval = 60.0f;  // 1 minute

        // Memory settings
        size_t maxMemoryUsageMB = 2048;
        float memoryCleanupThreshold = 0.9f;  // 90%
        size_t memoryPoolBlockSize = 64;  // KB

        // Performance settings
        float targetFrameRate = 60.0f;
        float maxFrameTime = 16.67f;  // ms
        float performanceCheckInterval = 5.0f;  // seconds

        // Optimization settings
        bool enableLOD = true;
        bool enableFrustumCulling = true;
        bool enableOcclusionCulling = false;
        bool enableDistanceCulling = true;
        float cullingDistance = 1000.0f;
        int maxVisibleChunks = 1000;
    };

    /**
     * @struct PerformanceStats
     * @brief Performance monitoring statistics
     */
    struct PerformanceStats {
        // Timing
        float frameTime = 0.0f;
        float updateTime = 0.0f;
        float renderTime = 0.0f;
        float physicsTime = 0.0f;
        float aiTime = 0.0f;
        float networkTime = 0.0f;

        // Rates
        float framesPerSecond = 0.0f;
        float updatesPerSecond = 0.0f;
        float trianglesPerSecond = 0.0f;
        float verticesPerSecond = 0.0f;

        // Counts
        int visibleChunks = 0;
        int activeEntities = 0;
        int drawCalls = 0;
        int trianglesRendered = 0;

        // Performance indicators
        float cpuUsage = 0.0f;
        float gpuUsage = 0.0f;
        float memoryUsageMB = 0.0f;
        float networkLatency = 0.0f;

        // Bottlenecks
        std::string mainBottleneck = "none";
        float bottleneckSeverity = 0.0f;

        // Quality settings
        float lodQuality = 1.0f;
        float textureQuality = 1.0f;
        float shadowQuality = 1.0f;
        float effectQuality = 1.0f;
    };

    /**
     * @struct MemoryStats
     * @brief Memory usage statistics
     */
    struct MemoryStats {
        size_t totalAllocated = 0;
        size_t totalUsed = 0;
        size_t peakUsage = 0;
        size_t poolAllocations = 0;
        size_t heapAllocations = 0;

        // Pool statistics
        std::unordered_map<MemoryPoolType, size_t> poolUsage;
        std::unordered_map<MemoryPoolType, size_t> poolPeakUsage;

        // Fragmentation
        float fragmentationRatio = 0.0f;
        int fragmentedBlocks = 0;

        // Garbage collection
        int gcCycles = 0;
        float lastGCCycle = 0.0f;
        size_t memoryFreed = 0;
    };

    /**
     * @struct ThreadPoolStats
     * @brief Thread pool performance statistics
     */
    struct ThreadPoolStats {
        int activeThreads = 0;
        int idleThreads = 0;
        int totalThreads = 0;
        int queuedTasks = 0;
        int completedTasks = 0;
        int failedTasks = 0;

        // Timing
        float averageTaskTime = 0.0f;
        float longestTaskTime = 0.0f;
        float shortestTaskTime = 0.0f;

        // Thread utilization
        float threadUtilization = 0.0f;
        std::vector<float> threadUtilizations;

        // Queue statistics
        int maxQueueSize = 0;
        float averageQueueTime = 0.0f;
    };

    /**
     * @struct CacheStats
     * @brief Cache performance statistics
     */
    struct CacheStats {
        int totalEntries = 0;
        int hits = 0;
        int misses = 0;
        float hitRate = 0.0f;

        // Cache types
        std::unordered_map<CacheType, int> typeHits;
        std::unordered_map<CacheType, int> typeMisses;

        // Memory usage
        size_t memoryUsed = 0;
        size_t memoryLimit = 0;

        // Eviction statistics
        int evictions = 0;
        float averageLifetime = 0.0f;
    };

    /**
     * @struct OptimizationTask
     * @brief Task for optimization system
     */
    struct OptimizationTask {
        uint64_t id;
        std::string name;
        TaskPriority priority;
        OptimizationType type;
        std::function<void()> function;
        std::chrono::steady_clock::time_point submitTime;
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point endTime;
        std::atomic<bool> completed;
        std::atomic<bool> cancelled;
        void* userData;

        // Performance tracking
        float executionTime = 0.0f;
        bool success = false;
        std::string errorMessage;
    };

    /**
     * @class MemoryPool
     * @brief Custom memory pool for efficient allocations
     */
    class MemoryPool {
    public:
        MemoryPool(size_t blockSize, size_t initialBlocks = 64);
        ~MemoryPool();

        void* Allocate(size_t size);
        void Deallocate(void* ptr);
        void Cleanup();

        size_t GetUsedBlocks() const { return m_usedBlocks; }
        size_t GetTotalBlocks() const { return m_totalBlocks; }
        size_t GetMemoryUsage() const { return m_totalBlocks * m_blockSize; }

    private:
        size_t m_blockSize;
        size_t m_totalBlocks;
        size_t m_usedBlocks;
        void* m_memory;
        std::vector<void*> m_freeList;
        mutable std::mutex m_mutex;
    };

    /**
     * @class Cache
     * @brief Generic cache with automatic expiration
     */
    template<typename Key, typename Value>
    class Cache {
    public:
        Cache(size_t maxSize, float expirationTime = 300.0f);
        ~Cache();

        bool Put(const Key& key, const Value& value);
        bool Get(const Key& key, Value& value);
        bool Remove(const Key& key);
        void Clear();
        void Cleanup();

        size_t Size() const;
        size_t Capacity() const { return m_maxSize; }
        float HitRate() const;

    private:
        struct CacheEntry {
            Value value;
            std::chrono::steady_clock::time_point timestamp;
            int accessCount;
        };

        size_t m_maxSize;
        float m_expirationTime;
        std::unordered_map<Key, CacheEntry> m_entries;
        mutable std::shared_mutex m_mutex;

        // Statistics
        mutable int m_hits = 0;
        mutable int m_misses = 0;
    };

    /**
     * @class ThreadPool
     * @brief Advanced thread pool with priority queue
     */
    class ThreadPool {
    public:
        ThreadPool(int minThreads = 2, int maxThreads = 8);
        ~ThreadPool();

        uint64_t SubmitTask(const std::string& name, TaskPriority priority,
                          std::function<void()> task, void* userData = nullptr);
        bool CancelTask(uint64_t taskId);
        bool WaitForTask(uint64_t taskId, float timeout = 30.0f);

        void SetThreadCount(int count);
        int GetActiveThreadCount() const { return m_activeThreads; }
        int GetQueuedTaskCount() const;

        const ThreadPoolStats& GetStats() const { return m_stats; }

    private:
        void WorkerThread();
        void AdjustThreadCount();
        void ProcessCompletedTasks();

        // Thread management
        std::vector<std::thread> m_threads;
        std::atomic<int> m_activeThreads;
        std::atomic<bool> m_shutdown;

        // Task management
        struct PrioritizedTask {
            uint64_t id;
            std::string name;
            TaskPriority priority;
            std::function<void()> function;
            void* userData;
            std::chrono::steady_clock::time_point submitTime;

            bool operator<(const PrioritizedTask& other) const {
                return priority < other.priority;
            }
        };

        std::priority_queue<PrioritizedTask> m_taskQueue;
        std::unordered_map<uint64_t, std::shared_future<void>> m_activeTasks;
        std::unordered_map<uint64_t, PrioritizedTask> m_pendingTasks;

        // Synchronization
        mutable std::mutex m_queueMutex;
        mutable std::mutex m_taskMutex;
        std::condition_variable m_condition;
        std::condition_variable m_completionCondition;

        // Configuration
        int m_minThreads;
        int m_maxThreads;

        // Statistics
        ThreadPoolStats m_stats;
        mutable std::mutex m_statsMutex;
    };

    /**
     * @class PerformanceProfiler
     * @brief Real-time performance profiling system
     */
    class PerformanceProfiler {
    public:
        static PerformanceProfiler& GetInstance();

        void BeginFrame();
        void EndFrame();
        void BeginSection(const std::string& name);
        void EndSection(const std::string& name);

        const PerformanceStats& GetStats() const { return m_stats; }
        std::vector<std::string> GetBottlenecks() const;
        void Reset();

    private:
        PerformanceProfiler() = default;

        struct ProfileSection {
            std::string name;
            std::chrono::steady_clock::time_point startTime;
            float duration = 0.0f;
            int callCount = 0;
        };

        PerformanceStats m_stats;
        std::unordered_map<std::string, ProfileSection> m_sections;
        std::chrono::steady_clock::time_point m_frameStart;
        std::vector<std::string> m_sectionStack;
        mutable std::shared_mutex m_mutex;
    };

    /**
     * @class OptimizationSystem
     * @brief Main optimization system coordinating all optimizations
     *
     * Features:
     * - Advanced multi-threading with priority queues
     * - Intelligent caching with automatic expiration
     * - Custom memory pools for performance
     * - Real-time performance profiling
     * - Automatic optimization adjustments
     * - Load balancing and resource management
     * - Quality settings management
     * - Performance bottleneck detection
     * - Memory leak detection and cleanup
     * - GPU/CPU utilization optimization
     * - Background task processing
     * - Adaptive quality scaling
     * - Performance prediction and preloading
     */
    class OptimizationSystem {
    public:
        static OptimizationSystem& GetInstance();

        /**
         * @brief Initialize the optimization system
         * @param config Optimization configuration
         * @return true if successful
         */
        bool Initialize(const OptimizationConfig& config);

        /**
         * @brief Shutdown the optimization system
         */
        void Shutdown();

        /**
         * @brief Update optimization system (call every frame)
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        // Threading System
        /**
         * @brief Submit task to thread pool
         * @param name Task name
         * @param priority Task priority
         * @param task Task function
         * @param userData User data
         * @return Task ID
         */
        uint64_t SubmitTask(const std::string& name, TaskPriority priority,
                          std::function<void()> task, void* userData = nullptr);

        /**
         * @brief Submit background task
         * @param name Task name
         * @param task Task function
         * @return Task ID
         */
        uint64_t SubmitBackgroundTask(const std::string& name, std::function<void()> task);

        /**
         * @brief Cancel task
         * @param taskId Task ID
         * @return true if cancelled
         */
        bool CancelTask(uint64_t taskId);

        /**
         * @brief Wait for task completion
         * @param taskId Task ID
         * @param timeout Timeout in seconds
         * @return true if completed
         */
        bool WaitForTask(uint64_t taskId, float timeout = 30.0f);

        /**
         * @brief Get thread pool statistics
         * @return Thread pool stats
         */
        const ThreadPoolStats& GetThreadPoolStats() const;

        // Caching System
        /**
         * @brief Put item in cache
         * @param type Cache type
         * @param key Cache key
         * @param value Cache value
         * @return true if cached
         */
        template<typename Key, typename Value>
        bool CachePut(CacheType type, const Key& key, const Value& value);

        /**
         * @brief Get item from cache
         * @param type Cache type
         * @param key Cache key
         * @param value Output value
         * @return true if found
         */
        template<typename Key, typename Value>
        bool CacheGet(CacheType type, const Key& key, Value& value);

        /**
         * @brief Clear cache
         * @param type Cache type
         * @return Number of items cleared
         */
        int ClearCache(CacheType type);

        /**
         * @brief Get cache statistics
         * @param type Cache type
         * @return Cache stats
         */
        const CacheStats& GetCacheStats(CacheType type) const;

        // Memory Management
        /**
         * @brief Allocate from memory pool
         * @param poolType Pool type
         * @param size Allocation size
         * @return Allocated memory
         */
        void* MemoryAllocate(MemoryPoolType poolType, size_t size);

        /**
         * @brief Deallocate from memory pool
         * @param poolType Pool type
         * @param ptr Memory pointer
         */
        void MemoryDeallocate(MemoryPoolType poolType, void* ptr);

        /**
         * @brief Get memory statistics
         * @return Memory stats
         */
        const MemoryStats& GetMemoryStats() const { return m_memoryStats; }

        /**
         * @brief Trigger garbage collection
         * @return Memory freed in bytes
         */
        size_t GarbageCollect();

        // Performance Profiling
        /**
         * @brief Begin performance frame
         */
        void BeginFrame();

        /**
         * @brief End performance frame
         */
        void EndFrame();

        /**
         * @brief Begin profiled section
         * @param name Section name
         */
        void BeginSection(const std::string& name);

        /**
         * @brief End profiled section
         * @param name Section name
         */
        void EndSection(const std::string& name);

        /**
         * @brief Get performance statistics
         * @return Performance stats
         */
        const PerformanceStats& GetPerformanceStats() const;

        /**
         * @brief Get performance bottlenecks
         * @return List of bottlenecks
         */
        std::vector<std::string> GetBottlenecks() const;

        // Quality Management
        /**
         * @brief Set quality level
         * @param quality Quality level (0-1)
         */
        void SetQualityLevel(float quality);

        /**
         * @brief Get current quality level
         * @return Quality level
         */
        float GetQualityLevel() const { return m_currentQuality; }

        /**
         * @brief Adjust quality based on performance
         */
        void AutoAdjustQuality();

        // Optimization Control
        /**
         * @brief Enable optimization type
         * @param type Optimization type
         * @param enable Enable state
         */
        void EnableOptimization(OptimizationType type, bool enable);

        /**
         * @brief Is optimization enabled
         * @param type Optimization type
         * @return true if enabled
         */
        bool IsOptimizationEnabled(OptimizationType type) const;

        /**
         * @brief Set configuration
         * @param config New configuration
         */
        void SetConfig(const OptimizationConfig& config) { m_config = config; }

        /**
         * @brief Get current configuration
         * @return Current configuration
         */
        const OptimizationConfig& GetConfig() const { return m_config; }

        // Debug
        /**
         * @brief Enable debug mode
         * @param enable Enable state
         */
        void EnableDebugMode(bool enable) { m_debugMode = enable; }

        /**
         * @brief Get debug information
         * @return Debug info string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Validate optimization system
         * @return true if valid
         */
        bool Validate() const;

    private:
        OptimizationSystem() = default;
        ~OptimizationSystem();

        // Prevent copying
        OptimizationSystem(const OptimizationSystem&) = delete;
        OptimizationSystem& operator=(const OptimizationSystem&) = delete;

        // Core systems
        void InitializeThreadPool();
        void InitializeCaches();
        void InitializeMemoryPools();
        void InitializeProfiler();

        // Update methods
        void UpdateThreadPool(float deltaTime);
        void UpdateCaches(float deltaTime);
        void UpdateMemoryPools(float deltaTime);
        void UpdateProfiler(float deltaTime);

        // Optimization methods
        void PerformAutoOptimization();
        void AdjustThreadPoolSize();
        void CleanupExpiredCacheEntries();
        void BalanceMemoryUsage();
        void DetectBottlenecks();

        // Quality adjustment
        void AdjustGraphicsQuality(float targetFPS);
        void AdjustPerformanceSettings();

        // Helper methods
        uint64_t GenerateTaskId();
        float GetCurrentTime() const;
        size_t GetMemoryUsage() const;
        float GetCPUUsage() const;
        float GetGPUUsage() const;

        // Member variables
        OptimizationConfig m_config;
        bool m_initialized = false;
        bool m_debugMode = false;

        // Core systems
        std::unique_ptr<ThreadPool> m_threadPool;
        std::unique_ptr<PerformanceProfiler> m_profiler;

        // Caches
        std::unordered_map<CacheType, std::shared_ptr<void>> m_caches;

        // Memory pools
        std::unordered_map<MemoryPoolType, std::unique_ptr<MemoryPool>> m_memoryPools;

        // Optimization state
        std::unordered_map<OptimizationType, bool> m_enabledOptimizations;
        float m_currentQuality = 1.0f;
        float m_lastPerformanceCheck = 0.0f;

        // Statistics
        PerformanceStats m_performanceStats;
        MemoryStats m_memoryStats;
        std::unordered_map<CacheType, CacheStats> m_cacheStats;

        // Task management
        std::atomic<uint64_t> m_nextTaskId;
        std::unordered_map<uint64_t, std::shared_ptr<OptimizationTask>> m_activeTasks;

        // Thread safety
        mutable std::shared_mutex m_systemMutex;
    };

    // Template implementations
    template<typename Key, typename Value>
    bool OptimizationSystem::CachePut(CacheType type, const Key& key, const Value& value) {
        auto it = m_caches.find(type);
        if (it == m_caches.end()) {
            return false;
        }

        auto cache = std::static_pointer_cast<Cache<Key, Value>>(it->second);
        return cache->Put(key, value);
    }

    template<typename Key, typename Value>
    bool OptimizationSystem::CacheGet(CacheType type, const Key& key, Value& value) {
        auto it = m_caches.find(type);
        if (it == m_caches.end()) {
            return false;
        }

        auto cache = std::static_pointer_cast<Cache<Key, Value>>(it->second);
        return cache->Get(key, value);
    }

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_OPTIMIZATION_SYSTEM_HPP
