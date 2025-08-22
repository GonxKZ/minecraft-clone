/**
 * @file MemoryManager.cpp
 * @brief VoxelCraft Engine Memory Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the memory management system for the VoxelCraft engine.
 */

#include "MemoryManager.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <stdexcept>

namespace VoxelCraft {

    // Global memory manager instance
    static std::unique_ptr<MemoryManager> s_instance;
    static std::mutex s_instanceMutex;

    // MemoryPool implementation

    MemoryPool::MemoryPool(MemoryPoolID id, const std::string& name, MemoryPoolType type, size_t initialSize)
        : m_id(id)
        , m_name(name)
        , m_type(type)
        , m_initialSize(initialSize)
        , m_currentSize(initialSize)
        , m_usedSize(0)
        , m_peakUsedSize(0)
        , m_allocationCount(0)
    {
    }

    MemoryPool::~MemoryPool() {
        VOXELCRAFT_TRACE("Memory pool '{}' destroyed", m_name);
    }

    MemoryPoolInfo MemoryPool::GetInfo() const {
        MemoryPoolInfo info;
        info.id = m_id;
        info.name = m_name;
        info.type = m_type;
        info.totalSize = m_currentSize;
        info.usedSize = m_usedSize;
        info.peakUsedSize = m_peakUsedSize;
        info.allocationCount = m_allocationCount;
        info.utilization = m_currentSize > 0 ? static_cast<double>(m_usedSize) / m_currentSize : 0.0;
        info.autoDefragment = false;
        info.lastDefragmentTime = 0.0;

        return info;
    }

    // FixedMemoryPool implementation

    FixedMemoryPool::FixedMemoryPool(MemoryPoolID id, const std::string& name, size_t blockSize, size_t blockCount)
        : MemoryPool(id, name, MemoryPoolType::Fixed, blockSize * blockCount)
        , m_blockSize(blockSize)
        , m_blockCount(blockCount)
    {
        // Allocate memory
        m_memory.resize(blockSize * blockCount);
        m_memoryStart = m_memory.data();
        m_memoryEnd = m_memory.data() + m_memory.size();

        // Initialize free blocks
        for (size_t i = 0; i < blockCount; ++i) {
            void* block = m_memory.data() + (i * blockSize);
            m_freeBlocks.push_back(block);
        }

        VOXELCRAFT_INFO("Fixed memory pool '{}' created: {} blocks of {} bytes each",
                      name, blockCount, blockSize);
    }

    FixedMemoryPool::~FixedMemoryPool() {
        VOXELCRAFT_TRACE("Fixed memory pool '{}' destroyed", m_name);
    }

    void* FixedMemoryPool::Allocate(size_t size, size_t alignment) {
        if (size != m_blockSize) {
            VOXELCRAFT_ERROR("Fixed pool '{}' allocation size mismatch: requested {}, block size {}",
                           m_name, size, m_blockSize);
            return nullptr;
        }

        if (m_freeBlocks.empty()) {
            VOXELCRAFT_WARNING("Fixed pool '{}' out of memory", m_name);
            return nullptr;
        }

        void* block = m_freeBlocks.back();
        m_freeBlocks.pop_back();

        m_usedSize += size;
        m_peakUsedSize = std::max(m_peakUsedSize, m_usedSize);
        m_allocationCount++;

        VOXELCRAFT_TRACE("Fixed pool '{}' allocation: {} bytes at {}", m_name, size, block);
        return block;
    }

    void FixedMemoryPool::Deallocate(void* address, size_t size) {
        if (!Contains(address)) {
            VOXELCRAFT_ERROR("Fixed pool '{}' deallocation of invalid address: {}", m_name, address);
            return;
        }

        m_freeBlocks.push_back(address);
        m_usedSize -= m_blockSize;
        m_allocationCount--;

        VOXELCRAFT_TRACE("Fixed pool '{}' deallocation: {} bytes at {}", m_name, m_blockSize, address);
    }

    bool FixedMemoryPool::Contains(void* address) const {
        return address >= m_memoryStart && address < m_memoryEnd;
    }

    void FixedMemoryPool::Reset() {
        m_freeBlocks.clear();
        for (size_t i = 0; i < m_blockCount; ++i) {
            void* block = m_memory.data() + (i * m_blockSize);
            m_freeBlocks.push_back(block);
        }
        m_usedSize = 0;
        m_allocationCount = 0;

        VOXELCRAFT_INFO("Fixed pool '{}' reset", m_name);
    }

    // DynamicMemoryPool implementation

    DynamicMemoryPool::DynamicMemoryPool(MemoryPoolID id, const std::string& name,
                                       size_t initialSize, float growthFactor)
        : MemoryPool(id, name, MemoryPoolType::Dynamic, initialSize)
        , m_growthFactor(growthFactor)
    {
        // Allocate initial memory block
        auto block = std::make_unique<char[]>(initialSize);
        m_memoryStart = block.get();
        m_memoryEnd = block.get() + initialSize;
        m_memoryBlocks.push_back(std::move(block));

        VOXELCRAFT_INFO("Dynamic memory pool '{}' created: {} bytes initial size",
                      name, initialSize);
    }

    DynamicMemoryPool::~DynamicMemoryPool() {
        VOXELCRAFT_TRACE("Dynamic memory pool '{}' destroyed", m_name);
    }

    void* DynamicMemoryPool::Allocate(size_t size, size_t alignment) {
        // Simple first-fit allocation strategy
        for (auto it = m_freeList.begin(); it != m_freeList.end(); ++it) {
            void* block = *it;
            // For simplicity, we'll use the first available block that's large enough
            if (block != nullptr) {
                m_freeList.erase(it);
                m_usedSize += size;
                m_peakUsedSize = std::max(m_peakUsedSize, m_usedSize);
                m_allocationCount++;
                return block;
            }
        }

        // No suitable free block found, expand pool
        Expand(size);
        return Allocate(size, alignment); // Retry allocation
    }

    void DynamicMemoryPool::Deallocate(void* address, size_t size) {
        if (!Contains(address)) {
            VOXELCRAFT_ERROR("Dynamic pool '{}' deallocation of invalid address: {}", m_name, address);
            return;
        }

        m_freeList.push_back(address);
        m_usedSize -= size;
        m_allocationCount--;

        VOXELCRAFT_TRACE("Dynamic pool '{}' deallocation: {} bytes at {}", m_name, size, address);
    }

    bool DynamicMemoryPool::Contains(void* address) const {
        return address >= m_memoryStart && address < m_memoryEnd;
    }

    size_t DynamicMemoryPool::Defragment() {
        // Simple defragmentation - sort free list
        std::sort(m_freeList.begin(), m_freeList.end());
        return 0; // No bytes recovered in this simple implementation
    }

    void DynamicMemoryPool::Reset() {
        m_freeList.clear();
        m_usedSize = 0;
        m_allocationCount = 0;

        // Add entire memory as free block
        m_freeList.push_back(m_memoryStart);

        VOXELCRAFT_INFO("Dynamic pool '{}' reset", m_name);
    }

    void DynamicMemoryPool::Expand(size_t minSize) {
        size_t newSize = std::max(minSize, static_cast<size_t>(m_currentSize * m_growthFactor));

        auto newBlock = std::make_unique<char[]>(newSize);
        char* newMemory = newBlock.get();

        // Add to free list
        m_freeList.push_back(newMemory);

        // Update memory range
        if (newMemory < m_memoryStart) {
            m_memoryStart = newMemory;
        }
        if (newMemory + newSize > m_memoryEnd) {
            m_memoryEnd = newMemory + newSize;
        }

        m_currentSize += newSize;
        m_memoryBlocks.push_back(std::move(newBlock));

        VOXELCRAFT_INFO("Dynamic pool '{}' expanded by {} bytes, new size: {}",
                      m_name, newSize, m_currentSize);
    }

    // MemoryManager implementation

    MemoryManager::MemoryManager()
        : m_nextPoolId(1)
        , m_nextAllocationId(1)
        , m_memoryLimit(0) // No limit by default
        , m_leakDetectionEnabled(true)
        , m_autoDefragment(false)
        , m_gcThreshold(0.8)
        , m_snapshotCount(0)
    {
        VOXELCRAFT_TRACE("Memory manager instance created");
    }

    MemoryManager::~MemoryManager() {
        VOXELCRAFT_TRACE("Memory manager instance destroyed");

        // Clean up pools
        std::unique_lock<std::shared_mutex> lock(m_poolMutex);
        m_pools.clear();
    }

    std::optional<MemoryPoolID> MemoryManager::CreatePool(
        const std::string& name,
        MemoryPoolType type,
        size_t initialSize,
        const std::unordered_map<std::string, std::any>& config
    ) {
        std::unique_lock<std::shared_mutex> lock(m_poolMutex);

        // Check if pool name already exists
        if (m_poolNames.count(name) > 0) {
            VOXELCRAFT_ERROR("Memory pool with name '{}' already exists", name);
            return std::nullopt;
        }

        MemoryPoolID poolId = m_nextPoolId++;
        std::unique_ptr<MemoryPool> pool;

        switch (type) {
            case MemoryPoolType::Fixed: {
                size_t blockSize = std::any_cast<size_t>(config.at("blockSize"));
                size_t blockCount = std::any_cast<size_t>(config.at("blockCount"));
                pool = std::make_unique<FixedMemoryPool>(poolId, name, blockSize, blockCount);
                break;
            }
            case MemoryPoolType::Dynamic: {
                float growthFactor = config.count("growthFactor") > 0 ?
                    std::any_cast<float>(config.at("growthFactor")) : 2.0f;
                pool = std::make_unique<DynamicMemoryPool>(poolId, name, initialSize, growthFactor);
                break;
            }
            default:
                VOXELCRAFT_ERROR("Unsupported memory pool type: {}", static_cast<int>(type));
                return std::nullopt;
        }

        m_pools[poolId] = std::move(pool);
        m_poolNames[name] = poolId;

        VOXELCRAFT_INFO("Created memory pool '{}' (ID: {}) of type {}", name, poolId, static_cast<int>(type));
        return poolId;
    }

    std::optional<MemoryPoolID> MemoryManager::CreateFixedPool(
        const std::string& name,
        size_t blockSize,
        size_t blockCount
    ) {
        std::unordered_map<std::string, std::any> config;
        config["blockSize"] = blockSize;
        config["blockCount"] = blockCount;

        return CreatePool(name, MemoryPoolType::Fixed, blockSize * blockCount, config);
    }

    MemoryPool* MemoryManager::GetPool(MemoryPoolID poolId) {
        std::shared_lock<std::shared_mutex> lock(m_poolMutex);
        auto it = m_pools.find(poolId);
        return it != m_pools.end() ? it->second.get() : nullptr;
    }

    MemoryPool* MemoryManager::GetPool(const std::string& name) {
        std::shared_lock<std::shared_mutex> lock(m_poolMutex);
        auto it = m_poolNames.find(name);
        if (it != m_poolNames.end()) {
            return GetPool(it->second);
        }
        return nullptr;
    }

    bool MemoryManager::DestroyPool(MemoryPoolID poolId) {
        std::unique_lock<std::shared_mutex> lock(m_poolMutex);
        auto it = m_pools.find(poolId);
        if (it != m_pools.end()) {
            std::string name = it->second->GetName();
            m_pools.erase(it);
            m_poolNames.erase(name);
            VOXELCRAFT_INFO("Destroyed memory pool '{}' (ID: {})", name, poolId);
            return true;
        }
        return false;
    }

    std::vector<MemoryPoolInfo> MemoryManager::GetPoolInfo() const {
        std::shared_lock<std::shared_mutex> lock(m_poolMutex);
        std::vector<MemoryPoolInfo> info;

        for (const auto& pair : m_pools) {
            info.push_back(pair.second->GetInfo());
        }

        return info;
    }

    void* MemoryManager::AllocateFromPool(MemoryPoolID poolId, size_t size, size_t alignment) {
        MemoryPool* pool = GetPool(poolId);
        if (!pool) {
            VOXELCRAFT_ERROR("Memory pool with ID {} not found", poolId);
            return nullptr;
        }

        void* address = pool->Allocate(size, alignment);
        if (!address) {
            VOXELCRAFT_ERROR("Failed to allocate {} bytes from pool '{}'", size, pool->GetName());
        }

        return address;
    }

    void* MemoryManager::AllocateFromPool(const std::string& poolName, size_t size, size_t alignment) {
        MemoryPool* pool = GetPool(poolName);
        if (!pool) {
            VOXELCRAFT_ERROR("Memory pool with name '{}' not found", poolName);
            return nullptr;
        }

        void* address = pool->Allocate(size, alignment);
        if (!address) {
            VOXELCRAFT_ERROR("Failed to allocate {} bytes from pool '{}'", size, pool->GetName());
        }

        return address;
    }

    bool MemoryManager::DeallocateFromPool(MemoryPoolID poolId, void* address, size_t size) {
        MemoryPool* pool = GetPool(poolId);
        if (!pool) {
            VOXELCRAFT_ERROR("Memory pool with ID {} not found", poolId);
            return false;
        }

        pool->Deallocate(address, size);
        return true;
    }

    void* MemoryManager::Allocate(size_t size, const std::string& type, const char* file, uint32_t line) {
        void* address = std::malloc(size);
        if (!address) {
            VOXELCRAFT_ERROR("Failed to allocate {} bytes", size);
            return nullptr;
        }

        // Track allocation if leak detection is enabled
        if (m_leakDetectionEnabled) {
            std::unique_lock<std::shared_mutex> lock(m_allocationMutex);

            AllocationID allocId = m_nextAllocationId++;
            MemoryAllocation alloc{
                allocId,
                address,
                size,
                type,
                file ? std::string(file) : "",
                line,
                0.0, // timestamp
                true,
                0    // poolId
            };

            m_allocations[address] = alloc;
            m_allocationIds[allocId] = address;
        }

        return address;
    }

    bool MemoryManager::Deallocate(void* address) {
        if (!address) {
            return false;
        }

        // Remove from tracking
        if (m_leakDetectionEnabled) {
            std::unique_lock<std::shared_mutex> lock(m_allocationMutex);
            auto it = m_allocations.find(address);
            if (it != m_allocations.end()) {
                m_allocationIds.erase(it->second.id);
                m_allocations.erase(it);
            }
        }

        std::free(address);
        return true;
    }

    void* MemoryManager::Reallocate(void* address, size_t newSize, const std::string& type) {
        void* newAddress = std::realloc(address, newSize);
        if (!newAddress) {
            VOXELCRAFT_ERROR("Failed to reallocate {} bytes", newSize);
            return nullptr;
        }

        // Update tracking
        if (m_leakDetectionEnabled && address != newAddress) {
            std::unique_lock<std::shared_mutex> lock(m_allocationMutex);
            auto it = m_allocations.find(address);
            if (it != m_allocations.end()) {
                MemoryAllocation alloc = it->second;
                alloc.address = newAddress;
                alloc.size = newSize;
                m_allocations.erase(it);
                m_allocations[newAddress] = alloc;
                m_allocationIds[alloc.id] = newAddress;
            }
        }

        return newAddress;
    }

    MemoryStatistics MemoryManager::GetStatistics() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_statistics;
    }

    MemoryLeakReport MemoryManager::GetLeakReport() const {
        std::shared_lock<std::shared_mutex> lock(m_allocationMutex);

        MemoryLeakReport report;
        report.totalLeakedBytes = 0;
        report.leakCount = 0;

        for (const auto& pair : m_allocations) {
            const MemoryAllocation& alloc = pair.second;
            if (alloc.isActive) {
                report.leakedAllocations.push_back(alloc);
                report.totalLeakedBytes += alloc.size;
                report.leakCount++;
            }
        }

        std::stringstream ss;
        ss << "Memory Leak Report:\n";
        ss << "Total leaked bytes: " << report.totalLeakedBytes << "\n";
        ss << "Number of leaks: " << report.leakCount << "\n";

        for (const auto& alloc : report.leakedAllocations) {
            ss << "- " << alloc.size << " bytes at " << alloc.address
               << " (" << alloc.type << ") from " << alloc.file << ":" << alloc.line << "\n";
        }

        report.report = ss.str();
        return report;
    }

    bool MemoryManager::CheckForLeaks() {
        auto report = GetLeakReport();
        if (report.leakCount > 0) {
            VOXELCRAFT_WARNING("Memory leaks detected:\n{}", report.report);
            return true;
        }
        return false;
    }

    std::optional<MemoryAllocation> MemoryManager::GetAllocationInfo(void* address) const {
        std::shared_lock<std::shared_mutex> lock(m_allocationMutex);
        auto it = m_allocations.find(address);
        if (it != m_allocations.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::vector<MemoryAllocation> MemoryManager::GetActiveAllocations() const {
        std::shared_lock<std::shared_mutex> lock(m_allocationMutex);
        std::vector<MemoryAllocation> active;

        for (const auto& pair : m_allocations) {
            if (pair.second.isActive) {
                active.push_back(pair.second);
            }
        }

        return active;
    }

    size_t MemoryManager::DefragmentAllPools() {
        std::shared_lock<std::shared_mutex> lock(m_poolMutex);
        size_t totalRecovered = 0;

        for (auto& pair : m_pools) {
            totalRecovered += pair.second->Defragment();
        }

        VOXELCRAFT_INFO("Defragmented all pools, recovered {} bytes", totalRecovered);
        return totalRecovered;
    }

    size_t MemoryManager::RunGarbageCollection() {
        // Simple GC - just run defragmentation for now
        return DefragmentAllPools();
    }

    std::string MemoryManager::OptimizeMemory() {
        std::stringstream ss;
        ss << "Memory Optimization Report:\n";

        size_t defragmented = DefragmentAllPools();
        ss << "- Defragmented " << defragmented << " bytes\n";

        if (CheckForLeaks()) {
            ss << "- Memory leaks detected (see log for details)\n";
        } else {
            ss << "- No memory leaks detected\n";
        }

        return ss.str();
    }

    void MemoryManager::SetMemoryLimit(size_t limit) {
        m_memoryLimit = limit;
        VOXELCRAFT_INFO("Memory limit set to {} bytes", limit);
    }

    void MemoryManager::SetLeakDetection(bool enabled) {
        m_leakDetectionEnabled = enabled;
        VOXELCRAFT_INFO("Leak detection {}", enabled ? "enabled" : "disabled");
    }

    bool MemoryManager::TakeMemorySnapshot(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_snapshotMutex);

        auto allocations = GetActiveAllocations();
        m_snapshots[name] = allocations;
        m_snapshotCount++;

        VOXELCRAFT_INFO("Memory snapshot '{}' taken with {} allocations", name, allocations.size());
        return true;
    }

    std::string MemoryManager::CompareMemorySnapshots(const std::string& snapshot1, const std::string& snapshot2) {
        std::lock_guard<std::mutex> lock(m_snapshotMutex);

        auto it1 = m_snapshots.find(snapshot1);
        auto it2 = m_snapshots.find(snapshot2);

        if (it1 == m_snapshots.end() || it2 == m_snapshots.end()) {
            return "One or both snapshots not found";
        }

        std::stringstream ss;
        ss << "Memory snapshot comparison: " << snapshot1 << " vs " << snapshot2 << "\n";

        // Simple comparison - count differences
        size_t count1 = it1->second.size();
        size_t count2 = it2->second.size();
        ss << "Allocation count: " << count1 << " -> " << count2 << " (";

        if (count2 > count1) {
            ss << "+" << (count2 - count1) << " allocations)\n";
        } else if (count2 < count1) {
            ss << "-" << (count1 - count2) << " allocations)\n";
        } else {
            ss << "no change)\n";
        }

        return ss.str();
    }

    std::string MemoryManager::GetMemoryReport() const {
        auto stats = GetStatistics();
        auto poolInfo = GetPoolInfo();

        std::stringstream ss;
        ss << "=== Memory Report ===\n";
        ss << "Total allocated: " << stats.totalAllocated << " bytes\n";
        ss << "Total used: " << stats.totalUsed << " bytes\n";
        ss << "Peak usage: " << stats.peakMemoryUsage << " bytes\n";
        ss << "Active allocations: " << stats.activeAllocations << "\n";
        ss << "Memory pools: " << poolInfo.size() << "\n";

        for (const auto& info : poolInfo) {
            ss << "- " << info.name << " (" << static_cast<int>(info.type) << "): "
               << info.usedSize << "/" << info.totalSize << " bytes ("
               << (info.utilization * 100.0) << "%)\n";
        }

        return ss.str();
    }

    void MemoryManager::UpdateStatistics() {
        std::lock_guard<std::mutex> lock(m_statsMutex);

        // Update basic stats
        size_t totalAllocated = 0;
        size_t totalUsed = 0;
        uint64_t activeAllocs = 0;

        {
            std::shared_lock<std::shared_mutex> allocLock(m_allocationMutex);
            activeAllocs = m_allocations.size();
            for (const auto& pair : m_allocations) {
                if (pair.second.isActive) {
                    totalAllocated += pair.second.size;
                    totalUsed += pair.second.size;
                    activeAllocs++;
                }
            }
        }

        m_statistics.totalAllocated = totalAllocated;
        m_statistics.totalUsed = totalUsed;
        m_statistics.activeAllocations = activeAllocs;
        m_statistics.peakMemoryUsage = std::max(m_statistics.peakMemoryUsage, totalUsed);
    }

    void MemoryManager::CheckMemoryPressure() {
        // Simple memory pressure check
        if (m_memoryLimit > 0 && m_statistics.totalUsed > m_memoryLimit * 0.9) {
            VOXELCRAFT_WARNING("High memory pressure: {}% of limit",
                             (m_statistics.totalUsed * 100) / m_memoryLimit);
            m_statistics.isLowMemory = true;
        } else {
            m_statistics.isLowMemory = false;
        }
    }

    void MemoryManager::CleanupCompletedAllocations() {
        // This would clean up completed async allocations in a real implementation
    }

    // MemoryTracker implementation

    MemoryTracker::MemoryTracker(const std::string& type, const char* file, uint32_t line)
        : m_type(type)
        , m_file(file)
        , m_line(line)
    {
    }

    MemoryTracker::~MemoryTracker() {
        for (const auto& alloc : m_trackedAllocations) {
            GetMemoryManager().Deallocate(alloc.first);
        }
    }

    void MemoryTracker::Track(void* address, size_t size) {
        m_trackedAllocations.emplace_back(address, size);
    }

    void MemoryTracker::Untrack(void* address) {
        auto it = std::find_if(m_trackedAllocations.begin(), m_trackedAllocations.end(),
            [address](const auto& pair) { return pair.first == address; });
        if (it != m_trackedAllocations.end()) {
            m_trackedAllocations.erase(it);
        }
    }

    // Global accessor

    MemoryManager& GetMemoryManager() {
        std::lock_guard<std::mutex> lock(s_instanceMutex);
        if (!s_instance) {
            s_instance = std::make_unique<MemoryManager>();
        }
        return *s_instance;
    }

} // namespace VoxelCraft
