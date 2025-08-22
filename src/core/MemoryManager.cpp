/**
 * @file MemoryManagerSimple.cpp
 * @brief VoxelCraft Memory Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "MemoryManager.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <cstring>

namespace VoxelCraft {

// Global instance
static std::unique_ptr<MemoryManager> s_instance;
static std::mutex s_instanceMutex;

MemoryManager::MemoryManager()
    : m_leakDetectionEnabled(true)
    , m_profilingEnabled(true)
    , m_defaultPoolBlockSize(64)
    , m_defaultPoolBlockCount(1024)
{
    std::memset(&m_metrics, 0, sizeof(MemoryMetrics));
}

MemoryManager::~MemoryManager() {
    Shutdown();
}

bool MemoryManager::Initialize(bool enableLeakDetection, bool enableProfiling) {
    VOXELCRAFT_INFO("Initializing Memory Manager");

    m_leakDetectionEnabled = enableLeakDetection;
    m_profilingEnabled = enableProfiling;

    VOXELCRAFT_INFO("Memory Manager initialized successfully");
    return true;
}

void MemoryManager::Shutdown() {
    VOXELCRAFT_INFO("Shutting down Memory Manager");
    m_allocations.clear();
    VOXELCRAFT_INFO("Memory Manager shutdown complete");
}

void MemoryManager::Update(double deltaTime) {
    UpdateStatistics();
}

void MemoryManager::UpdateStatistics() {
    // Simple statistics update
    m_metrics.allocationCount = m_allocations.size();
}

void* MemoryManager::Allocate(size_t size, MemoryPoolType poolType, const std::string& tag) {
    if (size == 0) return nullptr;

    void* address = malloc(size);
    if (address) {
        m_metrics.allocationCount++;
        if (m_profilingEnabled) {
            TrackAllocation({address, size, poolType, std::chrono::steady_clock::now(), tag});
        }
    } else {
        m_metrics.allocationFailures++;
    }

    return address;
}

void* MemoryManager::AllocateAligned(size_t size, size_t alignment, MemoryPoolType poolType, const std::string& tag) {
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#else
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return nullptr;
    }
    return ptr;
#endif
}

void MemoryManager::Deallocate(void* address, MemoryPoolType poolType) {
    if (!address) return;

    free(address);
    m_metrics.deallocationCount++;
    if (m_leakDetectionEnabled) {
        UntrackAllocation(address);
    }
}

void* MemoryManager::Reallocate(void* address, size_t newSize, MemoryPoolType poolType, const std::string& tag) {
    void* newAddress = realloc(address, newSize);
    if (newAddress) {
        m_metrics.allocationCount++;
        if (m_profilingEnabled && address != newAddress) {
            UntrackAllocation(address);
            TrackAllocation({newAddress, newSize, poolType, std::chrono::steady_clock::now(), tag});
        }
    }
    return newAddress;
}

bool MemoryManager::CreatePool(MemoryPoolType poolType, size_t blockSize, size_t blockCount) {
    if (m_pools.count(poolType) > 0) {
        return false;
    }

    auto pool = std::make_unique<MemoryPool>();
    pool->blockSize = blockSize;
    pool->totalBlocks = blockCount;
    pool->usedBlocks = 0;
    pool->freeList.resize(blockCount, true);
    pool->pool = malloc(blockSize * blockCount);

    if (!pool->pool) {
        return false;
    }

    m_pools[poolType] = std::move(pool);
    return true;
}

bool MemoryManager::DestroyPool(MemoryPoolType poolType) {
    auto it = m_pools.find(poolType);
    if (it == m_pools.end()) {
        return false;
    }

    auto& pool = it->second;
    if (pool->pool) {
        free(pool->pool);
    }

    m_pools.erase(it);
    return true;
}

const MemoryPool* MemoryManager::GetPool(MemoryPoolType poolType) const {
    auto it = m_pools.find(poolType);
    return it != m_pools.end() ? it->second.get() : nullptr;
}

void MemoryManager::GarbageCollect(bool aggressive) {
    UpdateStatistics();
}

void MemoryManager::Defragment() {
    UpdateStatistics();
}

MemoryLeakReport MemoryManager::CheckForLeaks() {
    MemoryLeakReport report;

    if (!m_leakDetectionEnabled) {
        return report;
    }

    std::lock_guard<std::mutex> lock(m_allocationsMutex);

    for (const auto& pair : m_allocations) {
        report.leakedAllocations.push_back(pair.second);
        report.totalLeakedBytes += pair.second.size;
    }

    report.leakCount = report.leakedAllocations.size();
    return report;
}

void MemoryManager::DumpStatistics() {
    UpdateStatistics();

    VOXELCRAFT_INFO("=== Memory Manager Statistics ===");
    VOXELCRAFT_INFO("Total Allocated: {} bytes", m_metrics.totalAllocated);
    VOXELCRAFT_INFO("Allocation Count: {}", m_metrics.allocationCount);
    VOXELCRAFT_INFO("==================================");
}

bool MemoryManager::ValidateMemory() {
    return true;
}

MemoryAllocation MemoryManager::GetAllocationInfo(void* address) const {
    std::lock_guard<std::mutex> lock(m_allocationsMutex);
    auto it = m_allocations.find(address);
    return it != m_allocations.end() ? it->second : MemoryAllocation{};
}

std::vector<MemoryAllocation> MemoryManager::GetActiveAllocations() const {
    std::lock_guard<std::mutex> lock(m_allocationsMutex);
    std::vector<MemoryAllocation> allocations;

    for (const auto& pair : m_allocations) {
        allocations.push_back(pair.second);
    }

    return allocations;
}

void MemoryManager::TrackAllocation(const MemoryAllocation& allocation) {
    if (!m_leakDetectionEnabled) return;

    std::lock_guard<std::mutex> lock(m_allocationsMutex);
    m_allocations[allocation.address] = allocation;
}

void MemoryManager::UntrackAllocation(void* address) {
    if (!m_leakDetectionEnabled) return;

    std::lock_guard<std::mutex> lock(m_allocationsMutex);
    m_allocations.erase(address);
}

// Global functions
void* MemoryAllocate(size_t size, MemoryPoolType poolType, const std::string& tag) {
    return GetMemoryManager().Allocate(size, poolType, tag);
}

void MemoryDeallocate(void* address, MemoryPoolType poolType) {
    GetMemoryManager().Deallocate(address, poolType);
}

void* MemoryAllocateAligned(size_t size, size_t alignment, MemoryPoolType poolType, const std::string& tag) {
    return GetMemoryManager().AllocateAligned(size, alignment, poolType, tag);
}

void* MemoryReallocate(void* address, size_t newSize, MemoryPoolType poolType, const std::string& tag) {
    return GetMemoryManager().Reallocate(address, newSize, poolType, tag);
}

MemoryManager& GetMemoryManager() {
    std::lock_guard<std::mutex> lock(s_instanceMutex);
    if (!s_instance) {
        s_instance = std::make_unique<MemoryManager>();
    }
    return *s_instance;
}

// Custom operators
void* operator new(size_t size) {
    void* ptr = GetMemoryManager().Allocate(size, MemoryPoolType::GENERAL, "new");
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void* operator new(size_t size, std::nothrow_t) noexcept {
    return GetMemoryManager().Allocate(size, MemoryPoolType::GENERAL, "new_nothrow");
}

void operator delete(void* address) noexcept {
    if (address) GetMemoryManager().Deallocate(address, MemoryPoolType::GENERAL);
}

void operator delete(void* address, std::nothrow_t) noexcept {
    if (address) GetMemoryManager().Deallocate(address, MemoryPoolType::GENERAL);
}

} // namespace VoxelCraft
