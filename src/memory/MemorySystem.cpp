/**
 * @file MemorySystem.cpp
 * @brief VoxelCraft Advanced Memory Management System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "MemorySystem.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <new>
#include <memory>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #include <dbghelp.h>
    #pragma comment(lib, "dbghelp.lib")
#else
    #include <execinfo.h>
    #include <unistd.h>
#endif

#include "../utils/Random.hpp"
#include "../logging/Logger.hpp"

namespace VoxelCraft {

    // MemoryPool implementations
    MemoryPool::MemoryPool(const MemoryPoolConfig& config)
        : m_config(config) {
    }

    // FixedSizeMemoryPool implementation
    FixedSizeMemoryPool::FixedSizeMemoryPool(const MemoryPoolConfig& config)
        : MemoryPool(config) {
        // Allocate memory for the pool
        m_memory = std::malloc(m_config.poolSize);
        if (!m_memory) {
            throw std::bad_alloc();
        }

        // Calculate number of blocks
        m_blockCount = m_config.poolSize / m_config.blockSize;
        m_freeBlocks = m_blockCount;

        // Initialize free list
        m_freeList = nullptr;
        Block* current = static_cast<Block*>(m_memory);

        for (size_t i = 0; i < m_blockCount; ++i) {
            current->next = m_freeList;
            current->isFree = true;
            current->magic = 0xDEADBEEF;
            m_freeList = current;
            current = reinterpret_cast<Block*>(reinterpret_cast<char*>(current) + m_config.blockSize);
        }
    }

    FixedSizeMemoryPool::~FixedSizeMemoryPool() {
        if (m_memory) {
            std::free(m_memory);
            m_memory = nullptr;
        }
    }

    void* FixedSizeMemoryPool::Allocate(size_t size, uint32_t flags) {
        if (size > m_config.blockSize) {
            return nullptr;  // Size too large for this pool
        }

        std::unique_lock<std::mutex> lock(m_mutex);

        if (!m_freeList) {
            return nullptr;  // No free blocks
        }

        Block* block = m_freeList;
        m_freeList = block->next;
        block->isFree = false;
        block->next = nullptr;
        m_freeBlocks--;

        void* userPtr = block;
        if (flags & MEMORY_FLAG_ZERO_MEMORY) {
            std::memset(userPtr, 0, size);
        }

        return userPtr;
    }

    void FixedSizeMemoryPool::Deallocate(void* ptr) {
        if (!ptr || !Contains(ptr)) {
            return;
        }

        std::unique_lock<std::mutex> lock(m_mutex);

        Block* block = static_cast<Block*>(ptr);
        if (block->magic != 0xDEADBEEF) {
            // Corrupted block
            return;
        }

        block->isFree = true;
        block->next = m_freeList;
        m_freeList = block;
        m_freeBlocks++;
    }

    bool FixedSizeMemoryPool::Contains(void* ptr) const {
        return ptr >= m_memory && ptr < static_cast<char*>(m_memory) + m_config.poolSize;
    }

    size_t FixedSizeMemoryPool::GetUsedMemory() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return (m_blockCount - m_freeBlocks) * m_config.blockSize;
    }

    size_t FixedSizeMemoryPool::GetFreeMemory() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_freeBlocks * m_config.blockSize;
    }

    size_t FixedSizeMemoryPool::GetTotalMemory() const {
        return m_config.poolSize;
    }

    float FixedSizeMemoryPool::GetFragmentationRatio() const {
        return 0.0f;  // Fixed-size pools don't fragment
    }

    void FixedSizeMemoryPool::Defragment() {
        // No defragmentation needed for fixed-size pools
    }

    void FixedSizeMemoryPool::Reset() {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Rebuild free list
        m_freeList = nullptr;
        m_freeBlocks = m_blockCount;

        Block* current = static_cast<Block*>(m_memory);
        for (size_t i = 0; i < m_blockCount; ++i) {
            current->next = m_freeList;
            current->isFree = true;
            current->magic = 0xDEADBEEF;
            m_freeList = current;
            current = reinterpret_cast<Block*>(reinterpret_cast<char*>(current) + m_config.blockSize);
        }
    }

    bool FixedSizeMemoryPool::Validate() const {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Count free blocks in free list
        size_t freeCount = 0;
        Block* current = m_freeList;
        while (current) {
            if (current->magic != 0xDEADBEEF) {
                return false;  // Corrupted magic number
            }
            freeCount++;
            current = current->next;
        }

        return freeCount == m_freeBlocks;
    }

    // DynamicMemoryPool implementation
    DynamicMemoryPool::DynamicMemoryPool(const MemoryPoolConfig& config)
        : MemoryPool(config), m_usedMemory(0) {
        m_memory = std::malloc(m_config.poolSize);
        if (!m_memory) {
            throw std::bad_alloc();
        }

        // Initialize with one large free block
        m_freeList = static_cast<FreeBlock*>(m_memory);
        m_freeList->size = m_config.poolSize - sizeof(FreeBlock);
        m_freeList->next = nullptr;
        m_freeList->prev = nullptr;
        m_freeList->isFree = true;
        m_freeList->magic = 0xCAFEBABE;
    }

    DynamicMemoryPool::~DynamicMemoryPool() {
        if (m_memory) {
            std::free(m_memory);
            m_memory = nullptr;
        }
    }

    void* DynamicMemoryPool::Allocate(size_t size, uint32_t flags) {
        std::unique_lock<std::mutex> lock(m_mutex);

        size_t alignedSize = (size + m_config.alignment - 1) & ~(m_config.alignment - 1);
        size_t totalSize = alignedSize + sizeof(FreeBlock);

        FreeBlock* block = FindBestFit(totalSize);
        if (!block) {
            return nullptr;  // No suitable block found
        }

        // Split the block if it's large enough
        if (block->size >= totalSize + sizeof(FreeBlock) + 64) {
            FreeBlock* newBlock = reinterpret_cast<FreeBlock*>(
                reinterpret_cast<char*>(block) + sizeof(FreeBlock) + alignedSize);

            newBlock->size = block->size - totalSize;
            newBlock->next = block->next;
            newBlock->prev = block;
            newBlock->isFree = true;
            newBlock->magic = 0xCAFEBABE;

            if (block->next) {
                block->next->prev = newBlock;
            }
            block->next = newBlock;
            block->size = alignedSize;
        }

        // Mark block as used
        block->isFree = false;
        m_usedMemory += block->size;

        void* userPtr = reinterpret_cast<char*>(block) + sizeof(FreeBlock);
        if (flags & MEMORY_FLAG_ZERO_MEMORY) {
            std::memset(userPtr, 0, size);
        }

        return userPtr;
    }

    void DynamicMemoryPool::Deallocate(void* ptr) {
        if (!ptr || !Contains(ptr)) {
            return;
        }

        std::unique_lock<std::mutex> lock(m_mutex);

        FreeBlock* block = reinterpret_cast<FreeBlock*>(
            reinterpret_cast<char*>(ptr) - sizeof(FreeBlock));

        if (block->magic != 0xCAFEBABE || !block->isFree) {
            return;  // Invalid block
        }

        block->isFree = true;
        m_usedMemory -= block->size;

        CoalesceFreeBlocks();
    }

    bool DynamicMemoryPool::Contains(void* ptr) const {
        return ptr >= m_memory && ptr < static_cast<char*>(m_memory) + m_config.poolSize;
    }

    size_t DynamicMemoryPool::GetUsedMemory() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_usedMemory;
    }

    size_t DynamicMemoryPool::GetFreeMemory() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_config.poolSize - m_usedMemory;
    }

    size_t DynamicMemoryPool::GetTotalMemory() const {
        return m_config.poolSize;
    }

    float DynamicMemoryPool::GetFragmentationRatio() const {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_usedMemory == 0) {
            return 0.0f;
        }

        // Calculate fragmentation as ratio of free blocks to total free memory
        size_t totalFreeMemory = GetFreeMemory();
        if (totalFreeMemory == 0) {
            return 0.0f;
        }

        size_t largestFreeBlock = 0;
        FreeBlock* current = m_freeList;
        while (current) {
            largestFreeBlock = std::max(largestFreeBlock, current->size);
            current = current->next;
        }

        return 1.0f - (static_cast<float>(largestFreeBlock) / totalFreeMemory);
    }

    void DynamicMemoryPool::Defragment() {
        std::unique_lock<std::mutex> lock(m_mutex);
        CoalesceFreeBlocks();
    }

    void DynamicMemoryPool::Reset() {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Reset to single large free block
        m_freeList = static_cast<FreeBlock*>(m_memory);
        m_freeList->size = m_config.poolSize - sizeof(FreeBlock);
        m_freeList->next = nullptr;
        m_freeList->prev = nullptr;
        m_freeList->isFree = true;
        m_freeList->magic = 0xCAFEBABE;
        m_usedMemory = 0;
    }

    bool DynamicMemoryPool::Validate() const {
        std::unique_lock<std::mutex> lock(m_mutex);

        size_t totalFreeSize = 0;
        FreeBlock* current = m_freeList;
        std::unordered_set<FreeBlock*> visited;

        while (current) {
            if (visited.find(current) != visited.end()) {
                return false;  // Circular reference
            }

            if (current->magic != 0xCAFEBABE) {
                return false;  // Corrupted magic number
            }

            if (!current->isFree) {
                return false;  // Free list contains used block
            }

            visited.insert(current);
            totalFreeSize += current->size;
            current = current->next;
        }

        return totalFreeSize == GetFreeMemory();
    }

    void DynamicMemoryPool::CoalesceFreeBlocks() {
        FreeBlock* current = m_freeList;
        while (current && current->next) {
            if (current->isFree && current->next->isFree) {
                // Merge with next block
                current->size += current->next->size + sizeof(FreeBlock);
                current->next = current->next->next;
                if (current->next) {
                    current->next->prev = current;
                }
            } else {
                current = current->next;
            }
        }
    }

    DynamicMemoryPool::FreeBlock* DynamicMemoryPool::FindBestFit(size_t size) {
        FreeBlock* bestFit = nullptr;
        FreeBlock* current = m_freeList;

        while (current) {
            if (current->isFree && current->size >= size) {
                if (!bestFit || current->size < bestFit->size) {
                    bestFit = current;
                    if (current->size == size) {
                        break;  // Perfect fit
                    }
                }
            }
            current = current->next;
        }

        return bestFit;
    }

    // LinearMemoryPool implementation
    LinearMemoryPool::LinearMemoryPool(const MemoryPoolConfig& config)
        : MemoryPool(config), m_usedMemory(0) {
        m_memory = std::malloc(m_config.poolSize);
        if (!m_memory) {
            throw std::bad_alloc();
        }
        m_current = m_memory;
    }

    LinearMemoryPool::~LinearMemoryPool() {
        if (m_memory) {
            std::free(m_memory);
            m_memory = nullptr;
        }
    }

    void* LinearMemoryPool::Allocate(size_t size, uint32_t flags) {
        std::unique_lock<std::mutex> lock(m_mutex);

        size_t alignedSize = (size + m_config.alignment - 1) & ~(m_config.alignment - 1);
        size_t remainingSpace = m_config.poolSize - m_usedMemory;

        if (alignedSize > remainingSpace) {
            return nullptr;  // Not enough space
        }

        void* userPtr = m_current;
        m_current = static_cast<char*>(m_current) + alignedSize;
        m_usedMemory += alignedSize;

        if (flags & MEMORY_FLAG_ZERO_MEMORY) {
            std::memset(userPtr, 0, size);
        }

        return userPtr;
    }

    void LinearMemoryPool::Deallocate(void* ptr) {
        // Linear pools don't support individual deallocation
        // Use Reset() to free all memory at once
    }

    bool LinearMemoryPool::Contains(void* ptr) const {
        return ptr >= m_memory && ptr < static_cast<char*>(m_memory) + m_config.poolSize;
    }

    size_t LinearMemoryPool::GetUsedMemory() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_usedMemory;
    }

    size_t LinearMemoryPool::GetFreeMemory() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_config.poolSize - m_usedMemory;
    }

    size_t LinearMemoryPool::GetTotalMemory() const {
        return m_config.poolSize;
    }

    float LinearMemoryPool::GetFragmentationRatio() const {
        return 0.0f;  // Linear pools don't fragment
    }

    void LinearMemoryPool::Defragment() {
        // No defragmentation needed for linear pools
    }

    void LinearMemoryPool::Reset() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_current = m_memory;
        m_usedMemory = 0;
    }

    bool LinearMemoryPool::Validate() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_current >= m_memory && m_current <= static_cast<char*>(m_memory) + m_config.poolSize;
    }

    // RingBufferMemoryPool implementation
    RingBufferMemoryPool::RingBufferMemoryPool(const MemoryPoolConfig& config)
        : MemoryPool(config), m_usedMemory(0), m_full(false) {
        m_memory = std::malloc(m_config.poolSize);
        if (!m_memory) {
            throw std::bad_alloc();
        }
    }

    RingBufferMemoryPool::~RingBufferMemoryPool() {
        if (m_memory) {
            std::free(m_memory);
            m_memory = nullptr;
        }
    }

    void* RingBufferMemoryPool::Allocate(size_t size, uint32_t flags) {
        std::unique_lock<std::mutex> lock(m_mutex);

        size_t alignedSize = (size + m_config.alignment - 1) & ~(m_config.alignment - 1);

        if (alignedSize > m_config.poolSize) {
            return nullptr;  // Size too large
        }

        if (m_full && alignedSize > GetAvailableSpace()) {
            return nullptr;  // Not enough space
        }

        // Handle wrap-around
        size_t spaceToEnd = m_config.poolSize - m_writePosition;
        void* userPtr = nullptr;

        if (alignedSize <= spaceToEnd) {
            userPtr = static_cast<char*>(m_memory) + m_writePosition;
            m_writePosition += alignedSize;
        } else {
            // Wrap around
            userPtr = m_memory;
            m_writePosition = alignedSize;
            m_full = true;
        }

        m_usedMemory += alignedSize;

        if (flags & MEMORY_FLAG_ZERO_MEMORY) {
            std::memset(userPtr, 0, size);
        }

        return userPtr;
    }

    void RingBufferMemoryPool::Deallocate(void* ptr) {
        // Ring buffer doesn't support individual deallocation
        // Use Reset() to free all memory
    }

    bool RingBufferMemoryPool::Contains(void* ptr) const {
        return ptr >= m_memory && ptr < static_cast<char*>(m_memory) + m_config.poolSize;
    }

    size_t RingBufferMemoryPool::GetUsedMemory() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_usedMemory;
    }

    size_t RingBufferMemoryPool::GetFreeMemory() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_config.poolSize - m_usedMemory;
    }

    size_t RingBufferMemoryPool::GetTotalMemory() const {
        return m_config.poolSize;
    }

    float RingBufferMemoryPool::GetFragmentationRatio() const {
        return 0.0f;  // Ring buffers don't fragment
    }

    void RingBufferMemoryPool::Defragment() {
        // No defragmentation needed for ring buffers
    }

    void RingBufferMemoryPool::Reset() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_readPosition = 0;
        m_writePosition = 0;
        m_usedMemory = 0;
        m_full = false;
    }

    bool RingBufferMemoryPool::Validate() const {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_readPosition >= m_config.poolSize || m_writePosition >= m_config.poolSize) {
            return false;
        }

        if (m_full && m_readPosition == m_writePosition) {
            return true;  // Valid full state
        }

        if (!m_full && m_readPosition <= m_writePosition) {
            return true;  // Valid non-full state
        }

        return false;
    }

    size_t RingBufferMemoryPool::GetAvailableSpace() const {
        if (m_full) {
            return 0;
        }

        if (m_writePosition >= m_readPosition) {
            return m_config.poolSize - (m_writePosition - m_readPosition);
        } else {
            return m_readPosition - m_writePosition;
        }
    }

    // MemoryLeakDetector implementation
    MemoryLeakDetector::MemoryLeakDetector() = default;

    void MemoryLeakDetector::TrackAllocation(const AllocationInfo& info) {
        if (!m_isActive) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_mutex);

        AllocationInfo trackedInfo = info;
        trackedInfo.allocationId = m_nextAllocationId++;

        if (m_mode == LeakDetectionMode::ADVANCED || m_mode == LeakDetectionMode::FULL) {
            CaptureStackTrace(trackedInfo);
        }

        m_trackedAllocations[info.address] = trackedInfo;
    }

    void MemoryLeakDetector::TrackDeallocation(void* address) {
        if (!m_isActive) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_mutex);

        auto it = m_trackedAllocations.find(address);
        if (it != m_trackedAllocations.end()) {
            it->second.isFreed = true;
            // Don't remove immediately, keep for analysis
        }
    }

    void MemoryLeakDetector::ReportLeaks() {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        size_t leakCount = 0;
        size_t totalMemoryLeaked = 0;

        std::stringstream report;
        report << "=== Memory Leak Report ===\n";

        for (const auto& pair : m_trackedAllocations) {
            if (IsLeak(pair.second)) {
                leakCount++;
                totalMemoryLeaked += pair.second.size;

                report << "Leak #" << leakCount << ":\n";
                report << "  Address: " << pair.second.address << "\n";
                report << "  Size: " << pair.second.size << " bytes\n";
                report << "  Type: " << static_cast<int>(pair.second.type) << "\n";
                report << "  File: " << pair.second.file << ":" << pair.second.line << "\n";
                report << "  Function: " << pair.second.function << "\n";
                report << "  Thread: " << pair.second.threadId << "\n";

                if (!pair.second.stackTrace.empty() && m_mode >= LeakDetectionMode::ADVANCED) {
                    report << "  Stack Trace:\n";
                    for (size_t i = 0; i < pair.second.stackTrace.size(); ++i) {
                        report << "    [" << i << "] " << pair.second.stackTrace[i] << "\n";
                    }
                }
                report << "\n";
            }
        }

        report << "Total Leaks: " << leakCount << "\n";
        report << "Total Memory Leaked: " << totalMemoryLeaked << " bytes\n";

        Logger::GetInstance().Warning(report.str(), "MemorySystem");

        if (leakCount > 0) {
            std::cout << report.str() << std::endl;
        }
    }

    void MemoryLeakDetector::StartLeakDetection() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_isActive = true;
        m_trackedAllocations.clear();
    }

    void MemoryLeakDetector::StopLeakDetection() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_isActive = false;
    }

    size_t MemoryLeakDetector::GetTrackedAllocationCount() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_trackedAllocations.size();
    }

    size_t MemoryLeakDetector::GetPotentialLeakCount() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        size_t leakCount = 0;
        for (const auto& pair : m_trackedAllocations) {
            if (IsLeak(pair.second)) {
                leakCount++;
            }
        }
        return leakCount;
    }

    void MemoryLeakDetector::CaptureStackTrace(AllocationInfo& info) {
#ifdef _WIN32
        // Windows stack trace capture
        const int maxFrames = 32;
        void* stack[maxFrames];
        USHORT frames = CaptureStackBackTrace(0, maxFrames, stack, NULL);

        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        for (USHORT i = 0; i < frames; i++) {
            SymFromAddr(process, (DWORD64)stack[i], 0, symbol);
            info.stackTrace.push_back(std::string(symbol->Name));
        }

        SymCleanup(process);
#else
        // Unix/Linux stack trace capture
        const int maxFrames = 32;
        void* buffer[maxFrames];
        int frames = backtrace(buffer, maxFrames);
        char** symbols = backtrace_symbols(buffer, frames);

        if (symbols) {
            for (int i = 0; i < frames; i++) {
                info.stackTrace.push_back(std::string(symbols[i]));
            }
            free(symbols);
        }
#endif
    }

    bool MemoryLeakDetector::IsLeak(const AllocationInfo& info) const {
        if (info.isFreed) {
            return false;
        }

        // Check if allocation is too old (potential leak)
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - info.timestamp);

        // Consider it a leak if older than 5 minutes and not freed
        return age.count() > 300;
    }

    // MemoryProfiler implementation
    MemoryProfiler::MemoryProfiler() {
        m_typeStats.resize(static_cast<size_t>(MemoryType::MAX_TYPES));
        m_lastUpdateTime = std::chrono::steady_clock::now();
    }

    void MemoryProfiler::RecordAllocation(MemoryType type, size_t size, void* address) {
        if (!m_isProfiling) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_mutex);

        m_currentStats.totalAllocated += size;
        m_currentStats.totalUsed += size;
        m_currentStats.allocationCount++;
        m_currentStats.activeAllocations++;
        m_currentStats.typeAllocations[static_cast<size_t>(type)]++;
        m_currentStats.typeMemoryUsage[static_cast<size_t>(type)] += size;

        m_currentStats.largestAllocation = std::max(m_currentStats.largestAllocation, size);
        m_currentStats.smallestAllocation = m_currentStats.smallestAllocation == 0 ?
            size : std::min(m_currentStats.smallestAllocation, size);

        UpdatePeakStats();
        UpdatePerformanceMetrics();
    }

    void MemoryProfiler::RecordDeallocation(MemoryType type, size_t size, void* address) {
        if (!m_isProfiling) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_mutex);

        m_currentStats.totalUsed -= size;
        m_currentStats.deallocationCount++;
        m_currentStats.activeAllocations--;
        m_currentStats.typeDeallocations[static_cast<size_t>(type)]++;
        m_currentStats.typeMemoryUsage[static_cast<size_t>(type)] -= size;

        UpdatePerformanceMetrics();
    }

    MemoryStats MemoryProfiler::GetCurrentStats() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_currentStats;
    }

    MemoryStats MemoryProfiler::GetStatsForType(MemoryType type) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_typeStats[static_cast<size_t>(type)];
    }

    MemoryStats MemoryProfiler::GetPeakStats() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_peakStats;
    }

    void MemoryProfiler::Reset() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        m_currentStats = MemoryStats();
        m_peakStats = MemoryStats();
        m_typeStats.clear();
        m_typeStats.resize(static_cast<size_t>(MemoryType::MAX_TYPES));

        m_lastAllocationCount = 0;
        m_lastDeallocationCount = 0;
        m_lastUpdateTime = std::chrono::steady_clock::now();
    }

    void MemoryProfiler::StartProfiling() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_isProfiling = true;
    }

    void MemoryProfiler::StopProfiling() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_isProfiling = false;
    }

    std::vector<std::pair<MemoryType, MemoryStats>> MemoryProfiler::GetAllTypeStats() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        std::vector<std::pair<MemoryType, MemoryStats>> result;
        for (size_t i = 0; i < m_typeStats.size(); ++i) {
            result.emplace_back(static_cast<MemoryType>(i), m_typeStats[i]);
        }

        return result;
    }

    float MemoryProfiler::GetAllocationRate() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<float>(now - m_lastUpdateTime).count();

        if (duration > 0.0f) {
            return (m_currentStats.allocationCount - m_lastAllocationCount) / duration;
        }

        return 0.0f;
    }

    float MemoryProfiler::GetDeallocationRate() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<float>(now - m_lastUpdateTime).count();

        if (duration > 0.0f) {
            return (m_currentStats.deallocationCount - m_lastDeallocationCount) / duration;
        }

        return 0.0f;
    }

    float MemoryProfiler::GetMemoryPressure() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        if (m_peakStats.totalUsed == 0) {
            return 0.0f;
        }

        return static_cast<float>(m_currentStats.totalUsed) / m_peakStats.totalUsed;
    }

    void MemoryProfiler::UpdatePeakStats() {
        m_peakStats.totalUsed = std::max(m_peakStats.totalUsed, m_currentStats.totalUsed);
        m_peakStats.peakUsage = std::max(m_peakStats.peakUsage, m_currentStats.totalUsed);
        m_peakStats.activeAllocations = std::max(m_peakStats.activeAllocations, m_currentStats.activeAllocations);

        for (size_t i = 0; i < m_typeStats.size(); ++i) {
            m_peakStats.typeMemoryUsage[i] = std::max(m_peakStats.typeMemoryUsage[i], m_currentStats.typeMemoryUsage[i]);
        }
    }

    void MemoryProfiler::UpdatePerformanceMetrics() {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<float>(now - m_lastUpdateTime).count();

        if (duration >= 1.0f) {  // Update every second
            m_lastAllocationCount = m_currentStats.allocationCount;
            m_lastDeallocationCount = m_currentStats.deallocationCount;
            m_lastUpdateTime = now;
        }
    }

    // MemoryDefragmenter implementation
    MemoryDefragmenter::MemoryDefragmenter() = default;

    void MemoryDefragmenter::RegisterPool(MemoryPool* pool) {
        if (!pool) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_pools.push_back(pool);
    }

    void MemoryDefragmenter::UnregisterPool(MemoryPool* pool) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        auto it = std::remove(m_pools.begin(), m_pools.end(), pool);
        m_pools.erase(it, m_pools.end());
    }

    void MemoryDefragmenter::DefragmentAll() {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        for (MemoryPool* pool : m_pools) {
            if (ShouldDefragment(pool)) {
                pool->Defragment();
                m_defragmentationCount++;
            }
        }
    }

    void MemoryDefragmenter::DefragmentPool(MemoryPool* pool) {
        if (!pool) {
            return;
        }

        if (ShouldDefragment(pool)) {
            pool->Defragment();
            m_defragmentationCount++;
        }
    }

    bool MemoryDefragmenter::ShouldDefragment(MemoryPool* pool) const {
        if (!pool) {
            return false;
        }

        return pool->GetFragmentationRatio() > m_threshold;
    }

    // Main MemorySystem implementation
    MemorySystem& MemorySystem::GetInstance() {
        static MemorySystem instance;
        return instance;
    }

    MemorySystem::MemorySystem()
        : m_leakDetectionMode(LeakDetectionMode::BASIC),
          m_profilingEnabled(true),
          m_autoDefragmentation(true),
          m_defragmentationThreshold(0.5f),
          m_allocationBreakpoint(0) {
    }

    MemorySystem::~MemorySystem() {
        Shutdown();
    }

    bool MemorySystem::Initialize() {
        if (!System::Initialize()) {
            return false;
        }

        Logger::GetInstance().Info("MemorySystem initializing", "MemorySystem");

        // Start leak detection
        if (m_leakDetectionMode != LeakDetectionMode::NONE) {
            m_leakDetector.SetDetectionMode(m_leakDetectionMode);
            m_leakDetector.StartLeakDetection();
        }

        // Start profiling
        if (m_profilingEnabled) {
            m_profiler.StartProfiling();
        }

        // Register with defragmenter
        m_defragmenter.EnableAutoDefragmentation(m_autoDefragmentation);

        Logger::GetInstance().Info("MemorySystem initialized successfully", "MemorySystem");
        return true;
    }

    void MemorySystem::Update(float deltaTime) {
        System::Update(deltaTime);

        // Update memory pressure
        UpdateMemoryPressure();

        // Check for defragmentation if enabled
        if (m_autoDefragmentation) {
            CheckForDefragmentation();
        }

        // Update performance metrics
        if (m_profilingEnabled) {
            // Performance metrics are updated automatically in profiler
        }
    }

    void MemorySystem::Shutdown() {
        if (!System::Shutdown()) {
            return;
        }

        Logger::GetInstance().Info("MemorySystem shutting down", "MemorySystem");

        // Report any memory leaks
        if (m_leakDetector.IsLeakDetectionActive()) {
            m_leakDetector.ReportLeaks();
            m_leakDetector.StopLeakDetection();
        }

        // Print final memory report
        PrintMemoryReport();

        // Destroy all pools
        {
            std::unique_lock<std::shared_mutex> lock(m_poolsMutex);
            for (auto& pair : m_pools) {
                DestroyPool(pair.second);
            }
            m_pools.clear();
            m_poolsByType.clear();
        }

        // Stop profiling
        if (m_profiler.IsProfiling()) {
            m_profiler.StopProfiling();
        }

        Logger::GetInstance().Info("MemorySystem shutdown complete", "MemorySystem");
    }

    void* MemorySystem::Allocate(size_t size, MemoryType type, uint32_t flags,
                                const char* file, uint32_t line, const char* function) {
        // Check allocation breakpoint
        if (m_allocationBreakpoint > 0 && size == m_allocationBreakpoint) {
            // Breakpoint hit - could trigger debugger or logging
            Logger::GetInstance().Warning("Allocation breakpoint hit: " + std::to_string(size) + " bytes", "MemorySystem");
        }

        // Try to allocate from appropriate pool first
        if (auto pool = FindSuitablePool(size, type, flags)) {
            void* ptr = pool->Allocate(size, flags);
            if (ptr) {
                // Track allocation
                AllocationInfo info(ptr, size, type, flags, file ? file : "", line, function ? function : "");
                UpdateStatsOnAllocation(type, size, ptr);

                if (m_leakDetector.IsLeakDetectionActive()) {
                    m_leakDetector.TrackAllocation(info);
                }

                return ptr;
            }
        }

        // Fall back to system allocation
        void* ptr = AllocateFromSystem(size, flags);
        if (ptr) {
            AllocationInfo info(ptr, size, type, flags, file ? file : "", line, function ? function : "");
            UpdateStatsOnAllocation(type, size, ptr);

            if (m_leakDetector.IsLeakDetectionActive()) {
                m_leakDetector.TrackAllocation(info);
            }
        }

        return ptr;
    }

    void MemorySystem::Deallocate(void* ptr, MemoryType type) {
        if (!ptr) {
            return;
        }

        // Find which pool this belongs to
        bool deallocated = false;
        size_t deallocatedSize = 0;

        {
            std::shared_lock<std::shared_mutex> lock(m_poolsMutex);
            for (const auto& pair : m_pools) {
                if (pair.second->Contains(ptr)) {
                    pair.second->Deallocate(ptr);
                    deallocatedSize = 0; // We don't track individual sizes in pools easily
                    deallocated = true;
                    break;
                }
            }
        }

        if (!deallocated) {
            // System allocation - just free it
            std::free(ptr);
            deallocatedSize = 0; // We don't know the original size
        }

        // Update statistics
        UpdateStatsOnDeallocation(type, deallocatedSize, ptr);

        // Track deallocation for leak detection
        if (m_leakDetector.IsLeakDetectionActive()) {
            m_leakDetector.TrackDeallocation(ptr);
        }
    }

    MemoryPool* MemorySystem::CreatePool(const MemoryPoolConfig& config) {
        MemoryPool* pool = nullptr;

        try {
            switch (config.strategy) {
                case PoolStrategy::FIXED_SIZE:
                    pool = new FixedSizeMemoryPool(config);
                    break;
                case PoolStrategy::DYNAMIC_SIZE:
                    pool = new DynamicMemoryPool(config);
                    break;
                case PoolStrategy::LINEAR:
                    pool = new LinearMemoryPool(config);
                    break;
                case PoolStrategy::RING_BUFFER:
                    pool = new RingBufferMemoryPool(config);
                    break;
                default:
                    Logger::GetInstance().Error("Unknown pool strategy: " + std::to_string(static_cast<int>(config.strategy)), "MemorySystem");
                    return nullptr;
            }

            if (RegisterPool(pool)) {
                m_defragmenter.RegisterPool(pool);
                return pool;
            }

        } catch (const std::exception& e) {
            Logger::GetInstance().Error("Failed to create memory pool: " + std::string(e.what()), "MemorySystem");
            delete pool;
        }

        return nullptr;
    }

    bool MemorySystem::DestroyPool(MemoryPool* pool) {
        if (!pool) {
            return false;
        }

        m_defragmenter.UnregisterPool(pool);
        UnregisterPool(pool);
        delete pool;
        return true;
    }

    bool MemorySystem::DestroyPool(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_poolsMutex);

        auto it = m_pools.find(name);
        if (it == m_pools.end()) {
            return false;
        }

        MemoryPool* pool = it->second;
        m_pools.erase(it);

        // Remove from type map
        auto typeIt = m_poolsByType.find(pool->GetMemoryType());
        if (typeIt != m_poolsByType.end()) {
            auto& pools = typeIt->second;
            pools.erase(std::remove(pools.begin(), pools.end(), pool), pools.end());
        }

        m_defragmenter.UnregisterPool(pool);
        delete pool;
        return true;
    }

    MemoryPool* MemorySystem::GetPool(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_poolsMutex);

        auto it = m_pools.find(name);
        return it != m_pools.end() ? it->second : nullptr;
    }

    std::vector<MemoryPool*> MemorySystem::GetPools(MemoryType type) const {
        std::shared_lock<std::shared_mutex> lock(m_poolsMutex);

        auto it = m_poolsByType.find(type);
        return it != m_poolsByType.end() ? it->second : std::vector<MemoryPool*>();
    }

    size_t MemorySystem::GetPoolCount() const {
        std::shared_lock<std::shared_mutex> lock(m_poolsMutex);
        return m_pools.size();
    }

    MemoryStats MemorySystem::GetMemoryStats() const {
        return m_profiler.GetCurrentStats();
    }

    MemoryStats MemorySystem::GetMemoryStats(MemoryType type) const {
        return m_profiler.GetStatsForType(type);
    }

    MemoryStats MemorySystem::GetPeakMemoryStats() const {
        return m_profiler.GetPeakStats();
    }

    void MemorySystem::StartLeakDetection() {
        m_leakDetector.StartLeakDetection();
    }

    void MemorySystem::StopLeakDetection() {
        m_leakDetector.StopLeakDetection();
    }

    void MemorySystem::ReportMemoryLeaks() {
        m_leakDetector.ReportLeaks();
    }

    bool MemorySystem::IsLeakDetectionActive() const {
        return m_leakDetector.IsLeakDetectionActive();
    }

    void MemorySystem::DefragmentMemory() {
        m_defragmenter.DefragmentAll();
    }

    void MemorySystem::DefragmentPools(MemoryType type) {
        auto pools = GetPools(type);
        for (MemoryPool* pool : pools) {
            m_defragmenter.DefragmentPool(pool);
        }
    }

    void MemorySystem::EnableAutoDefragmentation(bool enable) {
        m_autoDefragmentation = enable;
        m_defragmenter.EnableAutoDefragmentation(enable);
    }

    bool MemorySystem::IsAutoDefragmentationEnabled() const {
        return m_autoDefragmentation;
    }

    bool MemorySystem::ValidateMemory() {
        std::shared_lock<std::shared_mutex> lock(m_poolsMutex);

        for (const auto& pair : m_pools) {
            if (!pair.second->Validate()) {
                Logger::GetInstance().Error("Memory pool validation failed: " + pair.first, "MemorySystem");
                return false;
            }
        }

        return true;
    }

    bool MemorySystem::ValidatePool(MemoryPool* pool) {
        return pool && pool->Validate();
    }

    bool MemorySystem::ValidateAllPools() {
        return ValidateMemory();
    }

    void MemorySystem::SetLeakDetectionMode(LeakDetectionMode mode) {
        m_leakDetectionMode = mode;
        m_leakDetector.SetDetectionMode(mode);
    }

    LeakDetectionMode MemorySystem::GetLeakDetectionMode() const {
        return m_leakDetectionMode;
    }

    void MemorySystem::SetProfilingEnabled(bool enabled) {
        m_profilingEnabled = enabled;
        if (enabled) {
            m_profiler.StartProfiling();
        } else {
            m_profiler.StopProfiling();
        }
    }

    bool MemorySystem::IsProfilingEnabled() const {
        return m_profilingEnabled;
    }

    void MemorySystem::SetDefragmentationThreshold(float threshold) {
        m_defragmentationThreshold = threshold;
        m_defragmenter.SetDefragmentationThreshold(threshold);
    }

    float MemorySystem::GetDefragmentationThreshold() const {
        return m_defragmentationThreshold;
    }

    std::string MemorySystem::GetMemoryInfo() const {
        std::stringstream ss;
        ss << "=== Memory System Info ===\n";
        ss << "Total Memory Usage: " << GetTotalMemoryUsage() << " bytes\n";
        ss << "Peak Memory Usage: " << GetPeakMemoryUsage() << " bytes\n";
        ss << "Memory Pressure: " << GetMemoryPressure() << "\n";
        ss << "Active Pools: " << GetPoolCount() << "\n";
        ss << "Leak Detection: " << (IsLeakDetectionActive() ? "Active" : "Inactive") << "\n";
        ss << "Profiling: " << (IsProfilingEnabled() ? "Enabled" : "Disabled") << "\n";
        ss << "Auto Defragmentation: " << (IsAutoDefragmentationEnabled() ? "Enabled" : "Disabled") << "\n";
        return ss.str();
    }

    void MemorySystem::PrintMemoryReport() const {
        auto stats = GetMemoryStats();
        std::stringstream report;

        report << "=== Memory Usage Report ===\n";
        report << "Total Allocated: " << stats.totalAllocated << " bytes\n";
        report << "Total Used: " << stats.totalUsed << " bytes\n";
        report << "Peak Usage: " << stats.peakUsage << " bytes\n";
        report << "Active Allocations: " << stats.activeAllocations << "\n";
        report << "Allocation Count: " << stats.allocationCount << "\n";
        report << "Deallocation Count: " << stats.deallocationCount << "\n";

        if (stats.allocationCount > 0) {
            report << "Average Allocation Size: " << stats.totalAllocated / stats.allocationCount << " bytes\n";
        }

        report << "Fragmentation Ratio: " << stats.fragmentationRatio << "\n";

        Logger::GetInstance().Info(report.str(), "MemorySystem");
    }

    void MemorySystem::ResetStatistics() {
        m_profiler.Reset();
    }

    size_t MemorySystem::GetTotalMemoryUsage() const {
        return m_profiler.GetCurrentStats().totalUsed;
    }

    size_t MemorySystem::GetPeakMemoryUsage() const {
        return m_profiler.GetPeakStats().peakUsage;
    }

    float MemorySystem::GetMemoryPressure() const {
        return m_profiler.GetMemoryPressure();
    }

    void MemorySystem::SetAllocationBreakpoint(size_t size) {
        m_allocationBreakpoint = size;
    }

    void MemorySystem::ClearAllocationBreakpoint() {
        m_allocationBreakpoint = 0;
    }

    void MemorySystem::TagMemory(void* ptr, const std::string& tag) {
        std::unique_lock<std::shared_mutex> lock(m_tagsMutex);
        m_memoryTags[ptr] = tag;
    }

    std::string MemorySystem::GetMemoryTag(void* ptr) const {
        std::shared_lock<std::shared_mutex> lock(m_tagsMutex);

        auto it = m_memoryTags.find(ptr);
        return it != m_memoryTags.end() ? it->second : "";
    }

    bool MemorySystem::RegisterPool(MemoryPool* pool) {
        if (!pool) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_poolsMutex);

        const std::string& name = pool->GetName();
        if (m_pools.find(name) != m_pools.end()) {
            return false;  // Pool with this name already exists
        }

        m_pools[name] = pool;
        m_poolsByType[pool->GetMemoryType()].push_back(pool);

        return true;
    }

    void MemorySystem::UnregisterPool(MemoryPool* pool) {
        if (!pool) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_poolsMutex);

        const std::string& name = pool->GetName();
        m_pools.erase(name);

        auto typeIt = m_poolsByType.find(pool->GetMemoryType());
        if (typeIt != m_poolsByType.end()) {
            auto& pools = typeIt->second;
            pools.erase(std::remove(pools.begin(), pools.end(), pool), pools.end());
        }
    }

    MemoryPool* MemorySystem::FindSuitablePool(size_t size, MemoryType type, uint32_t flags) {
        std::shared_lock<std::shared_mutex> lock(m_poolsMutex);

        auto it = m_poolsByType.find(type);
        if (it == m_poolsByType.end()) {
            return nullptr;
        }

        const auto& pools = it->second;

        // Find best fitting pool
        for (MemoryPool* pool : pools) {
            if (pool->GetFreeMemory() >= size) {
                return pool;
            }
        }

        return nullptr;
    }

    void* MemorySystem::AllocateFromSystem(size_t size, uint32_t flags) {
        void* ptr = nullptr;

        if (flags & MEMORY_FLAG_ALIGNED) {
            // Allocate aligned memory
            #ifdef _WIN32
                ptr = _aligned_malloc(size, 16);
            #else
                if (posix_memalign(&ptr, 16, size) != 0) {
                    ptr = nullptr;
                }
            #endif
        } else {
            ptr = std::malloc(size);
        }

        if (ptr && (flags & MEMORY_FLAG_ZERO_MEMORY)) {
            std::memset(ptr, 0, size);
        }

        return ptr;
    }

    void MemorySystem::UpdateMemoryPressure() {
        // Memory pressure is updated automatically by the profiler
    }

    void MemorySystem::CheckForDefragmentation() {
        static float timeSinceLastCheck = 0.0f;
        timeSinceLastCheck += 0.016f;  // Assume 60 FPS

        if (timeSinceLastCheck >= 1.0f) {  // Check every second
            m_defragmenter.DefragmentAll();
            timeSinceLastCheck = 0.0f;
        }
    }

    void MemorySystem::UpdateStatsOnAllocation(MemoryType type, size_t size, void* address) {
        m_profiler.RecordAllocation(type, size, address);
    }

    void MemorySystem::UpdateStatsOnDeallocation(MemoryType type, size_t size, void* address) {
        m_profiler.RecordDeallocation(type, size, address);
    }

} // namespace VoxelCraft
