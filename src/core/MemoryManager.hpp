/**
 * @file MemoryManager.hpp
 * @brief VoxelCraft Engine Advanced Memory Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file provides a comprehensive memory management system for the VoxelCraft engine
 * with support for memory pools, garbage collection, leak detection, and performance
 * monitoring for optimal memory usage in voxel-based games.
 */

#ifndef VOXELCRAFT_CORE_MEMORY_MANAGER_HPP
#define VOXELCRAFT_CORE_MEMORY_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <functional>
#include <optional>
#include <deque>

namespace VoxelCraft {

    /**
     * @typedef MemoryPoolID
     * @brief Unique identifier for memory pools
     */
    using MemoryPoolID = uint32_t;

    /**
     * @typedef AllocationID
     * @brief Unique identifier for memory allocations
     */
    using AllocationID = uint64_t;

    /**
     * @enum MemoryPoolType
     * @brief Type of memory pool for different allocation patterns
     */
    enum class MemoryPoolType {
        Fixed,          ///< Fixed-size block pool
        Dynamic,        ///< Dynamic size pool with growth
        Stack,          ///< Stack-based allocator
        Object,         ///< Object pool for specific types
        Linear,         ///< Linear allocator (single allocation)
        Ring,           ///< Ring buffer allocator
        Frame,          ///< Frame-based temporary allocator
        Persistent      ///< Persistent allocation pool
    };

    /**
     * @struct MemoryAllocation
     * @brief Information about a memory allocation
     */
    struct MemoryAllocation {
        AllocationID id;                    ///< Unique allocation ID
        void* address;                      ///< Memory address
        size_t size;                        ///< Allocation size in bytes
        std::string type;                   ///< Allocation type/category
        std::string file;                   ///< Source file
        uint32_t line;                      ///< Source line
        double timestamp;                   ///< Allocation timestamp
        bool isActive;                      ///< Active allocation flag
        MemoryPoolID poolId;                ///< Pool that owns this allocation
    };

    /**
     * @struct MemoryPoolInfo
     * @brief Information about a memory pool
     */
    struct MemoryPoolInfo {
        MemoryPoolID id;                    ///< Pool ID
        std::string name;                   ///< Pool name
        MemoryPoolType type;                ///< Pool type
        size_t blockSize;                   ///< Block size for fixed pools
        size_t totalSize;                   ///< Total pool size
        size_t usedSize;                    ///< Currently used size
        size_t peakUsedSize;                ///< Peak used size
        size_t allocationCount;             ///< Number of allocations
        double utilization;                 ///< Pool utilization (0.0 - 1.0)
        bool autoDefragment;                ///< Auto-defragmentation enabled
        double lastDefragmentTime;          ///< Last defragmentation time
    };

    /**
     * @struct MemoryStatistics
     * @brief Comprehensive memory statistics
     */
    struct MemoryStatistics {
        // Overall memory usage
        size_t totalAllocated;               ///< Total memory allocated (bytes)
        size_t totalUsed;                    ///< Total memory in use (bytes)
        size_t peakMemoryUsage;              ///< Peak memory usage (bytes)
        size_t memoryFragmentation;          ///< Memory fragmentation (bytes)

        // Pool statistics
        uint32_t totalPools;                 ///< Total number of pools
        uint32_t activePools;                ///< Number of active pools
        size_t poolMemoryUsage;              ///< Memory used by pools

        // Allocation statistics
        uint64_t totalAllocations;           ///< Total number of allocations
        uint64_t activeAllocations;          ///< Number of active allocations
        uint64_t allocationFailures;         ///< Number of allocation failures
        uint64_t deallocationCount;          ///< Number of deallocations

        // Performance metrics
        double averageAllocationTime;        ///< Average allocation time (μs)
        double averageDeallocationTime;      ///< Average deallocation time (μs)
        double allocationRate;               ///< Allocations per second
        double deallocationRate;             ///< Deallocations per second

        // Memory pressure
        double memoryPressure;               ///< Memory pressure (0.0 - 1.0)
        bool isLowMemory;                    ///< Low memory warning flag
        size_t availableMemory;              ///< Available system memory

        // Garbage collection
        uint64_t gcCycles;                   ///< Garbage collection cycles
        double lastGCTime;                   ///< Last GC time
        double gcTotalTime;                  ///< Total time spent in GC
    };

    /**
     * @struct MemoryLeakReport
     * @brief Memory leak detection report
     */
    struct MemoryLeakReport {
        std::vector<MemoryAllocation> leakedAllocations;  ///< List of leaked allocations
        size_t totalLeakedBytes;            ///< Total leaked memory (bytes)
        uint64_t leakCount;                 ///< Number of leaks detected
        double detectionTime;               ///< Time when leaks were detected
        std::string report;                 ///< Human-readable report
    };

    /**
     * @class MemoryPool
     * @brief Base class for memory pools
     */
    class MemoryPool {
    public:
        /**
         * @brief Constructor
         * @param id Pool ID
         * @param name Pool name
         * @param type Pool type
         * @param initialSize Initial pool size
         */
        MemoryPool(MemoryPoolID id, const std::string& name, MemoryPoolType type, size_t initialSize);

        /**
         * @brief Destructor
         */
        virtual ~MemoryPool();

        /**
         * @brief Allocate memory from pool
         * @param size Size to allocate
         * @param alignment Memory alignment
         * @return Allocated memory address or nullptr on failure
         */
        virtual void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t)) = 0;

        /**
         * @brief Deallocate memory from pool
         * @param address Memory address to deallocate
         * @param size Size of allocation
         */
        virtual void Deallocate(void* address, size_t size = 0) = 0;

        /**
         * @brief Get pool information
         * @return Pool information
         */
        virtual MemoryPoolInfo GetInfo() const;

        /**
         * @brief Check if address belongs to this pool
         * @param address Memory address
         * @return true if belongs to pool, false otherwise
         */
        virtual bool Contains(void* address) const = 0;

        /**
         * @brief Defragment the pool
         * @return Number of bytes recovered
         */
        virtual size_t Defragment() { return 0; }

        /**
         * @brief Reset the pool (free all allocations)
         */
        virtual void Reset() = 0;

        /**
         * @brief Get pool ID
         * @return Pool ID
         */
        MemoryPoolID GetID() const { return m_id; }

        /**
         * @brief Get pool name
         * @return Pool name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Get pool type
         * @return Pool type
         */
        MemoryPoolType GetType() const { return m_type; }

    protected:
        MemoryPoolID m_id;                   ///< Pool ID
        std::string m_name;                  ///< Pool name
        MemoryPoolType m_type;               ///< Pool type
        size_t m_initialSize;                ///< Initial pool size
        size_t m_currentSize;                ///< Current pool size
        size_t m_usedSize;                   ///< Used size
        size_t m_peakUsedSize;               ///< Peak used size
        size_t m_allocationCount;            ///< Number of allocations
    };

    /**
     * @class FixedMemoryPool
     * @brief Memory pool for fixed-size allocations
     */
    class FixedMemoryPool : public MemoryPool {
    public:
        /**
         * @brief Constructor
         * @param id Pool ID
         * @param name Pool name
         * @param blockSize Size of each block
         * @param blockCount Number of blocks
         */
        FixedMemoryPool(MemoryPoolID id, const std::string& name, size_t blockSize, size_t blockCount);

        /**
         * @brief Destructor
         */
        ~FixedMemoryPool() override;

        /**
         * @brief Allocate memory from pool
         * @param size Size to allocate (must match block size)
         * @param alignment Memory alignment
         * @return Allocated memory address or nullptr on failure
         */
        void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t)) override;

        /**
         * @brief Deallocate memory from pool
         * @param address Memory address to deallocate
         * @param size Size of allocation
         */
        void Deallocate(void* address, size_t size = 0) override;

        /**
         * @brief Check if address belongs to this pool
         * @param address Memory address
         * @return true if belongs to pool, false otherwise
         */
        bool Contains(void* address) const override;

        /**
         * @brief Reset the pool
         */
        void Reset() override;

    private:
        size_t m_blockSize;                  ///< Size of each block
        size_t m_blockCount;                 ///< Total number of blocks
        std::vector<void*> m_freeBlocks;     ///< Free blocks stack
        std::vector<char> m_memory;          ///< Pool memory
        void* m_memoryStart;                 ///< Start of pool memory
        void* m_memoryEnd;                   ///< End of pool memory
    };

    /**
     * @class DynamicMemoryPool
     * @brief Memory pool that can grow dynamically
     */
    class DynamicMemoryPool : public MemoryPool {
    public:
        /**
         * @brief Constructor
         * @param id Pool ID
         * @param name Pool name
         * @param initialSize Initial pool size
         * @param growthFactor Growth factor for expansion
         */
        DynamicMemoryPool(MemoryPoolID id, const std::string& name,
                         size_t initialSize, float growthFactor = 2.0f);

        /**
         * @brief Destructor
         */
        ~DynamicMemoryPool() override;

        /**
         * @brief Allocate memory from pool
         * @param size Size to allocate
         * @param alignment Memory alignment
         * @return Allocated memory address or nullptr on failure
         */
        void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t)) override;

        /**
         * @brief Deallocate memory from pool
         * @param address Memory address to deallocate
         * @param size Size of allocation
         */
        void Deallocate(void* address, size_t size = 0) override;

        /**
         * @brief Check if address belongs to pool
         * @param address Memory address
         * @return true if belongs to pool, false otherwise
         */
        bool Contains(void* address) const override;

        /**
         * @brief Defragment the pool
         * @return Number of bytes recovered
         */
        size_t Defragment() override;

        /**
         * @brief Reset the pool
         */
        void Reset() override;

    private:
        /**
         * @brief Expand the pool
         * @param minSize Minimum size needed
         */
        void Expand(size_t minSize);

        float m_growthFactor;                ///< Pool growth factor
        std::vector<std::unique_ptr<char[]>> m_memoryBlocks;  ///< Memory blocks
        std::vector<void*> m_freeList;       ///< Free memory list
        void* m_memoryStart;                 ///< Start of all memory
        void* m_memoryEnd;                   ///< End of all memory
    };

    /**
     * @class MemoryManager
     * @brief Central memory management system
     */
    class MemoryManager {
    public:
        /**
         * @brief Constructor
         */
        MemoryManager();

        /**
         * @brief Destructor
         */
        ~MemoryManager();

        /**
         * @brief Deleted copy constructor
         */
        MemoryManager(const MemoryManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        MemoryManager& operator=(const MemoryManager&) = delete;

        // Memory pool management

        /**
         * @brief Create a new memory pool
         * @param name Pool name
         * @param type Pool type
         * @param initialSize Initial pool size
         * @param config Pool-specific configuration
         * @return Pool ID or empty optional on failure
         */
        std::optional<MemoryPoolID> CreatePool(const std::string& name,
                                             MemoryPoolType type,
                                             size_t initialSize,
                                             const std::unordered_map<std::string, std::any>& config = {});

        /**
         * @brief Create fixed-size memory pool
         * @param name Pool name
         * @param blockSize Size of each block
         * @param blockCount Number of blocks
         * @return Pool ID or empty optional on failure
         */
        std::optional<MemoryPoolID> CreateFixedPool(const std::string& name,
                                                  size_t blockSize,
                                                  size_t blockCount);

        /**
         * @brief Get memory pool
         * @param poolId Pool ID
         * @return Pool pointer or nullptr if not found
         */
        MemoryPool* GetPool(MemoryPoolID poolId);

        /**
         * @brief Get memory pool by name
         * @param name Pool name
         * @return Pool pointer or nullptr if not found
         */
        MemoryPool* GetPool(const std::string& name);

        /**
         * @brief Destroy memory pool
         * @param poolId Pool ID
         * @return true if destroyed, false if not found
         */
        bool DestroyPool(MemoryPoolID poolId);

        /**
         * @brief Get all pool information
         * @return Vector of pool information
         */
        std::vector<MemoryPoolInfo> GetPoolInfo() const;

        // Memory allocation

        /**
         * @brief Allocate memory from specific pool
         * @param poolId Pool ID
         * @param size Size to allocate
         * @param alignment Memory alignment
         * @return Allocated memory address or nullptr on failure
         */
        void* AllocateFromPool(MemoryPoolID poolId,
                             size_t size,
                             size_t alignment = alignof(std::max_align_t));

        /**
         * @brief Allocate memory from pool by name
         * @param poolName Pool name
         * @param size Size to allocate
         * @param alignment Memory alignment
         * @return Allocated memory address or nullptr on failure
         */
        void* AllocateFromPool(const std::string& poolName,
                             size_t size,
                             size_t alignment = alignof(std::max_align_t));

        /**
         * @brief Deallocate memory from specific pool
         * @param poolId Pool ID
         * @param address Memory address to deallocate
         * @param size Size of allocation
         * @return true if deallocated, false on failure
         */
        bool DeallocateFromPool(MemoryPoolID poolId, void* address, size_t size = 0);

        // General memory allocation (with tracking)

        /**
         * @brief Allocate tracked memory
         * @param size Size to allocate
         * @param type Allocation type/category
         * @param file Source file
         * @param line Source line
         * @return Allocated memory address or nullptr on failure
         */
        void* Allocate(size_t size,
                      const std::string& type = "unknown",
                      const char* file = __FILE__,
                      uint32_t line = __LINE__);

        /**
         * @brief Deallocate tracked memory
         * @param address Memory address to deallocate
         * @return true if deallocated, false on failure
         */
        bool Deallocate(void* address);

        /**
         * @brief Reallocate tracked memory
         * @param address Current memory address
         * @param newSize New size
         * @param type Allocation type/category
         * @return New memory address or nullptr on failure
         */
        void* Reallocate(void* address,
                        size_t newSize,
                        const std::string& type = "unknown");

        // Memory statistics and monitoring

        /**
         * @brief Get memory statistics
         * @return Memory statistics
         */
        MemoryStatistics GetStatistics() const;

        /**
         * @brief Get memory leak report
         * @return Memory leak report
         */
        MemoryLeakReport GetLeakReport() const;

        /**
         * @brief Check for memory leaks
         * @return true if leaks detected, false otherwise
         */
        bool CheckForLeaks();

        /**
         * @brief Get allocation information
         * @param address Memory address
         * @return Allocation information or empty optional
         */
        std::optional<MemoryAllocation> GetAllocationInfo(void* address) const;

        /**
         * @brief Get all active allocations
         * @return Vector of active allocations
         */
        std::vector<MemoryAllocation> GetActiveAllocations() const;

        // Memory optimization

        /**
         * @brief Defragment all pools
         * @return Total bytes recovered
         */
        size_t DefragmentAllPools();

        /**
         * @brief Run garbage collection
         * @return Number of objects collected
         */
        size_t RunGarbageCollection();

        /**
         * @brief Optimize memory usage
         * @return Report of optimizations performed
         */
        std::string OptimizeMemory();

        // Configuration

        /**
         * @brief Set memory limit
         * @param limit Memory limit in bytes
         */
        void SetMemoryLimit(size_t limit);

        /**
         * @brief Get memory limit
         * @return Memory limit in bytes
         */
        size_t GetMemoryLimit() const { return m_memoryLimit; }

        /**
         * @brief Enable/disable leak detection
         * @param enabled Enable state
         */
        void SetLeakDetection(bool enabled);

        /**
         * @brief Check if leak detection is enabled
         * @return true if enabled, false otherwise
         */
        bool IsLeakDetectionEnabled() const { return m_leakDetectionEnabled; }

        // Memory debugging

        /**
         * @brief Take memory snapshot
         * @param name Snapshot name
         * @return true if successful, false otherwise
         */
        bool TakeMemorySnapshot(const std::string& name);

        /**
         * @brief Compare memory snapshots
         * @param snapshot1 First snapshot name
         * @param snapshot2 Second snapshot name
         * @return Comparison report
         */
        std::string CompareMemorySnapshots(const std::string& snapshot1,
                                         const std::string& snapshot2);

        /**
         * @brief Get memory usage report
         * @return Detailed memory usage report
         */
        std::string GetMemoryReport() const;

    private:
        /**
         * @brief Update memory statistics
         */
        void UpdateStatistics();

        /**
         * @brief Check memory pressure
         */
        void CheckMemoryPressure();

        /**
         * @brief Cleanup completed allocations
         */
        void CleanupCompletedAllocations();

        // Memory pools
        std::unordered_map<MemoryPoolID, std::unique_ptr<MemoryPool>> m_pools;
        std::unordered_map<std::string, MemoryPoolID> m_poolNames;
        std::shared_mutex m_poolMutex;
        std::atomic<MemoryPoolID> m_nextPoolId;

        // Memory tracking
        std::unordered_map<void*, MemoryAllocation> m_allocations;
        std::unordered_map<AllocationID, void*> m_allocationIds;
        mutable std::shared_mutex m_allocationMutex;
        std::atomic<AllocationID> m_nextAllocationId;

        // Statistics
        MemoryStatistics m_statistics;
        mutable std::mutex m_statsMutex;

        // Configuration
        size_t m_memoryLimit;                ///< Memory usage limit
        bool m_leakDetectionEnabled;         ///< Leak detection enabled
        bool m_autoDefragment;               ///< Auto-defragmentation enabled
        double m_gcThreshold;                ///< Garbage collection threshold
        size_t m_snapshotCount;              ///< Number of memory snapshots

        // Memory snapshots for debugging
        std::unordered_map<std::string, std::vector<MemoryAllocation>> m_snapshots;
        mutable std::mutex m_snapshotMutex;
    };

    /**
     * @class MemoryTracker
     * @brief RAII memory tracking helper
     */
    class MemoryTracker {
    public:
        /**
         * @brief Constructor
         * @param type Allocation type
         * @param file Source file
         * @param line Source line
         */
        MemoryTracker(const std::string& type,
                    const char* file = __FILE__,
                    uint32_t line = __LINE__);

        /**
         * @brief Destructor
         */
        ~MemoryTracker();

        /**
         * @brief Track allocation
         * @param address Memory address
         * @param size Allocation size
         */
        void Track(void* address, size_t size);

        /**
         * @brief Untrack allocation
         * @param address Memory address
         */
        void Untrack(void* address);

    private:
        std::string m_type;                  ///< Allocation type
        std::string m_file;                  ///< Source file
        uint32_t m_line;                     ///< Source line
        std::vector<std::pair<void*, size_t>> m_trackedAllocations;  ///< Tracked allocations
    };

    // Global memory manager accessor

    /**
     * @brief Get global memory manager instance
     * @return Memory manager reference
     */
    MemoryManager& GetMemoryManager();

    // Memory allocation macros

    /**
     * @def VOXELCRAFT_NEW(type, ...)
     * @brief Tracked memory allocation
     */
    #define VOXELCRAFT_NEW(type, ...) \
        new(MemoryManager::Get().Allocate(sizeof(type), #type, __FILE__, __LINE__)) type(__VA_ARGS__)

    /**
     * @def VOXELCRAFT_DELETE(ptr)
     * @brief Tracked memory deallocation
     */
    #define VOXELCRAFT_DELETE(ptr) \
        { MemoryManager::Get().Deallocate(ptr); ptr = nullptr; }

    /**
     * @def VOXELCRAFT_MALLOC(size)
     * @brief Tracked memory allocation
     */
    #define VOXELCRAFT_MALLOC(size) \
        MemoryManager::Get().Allocate(size, "malloc", __FILE__, __LINE__)

    /**
     * @def VOXELCRAFT_FREE(ptr)
     * @brief Tracked memory deallocation
     */
    #define VOXELCRAFT_FREE(ptr) \
        MemoryManager::Get().Deallocate(ptr)

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_MEMORY_MANAGER_HPP
