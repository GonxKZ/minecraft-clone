/**
 * @file MemoryManager.hpp
 * @brief VoxelCraft Memory Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_MEMORY_MANAGER_HPP
#define VOXELCRAFT_CORE_MEMORY_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>

namespace VoxelCraft {

/**
 * @enum MemoryPoolType
 * @brief Types of memory pools
 */
enum class MemoryPoolType {
    GENERAL = 0,    ///< General purpose allocations
    CHUNK_DATA,     ///< Chunk data allocations
    ENTITY_DATA,    ///< Entity data allocations
    TEXTURE_DATA,   ///< Texture data allocations
    AUDIO_DATA,     ///< Audio data allocations
    TEMPORARY       ///< Temporary allocations
};

/**
 * @struct MemoryPool
 * @brief Memory pool for efficient allocations
 */
struct MemoryPool {
    void* pool;                     ///< Pool memory block
    size_t blockSize;               ///< Size of each block
    size_t totalBlocks;             ///< Total number of blocks
    size_t usedBlocks;              ///< Number of used blocks
    std::vector<bool> freeList;     ///< Free block tracking
    std::mutex mutex;               ///< Pool synchronization
};

/**
 * @struct MemoryAllocation
 * @brief Information about a memory allocation
 */
struct MemoryAllocation {
    void* address;                  ///< Memory address
    size_t size;                    ///< Allocation size
    MemoryPoolType poolType;        ///< Pool type
    std::chrono::steady_clock::time_point timestamp; ///< Allocation timestamp
    std::string tag;                ///< Allocation tag for debugging
};

/**
 * @struct MemoryMetrics
 * @brief Memory usage metrics
 */
struct MemoryMetrics {
    size_t totalAllocated;          ///< Total memory allocated (bytes)
    size_t totalUsed;               ///< Total memory used (bytes)
    size_t totalFree;               ///< Total free memory (bytes)
    size_t peakUsage;               ///< Peak memory usage (bytes)
    uint64_t allocationCount;       ///< Total number of allocations
    uint64_t deallocationCount;     ///< Total number of deallocations
    uint64_t allocationFailures;    ///< Number of allocation failures
    double fragmentationRatio;      ///< Memory fragmentation ratio (0-1)
    uint32_t activePools;           ///< Number of active pools
    uint32_t totalPools;            ///< Total number of pools
};

/**
 * @struct MemoryLeakReport
 * @brief Memory leak detection report
 */
struct MemoryLeakReport {
    std::vector<MemoryAllocation> leakedAllocations;
    size_t totalLeakedBytes;
    uint32_t leakCount;
};

/**
 * @class MemoryManager
 * @brief Advanced memory management system
 *
 * The MemoryManager provides:
 * - Custom memory pools for different allocation types
 * - Memory leak detection and reporting
 * - Performance monitoring and profiling
 * - Automatic memory defragmentation
 * - Thread-safe memory operations
 * - Memory usage statistics
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

    /**
     * @brief Initialize the memory manager
     * @param enableLeakDetection Enable memory leak detection
     * @param enableProfiling Enable memory profiling
     * @return true if successful, false otherwise
     */
    bool Initialize(bool enableLeakDetection = true, bool enableProfiling = true);

    /**
     * @brief Shutdown the memory manager
     */
    void Shutdown();

    /**
     * @brief Update memory manager state
     * @param deltaTime Time elapsed since last update
     */
    void Update(double deltaTime);

    /**
     * @brief Update memory usage statistics
     */
    void UpdateStatistics();

    /**
     * @brief Get memory metrics
     * @return Memory usage metrics
     */
    const MemoryMetrics& GetMetrics() const { return m_metrics; }

    /**
     * @brief Allocate memory from specific pool
     * @param size Size to allocate
     * @param poolType Pool type to use
     * @param tag Allocation tag for debugging
     * @return Allocated memory address, nullptr on failure
     */
    void* Allocate(size_t size, MemoryPoolType poolType = MemoryPoolType::GENERAL, const std::string& tag = "");

    /**
     * @brief Allocate aligned memory
     * @param size Size to allocate
     * @param alignment Memory alignment
     * @param poolType Pool type to use
     * @param tag Allocation tag for debugging
     * @return Allocated memory address, nullptr on failure
     */
    void* AllocateAligned(size_t size, size_t alignment, MemoryPoolType poolType = MemoryPoolType::GENERAL, const std::string& tag = "");

    /**
     * @brief Deallocate memory
     * @param address Memory address to deallocate
     * @param poolType Pool type where memory was allocated
     */
    void Deallocate(void* address, MemoryPoolType poolType = MemoryPoolType::GENERAL);

    /**
     * @brief Reallocate memory
     * @param address Current memory address
     * @param newSize New size
     * @param poolType Pool type
     * @param tag Allocation tag for debugging
     * @return New memory address, nullptr on failure
     */
    void* Reallocate(void* address, size_t newSize, MemoryPoolType poolType = MemoryPoolType::GENERAL, const std::string& tag = "");

    /**
     * @brief Create a new memory pool
     * @param poolType Pool type
     * @param blockSize Size of each block
     * @param blockCount Number of blocks
     * @return true if successful, false otherwise
     */
    bool CreatePool(MemoryPoolType poolType, size_t blockSize, size_t blockCount);

    /**
     * @brief Destroy a memory pool
     * @param poolType Pool type to destroy
     * @return true if successful, false otherwise
     */
    bool DestroyPool(MemoryPoolType poolType);

    /**
     * @brief Get pool information
     * @param poolType Pool type
     * @return Pool information, nullptr if not found
     */
    const MemoryPool* GetPool(MemoryPoolType poolType) const;

    /**
     * @brief Perform garbage collection
     * @param aggressive Perform aggressive garbage collection
     */
    void GarbageCollect(bool aggressive = false);

    /**
     * @brief Defragment memory pools
     */
    void Defragment();

    /**
     * @brief Check for memory leaks
     * @return Memory leak report
     */
    MemoryLeakReport CheckForLeaks();

    /**
     * @brief Dump memory statistics to log
     */
    void DumpStatistics();

    /**
     * @brief Set memory allocation callback
     * @param callback Callback function
     */
    void SetAllocationCallback(std::function<void(const MemoryAllocation&)> callback) {
        m_allocationCallback = callback;
    }

    /**
     * @brief Set memory deallocation callback
     * @param callback Callback function
     */
    void SetDeallocationCallback(std::function<void(void*)> callback) {
        m_deallocationCallback = callback;
    }

    /**
     * @brief Enable/disable leak detection
     * @param enabled Enable leak detection
     */
    void SetLeakDetection(bool enabled) { m_leakDetectionEnabled = enabled; }

    /**
     * @brief Check if leak detection is enabled
     * @return true if enabled
     */
    bool IsLeakDetectionEnabled() const { return m_leakDetectionEnabled; }

    /**
     * @brief Enable/disable profiling
     * @param enabled Enable profiling
     */
    void SetProfilingEnabled(bool enabled) { m_profilingEnabled = enabled; }

    /**
     * @brief Check if profiling is enabled
     * @return true if enabled
     */
    bool IsProfilingEnabled() const { return m_profilingEnabled; }

    // Memory debugging functions

    /**
     * @brief Validate memory integrity
     * @return true if memory is valid
     */
    bool ValidateMemory();

    /**
     * @brief Get allocation information
     * @param address Memory address
     * @return Allocation information, empty if not found
     */
    MemoryAllocation GetAllocationInfo(void* address) const;

    /**
     * @brief List all active allocations
     * @return Vector of active allocations
     */
    std::vector<MemoryAllocation> GetActiveAllocations() const;

private:
    // Memory pools
    std::unordered_map<MemoryPoolType, std::unique_ptr<MemoryPool>> m_pools;

    // Allocation tracking
    std::unordered_map<void*, MemoryAllocation> m_allocations;
    mutable std::mutex m_allocationsMutex;

    // Metrics
    MemoryMetrics m_metrics;
    mutable std::mutex m_metricsMutex;

    // Configuration
    bool m_leakDetectionEnabled;
    bool m_profilingEnabled;
    size_t m_defaultPoolBlockSize;
    size_t m_defaultPoolBlockCount;

    // Callbacks
    std::function<void(const MemoryAllocation&)> m_allocationCallback;
    std::function<void(void*)> m_deallocationCallback;

    // Internal methods

    /**
     * @brief Allocate from specific pool
     * @param pool Pool to allocate from
     * @param size Size to allocate
     * @param tag Allocation tag
     * @return Allocated address, nullptr on failure
     */
    void* AllocateFromPool(MemoryPool* pool, size_t size, const std::string& tag);

    /**
     * @brief Deallocate from specific pool
     * @param pool Pool to deallocate from
     * @param address Address to deallocate
     */
    void DeallocateFromPool(MemoryPool* pool, void* address);

    /**
     * @brief Create pool memory block
     * @param blockSize Size of each block
     * @param blockCount Number of blocks
     * @return Pool memory block
     */
    void* CreatePoolMemory(size_t blockSize, size_t blockCount);

    /**
     * @brief Destroy pool memory block
     * @param poolMemory Pool memory to destroy
     */
    void DestroyPoolMemory(void* poolMemory);

    /**
     * @brief Find free block in pool
     * @param pool Pool to search
     * @return Free block index, -1 if none found
     */
    int FindFreeBlock(const MemoryPool* pool);

    /**
     * @brief Calculate fragmentation ratio for pool
     * @param pool Pool to analyze
     * @return Fragmentation ratio (0-1)
     */
    double CalculateFragmentation(const MemoryPool* pool);

    /**
     * @brief Track allocation
     * @param allocation Allocation to track
     */
    void TrackAllocation(const MemoryAllocation& allocation);

    /**
     * @brief Untrack allocation
     * @param address Address to untrack
     */
    void UntrackAllocation(void* address);

    /**
     * @brief Update peak usage
     * @param currentUsage Current memory usage
     */
    void UpdatePeakUsage(size_t currentUsage);
};

// Global memory management functions

/**
 * @brief Global memory allocation
 * @param size Size to allocate
 * @param poolType Pool type
 * @param tag Allocation tag
 * @return Allocated memory
 */
void* MemoryAllocate(size_t size, MemoryPoolType poolType = MemoryPoolType::GENERAL, const std::string& tag = "");

/**
 * @brief Global memory deallocation
 * @param address Memory address
 * @param poolType Pool type
 */
void MemoryDeallocate(void* address, MemoryPoolType poolType = MemoryPoolType::GENERAL);

/**
 * @brief Global aligned memory allocation
 * @param size Size to allocate
 * @param alignment Memory alignment
 * @param poolType Pool type
 * @param tag Allocation tag
 * @return Allocated memory
 */
void* MemoryAllocateAligned(size_t size, size_t alignment, MemoryPoolType poolType = MemoryPoolType::GENERAL, const std::string& tag = "");

/**
 * @brief Global memory reallocation
 * @param address Current address
 * @param newSize New size
 * @param poolType Pool type
 * @param tag Allocation tag
 * @return New memory address
 */
void* MemoryReallocate(void* address, size_t newSize, MemoryPoolType poolType = MemoryPoolType::GENERAL, const std::string& tag = "");

/**
 * @brief Get global memory manager instance
 * @return Memory manager instance
 */
MemoryManager& GetMemoryManager();

} // namespace VoxelCraft

// Custom memory allocation operators

/**
 * @brief Custom new operator with memory tracking
 */
void* operator new(size_t size);

/**
 * @brief Custom new operator with memory tracking (nothrow)
 */
void* operator new(size_t size, std::nothrow_t) noexcept;

/**
 * @brief Custom new[] operator with memory tracking
 */
void* operator new[](size_t size);

/**
 * @brief Custom new[] operator with memory tracking (nothrow)
 */
void* operator new[](size_t size, std::nothrow_t) noexcept;

/**
 * @brief Custom delete operator with memory tracking
 */
void operator delete(void* address) noexcept;

/**
 * @brief Custom delete operator with memory tracking (nothrow)
 */
void operator delete(void* address, std::nothrow_t) noexcept;

/**
 * @brief Custom delete[] operator with memory tracking
 */
void operator delete[](void* address) noexcept;

/**
 * @brief Custom delete[] operator with memory tracking (nothrow)
 */
void operator delete[](void* address, std::nothrow_t) noexcept;

#endif // VOXELCRAFT_CORE_MEMORY_MANAGER_HPP