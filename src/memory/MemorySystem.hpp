/**
 * @file MemorySystem.hpp
 * @brief VoxelCraft Advanced Memory Management System Header
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_MEMORY_MEMORY_SYSTEM_HPP
#define VOXELCRAFT_MEMORY_MEMORY_SYSTEM_HPP

#include <memory>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <cstdint>
#include <new>

#include "../core/System.hpp"

namespace VoxelCraft {

    /**
     * @brief Memory allocation types
     */
    enum class MemoryType {
        GENERAL,           // General-purpose allocations
        TEMPORARY,         // Short-lived temporary allocations
        PERSISTENT,        // Long-lived persistent allocations
        RENDERING,         // GPU-related allocations
        AUDIO,            // Audio system allocations
        PHYSICS,          // Physics engine allocations
        AI,               // AI system allocations
        NETWORK,          // Network system allocations
        SCRIPTING,        // Scripting engine allocations
        ASSET,            // Asset loading allocations
        STREAMING,        // Streaming system allocations
        UI,               // UI system allocations
        WORLD,            // World data allocations
        ENTITY,           // Entity data allocations
        PARTICLE,         // Particle system allocations
        ANIMATION,        // Animation system allocations
        TERRAIN,          // Terrain generation allocations
        PATHFINDING,      // Pathfinding system allocations
        MULTITHREADING,   // Multithreading allocations
        DEBUG,            // Debug system allocations
        CUSTOM_1,         // Custom allocation type 1
        CUSTOM_2,         // Custom allocation type 2
        CUSTOM_3,         // Custom allocation type 3
        CUSTOM_4,         // Custom allocation type 4
        MAX_TYPES
    };

    /**
     * @brief Memory pool allocation strategies
     */
    enum class PoolStrategy {
        FIXED_SIZE,       // Fixed-size blocks
        DYNAMIC_SIZE,     // Variable-size blocks with coalescing
        LINEAR,          // Linear allocator (stack-like)
        RING_BUFFER,     // Ring buffer allocator
        OBJECT_POOL,     // Object pooling for specific types
        SLAB,           // Slab allocator for small objects
    };

    /**
     * @brief Memory allocation flags
     */
    enum MemoryFlags {
        MEMORY_FLAG_NONE          = 0,
        MEMORY_FLAG_ZERO_MEMORY   = 1 << 0,
        MEMORY_FLAG_ALIGNED       = 1 << 1,
        MEMORY_FLAG_TRACKED       = 1 << 2,
        MEMORY_FLAG_LEAK_CHECK    = 1 << 3,
        MEMORY_FLAG_SHARED        = 1 << 4,
        MEMORY_FLAG_PERSISTENT    = 1 << 5,
        MEMORY_FLAG_TEMPORARY     = 1 << 6,
        MEMORY_FLAG_NO_THROW      = 1 << 7,
        MEMORY_FLAG_LARGE_BLOCK   = 1 << 8,
    };

    /**
     * @brief Memory leak detection modes
     */
    enum class LeakDetectionMode {
        NONE,             // No leak detection
        BASIC,            // Basic leak detection using reference counting
        ADVANCED,         // Advanced detection with call stacks
        FULL,            // Full memory profiling and leak detection
    };

    /**
     * @brief Memory statistics structure
     */
    struct MemoryStats {
        uint64_t totalAllocated = 0;           // Total bytes allocated
        uint64_t totalUsed = 0;                // Total bytes currently in use
        uint64_t totalFree = 0;                // Total bytes free
        uint64_t peakUsage = 0;                // Peak memory usage
        uint64_t allocationCount = 0;          // Total allocation operations
        uint64_t deallocationCount = 0;        // Total deallocation operations
        uint64_t fragmentationRatio = 0;       // Memory fragmentation percentage
        uint32_t activeAllocations = 0;        // Number of active allocations
        uint32_t memoryPoolCount = 0;          // Number of memory pools
        uint64_t averageAllocationSize = 0;    // Average allocation size
        uint64_t largestAllocation = 0;        // Largest single allocation
        uint64_t smallestAllocation = UINT64_MAX; // Smallest single allocation

        // Per-type statistics
        std::vector<uint64_t> typeAllocations;
        std::vector<uint64_t> typeDeallocations;
        std::vector<uint64_t> typeMemoryUsage;

        MemoryStats() {
            typeAllocations.resize(static_cast<size_t>(MemoryType::MAX_TYPES), 0);
            typeDeallocations.resize(static_cast<size_t>(MemoryType::MAX_TYPES), 0);
            typeMemoryUsage.resize(static_cast<size_t>(MemoryType::MAX_TYPES), 0);
        }
    };

    /**
     * @brief Memory allocation information
     */
    struct AllocationInfo {
        void* address = nullptr;
        size_t size = 0;
        MemoryType type = MemoryType::GENERAL;
        uint32_t flags = 0;
        std::chrono::system_clock::time_point timestamp;
        std::string file;
        uint32_t line = 0;
        std::string function;
        uint32_t threadId = 0;
        uint64_t allocationId = 0;
        bool isFreed = false;

        // Stack trace for leak detection
        std::vector<std::string> stackTrace;

        // Reference counting
        std::atomic<uint32_t> refCount{1};

        AllocationInfo() = default;

        AllocationInfo(void* addr, size_t sz, MemoryType t, uint32_t flgs,
                      const std::string& f, uint32_t l, const std::string& func)
            : address(addr), size(sz), type(t), flags(flgs),
              timestamp(std::chrono::system_clock::now()),
              file(f), line(l), function(func), isFreed(false) {
        }
    };

    /**
     * @brief Memory pool configuration
     */
    struct MemoryPoolConfig {
        MemoryType memoryType = MemoryType::GENERAL;
        PoolStrategy strategy = PoolStrategy::FIXED_SIZE;
        size_t blockSize = 64;                 // Size of each block
        size_t poolSize = 1024 * 1024;         // Total pool size in bytes
        size_t alignment = 16;                 // Memory alignment
        bool enableDefragmentation = true;     // Enable automatic defragmentation
        bool enableProfiling = true;           // Enable profiling for this pool
        bool enableLeakDetection = true;       // Enable leak detection for this pool
        std::string name;                      // Pool name for debugging
        uint32_t threadSafety = 0;             // Thread safety level
    };

    /**
     * @brief Memory pool interface
     */
    class MemoryPool {
    public:
        MemoryPool(const MemoryPoolConfig& config);
        virtual ~MemoryPool() = default;

        virtual void* Allocate(size_t size, uint32_t flags = 0) = 0;
        virtual void Deallocate(void* ptr) = 0;
        virtual bool Contains(void* ptr) const = 0;
        virtual size_t GetUsedMemory() const = 0;
        virtual size_t GetFreeMemory() const = 0;
        virtual size_t GetTotalMemory() const = 0;
        virtual float GetFragmentationRatio() const = 0;
        virtual void Defragment() = 0;
        virtual void Reset() = 0;
        virtual bool Validate() const = 0;

        const MemoryPoolConfig& GetConfig() const { return m_config; }
        const std::string& GetName() const { return m_config.name; }
        MemoryType GetMemoryType() const { return m_config.memoryType; }

    protected:
        MemoryPoolConfig m_config;
    };

    /**
     * @brief Fixed-size memory pool
     */
    class FixedSizeMemoryPool : public MemoryPool {
    public:
        FixedSizeMemoryPool(const MemoryPoolConfig& config);
        ~FixedSizeMemoryPool() override;

        void* Allocate(size_t size, uint32_t flags = 0) override;
        void Deallocate(void* ptr) override;
        bool Contains(void* ptr) const override;
        size_t GetUsedMemory() const override;
        size_t GetFreeMemory() const override;
        size_t GetTotalMemory() const override;
        float GetFragmentationRatio() const override;
        void Defragment() override;
        void Reset() override;
        bool Validate() const override;

    private:
        struct Block {
            Block* next = nullptr;
            bool isFree = true;
            uint32_t magic = 0xDEADBEEF;  // Debug magic number
        };

        void* m_memory = nullptr;
        Block* m_freeList = nullptr;
        size_t m_blockCount = 0;
        size_t m_freeBlocks = 0;
        mutable std::mutex m_mutex;
    };

    /**
     * @brief Dynamic-size memory pool with coalescing
     */
    class DynamicMemoryPool : public MemoryPool {
    public:
        DynamicMemoryPool(const MemoryPoolConfig& config);
        ~DynamicMemoryPool() override;

        void* Allocate(size_t size, uint32_t flags = 0) override;
        void Deallocate(void* ptr) override;
        bool Contains(void* ptr) const override;
        size_t GetUsedMemory() const override;
        size_t GetFreeMemory() const override;
        size_t GetTotalMemory() const override;
        float GetFragmentationRatio() const override;
        void Defragment() override;
        void Reset() override;
        bool Validate() const override;

    private:
        struct FreeBlock {
            size_t size = 0;
            FreeBlock* next = nullptr;
            FreeBlock* prev = nullptr;
            bool isFree = true;
            uint32_t magic = 0xCAFEBABE;
        };

        void* m_memory = nullptr;
        FreeBlock* m_freeList = nullptr;
        size_t m_usedMemory = 0;
        mutable std::mutex m_mutex;

        void CoalesceFreeBlocks();
        FreeBlock* FindBestFit(size_t size);
    };

    /**
     * @brief Linear memory allocator (stack-like)
     */
    class LinearMemoryPool : public MemoryPool {
    public:
        LinearMemoryPool(const MemoryPoolConfig& config);
        ~LinearMemoryPool() override;

        void* Allocate(size_t size, uint32_t flags = 0) override;
        void Deallocate(void* ptr) override;
        bool Contains(void* ptr) const override;
        size_t GetUsedMemory() const override;
        size_t GetFreeMemory() const override;
        size_t GetTotalMemory() const override;
        float GetFragmentationRatio() const override;
        void Defragment() override;
        void Reset() override;
        bool Validate() const override;

    private:
        void* m_memory = nullptr;
        void* m_current = nullptr;
        size_t m_usedMemory = 0;
        mutable std::mutex m_mutex;
    };

    /**
     * @brief Ring buffer memory allocator
     */
    class RingBufferMemoryPool : public MemoryPool {
    public:
        RingBufferMemoryPool(const MemoryPoolConfig& config);
        ~RingBufferMemoryPool() override;

        void* Allocate(size_t size, uint32_t flags = 0) override;
        void Deallocate(void* ptr) override;
        bool Contains(void* ptr) const override;
        size_t GetUsedMemory() const override;
        size_t GetFreeMemory() const override;
        size_t GetTotalMemory() const override;
        float GetFragmentationRatio() const override;
        void Defragment() override;
        void Reset() override;
        bool Validate() const override;

    private:
        void* m_memory = nullptr;
        size_t m_readPosition = 0;
        size_t m_writePosition = 0;
        size_t m_usedMemory = 0;
        bool m_full = false;
        mutable std::mutex m_mutex;

        size_t GetAvailableSpace() const;
    };

    /**
     * @brief Object pool for specific types
     */
    template<typename T>
    class ObjectPool : public MemoryPool {
    public:
        ObjectPool(const MemoryPoolConfig& config)
            : MemoryPool(config), m_objectCount(0) {
            m_config.strategy = PoolStrategy::OBJECT_POOL;
        }

        ~ObjectPool() override {
            Clear();
        }

        template<typename... Args>
        T* AllocateObject(Args&&... args) {
            std::unique_lock<std::mutex> lock(m_mutex);

            if (!m_freeObjects.empty()) {
                T* obj = m_freeObjects.back();
                m_freeObjects.pop_back();
                new(obj) T(std::forward<Args>(args)...);  // Placement new
                return obj;
            }

            // Allocate new object
            T* obj = new T(std::forward<Args>(args)...);
            m_allocatedObjects.insert(obj);
            m_objectCount++;
            return obj;
        }

        void DeallocateObject(T* obj) {
            if (!obj) return;

            std::unique_lock<std::mutex> lock(m_mutex);

            auto it = m_allocatedObjects.find(obj);
            if (it == m_allocatedObjects.end()) {
                return;  // Object not from this pool
            }

            obj->~T();  // Call destructor
            m_freeObjects.push_back(obj);
        }

        void Clear() {
            std::unique_lock<std::mutex> lock(m_mutex);

            // Delete all objects
            for (T* obj : m_allocatedObjects) {
                delete obj;
            }

            for (T* obj : m_freeObjects) {
                delete obj;
            }

            m_allocatedObjects.clear();
            m_freeObjects.clear();
            m_objectCount = 0;
        }

        // MemoryPool interface implementation
        void* Allocate(size_t size, uint32_t flags = 0) override {
            return AllocateObject();
        }

        void Deallocate(void* ptr) override {
            DeallocateObject(static_cast<T*>(ptr));
        }

        bool Contains(void* ptr) const override {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_allocatedObjects.find(static_cast<T*>(ptr)) != m_allocatedObjects.end();
        }

        size_t GetUsedMemory() const override {
            std::unique_lock<std::mutex> lock(m_mutex);
            return (m_allocatedObjects.size() - m_freeObjects.size()) * sizeof(T);
        }

        size_t GetFreeMemory() const override {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_freeObjects.size() * sizeof(T);
        }

        size_t GetTotalMemory() const override {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_allocatedObjects.size() * sizeof(T);
        }

        float GetFragmentationRatio() const override {
            return 0.0f;  // Object pools don't fragment
        }

        void Defragment() override {
            // No defragmentation needed for object pools
        }

        void Reset() override {
            Clear();
        }

        bool Validate() const override {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_allocatedObjects.size() >= m_freeObjects.size();
        }

        size_t GetObjectCount() const {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_objectCount;
        }

        size_t GetFreeObjectCount() const {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_freeObjects.size();
        }

    private:
        mutable std::mutex m_mutex;
        std::unordered_set<T*> m_allocatedObjects;
        std::vector<T*> m_freeObjects;
        size_t m_objectCount;
    };

    /**
     * @brief Memory leak detector
     */
    class MemoryLeakDetector {
    public:
        MemoryLeakDetector();
        ~MemoryLeakDetector() = default;

        void TrackAllocation(const AllocationInfo& info);
        void TrackDeallocation(void* address);
        void ReportLeaks();

        void SetDetectionMode(LeakDetectionMode mode) { m_mode = mode; }
        LeakDetectionMode GetDetectionMode() const { return m_mode; }

        void StartLeakDetection();
        void StopLeakDetection();

        size_t GetTrackedAllocationCount() const;
        size_t GetPotentialLeakCount() const;

    private:
        mutable std::shared_mutex m_mutex;
        std::unordered_map<void*, AllocationInfo> m_trackedAllocations;
        LeakDetectionMode m_mode = LeakDetectionMode::BASIC;
        bool m_isActive = false;
        std::atomic<uint64_t> m_nextAllocationId{1};

        void CaptureStackTrace(AllocationInfo& info);
        bool IsLeak(const AllocationInfo& info) const;
    };

    /**
     * @brief Memory profiler
     */
    class MemoryProfiler {
    public:
        MemoryProfiler();
        ~MemoryProfiler() = default;

        void RecordAllocation(MemoryType type, size_t size, void* address);
        void RecordDeallocation(MemoryType type, size_t size, void* address);

        MemoryStats GetCurrentStats() const;
        MemoryStats GetStatsForType(MemoryType type) const;
        MemoryStats GetPeakStats() const;

        void Reset();
        void StartProfiling();
        void StopProfiling();
        bool IsProfiling() const { return m_isProfiling; }

        std::vector<std::pair<MemoryType, MemoryStats>> GetAllTypeStats() const;

        // Performance monitoring
        float GetAllocationRate() const;  // allocations per second
        float GetDeallocationRate() const; // deallocations per second
        float GetMemoryPressure() const;  // memory pressure percentage

    private:
        mutable std::shared_mutex m_mutex;
        MemoryStats m_currentStats;
        MemoryStats m_peakStats;
        std::vector<MemoryStats> m_typeStats;
        bool m_isProfiling = false;

        // Performance tracking
        uint64_t m_lastAllocationCount = 0;
        uint64_t m_lastDeallocationCount = 0;
        std::chrono::steady_clock::time_point m_lastUpdateTime;

        void UpdatePeakStats();
        void UpdatePerformanceMetrics();
    };

    /**
     * @brief Memory defragmenter
     */
    class MemoryDefragmenter {
    public:
        MemoryDefragmenter();
        ~MemoryDefragmenter() = default;

        void RegisterPool(MemoryPool* pool);
        void UnregisterPool(MemoryPool* pool);

        void DefragmentAll();
        void DefragmentPool(MemoryPool* pool);

        void SetDefragmentationThreshold(float threshold) { m_threshold = threshold; }
        float GetDefragmentationThreshold() const { return m_threshold; }

        void EnableAutoDefragmentation(bool enable) { m_autoDefragmentation = enable; }
        bool IsAutoDefragmentationEnabled() const { return m_autoDefragmentation; }

        size_t GetDefragmentationCount() const { return m_defragmentationCount; }

    private:
        mutable std::shared_mutex m_mutex;
        std::vector<MemoryPool*> m_pools;
        float m_threshold = 0.5f;  // Defragment when fragmentation > 50%
        bool m_autoDefragmentation = true;
        std::atomic<size_t> m_defragmentationCount{0};

        bool ShouldDefragment(MemoryPool* pool) const;
    };

    /**
     * @brief Main memory system
     */
    class MemorySystem : public System {
    public:
        static MemorySystem& GetInstance();

        MemorySystem();
        ~MemorySystem() override;

        // System interface
        bool Initialize() override;
        void Update(float deltaTime) override;
        void Shutdown() override;
        std::string GetName() const override { return "MemorySystem"; }
        System::Type GetType() const override { return System::Type::MEMORY; }

        // Memory allocation API
        void* Allocate(size_t size, MemoryType type = MemoryType::GENERAL,
                      uint32_t flags = 0, const char* file = nullptr,
                      uint32_t line = 0, const char* function = nullptr);

        void Deallocate(void* ptr, MemoryType type = MemoryType::GENERAL);

        template<typename T, typename... Args>
        T* AllocateObject(MemoryType type = MemoryType::GENERAL, Args&&... args) {
            void* memory = Allocate(sizeof(T), type, MEMORY_FLAG_TRACKED);
            if (!memory) return nullptr;

            return new(memory) T(std::forward<Args>(args)...);
        }

        template<typename T>
        void DeallocateObject(T* obj, MemoryType type = MemoryType::GENERAL) {
            if (!obj) return;

            obj->~T();
            Deallocate(obj, type);
        }

        // Memory pool management
        MemoryPool* CreatePool(const MemoryPoolConfig& config);
        bool DestroyPool(MemoryPool* pool);
        bool DestroyPool(const std::string& name);
        MemoryPool* GetPool(const std::string& name) const;
        std::vector<MemoryPool*> GetPools(MemoryType type) const;
        size_t GetPoolCount() const;

        // Object pool management
        template<typename T>
        ObjectPool<T>* CreateObjectPool(const std::string& name, MemoryType type = MemoryType::GENERAL) {
            MemoryPoolConfig config;
            config.name = name;
            config.memoryType = type;
            config.strategy = PoolStrategy::OBJECT_POOL;

            auto pool = new ObjectPool<T>(config);
            if (RegisterPool(pool)) {
                return pool;
            }

            delete pool;
            return nullptr;
        }

        template<typename T>
        ObjectPool<T>* GetObjectPool(const std::string& name) const {
            return static_cast<ObjectPool<T>*>(GetPool(name));
        }

        // Memory statistics and profiling
        MemoryStats GetMemoryStats() const;
        MemoryStats GetMemoryStats(MemoryType type) const;
        MemoryStats GetPeakMemoryStats() const;

        // Leak detection
        void StartLeakDetection();
        void StopLeakDetection();
        void ReportMemoryLeaks();
        bool IsLeakDetectionActive() const;

        // Defragmentation
        void DefragmentMemory();
        void DefragmentPools(MemoryType type);
        void EnableAutoDefragmentation(bool enable);
        bool IsAutoDefragmentationEnabled() const;

        // Memory validation
        bool ValidateMemory();
        bool ValidatePool(MemoryPool* pool);
        bool ValidateAllPools();

        // Configuration
        void SetLeakDetectionMode(LeakDetectionMode mode);
        LeakDetectionMode GetLeakDetectionMode() const;

        void SetProfilingEnabled(bool enabled);
        bool IsProfilingEnabled() const;

        void SetDefragmentationThreshold(float threshold);
        float GetDefragmentationThreshold() const;

        // Utility functions
        std::string GetMemoryInfo() const;
        void PrintMemoryReport() const;
        void ResetStatistics();

        size_t GetTotalMemoryUsage() const;
        size_t GetPeakMemoryUsage() const;
        float GetMemoryPressure() const;

        // Memory debugging
        void SetAllocationBreakpoint(size_t size);
        void ClearAllocationBreakpoint();

        // Memory tagging for debugging
        void TagMemory(void* ptr, const std::string& tag);
        std::string GetMemoryTag(void* ptr) const;

    private:
        // Core components
        mutable std::shared_mutex m_poolsMutex;
        std::unordered_map<std::string, MemoryPool*> m_pools;
        std::unordered_map<MemoryType, std::vector<MemoryPool*>> m_poolsByType;

        MemoryLeakDetector m_leakDetector;
        MemoryProfiler m_profiler;
        MemoryDefragmenter m_defragmenter;

        // Configuration
        LeakDetectionMode m_leakDetectionMode = LeakDetectionMode::BASIC;
        bool m_profilingEnabled = true;
        bool m_autoDefragmentation = true;
        float m_defragmentationThreshold = 0.5f;
        size_t m_allocationBreakpoint = 0;

        // Memory tagging for debugging
        mutable std::shared_mutex m_tagsMutex;
        std::unordered_map<void*, std::string> m_memoryTags;

        // Internal methods
        bool RegisterPool(MemoryPool* pool);
        void UnregisterPool(MemoryPool* pool);

        MemoryPool* FindSuitablePool(size_t size, MemoryType type, uint32_t flags);
        void* AllocateFromSystem(size_t size, uint32_t flags);

        void UpdateMemoryPressure();
        void CheckForDefragmentation();

        // Statistics tracking
        void UpdateStatsOnAllocation(MemoryType type, size_t size, void* address);
        void UpdateStatsOnDeallocation(MemoryType type, size_t size, void* address);
    };

    /**
     * @brief Custom memory allocator for STL containers
     */
    template<typename T>
    class MemoryAllocator {
    public:
        using value_type = T;

        MemoryAllocator(MemoryType type = MemoryType::GENERAL) noexcept
            : m_memoryType(type) {
        }

        template<typename U>
        MemoryAllocator(const MemoryAllocator<U>& other) noexcept
            : m_memoryType(other.m_memoryType) {
        }

        T* allocate(size_t n) {
            return static_cast<T*>(MemorySystem::GetInstance().Allocate(
                n * sizeof(T), m_memoryType, MEMORY_FLAG_TRACKED));
        }

        void deallocate(T* p, size_t n) {
            MemorySystem::GetInstance().Deallocate(p, m_memoryType);
        }

        bool operator==(const MemoryAllocator& other) const {
            return m_memoryType == other.m_memoryType;
        }

        bool operator!=(const MemoryAllocator& other) const {
            return !(*this == other);
        }

    private:
        MemoryType m_memoryType;
    };

    /**
     * @brief Smart pointer with memory system integration
     */
    template<typename T>
    class MemoryPtr {
    public:
        MemoryPtr() = default;
        MemoryPtr(T* ptr, MemoryType type = MemoryType::GENERAL)
            : m_ptr(ptr), m_memoryType(type) {
        }

        ~MemoryPtr() {
            if (m_ptr) {
                MemorySystem::GetInstance().DeallocateObject(m_ptr, m_memoryType);
            }
        }

        MemoryPtr(const MemoryPtr&) = delete;
        MemoryPtr& operator=(const MemoryPtr&) = delete;

        MemoryPtr(MemoryPtr&& other) noexcept
            : m_ptr(other.m_ptr), m_memoryType(other.m_memoryType) {
            other.m_ptr = nullptr;
        }

        MemoryPtr& operator=(MemoryPtr&& other) noexcept {
            if (this != &other) {
                if (m_ptr) {
                    MemorySystem::GetInstance().DeallocateObject(m_ptr, m_memoryType);
                }
                m_ptr = other.m_ptr;
                m_memoryType = other.m_memoryType;
                other.m_ptr = nullptr;
            }
            return *this;
        }

        T* get() const { return m_ptr; }
        T* operator->() const { return m_ptr; }
        T& operator*() const { return *m_ptr; }

        explicit operator bool() const { return m_ptr != nullptr; }

        void reset(T* ptr = nullptr) {
            if (m_ptr) {
                MemorySystem::GetInstance().DeallocateObject(m_ptr, m_memoryType);
            }
            m_ptr = ptr;
        }

    private:
        T* m_ptr = nullptr;
        MemoryType m_memoryType = MemoryType::GENERAL;
    };

    /**
     * @brief Memory system utility macros
     */
    #define VOXELCRAFT_ALLOC(size, type) \
        MemorySystem::GetInstance().Allocate(size, type, MEMORY_FLAG_TRACKED, __FILE__, __LINE__, __FUNCTION__)

    #define VOXELCRAFT_DEALLOC(ptr, type) \
        MemorySystem::GetInstance().Deallocate(ptr, type)

    #define VOXELCRAFT_NEW(type, memory_type, ...) \
        MemorySystem::GetInstance().AllocateObject<type>(memory_type, __VA_ARGS__)

    #define VOXELCRAFT_DELETE(ptr, memory_type) \
        MemorySystem::GetInstance().DeallocateObject(ptr, memory_type)

    // STL container type aliases with custom allocator
    template<typename T>
    using MemoryVector = std::vector<T, MemoryAllocator<T>>;

    template<typename T>
    using MemoryList = std::list<T, MemoryAllocator<T>>;

    template<typename K, typename V>
    using MemoryMap = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, MemoryAllocator<std::pair<const K, V>>>;

    template<typename T>
    using MemorySet = std::unordered_set<T, std::hash<T>, std::equal_to<T>, MemoryAllocator<T>>;

} // namespace VoxelCraft

#endif // VOXELCRAFT_MEMORY_MEMORY_SYSTEM_HPP
